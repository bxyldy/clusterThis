/* ******************************************************************************
*
* clusterThis mantra DSO for render time geo clustering
*
* Description : This DSO  will instance geomtery or volumes during the render of mantra IFD's
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
#include <SYS/SYS_Math.h>

#include <sys/stat.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <limits>
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

#include "version.h"
#include "VRAY_clusterThis.h"
#include "VRAY_clusterThisUtil.C"
#include "VRAY_clusterThisRender.C"
#include "VRAY_clusterThisInstance.C"
#include "VRAY_clusterThisAttributeUtils.C"
#include "VRAY_clusterCVEXUtil.C"
#include "VRAY_clusterThisRunCVEX.C"
#include "VRAY_clusterThisPostProcess.C"

class VRAY_clusterThis_Exception;



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
   VRAY_ProceduralArg("motion_blur", "integer", "0"),
   VRAY_ProceduralArg("backtrack_mb", "integer", "0"),
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
   VRAY_ProceduralArg("geo_file", "string", "default.bgeo"),
   VRAY_ProceduralArg("filter_type", "integer", "0"),
   VRAY_ProceduralArg("filter_amp", "real", "0.0"),
   VRAY_ProceduralArg("temp_file_path", "string", "/tmp/geo/"),
   VRAY_ProceduralArg("temp_file", "integer", "0"),
   VRAY_ProceduralArg("save_temp_file", "integer", "0"),
   VRAY_ProceduralArg("otl_version", "string", "DCA_VERSION"),

   VRAY_ProceduralArg("CVEX_shader", "string", "default.vex"),
   VRAY_ProceduralArg("CVEX_exec", "integer", "0"),
   VRAY_ProceduralArg("CVEX_shader_prim", "string", "default.vex"),
   VRAY_ProceduralArg("CVEX_exec_prim", "integer", "0"),
   VRAY_ProceduralArg("CVEX_shader_pre", "string", "default.vex"),
   VRAY_ProceduralArg("CVEX_exec_pre", "integer", "0"),
   VRAY_ProceduralArg("CVEX_shader_post", "string", "default.vex"),
   VRAY_ProceduralArg("CVEX_exec_post", "integer", "0"),

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
   VRAY_ProceduralArg("nn_post_process", "integer", "1"),
   VRAY_ProceduralArg("nn_post_pos_influence", "real", "0.1"),
   VRAY_ProceduralArg("nn_post_vel_influence", "real", "0.1"),
   VRAY_ProceduralArg("vdb_post_process", "integer", "1"),
   VRAY_ProceduralArg("vdb_post_raster_type", "integer", "0"),
   VRAY_ProceduralArg("vdb_post_ws_units", "integer", "1"),
   VRAY_ProceduralArg("vdb_post_fog_volume", "integer", "0"),
   VRAY_ProceduralArg("vdb_post_dx", "real", "1.0"),
   VRAY_ProceduralArg("vdb_post_gradient_width", "real", "0.5"),
   VRAY_ProceduralArg("vdb_post_voxel_size", "real", "0.025"),
   VRAY_ProceduralArg("vdb_post_radius_min", "real", "1.5"),
   VRAY_ProceduralArg("vdb_post_radius_mult", "real", "1.0"),
   VRAY_ProceduralArg("vdb_post_velocity_mult", "real", "1.0"),
   VRAY_ProceduralArg("vdb_post_bandwidth", "real", "0.2"),
   VRAY_ProceduralArg("vdb_post_falloff", "real", "0.5"),
   VRAY_ProceduralArg("vdb_post_pos_influence", "real", "0.1"),
   VRAY_ProceduralArg("vdb_post_vel_influence", "real", "0.1"),
   VRAY_ProceduralArg("vdb_post_normal_influence", "real", "0.1"),
   VRAY_ProceduralArg("vdb_post_median_filter", "integer", "0"),
   VRAY_ProceduralArg("vdb_post_mean_filter", "integer", "0"),
   VRAY_ProceduralArg("vdb_post_mean_curvature_filter", "integer", "0"),
   VRAY_ProceduralArg("vdb_post_laplacian_filter", "integer", "0"),
   VRAY_ProceduralArg("vdb_post_median_iterations", "integer", "4"),
   VRAY_ProceduralArg("vdb_post_mean_iterations", "integer", "4"),
   VRAY_ProceduralArg("vdb_post_mean_curvature_iterations", "integer", "4"),
   VRAY_ProceduralArg("vdb_post_laplacian_iterations", "integer", "4"),
   VRAY_ProceduralArg("vdb_post_offset_filter", "integer", "0"),
   VRAY_ProceduralArg("vdb_post_offset_filter_amount", "real", "0.1"),
   VRAY_ProceduralArg("vdb_post_renormalize_filter", "integer", "0"),
   VRAY_ProceduralArg("vdb_post_write_debug_file", "integer", "0"),

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

   openvdb::tools::ParticlesToLevelSet<openvdb::ScalarGrid, ParticleList, hvdb::Interrupter> raster(*outputGrid, boss);

//   std::cout << "VRAY_clusterThis::convert() " << std::endl;


   raster.setRmin(settings.mRadiusMin);

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
         openvdb::tools::levelSetToFogVolume(*outputGrid, cutOffDist, false);
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

#ifdef DEBUG
   std::cout << "VRAY_clusterThis::VRAY_clusterThis() - Constructor" << std::endl;
#endif

   if(myVerbose > CLUSTER_MSG_INFO)
      std::cout << "VRAY_clusterThis - Constructor" << std::endl;

   // Init member variables
   myBox.initBounds(0, 0, 0);
   myVelBox.initBounds(0, 0, 0);

   myPrimType = CLUSTER_PRIM_SPHERE;
   myUseGeoFile = 0;
   mySrcGeoFname = "";
   myNumCopies = 0;
   myNoiseType = 0;
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

   myMethod = CLUSTER_INSTANCE_NOW;
   myVerbose = CLUSTER_MSG_QUIET;
   myUseTempFile = 0;
   mySaveTempFile = 0;

   myUsePointRadius = 0;
   myUseBacktrackMB = 0;

   // CVEX parms
   myCVEX_Exec = 0;
   myCVEX_Exec_prim = 0;
   myCVEX_Exec_pre = 0;
   myCVEX_Exec_post = 0;
   myCVEXFname = "";
   myCVEXFname_prim = "";
   myCVEXFname_pre = "";
   myCVEXFname_post = "";

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


   // Post processing parms
   myPostProcess = 0;

   // Nearest neighbor post processing parms
   myNNPostProcess = 0;
   myNNPostPosInfluence = 0.1;
   myNNPostVelInfluence = 0.1;

   // VDB post processing parms
   myVDBPostProcess = 0;
   myPostRasterType = 0;
   myPostDx = 1.0;
   myPostFogVolume = 0;
   myPostGradientWidth = 0.5;
   myPostVoxelSize = 0.025;
   myPostRadiusMin = 1.5;
   myPostBandWidth = 0.2;
   myPostWSUnits = 1;
   myPostVDBRadiusMult = 1.0;
   myPostVDBVelocityMult = 1.0;
   myPostFalloff = 0.5;
   myPostPosInfluence = 0.1;
   myPostNormalInfluence = 0.1;
   myPostVelInfluence = 0.1;
   myPostVDBMedianFilter = 0;
   myPostVDBMeanFilter = 0;
   myPostVDBMeanCurvatureFilter = 0;
   myPostVDBLaplacianFilter = 0;
   myPostVDBOffsetFilter = 0;
   myPostVDBOffsetFilterAmount = 0.1;
   myPostVDBReNormalizeFilter = 0;
   myPostVDBWriteDebugFiles = 0;
   myPostVDBMedianIterations = 4;
   myPostVDBMeanIterations = 4;
   myPostVDBMeanCurvatureIterations = 4;
   myPostVDBLaplacianIterations = 4;

   VRAY_clusterThis::exitData.exitTime = 3.333;
   VRAY_clusterThis::exitData.exitCode = 3;

   myInstanceNum = 0;
   myTimeScale = 0.5F / myFPS;

   int exitCallBackStatus = -1;
   exitCallBackStatus = UT_Exit::addExitCallback(VRAY_clusterThis::exitClusterThis, (void *)this);

   if((exitCallBackStatus != 1) && exitCallBackStatus != 0) {
         std::cout << "VRAY_clusterThis::VRAY_clusterThis() - error adding ExitCallback() - exitCallBackStatus = " << std::hex
                   << exitCallBackStatus << std::dec << std::endl;
      }


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
int VRAY_clusterThis::initialize(const UT_BoundingBox * box)
{
   if(myVerbose > CLUSTER_MSG_INFO)
      std::cout << "VRAY_clusterThis::initialize()" << std::endl;

   void     *     handle;
   const char  *  name;
   UT_BoundingBox tbox, tvbox;
   UT_Matrix4     xform;
   UT_String      str;


   // Get the OTL parameters
   VRAY_clusterThis::getOTLParameters();


   if(myVerbose > CLUSTER_MSG_QUIET) {
         std::cout << "VRAY_clusterThis::initialize() - Version: " << MAJOR_VER << "." << MINOR_VER << "." << BUILD_VER << std::endl;
         std::cout << "VRAY_clusterThis::initialize() - Built for Houdini Version: " << UT_MAJOR_VERSION
                   << "." << UT_MINOR_VERSION << "." << UT_BUILD_VERSION_INT << std::endl;
         std::cout << "VRAY_clusterThis::initialize() - Initializing ..." <<  std::endl;
      }



   // Find the geometry object to render
//   name = 0;
//   if(VRAY_Procedural::import("object", str))
//      name = str.isstring() ? (const char *)str : 0;
////   handle = VRAY_Procedural::queryObject(name);
//   handle = VRAY_Procedural::queryObject(0);
//   if(!handle) {
//         VRAYerror("%s couldn't find object '%s'", VRAY_Procedural::getClassName(), name);
//         return 0;
//      }
//   name = VRAY_Procedural::queryObjectName(handle);
//
////   std::cout << "VRAY_clusterThis::initialize() name: " << name << std::endl;
//
//   myGdp = (GU_Detail *)VRAY_Procedural::queryGeometry(handle, 0);
//   if(!myGdp) {
//         VRAYerror("%s object '%s' has no geometry", VRAY_Procedural::getClassName(), name);
//         return 0;
//      }



   handle = VRAY_Procedural::queryObject(0);
   myGdp = VRAY_Procedural::allocateGeometry();

   if(myUseGeoFile) {
         // If the file failed to load, throw an exception
         if(!(myGdp->load((const char *)mySrcGeoFname).success()))
            throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() - Failed to read source geometry file ", 1);

         if(myVerbose > CLUSTER_MSG_INFO)
            cout << "VRAY_clusterThis::render() - Successfully loaded source geo file: " << mySrcGeoFname << endl;
      }
   else {
         myGdp->copy(*VRAY_Procedural::queryGeometry(handle, 0));
         if(myVerbose > CLUSTER_MSG_INFO)
            cout << "VRAY_clusterThis::render() - Copied incoming geometry" << endl;
      }




   // Get the point's attribute references
   VRAY_clusterThis::getAttributeRefs(myGdp);

   // Check for required attributes
   VRAY_clusterThis::checkRequiredAttributes();


   VRAY_Procedural::querySurfaceShader(handle, myMaterial);
   myMaterial.harden();
//         myPointAttributes.material = myMaterial;

   myObjectName = VRAY_Procedural::queryObjectName(handle);

//      cout << "VRAY_clusterThis::render() Object Name: " << myObjectName << std::endl;
//      cout << "VRAY_clusterThis::render() Root Name: " << queryRootName() << std::endl;

#ifdef DEBUG
   cout << "Geometry Samples: " << queryGeometrySamples(handle) << std::endl;
#endif


//   if(import("object:name", str)) {
//         cout << "object:name: " << str << std::endl;
//      }
//
//   if(import("object:surface", str)) {
//         cout << "object:surface: " << str << std::endl;
//      }
//
//   if(import("plane:variable", str)) {
//         cout << "plane:variable: " << str << std::endl;
//      }
//
//   if(import("image:resolution", str)) {
//         cout << "image:resolution: " << str << std::endl;
//      }
//
//   if(import("object:categories", str)) {
//         cout << "object:categories: " << str << std::endl;
//      }
//
//   if(import("object:renderpoints", str)) {
//         cout << "object:renderpoints: " << str << std::endl;
//      }



//   changeSetting("object:geo_velocityblur", "on");

//   int     vblur = 0;
//   import("object:velocityblur", &vblur, 0);
//
//   if(vblur) {
//         str = 0;
//         import("velocity", str);
//         if(str.isstring()) {
////               const char  *  name;
////               name = queryObjectName(handle);
//               VRAYwarning("%s[%s] cannot get %s",
//                           VRAY_Procedural::getClassName(), (const char *)myObjectName, " motion blur attr");
//
//            }
//      }



   myXformInverse = queryTransform(handle, 0);
   myXformInverse.invert();


   // Build point tree for various lookups
   for(uint32 i = myGdp->points().entries(); i-- > 0;) {
         // Append to our list of points to be used for various tasks,
         // like breaking up the point cloud into regular grids, etc.
         mySRCPointList.append(i);

         // Build the geo point tree to be used for nearest neighbor caculations
         const GEO_Point * ppt;
         ppt = myGdp->points()(i);
         fpreal radius = static_cast<fpreal>(ppt->getValue<fpreal>(myInstAttrRefs.pointRadius, 0));
         mySRCPointTree.appendPtRadius(myGdp, ppt, radius);
      }


#ifdef DEBUG
   std::cout << "VRAY_clusterThis::initialize() mySRCPointTree.getMemoryUsage(): " << mySRCPointTree.getMemoryUsage() << std::endl;
#endif


   // find how much noise is being generated to help with BBox calculations
   fpreal noise_bias;

   if(myNoiseType < 4) {
         myNoise.setSeed(myPointAttributes.id);
         noise_bias = (myNoise.turbulence(myPointAttributes.myPos, myFractalDepth, myRough, myNoiseAtten) * myNoiseAmp) + 1.0;
      }
   else {
         myMersenneTwister.setSeed(myPointAttributes.id);
         noise_bias = (myMersenneTwister.frandom() * myNoiseAmp) + 1.0;
      }

#ifdef DEBUG
   cout << "VRAY_clusterThis::initialize() " << "noise_bias: " << noise_bias << endl;
#endif


   // Calculate our BBox for the incoming point cloud and add noise, radius, etc. to enlarge the BBox to
   // accomodate the instanced geometry
   fpreal scale;

   int first = 1;
   xform = myXformInverse;
   for(uint32 i = myGdp->points().entries(); i-- > 0;) {
         GEO_Point * ppt = myGdp->points()(i);

         myPointAttributes.pscale = static_cast<fpreal>(ppt->getValue<fpreal>(myPointAttrRefs.pscale, 0));

         if(myUsePointRadius) {
               myPointAttributes.radius = static_cast<fpreal>(ppt->getValue<fpreal>(myPointAttrRefs.radius, 0));
               scale = (myPointAttributes.radius + noise_bias) * myPointAttributes.pscale;
            }
         else {
               scale = (myRadius + noise_bias) * myPointAttributes.pscale;
            }

         getRoughBBox(tbox, tvbox, ppt, scale, myPointAttrRefs.v, myTimeScale, xform);

         if(first) {
               myBox = tbox;
               myVelBox = tvbox;
               first = 0;
            }
         else {
               myBox.enlargeBounds(tbox);
               myVelBox.enlargeBounds(tvbox);
            }

      }

   if(first) {
         std::cout << "VRAY_clusterThis::initialize() " << getClassName() << " found no points in: " << name << std::endl;
         VRAYwarning("%s found no points in %s", getClassName(), name);
         return 0;
      }


#ifdef DEBUG
   std::cout << "VRAY_clusterThis::initialize() mySRCPointList.getMemoryUsage(): " << mySRCPointList.getMemoryUsage() << std::endl;
#endif

//   std::cout << "VRAY_clusterThis::initialize() 2 \nmyBox: " << myBox << "myVelBox: " << myVelBox << std::endl;


   if(box) {
         if(myPointAttrRefs.v.isValid()) {
               if(testClampBox(myBox, *box) || testClampBox(myVelBox, *box))
//                  VRAYwarning("%s[%s] cannot render a partial box %s", getClassName(), name, "with motion blur");
                  std::cout << "VRAY_clusterThis::initialize() " << getClassName() << " WARNING: cannot render a partial box " << name << std::endl;
            }
         else {
               clampBox(myBox, *box);
               clampBox(myVelBox, *box);
            }
      }

//   std::cout << "VRAY_clusterThis::initialize() 3 \nmyBox: " << myBox << "myVelBox: " << myVelBox << std::endl;
//
//   if(box) {
//         std::cout << "VRAY_clusterThis::initialize() box min: " << box->xmin() << " " << box->ymin() << " " << box->zmin() << std::endl;
//         std::cout << "VRAY_clusterThis::initialize() box max: " << box->xmax() << " " << box->ymax() << " " << box->zmax() << std::endl;
//      }


   return 1;
}



/* ******************************************************************************
*  Function Name : getOTLParameters
*
*  Description : Get OTL parameters
*
*  Input Arguments : None
*
*  Return Value : None
*
***************************************************************************** */
int VRAY_clusterThis::getOTLParameters()
{

//   UT_String geo_fname, temp_fname, cvex_fname, otl_version;
   const int * int_ptr;
   const fpreal * flt_ptr;
   const char ** char_handle;

   if(int_ptr = VRAY_Procedural::getIParm("prim_type"))
      myPrimType = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("num_copy"))
      myNumCopies = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("recursion"))
      myRecursion = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("motion_blur"))
      myDoMotionBlur = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("backtrack_mb"))
      myUseBacktrackMB = *int_ptr;

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

   if(int_ptr = VRAY_Procedural::getIParm("use_geo_file"))
      myUseGeoFile = *int_ptr;

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


   if(int_ptr = VRAY_Procedural::getIParm("temp_file"))
      myUseTempFile = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("save_temp_file"))
      mySaveTempFile =  *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("verbose"))
      myVerbose = *int_ptr;

