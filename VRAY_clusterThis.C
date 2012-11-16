/* ******************************************************************************
*
*  clusterThis mantra DSO for render time geo clustering
*
* $RCSfile: VRAY_clusterThis.C,v $
*
* Description : This DSO  will instance geomtery or volumes during the render of mantra IFD's
*
* $Revision: 1.60 $
*
* $Source: /dca/cvsroot/houdini/VRAY_clusterThis/VRAY_clusterThis.C,v $
*
* $Author: mstory $
*
*  Version 2.0.1
*  Date: October 5, 2012
*  Author: Mark Story
*
* See Change History at the end of the file.
*
*    Digital Cinema Arts (C) 2008-2012
*
* This work is licensed under the Creative Commons Attribution-ShareAlike 2.5 License.
* To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/2.5/ or send a letter to
* Creative Commons, 543 Howard Street, 5th Floor, San Francisco, California, 94105, USA.
*
***************************************************************************** */

///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2012 DreamWorks Animation LLC
//
// All rights reserved. This software is distributed under the
// Mozilla Public License 2.0 ( http://www.mozilla.org/MPL/2.0/ )
//
// Redistributions of source code must retain the above copyright
// and license notice and the following restrictions and disclaimer.
//
// *     Neither the name of DreamWorks Animation nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// IN NO EVENT SHALL THE COPYRIGHT HOLDERS' AND CONTRIBUTORS' AGGREGATE
// LIABILITY FOR ALL CLAIMS REGARDLESS OF THEIR BASIS EXCEED US$250.00.
//
///////////////////////////////////////////////////////////////////////////
//

#ifndef __VRAY_clusterThis_C__
#define __VRAY_clusterThis_C__

#include <GU/GU_Detail.h>
#include <GEO/GEO_PointTree.h>
#include <GEO/GEO_Primitive.h>
#include <GU/GU_PrimCircle.h>
#include <GU/GU_PrimSphere.h>
#include <GU/GU_PrimTube.h>
#include <GU/GU_Grid.h>
#include <GU/GU_PrimNURBCurve.h>
#include <GEO/GEO_PrimSphere.h>
#include <GU/GU_PrimMetaBall.h>
#include <UT/UT_Matrix3.h>
#include <UT/UT_Matrix4.h>
#include <UT/UT_XformOrder.h>
#include <UT/UT_Noise.h>
#include <UT/UT_MTwister.h>
#include <UT/UT_BoundingBox.h>
#include <VRAY/VRAY_Procedural.h>
#include <VRAY/VRAY_IO.h>
#include <GEO/GEO_IORib.h>
#include <UT/UT_Exit.h>
#include <UT/UT_Options.h>
#include <UT/UT_IStream.h>
#include <UT/UT_Version.h>
#include <GA/GA_AttributeRef.h>
#include <GEO/GEO_AttributeHandle.h>
#include <GU/GU_PrimVolume.h>
#include <UT/UT_VoxelArray.h>
#include <SHOP/SHOP_Node.h>
#include <CVEX/CVEX_Context.h>
#include <UT/UT_StringArray.h>
#include <MOT/MOT_Director.h>

#include <sys/stat.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <ios>
#include <assert.h>

#include <openvdb/openvdb.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <openvdb/tools/LevelSetUtil.h>
#include <openvdb/tools/ParticlesToLevelSet.h>
#include <openvdb/tools/GridTransformer.h>
#include <openvdb/tools/GridSampling.h>
#include <openvdb/tools/Gradient.h>
#include <openvdb/tools/Filter.h>

#include <houdini_utils/ParmFactory.h>
#include <openvdb_houdini/Utils.h>
#include <openvdb_houdini/SOP_NodeVDB.h>
#include <openvdb_houdini/GU_PrimVDB.h>

namespace hvdb = openvdb_houdini;
namespace hutil = houdini_utils;

class VRAY_clusterThisChild;
class VRAY_clusterThis_Exception;


namespace
{

// This class is required by openvdb::tools::ParticlesToLeveSet
   class ParticleList
   {
      public:
         ParticleList(const GA_Detail * gdp,
                      openvdb::Real radiusMult = 1,
                      openvdb::Real velocityMult = 1) :
            mGdp(gdp),
            mScaleHandle(gdp, GEO_POINT_DICT, "radius"),
            mVelHandle(gdp, GEO_POINT_DICT, "v"),
            mHasRadius(mScaleHandle.isValid()),
            mHasVelocity(mVelHandle.isValid()),
            mRadiusMult(radiusMult),
            mVelocityMult(velocityMult) {
         }

         bool hasRadius()   const {
            return mHasRadius;
         }
         bool hasVelocity() const {
            return mHasVelocity;
         }

         // The public methods below are the only ones required
         // by tools::ParticlesToLevelSet
         size_t size() const {
            return mGdp->getNumPoints();
         }
         openvdb::Vec3R pos(int n) const {
            UT_Vector3 p = mGdp->getPos3(this->offset(n));
            return openvdb::Vec3R(p[0], p[1], p[2]);
         }
         openvdb::Vec3R vel(int n) const {
            if(!mHasVelocity)
               return openvdb::Vec3R(0, 0, 0);
            UT_Vector3 v = mVelHandle.get(this->offset(n));
            return mVelocityMult * openvdb::Vec3R(v[0], v[1], v[2]);
         }
         openvdb::Real radius(int n) const {
            if(!mHasRadius)
               return mRadiusMult;
            return mRadiusMult * mScaleHandle.get(this->offset(n));
         }

      protected:
         GA_Offset offset(int n) const {
            return mGdp->pointOffset(n);
         }

         const GA_Detail  *  mGdp;
         GA_ROHandleF        mScaleHandle;
         GA_ROHandleV3       mVelHandle;
         const bool          mHasRadius, mHasVelocity;
         const openvdb::Real mRadiusMult; // multiplier for radius
         const openvdb::Real mVelocityMult; // multiplier for velocity
   };// ParticleList

// Convenient settings struct
   struct Settings {
      Settings(): mRasterizeTrails(false), mFogVolume(false), mDx(1.0), mGradientWidth(-1.0) {}
      bool mRasterizeTrails, mFogVolume;
      float mDx, mGradientWidth;
   };

} // unnamed namespace




namespace dca
{

   static short int myPasses(int mode)
   {
      static short int num_passes; // keep track of how many times the DSO gets called
      if(mode)
         return num_passes;
      else
         num_passes++;
      return 0;

   }



