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
class VRAY_clusterThisChild : public VRAY_Procedural
{
   public:
      VRAY_clusterThisChild(VRAY_clusterThis * theClusterObj, UT_BoundingBox & box)
         :
         myPrimType(theClusterObj->myPrimType),
         myRadius(theClusterObj->myRadius),
         myNumCopies(theClusterObj->myNumCopies),
         myRecursion(theClusterObj->myRecursion),
         myBirthProb(theClusterObj->myBirthProb),
         myDoMotionBlur(theClusterObj->myDoMotionBlur),
         myShutter(theClusterObj->myShutter),
         myShutter2(theClusterObj->myShutter2),
         myInstanceNum(theClusterObj->myInstanceNum),
         myGeoFile(theClusterObj->myGeoFile),

         myGdp(theClusterObj->myGdp),
         myPointList(theClusterObj->myPointList),
         myParent(theClusterObj)

      {
#ifdef DEBUG
         std::cout << "VRAY_clusterThisChild::VRAY_clusterThisChild() - Constructor" << std::endl;
#endif

//         std::cout << "VRAY_clusterThisChild::VRAY_clusterThisChild() - box: " << box << std::endl;

//         myBox     = theClusterObj->myBox;
         myBox     = box;

         mySize[0] = theClusterObj->mySize[0];
         mySize[1] = theClusterObj->mySize[1];
         mySize[2] = theClusterObj->mySize[2];

         myFreqX = theClusterObj->myFreqX;
         myFreqY = theClusterObj->myFreqY;
         myFreqZ = theClusterObj->myFreqZ;
         myOffsetX = theClusterObj->myOffsetX;
         myOffsetY = theClusterObj->myOffsetY;
         myOffsetZ = theClusterObj->myOffsetZ;

         myNoiseType = theClusterObj->myNoiseType;
         myFractalDepth = theClusterObj->myFractalDepth;
         myRough = theClusterObj->myRough;
         myNoiseAtten = theClusterObj->myNoiseAtten;
         myNoiseAmp = theClusterObj->myNoiseAmp;

         myCVEXFname = theClusterObj->myCVEXFname;
         myCVEX_Exec = theClusterObj->myCVEX_Exec;
         myCVEXFname_prim = theClusterObj->myCVEXFname_prim;
         myCVEX_Exec_prim = theClusterObj->myCVEX_Exec_prim;
         myCVEXFname_pre = theClusterObj->myCVEXFname_pre;
         myCVEX_Exec_pre = theClusterObj->myCVEX_Exec_pre;
         myCVEXFname_post = theClusterObj->myCVEXFname_post;
         myCVEX_Exec_post = theClusterObj->myCVEX_Exec_post;


         myParentPointAttrRefs.Cd = theClusterObj->myPointAttrRefs.Cd;
         myParentPointAttrRefs.Alpha = theClusterObj->myPointAttrRefs.Alpha;
         myParentPointAttrRefs.v = theClusterObj->myPointAttrRefs.v;
         myParentPointAttrRefs.N = theClusterObj->myPointAttrRefs.N;
         myParentPointAttrRefs.pscale = theClusterObj->myPointAttrRefs.pscale;
         myParentPointAttrRefs.id = theClusterObj->myPointAttrRefs.id;



//         myInstAttrRefs.Cd = theClusterObj->myInstAttrRefs.Cd;
//         myInstAttrRefs.Alpha = theClusterObj->myInstAttrRefs.Alpha;
//         myInstAttrRefs.v = theClusterObj->myInstAttrRefs.v;
//         myInstAttrRefs.N = theClusterObj->myInstAttrRefs.N;
//         myInstAttrRefs.pscale = theClusterObj->myInstAttrRefs.pscale;
//         myInstAttrRefs.id = theClusterObj->myInstAttrRefs.id;
//         myInstAttrRefs.width = theClusterObj->myInstAttrRefs.width;
//         myInstAttrRefs.weight = theClusterObj->myInstAttrRefs.weight;
//         myInstAttrRefs.material = theClusterObj->myInstAttrRefs.material;
//
//         myInstMBAttrRefs.Cd = theClusterObj->myInstMBAttrRefs.Cd;
//         myInstMBAttrRefs.Alpha = theClusterObj->myInstMBAttrRefs.Alpha;
//         myInstMBAttrRefs.v = theClusterObj->myInstMBAttrRefs.v;
//         myInstMBAttrRefs.N = theClusterObj->myInstMBAttrRefs.N;
//         myInstMBAttrRefs.pscale = theClusterObj->myInstMBAttrRefs.pscale;
//         myInstMBAttrRefs.id = theClusterObj->myInstMBAttrRefs.id;
//         myInstMBAttrRefs.width = theClusterObj->myInstMBAttrRefs.width;
//         myInstMBAttrRefs.weight = theClusterObj->myInstMBAttrRefs.weight;
//         myInstMBAttrRefs.material = theClusterObj->myInstMBAttrRefs.material;


//         VRAY_clusterThisChild::dumpParameters();

         myLOD = 0;
      }

