/* ******************************************************************************
*
*  clusterThis mantra DSO for render time geo clustering
*
* $RCSfile: VRAY_clusterThis.h,v $
*
* Description : This DSO  will instance geomtery during the render of mantra IFD's
*
* $Revision: 1.40 $
*
* $Source: /dca/cvsroot/houdini/VRAY_clusterThis/VRAY_clusterThis.h,v $
*
* $Author: mstory $
*
*
* See Change History at the end of the file.
*
*    Digital Cinema Arts (C) 2008
*
* This work is licensed under the Creative Commons Attribution-ShareAlike 2.5 License.
* To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/2.5/ or send a letter to
* Creative Commons, 543 Howard Street, 5th Floor, San Francisco, California, 94105, USA.
*
***************************************************************************** */

#ifndef __VRAY_clusterThis_h__
#define __VRAY_clusterThis_h__


//#define DEBUG


/* ******************************************************************************
*  Class Name : VRAY_clusterThis_Exception()
*
*  Description :  Exception class for VRAY_clusterThis mantra DSO
*
*
*  Input Arguments : None
*
*  Return Value : None
*
***************************************************************************** */
class VRAY_clusterThis_Exception {
   std::string e_msg;
   int e_code;

public:
   VRAY_clusterThis_Exception(std::string msg, int code);
//   ~VRAY_clusterThis_Exception();

   void what() {
      std::cout << "VRAY_clusterThis_Exception::what() - VRAY_clusterThis exception:  " << e_msg << endl;
   }

};


/* ******************************************************************************
*  Class Name : VRAY_clusterThis()
*
*  Description :  Class for VRAY_clusterThis mantra DSO
*
*
*  Input Arguments : None
*
*  Return Value : None
*
***************************************************************************** */
class VRAY_clusterThis : public VRAY_Procedural {
public:
   VRAY_clusterThis();
   virtual ~VRAY_clusterThis();

   virtual const char * getClassName();
   virtual int initialize(const UT_BoundingBox *);
   virtual void getBoundingBox(UT_BoundingBox & box);
   virtual bool hasVolume() {
      return true;
   }
   virtual void render();

   static void exitClusterThis(void * data);
   void exitClusterThisReal(const char * fname);
   struct exitData_struct {
      float exitTime;
      int exitCode;
   } exitData;


private:

   struct pt_attr_offset_struct {

      // Required attributes
      GA_RWAttributeRef Cd;
      GA_RWAttributeRef Alpha;
      GA_RWAttributeRef v;
      GA_RWAttributeRef N;
      GA_RWAttributeRef up;
      GA_RWAttributeRef orient;
      GA_RWAttributeRef material;
      GA_RWAttributeRef id;
      GA_RWAttributeRef pscale;
      GA_RWAttributeRef weight;
      GA_RWAttributeRef width;
      GA_RWAttributeRef prim_type;
      GA_RWAttributeRef geo_fname;

      // Optional attributes
      /*        GA_RWAttributeRef num_neighbors;
              GA_RWAttributeRef force;
              GA_RWAttributeRef vorticity;
              GA_RWAttributeRef uv;
              GA_RWAttributeRef age;
              GA_RWAttributeRef viscosity;
              GA_RWAttributeRef density;
              GA_RWAttributeRef pressure;
              GA_RWAttributeRef mass;
              GA_RWAttributeRef temperature;*/

   } myPointAttrOffsets;


   struct file_attr_offset_struct {

      // Required attributes
      GA_RWAttributeRef Cd;
      GA_RWAttributeRef Alpha;
      GA_RWAttributeRef v;
      GA_RWAttributeRef N;
      GA_RWAttributeRef orient;
      GA_RWAttributeRef material;
      GA_RWAttributeRef id;
      GA_RWAttributeRef lod;
      GA_RWAttributeRef pscale;
      GA_RWAttributeRef up;
      GA_RWAttributeRef angle;
      GA_RWAttributeRef offset;
      GA_RWAttributeRef amp;

      GA_RWAttributeRef low_bound;
      GA_RWAttributeRef high_bound;
      GA_RWAttributeRef deformspace;
      GA_RWAttributeRef xformobj;

      GA_RWAttributeRef pointCd;
      GA_RWAttributeRef pointAlpha;
      GA_RWAttributeRef pointV;
      GA_RWAttributeRef pointN;
      GA_RWAttributeRef pointMaterial;
      GA_RWAttributeRef pointPscale;
      GA_RWAttributeRef pointId;
      GA_RWAttributeRef pointLOD;
      GA_RWAttributeRef pointUp;
      GA_RWAttributeRef pointAngle;
      GA_RWAttributeRef pointOffset;
      GA_RWAttributeRef pointAmp;

   } myFileAttrOffsets;