   static inline int calculateNewInstPosition(fpreal theta, uint32 i, uint32 j)
   {
#ifdef DEBUG
      cout << "VRAY_clusterThis::calculateNewPosition() i: " << i << " j: " << j << endl;
#endif

      // Calculate a new position for the object ...
//   fpreal delta = theta * i;
//   fpreal dx, dy, dz = 0.0;
//   dx = SYSsin(delta * myFreqX + myOffsetX);
//   dy = SYScos(delta * myFreqY + myOffsetY);
//   dz = SYScos(delta * myFreqZ + myOffsetZ);

#ifdef DEBUG
      cout << "VRAY_clusterThis::calculateNewPosition() " << "delta: " << delta << endl;
      cout << "VRAY_clusterThis::calculateNewPosition() " << "dx: " << dx << " dy: " << dy << " dz: " << dz << endl;
#endif

//   myNoise.setSeed(myPointAttributes.id);

      // Calculate a bit of noise to add to the new position ...
      // TODO:
//   fpreal noise_bias = (myNoise.turbulence(myPointAttributes.myPos, myFractalDepth, myRough, myNoiseAtten) * myNoiseAmp) + 1.0;

      // myNoise.turbulence(myPos, myFractalDepth, myNoiseVec, myRough, myNoiseAtten);
      // cout << "VRAY_clusterThis::render() " << "myNoiseVec: " << myNoiseVec.x() << " " << myNoiseVec.x() << " " << myNoiseVec.x() << endl;

#ifdef DEBUG
      cout << "VRAY_clusterThis::calculateNewPosition() " << "noise_bias: " << noise_bias << endl;
#endif

//   // Calculate the new object's position
//   myPointAttributes.myNewPos[0] = (fpreal) myPointAttributes.myPos.x() +
//                                   ((dx * myRadius) * noise_bias * SYSsin(static_cast<fpreal>(j + i)));
//   myPointAttributes.myNewPos[1] = (fpreal) myPointAttributes.myPos.y() +
//                                   ((dy * myRadius) * noise_bias * SYScos(static_cast<fpreal>(j + i)));
//   myPointAttributes.myNewPos[2] = (fpreal) myPointAttributes.myPos.z() +
//                                   ((dz * myRadius) * noise_bias * (SYSsin(static_cast<fpreal>(j + i)) + SYScos(static_cast<fpreal>(j + i))));
////   myPointAttributes.myNewPos[2] = ( fpreal ) myPointAttributes.myPos.z() +
////                                    ( ( dz * myRadius ) * noise_bias * ( SYScos ( static_cast<fpreal>(j + i)) ) );
//
//   if (myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
//      myPointAttributes.myMBPos[0] = myPointAttributes.myNewPos[0] - myPointAttributes.v.x();
//      myPointAttributes.myMBPos[1] = myPointAttributes.myNewPos[1] - myPointAttributes.v.y();
//      myPointAttributes.myMBPos[2] = myPointAttributes.myNewPos[2] - myPointAttributes.v.z();
//   }

#ifdef DEBUG
      cout << "VRAY_clusterThis::calculateNewPosition() myPos:   "
           << myPointAttributes.myPos.x() << " " << myPointAttributes.myPos.y() << " " << myPointAttributes.myPos.z() << endl;
      cout << "VRAY_clusterThis::calculateNewPosition() newPos: "
           << myPointAttributes.myNewPos[0] << " " << myPointAttributes.myNewPos[1] << " " << myPointAttributes.myNewPos[2] << endl;
#endif

      return 0;

   }



   static inline void getInstBBox(UT_BoundingBox & box, UT_BoundingBox & vbox,
                                  const GEO_Point * point, const UT_Vector3 & sprite_scale,
                                  const GA_ROAttributeRef & voff,
                                  fpreal tscale, const UT_Matrix4 & xform)
   {
      fpreal     maxradius;
      static fpreal isin45 = 1.0F / SYSsin(M_PI / 4);
      UT_Vector3    pt;

      maxradius = SYSmax(sprite_scale.x(), sprite_scale.y()) * isin45 * 0.5F;

      pt = UT_Vector3(-maxradius, -maxradius, 0) * xform;
      box.initBounds(pt);
      pt = UT_Vector3(-maxradius,  maxradius, 0) * xform;
      box.enlargeBounds(pt);
      pt = UT_Vector3(maxradius, -maxradius, 0) * xform;
      box.enlargeBounds(pt);
      pt = UT_Vector3(maxradius,  maxradius, 0) * xform;
      box.enlargeBounds(pt);

      box.translate(point->getPos());
      vbox = box;

      if(voff.isValid()) {
            UT_Vector3  vel;
            int      i;
            fpreal      amount;

            vel = point->getValue<UT_Vector3>(voff);
            for(i = 0; i < 3; i++) {
                  amount = vel(i) * tscale;
                  if(amount < 0)
                     vbox.vals[i][1] -= amount;
                  else
                     vbox.vals[i][0] -= amount;

               }
         }
   }


}

using namespace dca;



// TODO: Ahh yes, the ol' order of include issues ... figure out the issue with declaring the static(s) & namespace above
// and how it's interfering with the include files below ... oy!

#include "version.h"
#include "VRAY_clusterThis.h"
#include "VRAY_clusterThisChild.h"
#include "VRAY_clusterThisChild.C"
#include "VRAY_clusterThisRender.C"
#include "VRAY_clusterThisInstance.C"
#include "VRAY_clusterThisAttributeUtils.C"
#include "VRAY_clusterCVEXUtil.C"
#include "VRAY_clusterThisRunCVEX.C"
#include "VRAY_clusterThisPostProcess.C"





