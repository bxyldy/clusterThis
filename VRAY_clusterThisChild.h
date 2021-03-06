/* ******************************************************************************
*
* clusterThisChild
*
* Digital Cinema Arts (C) 2008-2012
*
* This work is licensed under the Creative Commons Attribution-ShareAlike 2.5 License.
* To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/2.5/ or send a letter to
* Creative Commons, 543 Howard Street, 5th Floor, San Francisco, California, 94105, USA.
*
***************************************************************************** */

#ifndef __VRAY_clusterThisChild_h__
#define __VRAY_clusterThisChild_h__


/* ******************************************************************************
*  Class Name : VRAY_clusterThisChild()
*
*  Description :  Class for VRAY_clusterThisChild
*
*
*  Input Arguments : None
*
*  Return Value : None
*
***************************************************************************** */
class VRAY_clusterThisChild : public VRAY_Procedural {
public:
   VRAY_clusterThisChild(VRAY_clusterThis * theClusterObj)
      :
      myPrimType(theClusterObj->myPrimType),
      myRadius(theClusterObj->myRadius),
      myNumCopies(theClusterObj->myNumCopies),
      myRecursion(theClusterObj->myRecursion),
      myBirthProb(theClusterObj->myBirthProb),
      myDoMotionBlur(theClusterObj->myDoMotionBlur),
      myShutter(theClusterObj->myShutter),
      myInstanceNum(theClusterObj->myInstanceNum),
      myGeoFile(theClusterObj->myGeoFile),
      myParent(theClusterObj)

   {
#ifdef DEBUG
      std::cout << "VRAY_clusterThisChild::VRAY_clusterThisChild() - Constructor" << std::endl;
#endif

      myBox     = theClusterObj->myBox;
      bb_x1     = theClusterObj->bb_x1;
      bb_x2     = theClusterObj->bb_x2;
      bb_y1     = theClusterObj->bb_y1;
      bb_y2     = theClusterObj->bb_y2;
      bb_z1     = theClusterObj->bb_z1;
      bb_z2     = theClusterObj->bb_z2;

      mySize[0] = theClusterObj->mySize[0];
      mySize[1] = theClusterObj->mySize[1];
      mySize[2] = theClusterObj->mySize[2];

      myCVEXFname = theClusterObj->myCVEXFname;
      myCVEX_Exec = theClusterObj->myCVEX_Exec;
      myCVEXFname_prim = theClusterObj->myCVEXFname_prim;
      myCVEX_Exec_prim = theClusterObj->myCVEX_Exec_prim;
      myCVEXFname_pre = theClusterObj->myCVEXFname_pre;
      myCVEX_Exec_pre = theClusterObj->myCVEX_Exec_pre;
      myCVEXFname_post = theClusterObj->myCVEXFname_post;
      myCVEX_Exec_post = theClusterObj->myCVEX_Exec_post;

//    myPointAttributes.myPos = theClusterObj->myPointAttributes.myPos;
      myPointAttributes.myNewPos = theClusterObj->myPointAttributes.myNewPos;
      myPointAttributes.myMBPos = theClusterObj->myPointAttributes.myMBPos;
      myPointAttributes.Cd = theClusterObj->myPointAttributes.Cd;
      myPointAttributes.Alpha = theClusterObj->myPointAttributes.Alpha;
      myPointAttributes.v = theClusterObj->myPointAttributes.v;
      myPointAttributes.N = theClusterObj->myPointAttributes.N;
      myPointAttributes.id = theClusterObj->myPointAttributes.id;
      myPointAttributes.pscale = theClusterObj->myPointAttributes.pscale;
      myPointAttributes.width = theClusterObj->myPointAttributes.width;
      myPointAttributes.weight = theClusterObj->myPointAttributes.weight;
      myPointAttributes.material = theClusterObj->myPointAttributes.material;

//      myInstAttrRefs.Cd = theClusterObj->myInstAttrRefs.Cd;
//      myInstAttrRefs.Alpha = theClusterObj->myInstAttrRefs.Alpha;
//      myInstAttrRefs.v = theClusterObj->myInstAttrRefs.v;
//      myInstAttrRefs.N = theClusterObj->myInstAttrRefs.N;
//      myInstAttrRefs.pscale = theClusterObj->myInstAttrRefs.pscale;
//      myInstAttrRefs.id = theClusterObj->myInstAttrRefs.id;
//      myInstAttrRefs.width = theClusterObj->myInstAttrRefs.width;
//      myInstAttrRefs.weight = theClusterObj->myInstAttrRefs.weight;
//      myInstAttrRefs.material = theClusterObj->myInstAttrRefs.material;
//
//      myInstMBAttrOffsets.Cd = theClusterObj->myInstMBAttrOffsets.Cd;
//      myInstMBAttrOffsets.Alpha = theClusterObj->myInstMBAttrOffsets.Alpha;
//      myInstMBAttrOffsets.v = theClusterObj->myInstMBAttrOffsets.v;
//      myInstMBAttrOffsets.N = theClusterObj->myInstMBAttrOffsets.N;
//      myInstMBAttrOffsets.pscale = theClusterObj->myInstMBAttrOffsets.pscale;
//      myInstMBAttrOffsets.id = theClusterObj->myInstMBAttrOffsets.id;
//      myInstMBAttrOffsets.width = theClusterObj->myInstMBAttrOffsets.width;
//      myInstMBAttrOffsets.weight = theClusterObj->myInstMBAttrOffsets.weight;
//      myInstMBAttrOffsets.material = theClusterObj->myInstMBAttrOffsets.material;


//    dumpParameters();

      myLOD = 0;
   }