   struct inst_attr_offset_struct {

      // Required attributes
      GA_RWAttributeRef Cd;
      GA_RWAttributeRef Alpha;
      GA_RWAttributeRef v;
      GA_RWAttributeRef N;
      GA_RWAttributeRef orient;
      GA_RWAttributeRef material;
      GA_RWAttributeRef id;
      GA_RWAttributeRef inst_id;
      GA_RWAttributeRef pscale;
      GA_RWAttributeRef weight;
      GA_RWAttributeRef width;

      GA_RWAttributeRef pointCd;
      GA_RWAttributeRef pointAlpha;
      GA_RWAttributeRef pointV;
      GA_RWAttributeRef pointN;
      GA_RWAttributeRef pointMaterial;
      GA_RWAttributeRef pointId;
      GA_RWAttributeRef pointInstId;
      GA_RWAttributeRef pointPscale;

      // Optional attributes
      //GA_RWAttributeRef num_neighbors;
      //GA_RWAttributeRef force;
      //GA_RWAttributeRef vorticity;
      //GA_RWAttributeRef uv;
      //GA_RWAttributeRef age;
      //GA_RWAttributeRef viscosity;
      //GA_RWAttributeRef density;
      //GA_RWAttributeRef pressure;
      //GA_RWAttributeRef mass;
      //GA_RWAttributeRef temperature;

   } myInstAttrRefs;



   struct inst_mb_attr_offset_struct {

      // Required attributes
      GA_RWAttributeRef Cd;
      GA_RWAttributeRef Alpha;
      GA_RWAttributeRef v;
      GA_RWAttributeRef N;
      GA_RWAttributeRef orient;
      GA_RWAttributeRef material;
      GA_RWAttributeRef id;
      GA_RWAttributeRef inst_id;
      GA_RWAttributeRef pscale;
      GA_RWAttributeRef weight;
      GA_RWAttributeRef width;

      GA_RWAttributeRef pointCd;
      GA_RWAttributeRef pointAlpha;
      GA_RWAttributeRef pointV;
      GA_RWAttributeRef pointN;
      GA_RWAttributeRef pointMaterial;
      GA_RWAttributeRef pointId;
      GA_RWAttributeRef pointInstId;
      GA_RWAttributeRef pointPscale;

      // Optional attributes
      //GA_RWAttributeRef num_neighbors;
      //GA_RWAttributeRef force;
      //GA_RWAttributeRef vorticity;
      //GA_RWAttributeRef uv;
      //GA_RWAttributeRef age;
      //GA_RWAttributeRef viscosity;
      //GA_RWAttributeRef density;
      //GA_RWAttributeRef pressure;
      //GA_RWAttributeRef mass;
      //GA_RWAttributeRef temperature;

   } myInstMBAttrOffsets;


   struct pt_attr_struct {
      // Required attributes
      UT_Vector4 myPos;
      UT_Vector4 myNewPos;
      UT_Vector4 myMBPos;

      UT_Vector3 Cd;
      fpreal Alpha;
      UT_Vector3 v;
      UT_Vector3 N;
      UT_Vector4 orient;
      uint32 id;
      fpreal pscale;
      fpreal weight;
      fpreal width;
      UT_String material;

//         fpreal theta;

      // Optional attributes
      /*        uint32 num_neighbors;
              UT_Vector3 force;
              UT_Vector3 vorticity;
              UT_Vector3 uv;
              fpreal age;
              fpreal viscosity;
              fpreal density;
              fpreal pressure;
              fpreal mass;
              fpreal temperature;*/
   } myPointAttributes;


   void calculateNewPosition(fpreal theta, uint32 i, uint32 j);
   void dumpParameters();
   int preLoadGeoFile(GU_Detail * file_gdp);
   void createAttributeOffsets(GU_Detail * inst_gdp, GU_Detail * mb_gdp);
   int getAttributeOffsets(GU_Detail * gdp);
   int getAttributes(GEO_Point * ppt, GU_Detail * gdp);
   int addFileAttributeOffsets(GU_Detail * gdp);
   void setInstanceAttributes(GU_Detail * gdp, GEO_Primitive * myGeoPrim);
   void setPointInstanceAttributes(GU_Detail * gdp, GEO_Point * ppt);
   int setFileAttributes(GU_Detail * gdp);
   int runCVEX(GU_Detail * inst_gdp, GU_Detail * mb_gdp, UT_String theCVEXFname, uint method);