/* ******************************************************************************
*  Function Name : theArgs()
*
*  Description : Data for the arguments...
*
*  Input Arguments : None
*
*  Return Value :
*
***************************************************************************** */
static VRAY_ProceduralArg theArgs[] = {
   VRAY_ProceduralArg("minbound", "real", "-10 -10 -10"),
   VRAY_ProceduralArg("maxbound", "real", "10 10 10"),
   VRAY_ProceduralArg("prim_type", "integer", "0"),
   VRAY_ProceduralArg("use_geo_file", "integer", "0"),
   VRAY_ProceduralArg("src_geo_file", "string", "default.bgeo"),
   VRAY_ProceduralArg("num_copy", "integer", "10"),
   VRAY_ProceduralArg("recursion", "integer", "2"),
   VRAY_ProceduralArg("radius",    "real",  "0.1"),
   VRAY_ProceduralArg("size",  "real", "0.01 0.01 0.01"),
   VRAY_ProceduralArg("freq",    "real",  "1.0 1.0 1.0"),
   VRAY_ProceduralArg("offset",    "real",  "0.0 0.0 0.0"),
   VRAY_ProceduralArg("birth_prob", "real",  "0.5"),
   VRAY_ProceduralArg("add_proc", "integer", "0"),
   VRAY_ProceduralArg("motion_blur", "integer", "0"),
   VRAY_ProceduralArg("mb_shutter", "real", "0.1"),
   VRAY_ProceduralArg("mb_shutter2", "real", "0.9"),
   VRAY_ProceduralArg("verbose", "integer", "0"),
   VRAY_ProceduralArg("time", "real",  "0.0"),
   VRAY_ProceduralArg("noise_type", "integer", "0"),
   VRAY_ProceduralArg("noise_amp", "real", "0.1"),
   VRAY_ProceduralArg("noise_rough", "real", "0.1"),
   VRAY_ProceduralArg("noise_atten", "real", "0.0"),
   VRAY_ProceduralArg("noise_seed", "integer", "7"),
   VRAY_ProceduralArg("noise_fractal_depth", "integer", "3"),
   VRAY_ProceduralArg("copy_attrs", "integer", "1"),
   VRAY_ProceduralArg("speed_stretch", "real", "0.0 0.0 0.0"),
   VRAY_ProceduralArg("blend_attrs", "integer", "0"),
   VRAY_ProceduralArg("geo_file", "string", "default.bgeo"),
   VRAY_ProceduralArg("CVEX_shader", "string", "default.vex"),
   VRAY_ProceduralArg("CVEX_exec", "integer", "0"),
   VRAY_ProceduralArg("CVEX_shader_prim", "string", "default.vex"),
   VRAY_ProceduralArg("CVEX_exec_prim", "integer", "0"),
   VRAY_ProceduralArg("CVEX_shader_pre", "string", "default.vex"),
   VRAY_ProceduralArg("CVEX_exec_pre", "integer", "0"),
   VRAY_ProceduralArg("CVEX_shader_post", "string", "default.vex"),
   VRAY_ProceduralArg("CVEX_exec_post", "integer", "0"),
   VRAY_ProceduralArg("filter_type", "integer", "0"),
   VRAY_ProceduralArg("filter_amp", "real", "0.0"),
   VRAY_ProceduralArg("volume_data_1", "string", "file.vol"),
   VRAY_ProceduralArg("volume_data_2", "string", "file.vol"),
   VRAY_ProceduralArg("volume_data_3", "string", "file.vol"),
   VRAY_ProceduralArg("temp_file_path", "string", "/tmp/geo/"),
   VRAY_ProceduralArg("temp_file", "integer", "0"),
   VRAY_ProceduralArg("save_temp_file", "integer", "0"),
   VRAY_ProceduralArg("otl_version", "string", "DCA_VERSION"),

   VRAY_ProceduralArg("Cd_pt", "integer", "0"),
   VRAY_ProceduralArg("Alpha_pt", "integer", "0"),
   VRAY_ProceduralArg("v_pt", "integer", "0"),
   VRAY_ProceduralArg("N_pt", "integer", "0"),
   VRAY_ProceduralArg("pscale_pt", "integer", "0"),

   VRAY_ProceduralArg("Cd_prim", "integer", "0"),
   VRAY_ProceduralArg("Alpha_prim", "integer", "0"),
   VRAY_ProceduralArg("v_prim", "integer", "0"),
   VRAY_ProceduralArg("N_prim", "integer", "0"),
   VRAY_ProceduralArg("pscale_prim", "integer", "0"),
   VRAY_ProceduralArg("weight_prim", "integer", "0"),
   VRAY_ProceduralArg("width_prim", "integer", "0"),

   VRAY_ProceduralArg("cvex_Cd_pt", "integer", "0"),
   VRAY_ProceduralArg("cvex_Alpha_pt", "integer", "0"),
   VRAY_ProceduralArg("cvex_v_pt", "integer", "0"),
   VRAY_ProceduralArg("cvex_N_pt", "integer", "0"),
   VRAY_ProceduralArg("cvex_pscale_pt", "integer", "0"),

   VRAY_ProceduralArg("cvex_Cd_prim", "integer", "0"),
   VRAY_ProceduralArg("cvex_Alpha_prim", "integer", "0"),
   VRAY_ProceduralArg("cvex_v_prim", "integer", "0"),
   VRAY_ProceduralArg("cvex_N_prim", "integer", "0"),
   VRAY_ProceduralArg("cvex_pscale_prim", "integer", "0"),
   VRAY_ProceduralArg("cvex_weight_prim", "integer", "0"),
   VRAY_ProceduralArg("cvex_width_prim", "integer", "0"),

   VRAY_ProceduralArg("post_process", "integer", "1"),
   VRAY_ProceduralArg("raster_type", "integer", "0"),
   VRAY_ProceduralArg("ws_units", "integer", "1"),
   VRAY_ProceduralArg("fog_volume", "integer", "0"),
   VRAY_ProceduralArg("dx", "real", "1.0"),
   VRAY_ProceduralArg("gradient_width", "real", "0.5"),
   VRAY_ProceduralArg("voxel_size", "real", "0.025"),
   VRAY_ProceduralArg("radius_min", "real", "1.5"),
   VRAY_ProceduralArg("bandwidth", "real", "0.2"),
   VRAY_ProceduralArg("falloff", "real", "0.5"),
   VRAY_ProceduralArg("vdb_pos_influence", "real", "0.1"),
   VRAY_ProceduralArg("vdb_vel_influence", "real", "0.1"),
   VRAY_ProceduralArg("vdb_normal_influence", "real", "0.1"),

   VRAY_ProceduralArg("vdb_median_filter", "integer", "0"),
   VRAY_ProceduralArg("vdb_mean_filter", "integer", "0"),
   VRAY_ProceduralArg("vdb_mean_curvature_filter", "integer", "0"),
   VRAY_ProceduralArg("vdb_laplacian_filter", "integer", "0"),
   VRAY_ProceduralArg("vdb_offset_filter", "integer", "0"),
   VRAY_ProceduralArg("vdb_offset_filter_amount", "real", "0.1"),
   VRAY_ProceduralArg("vdb_renormalize_filter", "integer", "0"),
   VRAY_ProceduralArg("vdb_write_debug_file", "integer", "0"),

   VRAY_ProceduralArg()
};


/* ******************************************************************************
*  Function Name : allocProcedural()
*
*  Description :
*
*  Input Arguments : None
*
*  Return Value :
*
***************************************************************************** */
VRAY_Procedural * allocProcedural(const char *)
{
//   std::cout << "VRAY_clusterThis::allocProcedural()" << std::endl;
   return new VRAY_clusterThis();
}



/* ******************************************************************************
*  Function Name : getProceduralArgs()
*
*  Description :
*
*  Input Arguments : None
*
*  Return Value :
*
***************************************************************************** */
const VRAY_ProceduralArg * getProceduralArgs(const char *)
{
//   std::cout << "VRAY_clusterThis::getProceduralArgs()" << std::endl;
   return theArgs;
}



/* ******************************************************************************
*  Function Name : VRAY_clusterThis_Exception()
*
*  Description : Constructor for a "VRAY_clusterThis_Exception Exception" object
*
*  Input Arguments : None
*
*  Return Value :
*
***************************************************************************** */
VRAY_clusterThis_Exception::VRAY_clusterThis_Exception(std::string msg, int code)
{

   e_msg = msg;
   e_code = code;

};


//VRAY_clusterThis_Exception::~VRAY_clusterThis_Exception() {


//   };




/* ******************************************************************************
*  Function Name : exitClusterThis()
*
*  Description :  Start the exit process
*
*  Input Arguments : void *data
*
*  Return Value :
*
***************************************************************************** */
void VRAY_clusterThis::exitClusterThis(void * data)
{
   VRAY_clusterThis * me = (VRAY_clusterThis *)data;

   if(me->myVerbose > CLUSTER_MSG_INFO)
      std::cout << std::endl << std::endl << "VRAY_clusterThis::exitClusterThis() - Preparing to exit!" << std::endl;


//   if(me->myVerbose > CLUSTER_MSG_INFO)
//      cout << "VRAY_clusterThis::exitClusterThis() myTempFname: " << (const char *)me->myTempFname << endl;


//   if(me->tempFileDeleted) {
//         me->tempFileDeleted = true;
//         cout << "VRAY_clusterThis::exitClusterThis(): " << me->tempFileDeleted << endl;
////         me->exitClusterThisReal((const char *)me->myTempFname);
//         me->exitClusterThisReal(data);
//      }


   if(me->myVerbose > CLUSTER_MSG_INFO)
      cout << "VRAY_clusterThis::exitClusterThis() - Running exit processing" << endl;

//   cout << "VRAY_clusterThis::exitClusterThis(): " << me->tempFileDeleted << endl;
//
//   cout << "VRAY_clusterThis::exitClusterThis() - temp filename " << me->myTempFname << endl;

//   const char * fname = me->myTempFname;
//
////   ofstream myStream;
//
////   myStream.open("exit_data.txt", ios_base::app);
////   myStream << this->exitData.exitTime << std::endl;
////   myStream << this->exitData.exitCode << std::endl;
////   myStream.flush();
////   myStream.close();
////   cout << "VRAY_clusterThis::exitClusterThisReal() : " << this->exitData.exitTime << endl;
////
//
//   struct stat fileResults;
//
//   if(me->myUseTempFile && !me->mySaveTempFile) {
//         if((UT_String(fname)).isstring() && stat(fname, &fileResults) == 0) {
//               if(me->myVerbose > CLUSTER_MSG_INFO)
//                  cout << "VRAY_clusterThis::exitClusterThis() - Found temp file " << fname << endl;
//               if(!remove(fname) && (me->myVerbose > CLUSTER_MSG_INFO))
//                  cout << "VRAY_clusterThis::exitClusterThis() - Removed geometry temp file: " << fname << endl;
//            }
//         else
//            if(me->myVerbose > CLUSTER_MSG_INFO)
//               cout << "VRAY_clusterThis::exitClusterThis() - Did not find temp file " << fname << endl << endl;
//      }
//
//

   const char * fname = me->myTempFname;

   me->exitClusterThisReal(fname);
//         me->exitClusterThisReal(data);

}



