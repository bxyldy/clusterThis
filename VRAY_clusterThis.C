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
*  Version 2.0.0
*  Date: September, 5, 2012
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

#ifndef __VRAY_clusterThis_C__
#define __VRAY_clusterThis_C__

#include <GU/GU_Detail.h>
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
#include <UT/UT_BoundingBox.h>
#include <VRAY/VRAY_Procedural.h>
#include <VRAY/VRAY_Volume.h>
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

#define DCA_VERSION "2.0.0"

#include "VRAY_clusterThis.h"
#include "VRAY_clusterThisChild.h"
#include "VRAY_clusterThisChild.C"
#include "VRAY_clusterThisRender.C"
#include "VRAY_clusterThisInstance.C"
#include "VRAY_clusterThisAttributeUtils.C"
#include "VRAY_clusterCVEXUtil.C"
#include "VRAY_clusterThisRunCVEX.C"


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
   VRAY_ProceduralArg("recursion","integer", "2"),
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
   VRAY_ProceduralArg("noise_type","integer", "0"),
   VRAY_ProceduralArg("noise_amp", "real", "0.1"),
   VRAY_ProceduralArg("noise_rough","real", "0.1"),
   VRAY_ProceduralArg("noise_atten","real", "0.0"),
   VRAY_ProceduralArg("noise_seed","integer", "7"),
   VRAY_ProceduralArg("noise_fractal_depth","integer", "3"),
   VRAY_ProceduralArg("copy_attrs","integer", "1"),
   VRAY_ProceduralArg("speed_stretch","real", "0.0 0.0 0.0"),
   VRAY_ProceduralArg("blend_attrs","integer", "0"),
   VRAY_ProceduralArg("geo_file", "string", "default.bgeo"),
   VRAY_ProceduralArg("CVEX_shader", "string", "default.vex"),
   VRAY_ProceduralArg("CVEX_exec", "integer", "0"),
   VRAY_ProceduralArg("CVEX_shader_prim", "string", "default.vex"),
   VRAY_ProceduralArg("CVEX_exec_prim", "integer", "0"),
   VRAY_ProceduralArg("CVEX_shader_pre", "string", "default.vex"),
   VRAY_ProceduralArg("CVEX_exec_pre", "integer", "0"),
   VRAY_ProceduralArg("CVEX_shader_post", "string", "default.vex"),
   VRAY_ProceduralArg("CVEX_exec_post", "integer", "0"),
   VRAY_ProceduralArg("filter_type","integer", "0"),
   VRAY_ProceduralArg("filter_amp","real", "0.0"),
   VRAY_ProceduralArg("volume_data_1","string", "file.vol"),
   VRAY_ProceduralArg("volume_data_2","string", "file.vol"),
   VRAY_ProceduralArg("volume_data_3","string", "file.vol"),
   VRAY_ProceduralArg("temp_file_path","string", "/tmp/geo/"),
   VRAY_ProceduralArg("temp_file", "integer", "0"),
   VRAY_ProceduralArg("save_temp_file", "integer", "0"),
   VRAY_ProceduralArg("otl_version","string", DCA_VERSION),

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

//   cout << "VRAY_clusterThis_Exception: in constructor ... " << endl;

   e_msg = msg;
   e_code = code;

};


//VRAY_clusterThis_Exception::~VRAY_clusterThis_Exception() {