   // Instancing methods
   int instancePoint(GU_Detail * inst_gdp, GU_Detail * mb_gdp);
   int instanceSphere(GU_Detail * inst_gdp, GU_Detail * mb_gdp);
   int instanceCube(GU_Detail * inst_gdp, GU_Detail * mb_gdp);
   int instanceGrid(GU_Detail * inst_gdp, GU_Detail * mb_gdp);
   int instanceTube(GU_Detail * inst_gdp, GU_Detail * mb_gdp);
   int instanceCircle(GU_Detail * inst_gdp, GU_Detail * mb_gdp);
   int instanceCurve(GU_Detail * inst_gdp, GU_Detail * mb_gdp, fpreal theta);
   int instanceMetaball(GU_Detail * inst_gdp, GU_Detail * mb_gdp);
   int instanceFile(GU_Detail * file_gdp, GU_Detail * inst_gdp, GU_Detail * mb_gdp);

   UT_BoundingBox myBox;
   fpreal bb_x1, bb_x2, bb_y1, bb_y2, bb_z1, bb_z2;
   UT_String myMaterial;

   // Parameters
   uint32   myNumCopies;
   bool     myUseGeoFile;
   UT_String mySrcGeoFname;
   uint32   myPrimType;
   uint32   myMethod;
   fpreal   mySize[3];
   uint32   myRecursion;
   fpreal   myFreqX, myFreqY, myFreqZ;
   fpreal   myOffsetX, myOffsetY, myOffsetZ;
   fpreal   myRadius;
   fpreal   myBirthProb;
   uint32   myDoMotionBlur;
   fpreal   myShutter;
   UT_Noise::UT_NoiseType  myNoiseType;
   fpreal   myRough;
   fpreal   myCurrentTime;
   fpreal   myNoiseAmp;
   fpreal   myNoiseAtten;
   UT_Vector3 myNoiseVec;
   UT_Noise myNoise;
   int      myNoiseSeed;
   int      myFractalDepth;
   bool     myCopyAttrs;
   bool     myBlendAttrs;
   uint32   myFilterType;
   fpreal   myFilterAmp;
   UT_String myGeoFile;
   UT_String myTempFname;
   bool      myUseTempFile;
   bool      mySaveTempFile;
   UT_String myCVEXFname;
   bool     myCVEX_Exec;
   UT_String myCVEXFname_prim;
   bool     myCVEX_Exec_prim;
   UT_String myCVEXFname_pre;
   bool     myCVEX_Exec_pre;
   UT_String myCVEXFname_post;
   bool     myCVEX_Exec_post;
   int      myVerbose;
   UT_String myObjectName;
   UT_String myOTLVersion;

   // A struct to keep track os CVEX vars to pass to the CVEX code
   struct cvex_pt_vars_struct {
   uint  cvex_Cd_pt:
      1;
   uint  cvex_Alpha_pt:
      1;
   uint  cvex_v_pt:
      1;
   uint  cvex_N_pt:
      1;
   uint  cvex_pscale_pt:
      1;
   } myCVEXPointVars;

   struct cvex_prim_vars_struct {
   uint  cvex_Cd_prim:
      1;
   uint  cvex_Alpha_prim:
      1;
   uint  cvex_v_prim:
      1;
   uint  cvex_N_prim:
      1;
   uint  cvex_pscale_prim:
      1;
   uint  cvex_weight_prim:
      1;
   uint  cvex_width_prim:
      1;
   } myCVEXPrimVars;

   fpreal   myVelocityScale;
   long int myInstanceNum;

   GU_Detail * myFileGDP;

   friend class VRAY_clusterThisChild;

   enum clusterPrimTypeEnum {
      CLUSTER_POINT = 0,
      CLUSTER_PRIM_SPHERE,
      CLUSTER_PRIM_CUBE,
      CLUSTER_PRIM_GRID,
      CLUSTER_PRIM_TUBE,
      CLUSTER_PRIM_CIRCLE,
      CLUSTER_PRIM_CURVE,
      CLUSTER_PRIM_METABALL,
      CLUSTER_FILE
   };

   enum clusterMotionBlurTypeEnum {
      CLUSTER_MB_NONE = 0,
      CLUSTER_MB_VELOCITY,
      CLUSTER_MB_DEFORMATION
   };

   enum clusterVerboseTypeEnum {
      CLUSTER_MSG_QUIET = 0,
      CLUSTER_MSG_INFO,
      CLUSTER_MSG_VERBOSE
   };

   enum clusterInstanceMethod {
      CLUSTER_INSTANCE_NOW = 0,
      CLUSTER_INSTANCE_DEFERRED
   };