/* ******************************************************************************
*  Function Name : exitClusterThisReal()
*
*  Description : Clean up temp file if used, save stats to file and DB
*
*  Input Arguments : None
*
*  Return Value :
*
* ***************************************************************************** */
//void VRAY_clusterThis::exitClusterThisReal(void * data)
void VRAY_clusterThis::exitClusterThisReal(const char * fname)
{
   struct stat fileResults;
//   VRAY_clusterThis * me = (VRAY_clusterThis *)data;


   if(myVerbose > CLUSTER_MSG_INFO)
      cout << "VRAY_clusterThis::exitClusterThisReal() - Running exit processing" << endl;


//   cout << "VRAY_clusterThis::exitClusterThisReal(): " << tempFileDeleted << endl;
//
//   cout << "VRAY_clusterThis::exitClusterThisReal() - temp filename " << myTempFname << endl;
//
//   const char * fname = me->myTempFname;

//   ofstream myStream;

//   myStream.open("exit_data.txt", ios_base::app);
//   myStream << this->exitData.exitTime << std::endl;
//   myStream << this->exitData.exitCode << std::endl;
//   myStream.flush();
//   myStream.close();
//   cout << "VRAY_clusterThis::exitClusterThisReal() : " << this->exitData.exitTime << endl;
//


   if(this->myUseTempFile && !this->mySaveTempFile) {
         if((UT_String(fname)).isstring() && stat(fname, &fileResults) == 0) {
               if(myVerbose > CLUSTER_MSG_INFO)
                  cout << "VRAY_clusterThis::exitClusterThisReal() - Found temp file " << fname << endl;
               if(!remove(fname) && (myVerbose > CLUSTER_MSG_INFO))
                  cout << "VRAY_clusterThis::exitClusterThisReal() - Removed geometry temp file: " << fname << endl;
            }
         else
            if(myVerbose > CLUSTER_MSG_INFO)
               cout << "VRAY_clusterThis::exitClusterThisReal() - Did not find temp file " << fname << endl << endl;
      }



   if(myVerbose > CLUSTER_MSG_INFO)
      std::cout << "VRAY_clusterThis::exitClusterThisReal() - Exiting" << std::endl;
}





/* ******************************************************************************
*  Function Name : convert()
*
*  Description : convert point cloud to VDB level set or fog volume
*
*  Input Arguments : None
*
*  Return Value :
*
* ***************************************************************************** */
void VRAY_clusterThis::convert(
   openvdb::ScalarGrid::Ptr outputGrid,
   ParticleList & paList,
   const Settings & settings,
   hvdb::Interrupter & boss)
{
//    openvdb::tools::ParticlesToLevelSet<openvdb::ScalarGrid, ParticleList> raster(*outputGrid);
   openvdb::tools::ParticlesToLevelSet<openvdb::ScalarGrid, ParticleList, hvdb::Interrupter> raster(*outputGrid, &boss);

//   std::cout << "VRAY_clusterThis::convert() " << std::endl;


   raster.setRmin(myRadiusMin);

   if(myVerbose == CLUSTER_MSG_DEBUG) {
         std::cout << "VRAY_clusterThis::convert(): raster.getVoxelSize(): " << raster.getVoxelSize() << std::endl;
         std::cout << "VRAY_clusterThis::convert(): raster.getRmin(): " << raster.getRmin() << std::endl;
         std::cout << "VRAY_clusterThis::convert(): raster.getHalfWidth(): " << raster.getHalfWidth() << std::endl;
      }

   if(raster.getHalfWidth() < openvdb::Real(2)) {
         std::cout << "VRAY_clusterThis::convert(): Half width of narrow-band < 2 voxels which creates holes when meshed!" << std::endl;
      }
   else
      if(raster.getHalfWidth() > openvdb::Real(1000)) {
            throw std::runtime_error(
               "VRAY_clusterThis::convert(): Half width of narrow-band > 1000 voxels which exceeds memory limitations!");
         }

   if(settings.mRasterizeTrails && paList.hasVelocity()) {
         if(myVerbose == CLUSTER_MSG_DEBUG)
            std::cout << "VRAY_clusterThis::convert(): rasterizing trails"  << std::endl;
         raster.rasterizeTrails(paList, settings.mDx);
      }
   else {
         if(myVerbose == CLUSTER_MSG_DEBUG)
            std::cout << "VRAY_clusterThis::convert(): rasterizing spheres"  << std::endl;
         raster.rasterizeSpheres(paList);
      }

   if(boss.wasInterrupted()) {
         if(myVerbose == CLUSTER_MSG_DEBUG)
            std::cout << "VRAY_clusterThis::convert(): Process was interrupted"  << std::endl;
         return;
      }

   // Convert the level-set into a fog volume.
   if(settings.mFogVolume) {
         if(myVerbose == CLUSTER_MSG_DEBUG)
            std::cout << "VRAY_clusterThis::convert(): converting to fog volume"  << std::endl;
         float cutOffDist = std::numeric_limits<float>::max();
         if(settings.mGradientWidth > 1e-6)
            cutOffDist = settings.mGradientWidth;
         openvdb::tools::levelSetToFogVolume(outputGrid, cutOffDist, false);
      }

// print stats of the vdb grid
   if(myVerbose == CLUSTER_MSG_DEBUG)
      outputGrid->print();

}




int VRAY_clusterThis::convertVDBUnits()
{
//     const bool toWSUnits = static_cast<bool>(evalInt("worldSpaceUnits", 0, 0));
//
//     if (toWSUnits) {
//         setFloat("bandWidthWS", 0, 0, evalFloat("bandWidth", 0, 0) * mVoxelSize);
//         return 1;
//     }
//
//     setFloat("bandWidth", 0, 0, evalFloat("bandWidthWS", 0, 0) / mVoxelSize);
//
   return 1;
}