      virtual ~VRAY_clusterThisChild() {}

      virtual const char * getClassName();
      virtual int initialize(const UT_BoundingBox *);
      virtual void getBoundingBox(UT_BoundingBox & box);
      virtual void render();
      void initChild(const UT_BoundingBox & box);

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

      void createAttributeRefs(GU_Detail * inst_gdp, GU_Detail * mb_gdp);
      void setInstanceAttributes(GU_Detail * gdp, GEO_Primitive * myGeoPrim);
      int getAttributes(GEO_Point * ppt);
      void calculateNewPosition(fpreal theta, uint32 i, uint32 j);
      void dumpParameters();

      // Point attribute structure
      struct attribute_struct {
         // Required attributes
         UT_Vector4 myPos;
         UT_Vector4 myNewPos;
         UT_Vector4 myMBPos;
         UT_Vector3 Cd;
         fpreal Alpha;
         UT_Vector3 v;
         UT_Vector3 N;
         UT_Vector4 backtrack;
         UT_Vector3 up;
//         UT_Vector4 orient;
         fpreal radius;
         fpreal vdb_radius;
         UT_String geo_fname;
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


      struct pt_attr_ref_struct {

         // Required attributes
         GA_RWAttributeRef Cd;
         GA_RWAttributeRef Alpha;
         GA_RWAttributeRef v;
         GA_RWAttributeRef backtrack;
         GA_RWAttributeRef N;
         GA_RWAttributeRef up;
//         GA_RWAttributeRef orient;
         GA_RWAttributeRef material;
         GA_RWAttributeRef id;
         GA_RWAttributeRef pscale;
         GA_RWAttributeRef radius;
         GA_RWAttributeRef weight;
         GA_RWAttributeRef width;
         GA_RWAttributeRef vdb_radius;
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

      } myParentPointAttrRefs;


      struct inst_attr_ref_struct {

         // Required attributes
         GA_RWAttributeRef Cd;
         GA_RWAttributeRef Alpha;
         GA_RWAttributeRef v;
         GA_RWAttributeRef backtrack;
         GA_RWAttributeRef N;
         GA_RWAttributeRef up;
         GA_RWAttributeRef material;
         GA_RWAttributeRef id;
         GA_RWAttributeRef vdb_radius;
         GA_RWAttributeRef inst_id;
         GA_RWAttributeRef pscale;
         GA_RWAttributeRef radius;
         GA_RWAttributeRef weight;
         GA_RWAttributeRef width;
         GA_RWAttributeRef geo_fname;