   enum clusterCVEXMethod {
      CLUSTER_CVEX_POINT = 0,
      CLUSTER_CVEX_PRIM
   };

};



#endif


/**********************************************************************************/
//  $Log: VRAY_clusterThis.h,v $
//  Revision 1.40  2012-09-09 05:00:54  mstory
//  More cleanup and testing.
//
//  Revision 1.39  2012-09-07 15:39:22  mstory
//   Removed all volume instancing (used in different project) and continu… …
//
//  …ed H12 modifications.
//
//  --mstory
//
//  Revision 1.38  2012-09-05 23:02:38  mstory
//  Modifications for H12.
//
//  Revision 1.37  2012-09-04 03:25:28  mstory
//  .
//
//  Revision 1.34  2011-02-15 00:59:15  mstory
//  Refactored out rededundant attribute code in the child (deferred) instancicng mode.
//  Made remaining changes for H11 (and beyond) versions way of handiling attributes.
//
//
//  --mstory
//
//  Revision 1.33  2011-02-06 19:49:15  mstory
//  Modified for Houdini version 11.
//
//  Refactored a lot of the attribute code, cleaned up odds and ends.
//
//  Revision 1.32  2010-04-12 06:39:42  mstory
//  Finished CVEX modifications.
//
//  Revision 1.31  2010-04-10 10:11:42  mstory
//  Added additional CVEX processing.  Fixed a few annoying bugs.  Adding external disk geo source.
//
//  Revision 1.30  2010-02-23 08:36:22  mstory
//  Fixed most of the CVEX problems with primtive instancng.  Fixed seg faults from uninitilialized pointers in the CVEX variables,
//
//  Revision 1.29  2009-11-20 14:59:57  mstory
//  Release 1.4.7 ready.
//
//  Revision 1.28  2009-11-19 16:26:51  mstory
//  Adding point inst id to child objects (for deferred instancing), need to add to prims as well.
//
//  Revision 1.27  2009-11-16 17:47:12  mstory
//  Fixed the curve instancing, still need to determine all attribites required for the curve (i.e. width)
//
//  Revision 1.26  2009-11-16 08:32:45  mstory
//  Added instance ID for each instance passed to CVEX processing.
//
//  Revision 1.25  2009-04-06 16:40:58  mstory
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
//  Revision 1.24  2009-02-10 21:55:59  mstory
//  Added all attributes for the CVEX processing of instanced geo.
//  Added OTL version checking.
//
//  Revision 1.23  2009-02-05 00:59:05  mstory
//  Added simple CVEX processng.
//  Addded temp file for caching geo during deep shad passes.
//
//  Revision 1.22  2008-12-04 05:37:41  mstory
//  .
//
//  Revision 1.21  2008-11-27 05:32:39  mstory
//  Added Alpha attribute and fixed bug where it crashes mantra if the weight attr wasn't in the input geo.
//
//  Revision 1.20  2008-11-19 01:11:43  mstory
//  Added point instancing.  Fixed the file instancing problem.
//  Most of the shader assignment issues straightened out.
//
//  Revision 1.19  2008-10-30 19:51:54  mstory
//  Added file instancing (still needs work).
//
//  Revision 1.18  2008-10-30 07:03:06  mstory
//  Added deformation motion blur and metaball instancing.
//
//  Revision 1.17  2008-10-20 22:12:14  mstory
//  Cleaned up unused vars, etc.  Ready for enxt release.
//
//  Revision 1.16  2008-10-20 19:35:00  mstory
//  Added a switch to be able to choose using the addProcedural() method of allocating procedurals.
//
//  Revision 1.14  2008-10-11 18:15:06  mstory
//  .
//
//  Revision 1.12  2008-10-06 21:58:40  mstory
//  .
//
//  Revision 1.11  2008-10-06 04:20:05  mstory
//  Added the beginning of volume instancing, file instancing and almost have multi pass temp file working.
//
//  Revision 1.10  2008-10-04 04:42:44  mstory
//  Added simple exception processing.
//
//  Revision 1.9  2008-10-04 04:40:34  mstory
//  .
//
//  Revision 1.8  2008-10-03 00:01:00  mstory
//  Working out motion blur, material assignments, etc.; much more work to do ....
//
//  Revision 1.7  2008-10-01 22:18:43  mstory
//  Changed the "recursion algorythm" ... needs proper design, coming soon to a DSO near you!
//
//  Revision 1.2  2008-08-28 03:08:08  mstory
//  Lots of changes!!!
//
/**********************************************************************************/