   virtual ~VRAY_clusterThisChild() {}

   virtual const char * getClassName();
   virtual int initialize(const UT_BoundingBox *);
   virtual void getBoundingBox(UT_BoundingBox & box);
   virtual void render();
   void calculateNewPosition(fpreal theta, uint32 i, uint32 j);

private:

   // Instancing methods
   int instancePoint();
   int instanceSphere();
   int instanceCube();
   int instanceGrid();
   int instanceTube();
   int instanceCircle();
   int instanceCurve();
   int instanceFile();
   int instanceMetaball();
   void setInstanceAttributes(GU_Detail * gdp, GEO_Primitive * myGeoPrim);
   void dumpParameters();

   // Point attribute structure
   struct attribute_struct {
      // Required attributes
//      UT_Vector4 myPos;
      UT_Vector4 myNewPos;
      UT_Vector4 myMBPos;
      UT_Vector3 Cd;
      fpreal Alpha;
      UT_Vector3 v;
      UT_Vector3 N;
      int id;
      fpreal pscale;
      fpreal width;
      fpreal weight;
      UT_String material;

      // Optional attributes
      //int num_neighbors;
      //UT_Vector3 force;
      //UT_Vector3 vorticity;
      //UT_Vector3 uv;
      //fpreal age;
      //fpreal viscosity;
      //fpreal density;
      //fpreal pressure;
      //fpreal mass;
      //fpreal temperature;
   } myPointAttributes;



//   struct inst_attr_offset_struct {
//
//      // Required attributes
//      GA_RWAttributeRef Cd;
//      GA_RWAttributeRef Alpha;
//      GA_RWAttributeRef v;
//      GA_RWAttributeRef N;
//      GA_RWAttributeRef material;
//      GA_RWAttributeRef id;
//      GA_RWAttributeRef inst_id;
//      GA_RWAttributeRef pscale;
//      GA_RWAttributeRef weight;
//      GA_RWAttributeRef width;
//
//      GA_RWAttributeRef pointCd;
//      GA_RWAttributeRef pointAlpha;
//      GA_RWAttributeRef pointV;
//      GA_RWAttributeRef pointN;
//      GA_RWAttributeRef pointMaterial;
//      GA_RWAttributeRef pointId;
//      GA_RWAttributeRef pointInstId;
//      GA_RWAttributeRef pointPscale;
//
//      // Optional attributes
//      /*        GA_RWAttributeRef num_neighbors;
//              GA_RWAttributeRef force;
//              GA_RWAttributeRef vorticity;
//              GA_RWAttributeRef uv;
//              GA_RWAttributeRef age;
//              GA_RWAttributeRef viscosity;
//              GA_RWAttributeRef density;
//              GA_RWAttributeRef pressure;
//              GA_RWAttributeRef mass;
//              GA_RWAttributeRef temperature;*/
//
//   } myInstAttrRefs;
//
//   struct inst_mb_attr_offset_struct {
//
//      // Required attributes
//      GA_RWAttributeRef Cd;
//      GA_RWAttributeRef Alpha;
//      GA_RWAttributeRef v;
//      GA_RWAttributeRef N;
//      GA_RWAttributeRef material;
//      GA_RWAttributeRef id;
//      GA_RWAttributeRef pscale;
//      GA_RWAttributeRef weight;
//      GA_RWAttributeRef width;
//
//      GA_RWAttributeRef pointCd;
//      GA_RWAttributeRef pointAlpha;
//      GA_RWAttributeRef pointV;
//      GA_RWAttributeRef pointN;
//      GA_RWAttributeRef pointMaterial;
//      GA_RWAttributeRef pointId;
//      GA_RWAttributeRef pointInstId;
//      GA_RWAttributeRef pointPscale;
//
//      // Optional attributes
//      /*        GA_RWAttributeRef num_neighbors;
//              GA_RWAttributeRef force;
//              GA_RWAttributeRef vorticity;
//              GA_RWAttributeRef uv;
//              GA_RWAttributeRef age;
//              GA_RWAttributeRef viscosity;
//              GA_RWAttributeRef density;
//              GA_RWAttributeRef pressure;
//              GA_RWAttributeRef mass;
//              GA_RWAttributeRef temperature;*/
//
//   } myInstMBAttrOffsets;
//

   VRAY_clusterThis * myParent;

   // Parameters
   int     myPrimType;
   fpreal  mySize[3];
   fpreal  myRadius;
   int     myDoMotionBlur;
   fpreal  myShutter;
   fpreal  myShutter2;
   UT_String myGeoFile;
   UT_String myObjectName;
   uint32   myNumCopies;
   uint32   myRecursion;
   fpreal   myBirthProb;
   UT_String myCVEXFname;
   bool     myCVEX_Exec;
   UT_String myCVEXFname_prim;
   bool     myCVEX_Exec_prim;
   UT_String myCVEXFname_pre;
   bool     myCVEX_Exec_pre;
   UT_String myCVEXFname_post;
   bool     myCVEX_Exec_post;

   // Other vars ...
//   UT_String myMaterial;
//   fpreal myTheta;

   UT_BoundingBox myBox;
   fpreal bb_x1, bb_x2, bb_y1, bb_y2, bb_z1, bb_z2;

//   GU_Detail * myFileGDP;
   fpreal myLOD;
   long int myInstanceNum;

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

   enum clusterCVEXMethod {
      CLUSTER_CVEX_POINT = 0,
      CLUSTER_CVEX_PRIM
   };

};



#endif


/**********************************************************************************/
//  $Log:
//
/**********************************************************************************/