//   if(char_handle = VRAY_Procedural::getSParm("otl_version")) {
//         myOTLVersion = (UT_String)(*char_handle);
//         myOTLVersion.harden();
//      }




   if(char_handle = VRAY_Procedural::getSParm("CVEX_shader")) {
         myCVEXFname = (UT_String)(*char_handle);
         myCVEXFname.harden();
      }


   if(int_ptr = VRAY_Procedural::getIParm("CVEX_exec"))
      myCVEX_Exec =  *int_ptr;


   if(char_handle = VRAY_Procedural::getSParm("CVEX_shader_prim")) {
         myCVEXFname_prim = (UT_String)(*char_handle);
         myCVEXFname_prim.harden();
      }

   if(int_ptr = VRAY_Procedural::getIParm("CVEX_exec_prim"))
      myCVEX_Exec_prim = *int_ptr;


   if(char_handle = VRAY_Procedural::getSParm("CVEX_shader_pre")) {
         myCVEXFname_pre = (UT_String)(*char_handle);
         myCVEXFname_pre.harden();
      }

   if(int_ptr = VRAY_Procedural::getIParm("CVEX_exec_pre"))
      myCVEX_Exec_pre = *int_ptr;

   if(char_handle = VRAY_Procedural::getSParm("CVEX_shader_post")) {
         myCVEXFname_post = (UT_String)(*char_handle);
         myCVEXFname_post.harden();
      }

   if(int_ptr = VRAY_Procedural::getIParm("CVEX_exec_post"))
      myCVEX_Exec_post = *int_ptr;


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


   // Post processing parms
   if(int_ptr = VRAY_Procedural::getIParm("post_process"))
      myPostProcess = *int_ptr;

   // Nearest neighbor post processing parms
   if(int_ptr = VRAY_Procedural::getIParm("nn_post_process"))
      myNNPostProcess = *int_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("nn_post_pos_influence"))
      myNNPostPosInfluence = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("nn_post_vel_influence"))
      myNNPostVelInfluence = *flt_ptr;

   // VDB pre processing parms
   if(int_ptr = VRAY_Procedural::getIParm("vdb_post_process"))
      myVDBPostProcess = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("vdb_post_raster_type"))
      myPostRasterType = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("vdb_post_ws_units"))
      myPostWSUnits = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("vdb_post_fog_volume"))
      myPostFogVolume = *int_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("vdb_post_dx"))
      myPostDx = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("vdb_post_gradient_width"))
      myPostGradientWidth = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("vdb_post_voxel_size"))
      myPostVoxelSize = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("vdb_post_radius_min"))
      myPostRadiusMin = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("vdb_post_radius_mult"))
      myPostVDBRadiusMult = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("vdb_post_velocity_mult"))
      myPostVDBVelocityMult = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("vdb_post_bandwidth"))
      myPostBandWidth = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("vdb_post_falloff"))
      myPostFalloff = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("vdb_post_pos_influence"))
      myPostPosInfluence = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("vdb_post_vel_influence"))
      myPostVelInfluence = *flt_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("vdb_post_normal_influence"))
      myPostNormalInfluence = *flt_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("vdb_post_median_filter"))
      myPostVDBMedianFilter = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("vdb_post_mean_filter"))
      myPostVDBMeanFilter = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("vdb_post_mean_curvature_filter"))
      myPostVDBMeanCurvatureFilter = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("vdb_post_laplacian_filter"))
      myPostVDBLaplacianFilter = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("vdb_post_median_iterations"))
      myPostVDBMedianIterations = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("vdb_post_mean_iterations"))
      myPostVDBMeanIterations = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("vdb_post_mean_curvature_iterations"))
      myPostVDBMeanCurvatureIterations = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("vdb_post_laplacian_iterations"))
      myPostVDBLaplacianIterations = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("vdb_post_offset_filter"))
      myPostVDBOffsetFilter = *int_ptr;

   if(flt_ptr = VRAY_Procedural::getFParm("vdb_post_offset_filter_amount"))
      myPostVDBOffsetFilterAmount = *flt_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("vdb_post_renormalize_filter"))
      myPostVDBReNormalizeFilter = *int_ptr;

   if(int_ptr = VRAY_Procedural::getIParm("vdb_post_write_debug_file"))
      myPostVDBWriteDebugFiles = *int_ptr;


   return 0;

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

   box = myVelBox;