//   cout << "VRAY_clusterThis_Exception: in destructor ... " << endl;

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
//   if (myVerbose > CLUSTER_MSG_INFO)
//      std::cout << "VRAY_clusterThis::exitClusterThis() - Preparing to exit!" << std::endl;
   VRAY_clusterThis * me = (VRAY_clusterThis *)data;
   me->exitClusterThisReal((const char *)me->myTempFname);
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
void VRAY_clusterThis::exitClusterThisReal(const char * fname)
{
   struct stat fileResults;

   if (myVerbose > CLUSTER_MSG_INFO)
      cout << "VRAY_clusterThis::exitClusterThisReal() - Running exit processing" << endl;

//    ofstream myStream;
//
//   myStream.open("exit_data.txt", ios_base::app);
//   myStream << this->exitData.exitTime << std::endl;
//   myStream << this->exitData.exitCode << std::endl;
//   myStream.flush();
//   myStream.close();
//      cout << "VRAY_clusterThis::exitClusterThisReal() : " << this->exitData.exitTime << endl;

   if (this->myUseTempFile && !this->mySaveTempFile) {
      if ((UT_String(fname)).isstring() && stat(fname, &fileResults)==0) {
         if (myVerbose > CLUSTER_MSG_INFO)
            cout << "VRAY_clusterThis::exitClusterThisReal() - Found temp file " << fname << endl;
         if (!remove(fname) && (myVerbose > CLUSTER_MSG_INFO))
            cout << "VRAY_clusterThis::exitClusterThisReal() - Removed geometry temp file: " << fname << endl;
      } else if (myVerbose > CLUSTER_MSG_INFO)
         cout << "VRAY_clusterThis::exitClusterThisReal() - Did not find temp file " << fname << endl << endl;
   }

   if (myVerbose > CLUSTER_MSG_INFO)
      std::cout << "VRAY_clusterThis::exitClusterThisReal() - Exiting" << std::endl;
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
   myShutter      = 0.1;
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

   VRAY_clusterThis::exitData.exitTime = 3.333;
   VRAY_clusterThis::exitData.exitCode = 3;

   myInstanceNum = 0;

   int exitCallBackStatus = -1;
   exitCallBackStatus = UT_Exit::addExitCallback(VRAY_clusterThis::exitClusterThis, (void *)this);

   if ((exitCallBackStatus != 1) && exitCallBackStatus != 0) {
      std::cout << "VRAY_clusterThis::VRAY_clusterThis() - error adding ExitCallback() - exitCallBackStatus = " << std::hex
                << exitCallBackStatus << std::dec << std::endl;
   }

//    exitCallBackStatus = 0;

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
   if (myVerbose > CLUSTER_MSG_INFO)
      std::cout << "VRAY_clusterThis::initialize()" << std::endl;

   UT_String geo_fname, temp_fname, cvex_fname, otl_version;
   const int * int_ptr;
   const fpreal * flt_ptr;
   const char ** char_handle;

   // Import the object:velocityscale settings.  This setting stores the
   // shutter time (in seconds) on a per object basis.  It's used primarily
   // for velocity blur.

   if (!import("object:velocityscale", &myVelocityScale, 1))
      myVelocityScale = 0;

//const int *  getIParm (const char *name) const
//const fpreal *  getFParm (const char *name) const
//const char **   getSParm (const char *name) const
//const int *  getIParm (int token) const
//const fpreal *  getFParm (int token) const
//const char **   getSParm (int token) const


//    ray_procedural clusterThis minbound -0.699999988079 -0.699999988079 -0.699999988079 maxbound 0.699999988079 0.699999988079 0.699999988079 prim_type 8 num_copy 14 recursion 19 radius 0.5 size 0.20000000298 0.20000000298 0.20000000298 freq 2 3 2 noise_type 3 noise_amp 0 noise_rough 0.391999989748 noise_fractal_depth 7 noise_atten 0.922999978065 temp_file_path "cache/temp_geo.bgeo"

   // Get OTL parameters

   if (flt_ptr = VRAY_Procedural::getFParm("minbound")) {
      bb_x1 = *flt_ptr++;
      bb_y1 = *flt_ptr++;
      bb_z1 = *flt_ptr;
      myBox.initBounds(bb_x1, bb_y1, bb_z1);
   }

   if (flt_ptr = VRAY_Procedural::getFParm("maxbound")) {
      bb_x2 = *flt_ptr++;
      bb_y2 = *flt_ptr++;
      bb_z2 = *flt_ptr;
      myBox.enlargeBounds(bb_x2, bb_y2, bb_z2);
   }

   if (int_ptr = VRAY_Procedural::getIParm("prim_type"))
      myPrimType = *int_ptr;

   if (int_ptr = VRAY_Procedural::getIParm("num_copy"))
      myNumCopies = *int_ptr;

   if (int_ptr = VRAY_Procedural::getIParm("recursion"))
      myRecursion = *int_ptr;

   if (int_ptr = VRAY_Procedural::getIParm("add_proc"))
      myMethod = *int_ptr;

   if (int_ptr = VRAY_Procedural::getIParm("motion_blur"))
      myDoMotionBlur = *int_ptr;

   if (flt_ptr = VRAY_Procedural::getFParm("mb_shutter"))
      myShutter = *flt_ptr;

   if (flt_ptr = VRAY_Procedural::getFParm("radius"))
      myRadius = *flt_ptr;

   if (int_ptr = VRAY_Procedural::getIParm("noise_type"))
      myNoiseType = static_cast<UT_Noise::UT_NoiseType>(*int_ptr);

   if (flt_ptr = VRAY_Procedural::getFParm("freq")) {
      myFreqX = *flt_ptr++;
      myFreqY = *flt_ptr++;
      myFreqZ = *flt_ptr;
   }

   if (flt_ptr = VRAY_Procedural::getFParm("offset")) {
      myOffsetX = *flt_ptr++;
      myOffsetY = *flt_ptr++;
      myOffsetZ = *flt_ptr;
   }

   if (flt_ptr = VRAY_Procedural::getFParm("radius"))
      myRadius = *flt_ptr;

   if (flt_ptr = VRAY_Procedural::getFParm("noise_rough"))
      myRough = *flt_ptr;

   if (flt_ptr = VRAY_Procedural::getFParm("size")) {
      mySize[0] = *flt_ptr++;
      mySize[1] = *flt_ptr++;
      mySize[2] = *flt_ptr;
   }

   if (flt_ptr = VRAY_Procedural::getFParm("birth_prob"))
      myBirthProb = *flt_ptr;

   if (flt_ptr = VRAY_Procedural::getFParm("time"))
      myCurrentTime = *flt_ptr;

   if (flt_ptr = VRAY_Procedural::getFParm("noise_amp"))
      myNoiseAmp = *flt_ptr;

   if (int_ptr = VRAY_Procedural::getIParm("filter_type"))
      myFilterType = *int_ptr;

   if (flt_ptr = VRAY_Procedural::getFParm("filter_amp"))
      myFilterAmp = *flt_ptr;

   if (flt_ptr = VRAY_Procedural::getFParm("noise_atten"))
      myNoiseAtten = *flt_ptr;

   if (int_ptr = VRAY_Procedural::getIParm("noise_seed"))
      myNoiseSeed = *int_ptr;

   if (int_ptr = VRAY_Procedural::getIParm("noise_fractal_depth"))
      myFractalDepth = *int_ptr;

   if (int_ptr = VRAY_Procedural::getIParm("copy_attrs"))
      myCopyAttrs = bool (*int_ptr);

   if (int_ptr = VRAY_Procedural::getIParm("blend_attrs"))
      myBlendAttrs = bool (*int_ptr);

   if (int_ptr = VRAY_Procedural::getIParm("use_geo_file"))
      myUseGeoFile = bool (*int_ptr);

   if (char_handle = VRAY_Procedural::getSParm("src_geo_file")) {
      mySrcGeoFname = (UT_String)(*char_handle);
      mySrcGeoFname.harden();
   }


   if (char_handle = VRAY_Procedural::getSParm("geo_file")) {
      myGeoFile = (UT_String)(*char_handle);
      myGeoFile.harden();
   }

   if (char_handle = VRAY_Procedural::getSParm("temp_file_path")) {
      myTempFname = (UT_String)(*char_handle);
      myTempFname.harden();
   }

   if (char_handle = VRAY_Procedural::getSParm("CVEX_shader")) {
      myCVEXFname = (UT_String)(*char_handle);
      myCVEXFname.harden();
   }


   if (int_ptr = VRAY_Procedural::getIParm("CVEX_exec"))
      myCVEX_Exec = bool (*int_ptr);


   if (char_handle = VRAY_Procedural::getSParm("CVEX_shader_prim")) {
      myCVEXFname_prim = (UT_String)(*char_handle);
      myCVEXFname_prim.harden();
   }

   if (int_ptr = VRAY_Procedural::getIParm("CVEX_exec_prim"))
      myCVEX_Exec_prim = bool (*int_ptr);


   if (char_handle = VRAY_Procedural::getSParm("CVEX_shader_pre")) {
      myCVEXFname_pre = (UT_String)(*char_handle);
      myCVEXFname_pre.harden();
   }

   if (int_ptr = VRAY_Procedural::getIParm("CVEX_exec_pre"))
      myCVEX_Exec_pre = bool (*int_ptr);

   if (char_handle = VRAY_Procedural::getSParm("CVEX_shader_post")) {
      myCVEXFname_post = (UT_String)(*char_handle);
      myCVEXFname_post.harden();
   }

   if (int_ptr = VRAY_Procedural::getIParm("CVEX_exec_post"))
      myCVEX_Exec_post = bool (*int_ptr);


   if (int_ptr = VRAY_Procedural::getIParm("cvex_Cd_pt"))
      myCVEXPointVars.cvex_Cd_pt = *int_ptr;

   if (int_ptr = VRAY_Procedural::getIParm("cvex_Alpha_pt"))
      myCVEXPointVars.cvex_Alpha_pt = *int_ptr;

   if (int_ptr = VRAY_Procedural::getIParm("cvex_v_pt"))
      myCVEXPointVars.cvex_v_pt = *int_ptr;

   if (int_ptr = VRAY_Procedural::getIParm("cvex_N_pt"))
      myCVEXPointVars.cvex_N_pt = *int_ptr;

   if (int_ptr = VRAY_Procedural::getIParm("cvex_pscale_pt"))
      myCVEXPointVars.cvex_pscale_pt = *int_ptr;


   if (int_ptr = VRAY_Procedural::getIParm("cvex_Cd_prim"))
      myCVEXPrimVars.cvex_Cd_prim = *int_ptr;

   if (int_ptr = VRAY_Procedural::getIParm("cvex_Alpha_prim"))
      myCVEXPrimVars.cvex_Alpha_prim = *int_ptr;

   if (int_ptr = VRAY_Procedural::getIParm("cvex_v_prim"))
      myCVEXPrimVars.cvex_v_prim = *int_ptr;

   if (int_ptr = VRAY_Procedural::getIParm("cvex_N_prim"))
      myCVEXPrimVars.cvex_N_prim = *int_ptr;

   if (int_ptr = VRAY_Procedural::getIParm("cvex_pscale_prim"))
      myCVEXPrimVars.cvex_pscale_prim = *int_ptr;

   if (int_ptr = VRAY_Procedural::getIParm("cvex_weight_prim"))
      myCVEXPrimVars.cvex_weight_prim = *int_ptr;

   if (int_ptr = VRAY_Procedural::getIParm("cvex_width_prim"))
      myCVEXPrimVars.cvex_width_prim = *int_ptr;

   if (int_ptr = VRAY_Procedural::getIParm("temp_file"))
      myUseTempFile = bool (*int_ptr);

   if (int_ptr = VRAY_Procedural::getIParm("save_temp_file"))
      mySaveTempFile = bool (*int_ptr);

   if (int_ptr = VRAY_Procedural::getIParm("verbose"))
      myVerbose = *int_ptr;

   if (char_handle = VRAY_Procedural::getSParm("otl_version")) {
      myOTLVersion = (UT_String)(*char_handle);
      myOTLVersion.harden();
   }

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
   box = myBox;
      box.initBounds(myPointAttributes.myNewPos);
//    box.enlargeBounds(mySize[0] * 10, mySize[1] * 10, mySize[2] * 10);
   fpreal size = mySize[0];
   if(size < mySize[1])
      size = mySize[1];
   if(size < mySize[2])
      size = mySize[2];

   box.enlargeBounds(0, (1 + size) *(1 + size));

//#ifdef DEBUG
   std::cout << "VRAY_clusterThis::getBoundingBox() box: " << box << std::endl;
//#endif

   myLOD = getLevelOfDetail(box);

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
   if (myPointAttrOffsets.Cd.isInvalid()) {
      cout << "Incoming points must have Cd attribute! Throwing exception ..." << endl;
      throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have Cd attribute! ", 1);
   }

   if (myPointAttrOffsets.Alpha.isInvalid()) {
      cout << "Incoming points must have Alpha attribute! Throwing exception ..." << endl;
      throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have Alpha attribute! ", 1);
   }

   if (myPointAttrOffsets.v.isInvalid()) {
      cout << "Incoming points must have v attribute! Throwing exception ..." << endl;
      throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have v attribute! ", 1);
   }

   if (myPointAttrOffsets.N.isInvalid()) {
      cout << "Incoming points must have N attribute! Throwing exception ..." << endl;
      throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have N attribute! ", 1);
   }

   if (myPointAttrOffsets.pscale.isInvalid()) {
      cout << "Incoming points must have pscale attribute! Throwing exception ..." << endl;
      throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have pscale attribute! ", 1);
   }

   if (myPointAttrOffsets.id.isInvalid()) {
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
   fpreal dx, dy, dz = 0.0;
   dx = SYSsin(delta * myFreqX + myOffsetX);
   dy = SYScos(delta * myFreqY + myOffsetY);
   dz = SYScos(delta * myFreqZ + myOffsetZ);

#ifdef DEBUG
   cout << "VRAY_clusterThis::calculateNewPosition() " << "delta: " << delta << endl;
   cout << "VRAY_clusterThis::calculateNewPosition() " << "dx: " << dx << " dy: " << dy << " dz: " << dz << endl;
#endif

   myNoise.setSeed(myPointAttributes.id);

   // Calculate a bit of noise to add to the new position ...
   // TODO:
   fpreal noise_bias = (myNoise.turbulence(myPointAttributes.myPos, myFractalDepth, myRough, myNoiseAtten) * myNoiseAmp) + 1.0;

   // myNoise.turbulence(myPos, myFractalDepth, myNoiseVec, myRough, myNoiseAtten);
   // cout << "VRAY_clusterThis::render() " << "myNoiseVec: " << myNoiseVec.x() << " " << myNoiseVec.x() << " " << myNoiseVec.x() << endl;

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

   if (myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
      myPointAttributes.myMBPos[0] = myPointAttributes.myNewPos[0] - myPointAttributes.v.x();
      myPointAttributes.myMBPos[1] = myPointAttributes.myNewPos[1] - myPointAttributes.v.y();
      myPointAttributes.myMBPos[2] = myPointAttributes.myNewPos[2] - myPointAttributes.v.z();
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


   cout << "VRAY_clusterThis::dumpParameters() Current gdp bounding box (myBox):"  << endl;
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

   if (file_gdp->load((const char *)myGeoFile).success())
      return 0;
   else
      return 1;

}


#endif


/**********************************************************************************/
//  $Log: VRAY_clusterThis.C,v $
//  Revision 1.60  2012-09-09 05:00:54  mstory
//  More cleanup and testing.
//
//  Revision 1.59  2012-09-07 15:39:23  mstory
//   Removed all volume instancing (used in different project) and continu… …
//
//  …ed H12 modifications.
//
//  --mstory
//
//  Revision 1.58  2012-09-05 23:02:38  mstory
//  Modifications for H12.
//
//  Revision 1.57  2012-09-04 03:25:28  mstory
//  .
//
//  Revision 1.54  2011-02-15 01:00:23  mstory
//  .
//
//  Revision 1.53  2011-02-15 00:59:15  mstory
//  Refactored out rededundant attribute code in the child (deferred) instancicng mode.
//  Made remaining changes for H11 (and beyond) versions way of handiling attributes.
//
//
//  --mstory
//
//  Revision 1.52  2011-02-06 22:35:15  mstory
//  Fixed the exit processing function.
//
//  Ready for release 1.5.1
//
//  Revision 1.51  2011-02-06 19:49:15  mstory
//  Modified for Houdini version 11.
//
//  Refactored a lot of the attribute code, cleaned up odds and ends.
//
//  Revision 1.50  2010-04-12 06:39:42  mstory
//  Finished CVEX modifications.
//
//  Revision 1.49  2010-04-10 10:11:42  mstory
//  Added additional CVEX processing.  Fixed a few annoying bugs.  Adding external disk geo source.
//
//  Revision 1.48  2010-02-23 08:36:22  mstory
//  Fixed most of the CVEX problems with primtive instancng.  Fixed seg faults from uninitilialized pointers in the CVEX variables,
//
//  Revision 1.47  2009-11-20 14:59:57  mstory
//  Release 1.4.7 ready.
//
//  Revision 1.46  2009-11-19 16:26:51  mstory
//  Adding point inst id to child objects (for deferred instancing), need to add to prims as well.
//
//  Revision 1.45  2009-11-16 17:47:12  mstory
//  Fixed the curve instancing, still need to determine all attribites required for the curve (i.e. width)
//
//  Revision 1.44  2009-11-16 08:32:44  mstory
//  Added instance ID for each instance passed to CVEX processing.
//
//  Revision 1.41  2009-04-06 17:13:44  mstory
//  Clean up code a bit.
//
//  Revision 1.40  2009-04-06 16:40:58  mstory
//  Added volume and curve instancing.
//  Optimized attribute processing.
//  Added motion blur pass for CVEX processing.
//  Changed parameter code to use proper functions.
//  Added verbosity switch for console messages.
//  Added randomness for when to instance of objects
//  Using SYSsin() and SYScos () instead of std C functions.
//  Optimized memory usage for CVEX processing, correct memory allocationfor attributes and objects.
//  Added user selectable attributes for CVEX processing.
//
//  --mstory
//
//  Revision 1.39  2009-02-11 04:17:53  mstory
//  Added velocity blur for point instancing.
//
//  Revision 1.38  2009-02-10 21:55:58  mstory
//  Added all attributes for the CVEX processing of instanced geo.
//  Added OTL version checking.
//
//  Revision 1.37  2009-02-05 00:59:05  mstory
//  Added simple CVEX processng.
//  Addded temp file for caching geo during deep shad passes.
//
//  Revision 1.36  2008-12-04 05:37:41  mstory
//  .
//
//  Revision 1.35  2008-11-27 05:32:39  mstory
//  Added Alpha attribute and fixed bug where it crashes mantra if the weight attr wasn't in the input geo.
//
//  Revision 1.34  2008-11-19 01:27:00  mstory
//  Added memory useage to status out.
//
//  Revision 1.33  2008-11-19 01:11:43  mstory
//  Added point instancing.  Fixed the file instancing problem.
//  Most of the shader assignment issues straightened out.
//
//  Revision 1.32  2008-10-30 19:51:54  mstory
//  Added file instancing (still needs work).
//
//  Revision 1.31  2008-10-30 07:03:06  mstory
//  Added deformation motion blur and metaball instancing.
//
//  Revision 1.30  2008-10-20 22:51:45  mstory
//  .
//
//  Revision 1.29  2008-10-20 22:43:57  mstory
//  *** empty log message ***
//
//  Revision 1.28  2008-10-20 22:12:14  mstory
//  Cleaned up unused vars, etc.  Ready for enxt release.
//
//  Revision 1.27  2008-10-20 19:35:00  mstory
//  Added a switch to be able to choose using the addProcedural() method of allocating procedurals.
//
//  Revision 1.25  2008-10-11 18:15:06  mstory
//  .
//
//  Revision 1.23  2008-10-06 21:58:40  mstory
//  .
//
//  Revision 1.21  2008-10-06 06:16:52  mstory
//  .
//
//  Revision 1.20  2008-10-06 04:20:04  mstory
//  Added the beginning of volume instancing, file instancing and almost have multi pass temp file working.
//
//  Revision 1.19  2008-10-04 23:33:40  mstory
//  .
//
//  Revision 1.18  2008-10-04 04:42:44  mstory
//  Added simple exception processing.
//
//  Revision 1.17  2008-10-04 04:40:33  mstory
//  .
//
//  Revision 1.16  2008-10-04 00:39:32  mstory
//  fixed pscale
//
//  Revision 1.15  2008-10-03 16:49:10  mstory
//  .
//
//  Revision 1.14  2008-10-03 00:01:00  mstory
//  Working out motion blur, material assignments, etc.; much more work to do ....
//
//  Revision 1.13  2008-10-01 22:18:42  mstory
//  Changed the "recursion algorythm" ... needs proper design, coming soon to a DSO near you!
//
//  Revision 1.12  2008-09-30 21:55:00  mstory
//  .
//
//  Revision 1.11  2008-09-30 21:37:38  mstory
//  Added a few more status messages to be printed to the console.
//
//  Revision 1.10  2008-09-21 17:46:43  mstory
//  Added recursion and aliging to normals.
//
//  Revision 1.3  2008-08-28 03:08:08  mstory
//  Lots of changes!!!
//
/**********************************************************************************/