         GA_RWAttributeRef pointCd;
         GA_RWAttributeRef pointAlpha;
         GA_RWAttributeRef pointV;
         GA_RWAttributeRef pointN;
         GA_RWAttributeRef pointMaterial;
         GA_RWAttributeRef pointId;
         GA_RWAttributeRef pointInstId;
         GA_RWAttributeRef pointPscale;
         GA_RWAttributeRef pointWeight;
         GA_RWAttributeRef pointWidth;
         GA_RWAttributeRef pointRadius;
         GA_RWAttributeRef pointVDBRadius;

//
//         // Optional attributes
//         GA_RWAttributeRef num_neighbors;
//         GA_RWAttributeRef force;
//         GA_RWAttributeRef vorticity;
//         GA_RWAttributeRef uv;
//         GA_RWAttributeRef age;
//         GA_RWAttributeRef viscosity;
//         GA_RWAttributeRef density;
//         GA_RWAttributeRef pressure;
//         GA_RWAttributeRef mass;
//         GA_RWAttributeRef temperature;

      } myInstAttrRefs;

      struct inst_mb_attr_ref_struct {

         // Required attributes
         GA_RWAttributeRef Cd;
         GA_RWAttributeRef Alpha;
         GA_RWAttributeRef v;
         GA_RWAttributeRef backtrack;
         GA_RWAttributeRef N;
         GA_RWAttributeRef up;
         GA_RWAttributeRef material;
         GA_RWAttributeRef id;
         GA_RWAttributeRef vdb_radius;
         GA_RWAttributeRef inst_id;
         GA_RWAttributeRef pscale;
         GA_RWAttributeRef radius;
         GA_RWAttributeRef weight;
         GA_RWAttributeRef width;
         GA_RWAttributeRef geo_fname;


         GA_RWAttributeRef pointCd;
         GA_RWAttributeRef pointAlpha;
         GA_RWAttributeRef pointV;
         GA_RWAttributeRef pointN;
         GA_RWAttributeRef pointMaterial;
         GA_RWAttributeRef pointId;
         GA_RWAttributeRef pointInstId;
         GA_RWAttributeRef pointPscale;
         GA_RWAttributeRef pointWeight;
         GA_RWAttributeRef pointWidth;
         GA_RWAttributeRef pointRadius;
         GA_RWAttributeRef pointVDBRadius;

//         // Optional attributes
//         GA_RWAttributeRef num_neighbors;
//         GA_RWAttributeRef force;
//         GA_RWAttributeRef vorticity;
//         GA_RWAttributeRef uv;
//         GA_RWAttributeRef age;
//         GA_RWAttributeRef viscosity;
//         GA_RWAttributeRef density;
//         GA_RWAttributeRef pressure;
//         GA_RWAttributeRef mass;
//         GA_RWAttributeRef temperature;

      } myInstMBAttrRefs;



      // Parameters
      int     myPrimType;
      fpreal  mySize[3];
      fpreal  myRadius;
      int     myDoMotionBlur;
      fpreal  myShutter;
      fpreal  myShutter2;
      UT_String myGeoFile;
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


      // Member variables
      VRAY_clusterThis * myParent;
      GU_Detail * myGdp;
      UT_BoundingBox myBox;
      UT_BoundingBox myVelBox;
      fpreal   myFreqX, myFreqY, myFreqZ;
      fpreal   myOffsetX, myOffsetY, myOffsetZ;
      int      myNoiseType;
      fpreal  myFractalDepth;
      fpreal  myRough;
      fpreal  myNoiseAtten;
      fpreal  myNoiseAmp;

      UT_String myMaterial;
      UT_Matrix3 myXformInverse;
      UT_Noise myNoise;
      UT_MersenneTwister myMersenneTwister;
      bool myUsePointRadius;
//      bool myUsePointGeoFname;
      bool myUseBacktrackMB;
      UT_IntArray myPointList;
      UT_String myObjectName;
//      UT_String myOTLVersion;
      fpreal   myVelocityScale;
      long int myInstanceNum;
      fpreal   myLOD;
      static const fpreal myFPS = 24.0;
      GU_Detail * myFileGDP;
//   UT_String myMaterial;
//   fpreal myTheta;

//      openvdb::ScalarGrid::Ptr myGeoGrid;
//      openvdb::VectorGrid::Ptr myGradientGrid;



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