#ifdef DEBUG
   std::cout << "VRAY_clusterThis::getBoundingBox() box: " << box << std::endl;
#endif

}


/* ******************************************************************************
*  Function Name : checkRequiredAttributes
*
*  Description :  Check that all the required attributes are in the point cloud
*
*  Input Arguments : None
*
*  Return Value : None
*
***************************************************************************** */
void VRAY_clusterThis::checkRequiredAttributes()
{

   // TODO: Confirm that all required attrs are being checked for each instance type

//   std::cout << "VRAY_clusterThis::checkRequiredAttributes()" << std::endl;
   // Check for required attributes
   if(myPointAttrRefs.Cd.isInvalid()) {
         cout << "Incoming points must have Cd attribute! Throwing exception ..." << endl;
         throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have Cd attribute! ", 1);
      }

   if(myPointAttrRefs.Alpha.isInvalid()) {
         cout << "Incoming points must have Alpha attribute! Throwing exception ..." << endl;
         throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have Alpha attribute! ", 1);
      }

   if(myPointAttrRefs.v.isInvalid()) {
         cout << "Incoming points must have v attribute! Throwing exception ..." << endl;
         throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have v attribute! ", 1);
      }

   if(myPointAttrRefs.N.isInvalid()) {
         cout << "Incoming points must have N attribute! Throwing exception ..." << endl;
         throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have N attribute! ", 1);
      }

   if(myPointAttrRefs.pscale.isInvalid()) {
         cout << "Incoming points must have pscale attribute! Throwing exception ..." << endl;
         throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have pscale attribute! ", 1);
      }

   if(myPointAttrRefs.id.isInvalid()) {
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
   fpreal radius;
   dx = SYSsin(delta * myFreqX + myOffsetX);
   dy = SYScos(delta * myFreqY + myOffsetY);
   dz = SYScos(delta * myFreqZ + myOffsetZ);

#ifdef DEBUG
   cout << "VRAY_clusterThis::calculateNewPosition() " << "delta: " << delta << endl;
   cout << "VRAY_clusterThis::calculateNewPosition() " << "dx: " << dx << " dy: " << dy << " dz: " << dz << endl;
#endif

   if(myNoiseType < 4) {
         myNoise.setSeed(myPointAttributes.id);
         noise_bias = (myNoise.turbulence(myPointAttributes.myPos, myFractalDepth, myRough, myNoiseAtten) * myNoiseAmp) + 1.0;
//         cout << "VRAY_clusterThis::calculateNewPosition() turbulence: " << "noise_bias: " << noise_bias << endl;
      }
   else {
         myMersenneTwister.setSeed(myPointAttributes.id);
         noise_bias = (myMersenneTwister.frandom() * myNoiseAmp) + 1.0;
//         cout << "VRAY_clusterThis::calculateNewPosition() myMersenneTwister: " << "noise_bias: " << noise_bias << endl;
      }


#ifdef DEBUG
   cout << "VRAY_clusterThis::calculateNewPosition() " << "noise_bias: " << noise_bias << endl;
#endif

   if(myUsePointRadius)
      radius = myPointAttributes.radius;
   else
      radius = myRadius;


   // Calculate the new object's position
   myPointAttributes.myNewPos[0] = (fpreal) myPointAttributes.myPos.x() +
                                   ((dx * radius) * noise_bias * SYSsin(static_cast<fpreal>(j + i)));
   myPointAttributes.myNewPos[1] = (fpreal) myPointAttributes.myPos.y() +
                                   ((dy * radius) * noise_bias * SYScos(static_cast<fpreal>(j + i)));
   myPointAttributes.myNewPos[2] = (fpreal) myPointAttributes.myPos.z() +
                                   ((dz * radius) * noise_bias * (SYSsin(static_cast<fpreal>(j + i)) + SYScos(static_cast<fpreal>(j + i))));
//   myPointAttributes.myNewPos[2] = ( fpreal ) myPointAttributes.myPos.z() +
//                                    ( ( dz * radius ) * noise_bias * ( SYScos ( static_cast<fpreal>(j + i)) ) );

   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
//         myPointAttributes.myMBPos[0] = myPointAttributes.myNewPos[0] + myPointAttributes.v.x();
//         myPointAttributes.myMBPos[1] = myPointAttributes.myNewPos[1] + myPointAttributes.v.y();
//         myPointAttributes.myMBPos[2] = myPointAttributes.myNewPos[2] + myPointAttributes.v.z();

         if(myUseBacktrackMB) {
               myPointAttributes.myMBPos[0] = myPointAttributes.myNewPos[0] - myPointAttributes.backtrack.x();
               myPointAttributes.myMBPos[1] = myPointAttributes.myNewPos[1] - myPointAttributes.backtrack.y();
               myPointAttributes.myMBPos[2] = myPointAttributes.myNewPos[2] - myPointAttributes.backtrack.z();
            }
         else {
               myPointAttributes.myMBPos[0] = myPointAttributes.myNewPos[0] - myPointAttributes.v.x();
               myPointAttributes.myMBPos[1] = myPointAttributes.myNewPos[1] - myPointAttributes.v.y();
               myPointAttributes.myMBPos[2] = myPointAttributes.myNewPos[2] - myPointAttributes.v.z();
            }
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

   std::cout << std::endl;

   std::cout << "VRAY_clusterThis::dumpParameters() **** SETUP PARAMETERS ****" << std::endl;

   std::cout << "VRAY_clusterThis::dumpParameters() myPrimType: " << myPrimType << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myNumCopies: " << myNumCopies << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myUseGeoFile: " << myUseGeoFile << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() mySrcGeoFname: " << mySrcGeoFname << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myRecursion: " << myRecursion << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myRadius: " << myRadius << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myRough: " << myRough << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() mySize: " << mySize[0] << " " << mySize[1] << " " << mySize[2] << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myFreq: " << myFreqX << " " << myFreqY << " " << myFreqZ << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myOffset: " << myOffsetX << " " << myOffsetY << " " << myOffsetZ << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myBirthProb: " << myBirthProb << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myCurrentTime: " << myCurrentTime << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myDoMotionBlur: " << myDoMotionBlur << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myShutter: " << myShutter << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myVerbose: " << myVerbose << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myGeoFile: " << myGeoFile << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myTempFname: " << myTempFname << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myUseTempFile: " << myUseTempFile << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() mySaveTempFile: " << mySaveTempFile << std::endl;


   std::cout << "VRAY_clusterThis::dumpParameters() **** NOISE PARAMETERS ****" << std::endl;

   std::cout << "VRAY_clusterThis::dumpParameters() myNoiseType: " << myNoiseType << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myNoiseAmp: " << myNoiseAmp << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myNoiseAtten: " << myNoiseAtten << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myNoiseSeed: " << myNoiseSeed << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myFractalDepth: " << myFractalDepth << std::endl;


   std::cout << "VRAY_clusterThis::dumpParameters() **** CVEX PARAMETERS ****" << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myCVEXFname: " << myCVEXFname << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myCVEX_Exec: " << myCVEX_Exec << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myCVEXFname_prim: " << myCVEXFname_prim << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myCVEX_Exec_prim: " << myCVEX_Exec_prim << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myCVEXFname_pre: " << myCVEXFname_pre << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myCVEX_Exec_pre: " << myCVEX_Exec_pre << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myCVEXFname_post: " << myCVEXFname_post << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myCVEX_Exec_post: " << myCVEX_Exec_post << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myCVEXPointVars.cvex_Cd_pt: " << myCVEXPointVars.cvex_Cd_pt << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myCVEXPointVars.cvex_Alpha_pt: " << myCVEXPointVars.cvex_Alpha_pt << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myCVEXPointVars.cvex_N_pt: " << myCVEXPointVars.cvex_Alpha_pt << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myCVEXPointVars.cvex_v_pt: " << myCVEXPointVars.cvex_v_pt << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myCVEXPointVars.cvex_pscale_pt: " << myCVEXPointVars.cvex_pscale_pt << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myCVEXPrimVars.cvex_Cd_prim: " << myCVEXPrimVars.cvex_Cd_prim << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myCVEXPrimVars.cvex_Alpha_prim: " << myCVEXPrimVars.cvex_Alpha_prim << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myCVEXPrimVars.cvex_N_prim: " << myCVEXPrimVars.cvex_N_prim << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myCVEXPrimVars.cvex_v_prim: " << myCVEXPrimVars.cvex_v_prim << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myCVEXPrimVars.cvex_pscale_prim: " << myCVEXPrimVars.cvex_pscale_prim << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myCVEXPrimVars.cvex_weight_prim: " << myCVEXPrimVars.cvex_weight_prim << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myCVEXPrimVars.cvex_width_prim: " << myCVEXPrimVars.cvex_width_prim << std::endl;

   std::cout << "VRAY_clusterThis::dumpParameters() **** POST PROCESS PARAMETERS ****" << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostProcess: " << myPostProcess << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myNNPostProcess: " << myNNPostProcess << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myNNPostPosInfluence: " << myNNPostPosInfluence << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myNNPostVelInfluence: " << myNNPostVelInfluence << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myVDBPostProcess: " << myVDBPostProcess << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostRasterType: " << myPostRasterType << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostDx: " << myPostDx << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostFogVolume: " << myPostFogVolume << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostRadiusMin: " << myPostRadiusMin << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostVDBVelocityMult: " << myPostVDBVelocityMult << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostVDBRadiusMult: " << myPostVDBRadiusMult << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostGradientWidth: " << myPostGradientWidth << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostVoxelSize: " << myPostVoxelSize << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostBandWidth: " << myPostBandWidth << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostWSUnits: " << myPostWSUnits << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostFalloff: " << myPostFalloff << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostPosInfluence: " << myPostPosInfluence << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostVelInfluence: " << myPostVelInfluence << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostNormalInfluence: " << myPostNormalInfluence << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostVDBMedianFilter: " << myPostVDBMedianFilter << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostVDBMeanFilter: " << myPostVDBMeanFilter << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostVDBMeanCurvatureFilter: " << myPostVDBMeanCurvatureFilter << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostVDBLaplacianFilter: " << myPostVDBLaplacianFilter << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostVDBMedianIterations: " << myPostVDBMedianIterations << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostVDBMeanIterations: " << myPostVDBMeanIterations << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostVDBMeanCurvatureIterations: " << myPostVDBMeanCurvatureIterations << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostVDBLaplacianIterations: " << myPostVDBLaplacianIterations << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostVDBOffsetFilter: " << myPostVDBOffsetFilter << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostVDBOffsetFilterAmount: " << myPostVDBOffsetFilterAmount << std::endl;

//   std::cout << "VRAY_clusterThis::dumpParameters() myVDBReNormalizeFilter: " << myVDBReNormalizeFilter << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myPostVDBWriteDebugFiles: " << myPostVDBWriteDebugFiles << std::endl;

   std::cout << "VRAY_clusterThis::dumpParameters() **** MISC PARAMETERS ****" << std::endl;
//   std::cout << "VRAY_clusterThis::dumpParameters() myOTLVersion: " << myOTLVersion << std::endl;
   std::cout << "VRAY_clusterThis::dumpParameters() myVelocityScale: " << myVelocityScale << std::endl;
//   std::cout << "VRAY_clusterThis::dumpParameters() myFilterType: " << myFilterType << std::endl;
//   std::cout << "VRAY_clusterThis::dumpParameters() myFilterAmp: " << myFilterAmp << std::endl;
//   std::cout << "VRAY_clusterThis::dumpParameters() myCopyAttrs: " << myCopyAttrs << std::endl;
//   std::cout << "VRAY_clusterThis::dumpParameters() myBlendAttrs: " << myBlendAttrs << std::endl;

   std::cout << std::endl;


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