/* ******************************************************************************
*  Function Name : VRAY_clusterThis()
*
*  Description :  Constructor for a  VRAY_clusterThis object
*
*  Input Arguments : None
*
*  Return Value :
*
***************************************************************************** */
VRAY_clusterThis::VRAY_clusterThis()
{

   std::cout << "VRAY_clusterThis::VRAY_clusterThis() - Constructor" << std::endl;

   // Init member variables
   myBox.initBounds(0, 0, 0);
   bb_x1 = 0.0;
   bb_y1 = 0.0;
   bb_z1 = 0.0;
   bb_x2 = 0.0;
   bb_y2 = 0.0;
   bb_z2 = 0.0;
   myPrimType = CLUSTER_PRIM_SPHERE;
   myUseGeoFile = false;
   mySrcGeoFname = "";
   myNumCopies = 0;
   myNoiseType = static_cast<UT_Noise::UT_NoiseType>(0);
   myFreqX = 0.0;
   myFreqY = 0.0;
   myFreqZ = 0.0;
   myOffsetX = 0.0;
   myOffsetY = 0.0;
   myOffsetZ = 0.0;
   myRadius = 0.0;
   myRough = 0.0;
   myBirthProb = 0.0;
   mySize[0] = 0.0;
   mySize[1] = 0.0;
   mySize[2] = 0.0;
   myDoMotionBlur = CLUSTER_MB_NONE;
   myCurrentTime = 0.0;
   myShutter = 0.1;
   myNoiseAmp = 0.0;
   myFilterType = 0;
   myFilterAmp = 0.0;
   myNoiseAtten = 0.0;
   myNoiseSeed = 0;
   myFractalDepth = 0;
   myRecursion = 0;
   myCopyAttrs = false;
   myCVEX_Exec = false;
   myCVEX_Exec_prim = false;
   myCVEX_Exec_pre = false;
   myCVEX_Exec_post = false;
   myCVEXFname = "";
   myCVEXFname_prim = "";
   myCVEXFname_pre = "";
   myCVEXFname_post = "";
   myBlendAttrs = false;
   myMethod = CLUSTER_INSTANCE_NOW;
   myVerbose = CLUSTER_MSG_QUIET;
   myUseTempFile = false;
   mySaveTempFile = false;

   myCVEXPointVars.cvex_Cd_pt = 0;
   myCVEXPointVars.cvex_Alpha_pt = 0;
   myCVEXPointVars.cvex_v_pt = 0;
   myCVEXPointVars.cvex_N_pt = 0;
   myCVEXPointVars.cvex_pscale_pt = 0;

   myCVEXPrimVars.cvex_Cd_prim = 0;
   myCVEXPrimVars.cvex_Alpha_prim = 0;
   myCVEXPrimVars.cvex_N_prim = 0;
   myCVEXPrimVars.cvex_pscale_prim = 0;
   myCVEXPrimVars.cvex_weight_prim = 0;
   myCVEXPrimVars.cvex_width_prim = 0;

// VDB parms
   myPostProcess = 0;
   myRasterType = 0;
   myDx = 1.0;
   myFogVolume = 0;
   myGradientWidth = 0.5;
   myVoxelSize = 0.025;
   myRadiusMin = 1.5;
   myBandWidth = 0.2;
   myWSUnits = 1;
   myFalloff = 0.5;
   myPosInfluence = 0.1;
   myNormalInfluence = 0.1;
   myVelInfluence = 0.1;

   myVDBMedianFilter = 0;
   myVDBMeanFilter = 0;
   myVDBMeanCurvatureFilter = 0;
   myVDBLaplacianFilter = 0;
   myVDBOffsetFilter = 0;
   myVDBOffsetFilterAmount = 0.0;
   myVDBReNormalizeFilter = 0;
   myVDBWriteDebugFiles = 0;

   VRAY_clusterThis::exitData.exitTime = 3.333;
   VRAY_clusterThis::exitData.exitCode = 3;

   myInstanceNum = 0;

   int exitCallBackStatus = -1;
   exitCallBackStatus = UT_Exit::addExitCallback(VRAY_clusterThis::exitClusterThis, (void *)this);

   if((exitCallBackStatus != 1) && exitCallBackStatus != 0) {
         std::cout << "VRAY_clusterThis::VRAY_clusterThis() - error adding ExitCallback() - exitCallBackStatus = " << std::hex
                   << exitCallBackStatus << std::dec << std::endl;
      }

#ifdef DEBUG
   std::cout << "VRAY_clusterThis::VRAY_clusterThis() - exitCallBackStatus: " << std::hex << exitCallBackStatus <<  std::dec << std::endl;
#endif

}


/* ******************************************************************************
*  Function Name : ~VRAY_clusterThis()
*
*  Description :  Destructor for a  VRAY_clusterThis object
*
*  Input Arguments : None
*
*  Return Value :
*
***************************************************************************** */
VRAY_clusterThis::~VRAY_clusterThis()
{
//   std::cout << "VRAY_clusterThis::~VRAY_clusterThis() - Destructor" << std::endl;
}



/* ******************************************************************************
*  Function Name : getClassName()
*
*  Description : Get the class name for this VRAY_clusterThis object
*
*  Input Arguments : None
*
*  Return Value : const char *
*
***************************************************************************** */
const char * VRAY_clusterThis::getClassName()
{
//   std::cout << "VRAY_clusterThis::getClassName()" << std::endl;
   return "VRAY_clusterThis";
}



/* ******************************************************************************
*  Function Name : initialize
*
*  Description :  Initialize the VRAY_clusterThis object
*
*  Input Arguments : const UT_BoundingBox *
*
*  Return Value : int
*
***************************************************************************** */
int VRAY_clusterThis::initialize(const UT_BoundingBox *)
{
   if(myVerbose > CLUSTER_MSG_INFO)
      std::cout << "VRAY_clusterThis::initialize()" << std::endl;

//   UT_String geo_fname, temp_fname, cvex_fname, otl_version;
   const int * int_ptr;
   const fpreal * flt_ptr;
   const char ** char_handle;

   // Import the object:velocityscale settings.  This setting stores the
   // shutter time (in seconds) on a per object basis.  It's used primarily
   // for velocity blur.

   if(!import("object:velocityscale", &myVelocityScale, 1))
      myVelocityScale = 0;

//const int *  getIParm (const char *name) const
//const fpreal *  getFParm (const char *name) const
//const char **   getSParm (const char *name) const
//const int *  getIParm (int token) const
//const fpreal *  getFParm (int token) const
//const char **   getSParm (int token) const


//    ray_procedural clusterThis minbound -0.699999988079 -0.699999988079 -0.699999988079 maxbound 0.699999988079 0.699999988079 0.699999988079 prim_type 8 num_copy 14 recursion 19 radius 0.5 size 0.20000000298 0.20000000298 0.20000000298 freq 2 3 2 noise_type 3 noise_amp 0 noise_rough 0.391999989748 noise_fractal_depth 7 noise_atten 0.922999978065 temp_file_path "cache/temp_geo.bgeo"

   // Get OTL parameters

   if(flt_ptr = VRAY_Procedural::getFParm("minbound")) {
         bb_x1 = *flt_ptr++;
         bb_y1 = *flt_ptr++;
         bb_z1 = *flt_ptr;
         myBox.initBounds(bb_x1, bb_y1, bb_z1);
      }

   if(flt_ptr = VRAY_Procedural::getFParm("maxbound")) {
         bb_x2 = *flt_ptr++;
         bb_y2 = *flt_ptr++;
         bb_z2 = *flt_ptr;
         myBox.enlargeBounds(bb_x2, bb_y2, bb_z2);
      }

   if(int_ptr = VRAY_Procedural::getIParm("prim_type"))
      myPrimType = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("num_copy"))
      myNumCopies = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("recursion"))
      myRecursion = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("add_proc"))
      myMethod = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("motion_blur"))
      myDoMotionBlur = *int_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("mb_shutter"))
      myShutter = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("mb_shutter2"))
      myShutter2 = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("radius"))
      myRadius = *flt_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("noise_type"))
      myNoiseType = *int_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("freq")) {
         myFreqX = *flt_ptr++;
         myFreqY = *flt_ptr++;
         myFreqZ = *flt_ptr;
      }

   if(flt_ptr = VRAY_Procedural::getFParm("offset")) {
         myOffsetX = *flt_ptr++;
         myOffsetY = *flt_ptr++;
         myOffsetZ = *flt_ptr;
      }

   if(flt_ptr = VRAY_Procedural::getFParm("radius"))
      myRadius = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("noise_rough"))
      myRough = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("size")) {
         mySize[0] = *flt_ptr++;
         mySize[1] = *flt_ptr++;
         mySize[2] = *flt_ptr;
      }

   if(flt_ptr = VRAY_Procedural::getFParm("birth_prob"))
      myBirthProb = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("time"))
      myCurrentTime = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("noise_amp"))
      myNoiseAmp = *flt_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("filter_type"))
      myFilterType = *int_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("filter_amp"))
      myFilterAmp = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("noise_atten"))
      myNoiseAtten = *flt_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("noise_seed"))
      myNoiseSeed = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("noise_fractal_depth"))
      myFractalDepth = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("copy_attrs"))
      myCopyAttrs = bool (*int_ptr);

   if(int_ptr = VRAY_Procedural::getIParm("blend_attrs"))
      myBlendAttrs = bool (*int_ptr);

   if(int_ptr = VRAY_Procedural::getIParm("use_geo_file"))
      myUseGeoFile = bool (*int_ptr);

   if(char_handle = VRAY_Procedural::getSParm("src_geo_file")) {
         mySrcGeoFname = (UT_String)(*char_handle);
         mySrcGeoFname.harden();
      }


   if(char_handle = VRAY_Procedural::getSParm("geo_file")) {
         myGeoFile = (UT_String)(*char_handle);
         myGeoFile.harden();
      }

   if(char_handle = VRAY_Procedural::getSParm("temp_file_path")) {
         myTempFname = (UT_String)(*char_handle);
         myTempFname.harden();
      }

   if(char_handle = VRAY_Procedural::getSParm("CVEX_shader")) {
         myCVEXFname = (UT_String)(*char_handle);
         myCVEXFname.harden();
      }


   if(int_ptr = VRAY_Procedural::getIParm("CVEX_exec"))
      myCVEX_Exec = bool (*int_ptr);


   if(char_handle = VRAY_Procedural::getSParm("CVEX_shader_prim")) {
         myCVEXFname_prim = (UT_String)(*char_handle);
         myCVEXFname_prim.harden();
      }

   if(int_ptr = VRAY_Procedural::getIParm("CVEX_exec_prim"))
      myCVEX_Exec_prim = bool (*int_ptr);


   if(char_handle = VRAY_Procedural::getSParm("CVEX_shader_pre")) {
         myCVEXFname_pre = (UT_String)(*char_handle);
         myCVEXFname_pre.harden();
      }

   if(int_ptr = VRAY_Procedural::getIParm("CVEX_exec_pre"))
      myCVEX_Exec_pre = bool (*int_ptr);

   if(char_handle = VRAY_Procedural::getSParm("CVEX_shader_post")) {
         myCVEXFname_post = (UT_String)(*char_handle);
         myCVEXFname_post.harden();
      }

   if(int_ptr = VRAY_Procedural::getIParm("CVEX_exec_post"))
      myCVEX_Exec_post = bool (*int_ptr);


   if(int_ptr = VRAY_Procedural::getIParm("cvex_Cd_pt"))
      myCVEXPointVars.cvex_Cd_pt = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("cvex_Alpha_pt"))
      myCVEXPointVars.cvex_Alpha_pt = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("cvex_v_pt"))
      myCVEXPointVars.cvex_v_pt = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("cvex_N_pt"))
      myCVEXPointVars.cvex_N_pt = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("cvex_pscale_pt"))
      myCVEXPointVars.cvex_pscale_pt = *int_ptr;


   if(int_ptr = VRAY_Procedural::getIParm("cvex_Cd_prim"))
      myCVEXPrimVars.cvex_Cd_prim = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("cvex_Alpha_prim"))
      myCVEXPrimVars.cvex_Alpha_prim = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("cvex_v_prim"))
      myCVEXPrimVars.cvex_v_prim = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("cvex_N_prim"))
      myCVEXPrimVars.cvex_N_prim = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("cvex_pscale_prim"))
      myCVEXPrimVars.cvex_pscale_prim = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("cvex_weight_prim"))
      myCVEXPrimVars.cvex_weight_prim = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("cvex_width_prim"))
      myCVEXPrimVars.cvex_width_prim = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("temp_file"))
      myUseTempFile = bool (*int_ptr);

   if(int_ptr = VRAY_Procedural::getIParm("save_temp_file"))
      mySaveTempFile = bool (*int_ptr);

   if(int_ptr = VRAY_Procedural::getIParm("verbose"))
      myVerbose = *int_ptr;

   if(char_handle = VRAY_Procedural::getSParm("otl_version")) {
         myOTLVersion = (UT_String)(*char_handle);
         myOTLVersion.harden();
      }



   if(int_ptr = VRAY_Procedural::getIParm("post_process"))
      myPostProcess = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("raster_type"))
      myRasterType = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("ws_units"))
      myWSUnits = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("fog_volume"))
      myFogVolume = *int_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("dx"))
      myDx = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("gradient_width"))
      myGradientWidth = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("voxel_size"))
      myVoxelSize = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("radius_min"))
      myRadiusMin = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("bandwidth"))
      myBandWidth = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("falloff"))
      myFalloff = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("vdb_pos_influence"))
      myPosInfluence = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("vdb_vel_influence"))
      myVelInfluence = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("vdb_normal_influence"))
      myNormalInfluence = *flt_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("vdb_median_filter"))
      myVDBMedianFilter = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("vdb_mean_filter"))
      myVDBMeanFilter = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("vdb_mean_curvature_filter"))
      myVDBMeanCurvatureFilter = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("vdb_laplacian_filter"))
      myVDBLaplacianFilter = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("vdb_offset_filter"))
      myVDBOffsetFilter = *int_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("vdb_offset_filter_amount"))
      myVDBOffsetFilterAmount = *flt_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("vdb_renormalize_filter"))
      myVDBReNormalizeFilter = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("vdb_write_debug_file"))
      myVDBWriteDebugFiles = *int_ptr;


   return 1;
}



/* ******************************************************************************
*  Function Name : getBoundingBox
*
*  Description :  Get the bounding box for this VRAY_clusterThis object
*
*  Input Arguments : None
*
*  Return Value : None
*
***************************************************************************** */
void VRAY_clusterThis::getBoundingBox(UT_BoundingBox & box)
{
//   std::cout << "VRAY_clusterThis::getBoundingBox()" << std::endl;
   fpreal     maxradius;
   static fpreal isin45 = 1.0F / SYSsin(M_PI / 4);
   UT_Vector3    pt;

   maxradius = SYSmax(mySize[0], mySize[1] * isin45 * 0.5F);

   box = myBox;
   box.translate(myPointAttributes.myNewPos);
//    box.enlargeBounds(mySize[0] * 10, mySize[1] * 10, mySize[2] * 10);
   fpreal size = mySize[0];
   if(size < mySize[1])
      size = mySize[1];
   if(size < mySize[2])
      size = mySize[2];

   box.enlargeBounds(0, size * maxradius);


#ifdef DEBUG
   std::cout << "VRAY_clusterThis::getBoundingBox() box: " << box << std::endl;
#endif

}


/* ******************************************************************************
*  Function Name : getBoundingBox
*
*  Description :  Get the bounding box for this VRAY_clusterThis object
*
*  Input Arguments : None
*
*  Return Value : None
*
***************************************************************************** */
void VRAY_clusterThis::checkRequiredAttributes()
{
//   std::cout << "VRAY_clusterThis::checkRequiredAttributes()" << std::endl;
   // Check for required attributes
   if(myPointAttrOffsets.Cd.isInvalid()) {
         cout << "Incoming points must have Cd attribute! Throwing exception ..." << endl;
         throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have Cd attribute! ", 1);
      }

   if(myPointAttrOffsets.Alpha.isInvalid()) {
         cout << "Incoming points must have Alpha attribute! Throwing exception ..." << endl;
         throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have Alpha attribute! ", 1);
      }

   if(myPointAttrOffsets.v.isInvalid()) {
         cout << "Incoming points must have v attribute! Throwing exception ..." << endl;
         throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have v attribute! ", 1);
      }

   if(myPointAttrOffsets.N.isInvalid()) {
         cout << "Incoming points must have N attribute! Throwing exception ..." << endl;
         throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have N attribute! ", 1);
      }

   if(myPointAttrOffsets.pscale.isInvalid()) {
         cout << "Incoming points must have pscale attribute! Throwing exception ..." << endl;
         throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have pscale attribute! ", 1);
      }

   if(myPointAttrOffsets.id.isInvalid()) {
         cout << "Incoming points must have id attribute Throwing exception ..." << endl;
         throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have id attribute! ", 1);
      }

}


/* ******************************************************************************
*  Function Name : calculateNewPosition()
*
*  Description :   Calculate the position of the new instance
*
*  Input Arguments : int i, int j
*
*  Return Value : None
*
***************************************************************************** */
inline void VRAY_clusterThis::calculateNewPosition(fpreal theta, uint32 i, uint32 j)
{
#ifdef DEBUG
   cout << "VRAY_clusterThis::calculateNewPosition() i: " << i << " j: " << j << endl;
#endif

   // Calculate a new position for the object ...
   fpreal delta = theta * i;
   fpreal noise_bias;
   fpreal dx, dy, dz = 0.0;
   dx = SYSsin(delta * myFreqX + myOffsetX);
   dy = SYScos(delta * myFreqY + myOffsetY);
   dz = SYScos(delta * myFreqZ + myOffsetZ);

#ifdef DEBUG
   cout << "VRAY_clusterThis::calculateNewPosition() " << "delta: " << delta << endl;
   cout << "VRAY_clusterThis::calculateNewPosition() " << "dx: " << dx << " dy: " << dy << " dz: " << dz << endl;
#endif

   if(myNoiseType < 3) {
         myNoise.setSeed(myPointAttributes.id);
         noise_bias = (myNoise.turbulence(myPointAttributes.myPos, myFractalDepth, myRough, myNoiseAtten) * myNoiseAmp) + 1.0;
      }
   else {
         myMersenneTwister.setSeed(myNoiseSeed);
         noise_bias = (myMersenneTwister.frandom() * myNoiseAmp) + 1.0;
      }

#ifdef DEBUG
   cout << "VRAY_clusterThis::calculateNewPosition() " << "noise_bias: " << noise_bias << endl;
#endif

   // Calculate the new object's position
   myPointAttributes.myNewPos[0] = (fpreal) myPointAttributes.myPos.x() +
                                   ((dx * myRadius) * noise_bias * SYSsin(static_cast<fpreal>(j + i)));
   myPointAttributes.myNewPos[1] = (fpreal) myPointAttributes.myPos.y() +
                                   ((dy * myRadius) * noise_bias * SYScos(static_cast<fpreal>(j + i)));
   myPointAttributes.myNewPos[2] = (fpreal) myPointAttributes.myPos.z() +
                                   ((dz * myRadius) * noise_bias * (SYSsin(static_cast<fpreal>(j + i)) + SYScos(static_cast<fpreal>(j + i))));
//   myPointAttributes.myNewPos[2] = ( fpreal ) myPointAttributes.myPos.z() +
//                                    ( ( dz * myRadius ) * noise_bias * ( SYScos ( static_cast<fpreal>(j + i)) ) );

   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
         myPointAttributes.myMBPos[0] = myPointAttributes.myNewPos[0] + myPointAttributes.v.x();
         myPointAttributes.myMBPos[1] = myPointAttributes.myNewPos[1] + myPointAttributes.v.y();
         myPointAttributes.myMBPos[2] = myPointAttributes.myNewPos[2] + myPointAttributes.v.z();
//         myPointAttributes.myMBPos[0] = myPointAttributes.myNewPos[0] - myPointAttributes.v.x();
//         myPointAttributes.myMBPos[1] = myPointAttributes.myNewPos[1] - myPointAttributes.v.y();
//         myPointAttributes.myMBPos[2] = myPointAttributes.myNewPos[2] - myPointAttributes.v.z();
      }

#ifdef DEBUG
   cout << "VRAY_clusterThis::calculateNewPosition() myPos:   "
        << myPointAttributes.myPos.x() << " " << myPointAttributes.myPos.y() << " " << myPointAttributes.myPos.z() << endl;
   cout << "VRAY_clusterThis::calculateNewPosition() newPos: "
        << myPointAttributes.myNewPos[0] << " " << myPointAttributes.myNewPos[1] << " " << myPointAttributes.myNewPos[2] << endl;
#endif

}


/* ******************************************************************************
*  Function Name : dumpParameters()
*
*  Description :   Dump the user parameters to the console
*
*  Input Arguments : None
*
*  Return Value : None
*
***************************************************************************** */
void VRAY_clusterThis::dumpParameters()
{

   cout << "VRAY_clusterThis::dumpParameters() myPrimType: " << myPrimType << endl;
   cout << "VRAY_clusterThis::dumpParameters() myNumCopies: " << myNumCopies << endl;
   cout << "VRAY_clusterThis::dumpParameters() myUseGeoFile: " << myUseGeoFile << endl;
   cout << "VRAY_clusterThis::dumpParameters() mySrcGeoFname: " << mySrcGeoFname << endl;
   cout << "VRAY_clusterThis::dumpParameters() myRecursion: " << myRecursion << endl;
   cout << "VRAY_clusterThis::dumpParameters() myMethod: " << myMethod << endl;
   cout << "VRAY_clusterThis::dumpParameters() myNoiseType: " << myNoiseType << endl;
   cout << "VRAY_clusterThis::dumpParameters() myNoiseAmp: " << myNoiseAmp << endl;
   cout << "VRAY_clusterThis::dumpParameters() myNoiseAtten: " << myNoiseAtten << endl;
   cout << "VRAY_clusterThis::dumpParameters() myNoiseSeed: " << myNoiseSeed << endl;
   cout << "VRAY_clusterThis::dumpParameters() myFractalDepth: " << myFractalDepth << endl;
   cout << "VRAY_clusterThis::dumpParameters() myRadius: " << myRadius << endl;
   cout << "VRAY_clusterThis::dumpParameters() myRough: " << myRough << endl;
   cout << "VRAY_clusterThis::dumpParameters() mySize: " << mySize[0] << " " << mySize[1] << " " << mySize[2] << endl;
   cout << "VRAY_clusterThis::dumpParameters() myFreq: " << myFreqX << " " << myFreqY << " " << myFreqZ << endl;
   cout << "VRAY_clusterThis::dumpParameters() myOffset: " << myOffsetX << " " << myOffsetY << " " << myOffsetZ << endl;
   cout << "VRAY_clusterThis::dumpParameters() myBirthProb: " << myBirthProb << endl;
   cout << "VRAY_clusterThis::dumpParameters() myCurrentTime: " << myCurrentTime << endl;
   cout << "VRAY_clusterThis::dumpParameters() myDoMotionBlur: " << myDoMotionBlur << endl;
   cout << "VRAY_clusterThis::dumpParameters() myShutter: " << myShutter << endl;
   cout << "VRAY_clusterThis::dumpParameters() myFilterType: " << myFilterType << endl;
   cout << "VRAY_clusterThis::dumpParameters() myFilterAmp: " << myFilterAmp << endl;
   cout << "VRAY_clusterThis::dumpParameters() myCopyAttrs: " << myCopyAttrs << endl;
   cout << "VRAY_clusterThis::dumpParameters() myBlendAttrs: " << myBlendAttrs << endl;
   cout << "VRAY_clusterThis::dumpParameters() myGeoFile: " << myGeoFile << endl;
   cout << "VRAY_clusterThis::dumpParameters() myTempFname: " << myTempFname << endl;
   cout << "VRAY_clusterThis::dumpParameters() myUseTempFile: " << myUseTempFile << endl;
   cout << "VRAY_clusterThis::dumpParameters() mySaveTempFile: " << mySaveTempFile << endl;
   cout << "VRAY_clusterThis::dumpParameters() myCVEXFname: " << myCVEXFname << endl;
   cout << "VRAY_clusterThis::dumpParameters() myCVEX_Exec: " << myCVEX_Exec << endl;
   cout << "VRAY_clusterThis::dumpParameters() myCVEXFname_prim: " << myCVEXFname_prim << endl;
   cout << "VRAY_clusterThis::dumpParameters() myCVEX_Exec_prim: " << myCVEX_Exec_prim << endl;
   cout << "VRAY_clusterThis::dumpParameters() myCVEXFname_pre: " << myCVEXFname_pre << endl;
   cout << "VRAY_clusterThis::dumpParameters() myCVEX_Exec_pre: " << myCVEX_Exec_pre << endl;
   cout << "VRAY_clusterThis::dumpParameters() myCVEXFname_post: " << myCVEXFname_post << endl;
   cout << "VRAY_clusterThis::dumpParameters() myCVEX_Exec_post: " << myCVEX_Exec_post << endl;

   cout << "VRAY_clusterThis::dumpParameters() myCVEXPointVars.cvex_Cd_pt: " << myCVEXPointVars.cvex_Cd_pt << endl;
   cout << "VRAY_clusterThis::dumpParameters() myCVEXPointVars.cvex_Alpha_pt: " << myCVEXPointVars.cvex_Alpha_pt << endl;
   cout << "VRAY_clusterThis::dumpParameters() myCVEXPointVars.cvex_N_pt: " << myCVEXPointVars.cvex_Alpha_pt << endl;
   cout << "VRAY_clusterThis::dumpParameters() myCVEXPointVars.cvex_v_pt: " << myCVEXPointVars.cvex_v_pt << endl;
   cout << "VRAY_clusterThis::dumpParameters() myCVEXPointVars.cvex_pscale_pt: " << myCVEXPointVars.cvex_pscale_pt << endl;
   cout << "VRAY_clusterThis::dumpParameters() myCVEXPrimVars.cvex_Cd_prim: " << myCVEXPrimVars.cvex_Cd_prim << endl;
   cout << "VRAY_clusterThis::dumpParameters() myCVEXPrimVars.cvex_Alpha_prim: " << myCVEXPrimVars.cvex_Alpha_prim << endl;
   cout << "VRAY_clusterThis::dumpParameters() myCVEXPrimVars.cvex_N_prim: " << myCVEXPrimVars.cvex_N_prim << endl;
   cout << "VRAY_clusterThis::dumpParameters() myCVEXPrimVars.cvex_v_prim: " << myCVEXPrimVars.cvex_v_prim << endl;
   cout << "VRAY_clusterThis::dumpParameters() myCVEXPrimVars.cvex_pscale_prim: " << myCVEXPrimVars.cvex_pscale_prim << endl;
   cout << "VRAY_clusterThis::dumpParameters() myCVEXPrimVars.cvex_weight_prim: " << myCVEXPrimVars.cvex_weight_prim << endl;
   cout << "VRAY_clusterThis::dumpParameters() myCVEXPrimVars.cvex_width_prim: " << myCVEXPrimVars.cvex_width_prim << endl;

   cout << "VRAY_clusterThis::dumpParameters() myOTLVersion: " << myOTLVersion << endl;
   cout << "VRAY_clusterThis::dumpParameters() myVelocityScale: " << myVelocityScale << endl;
   cout << "VRAY_clusterThis::dumpParameters() myVerbose: " << myVerbose << endl;

   cout << "VRAY_clusterThis::dumpParameters() myPostProcess: " << myPostProcess << endl;
   cout << "VRAY_clusterThis::dumpParameters() myRasterType: " << myRasterType << endl;
   cout << "VRAY_clusterThis::dumpParameters() myDx: " << myDx << endl;
   cout << "VRAY_clusterThis::dumpParameters() myFogVolume: " << myFogVolume << endl;
   cout << "VRAY_clusterThis::dumpParameters() myGradientWidth: " << myGradientWidth << endl;
   cout << "VRAY_clusterThis::dumpParameters() myVoxelSize: " << myVoxelSize << endl;
   cout << "VRAY_clusterThis::dumpParameters() myBandWidth: " << myBandWidth << endl;
   cout << "VRAY_clusterThis::dumpParameters() myWSUnits: " << myWSUnits << endl;
   cout << "VRAY_clusterThis::dumpParameters() myFalloff: " << myFalloff << endl;
   cout << "VRAY_clusterThis::dumpParameters() myPosInfluence: " << myPosInfluence << endl;
   cout << "VRAY_clusterThis::dumpParameters() myVelInfluence: " << myVelInfluence << endl;
   cout << "VRAY_clusterThis::dumpParameters() myNormalInfluence: " << myNormalInfluence << endl;

   cout << "VRAY_clusterThis::dumpParameters() myVDBMedianFilter: " << myVDBMedianFilter << endl;
   cout << "VRAY_clusterThis::dumpParameters() myVDBMeanFilter: " << myVDBMeanFilter << endl;
   cout << "VRAY_clusterThis::dumpParameters() myVDBMeanCurvatureFilter: " << myVDBMeanCurvatureFilter << endl;
   cout << "VRAY_clusterThis::dumpParameters() myVDBLaplacianFilter: " << myVDBLaplacianFilter << endl;
   cout << "VRAY_clusterThis::dumpParameters() myVDBOffsetFilter: " << myVDBOffsetFilter << endl;
   cout << "VRAY_clusterThis::dumpParameters() myVDBOffsetFilterAmount: " << myVDBOffsetFilterAmount << endl;
   cout << "VRAY_clusterThis::dumpParameters() myVDBReNormalizeFilter: " << myVDBReNormalizeFilter << endl;
   cout << "VRAY_clusterThis::dumpParameters() myVDBWriteDebugFiles: " << myVDBWriteDebugFiles << endl;

   cout << "VRAY_clusterThis::dumpParameters() myBox:"  << endl;
   cout << "VRAY_clusterThis::dumpParameters() " << myBox.vals[0][0] << " " << myBox.vals[0][1] << endl;
   cout << "VRAY_clusterThis::dumpParameters() " << myBox.vals[1][0] << " " << myBox.vals[1][1] << endl;
   cout << "VRAY_clusterThis::dumpParameters() " << myBox.vals[2][0] << " " << myBox.vals[2][1] << endl;

   cout << "VRAY_clusterThis::dumpParameters() Bounding Box Parms:"  << endl;
   cout << "VRAY_clusterThis::dumpParameters() " << bb_x1 << " " << bb_x2 << endl;
   cout << "VRAY_clusterThis::dumpParameters() " << bb_y1 << " " << bb_y2 << endl;
   cout << "VRAY_clusterThis::dumpParameters() " << bb_z1 << " " << bb_z2 << endl;
}


/* ******************************************************************************
*  Function Name : preLoadGeoFile()
*
*  Description :
*
*  Input Arguments : GU_Detail *file_gdp
*
*  Return Value : file load status
*
***************************************************************************** */
int VRAY_clusterThis::preLoadGeoFile(GU_Detail * file_gdp)
{
//     UT_Options myOptions;

   if(file_gdp->load((const char *)myGeoFile).success())
      return 0;
   else
      return 1;

}


#endif







