/* ******************************************************************************
*
*  clusterThisChild
*
* $RCSfile: VRAY_clusterThisChild.h,v $
*
* Description : This class will instance geomtery from the addProcedural calls of it's "parent" class, VRAY_clusterThis
*
* $Revision: 1.21 $
*
* $Source: /dca/cvsroot/houdini/VRAY_clusterThis/VRAY_clusterThisChild.h,v $
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
     VRAY_clusterThisChild(VRAY_clusterThis *theClusterObj)
          :
          myPrimType(theClusterObj->myPrimType),
          myRadius(theClusterObj->myRadius),
          myNumCopies(theClusterObj->myNumCopies),
          myRecursion(theClusterObj->myRecursion),
          myBirthProb(theClusterObj->myBirthProb),
          myDoMotionBlur(theClusterObj->myDoMotionBlur),
          myShutter(theClusterObj->myShutter),
          myInstanceNum(theClusterObj->myInstanceNum),
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

          myFileGDP = theClusterObj->myFileGDP;

//    myPointAttributes.myPos = theClusterObj->myPointAttributes.myPos;
          myPointAttributes.myNewPos = theClusterObj->myPointAttributes.myNewPos;
          myPointAttributes.myMBPos = theClusterObj->myPointAttributes.myMBPos;
          myPointAttributes.Cd = theClusterObj->myPointAttributes.Cd;
          myPointAttributes.Alpha = theClusterObj->myPointAttributes.Alpha;
          myPointAttributes.v = theClusterObj->myPointAttributes.v;
          myPointAttributes.N = theClusterObj->myPointAttributes.N;
          myPointAttributes.id = theClusterObj->myPointAttributes.id;
          myPointAttributes.pscale = theClusterObj->myPointAttributes.pscale;
          myPointAttributes.weight = theClusterObj->myPointAttributes.weight;
          myPointAttributes.material = theClusterObj->myPointAttributes.material;

//    myPointAttributes.theta = theClusterObj->myPointAttributes.theta;

          myInstAttrOffsets.Cd = theClusterObj->myInstAttrOffsets.Cd;
          myInstAttrOffsets.Alpha = theClusterObj->myInstAttrOffsets.Alpha;
          myInstAttrOffsets.v = theClusterObj->myInstAttrOffsets.v;
          myInstAttrOffsets.N = theClusterObj->myInstAttrOffsets.N;
          myInstAttrOffsets.pscale = theClusterObj->myInstAttrOffsets.pscale;
          myInstAttrOffsets.id = theClusterObj->myInstAttrOffsets.id;
          myInstAttrOffsets.weight = theClusterObj->myInstAttrOffsets.weight;
          myInstAttrOffsets.material = theClusterObj->myInstAttrOffsets.material;

          myInstMBAttrOffsets.Cd = theClusterObj->myInstMBAttrOffsets.Cd;
          myInstMBAttrOffsets.Alpha = theClusterObj->myInstMBAttrOffsets.Alpha;
          myInstMBAttrOffsets.v = theClusterObj->myInstMBAttrOffsets.v;
          myInstMBAttrOffsets.N = theClusterObj->myInstMBAttrOffsets.N;
          myInstMBAttrOffsets.pscale = theClusterObj->myInstMBAttrOffsets.pscale;
          myInstMBAttrOffsets.id = theClusterObj->myInstMBAttrOffsets.id;
          myInstMBAttrOffsets.weight = theClusterObj->myInstMBAttrOffsets.weight;
          myInstMBAttrOffsets.material = theClusterObj->myInstMBAttrOffsets.material;

#ifdef DEBUG
          cout << "VRAY_clusterThisChild::VRAY_clusterThisChild() myPointAttributes.material: " << myPointAttributes.material << endl;
          cout << "VRAY_clusterThisChild::VRAY_clusterThisChild() myPointAttributes.id: " << myPointAttributes.id << endl;
          cout << "VRAY_clusterThisChild::VRAY_clusterThisChild() myPointAttributes.pscale: " << myPointAttributes.pscale << endl;
          cout << "VRAY_clusterThisChild::VRAY_clusterThisChild() mySize: " << mySize << endl;
#endif

//    dumpParameters();

          myLOD = 0;

     }

     virtual ~VRAY_clusterThisChild() {}

     virtual const char  *getClassName();
     virtual int      initialize(const UT_BoundingBox *);
     virtual void  getBoundingBox(UT_BoundingBox &box);
     virtual void  render();
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
     void setInstanceAttributes(GU_Detail *gdp, GEO_Primitive *myGeoPrim);
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
          fpreal weight;
//        UT_String shader;
          UT_String material;

//      fpreal theta;

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



     struct inst_attr_offset_struct {

#if HOUDINI_MAJOR_RELEASE>=11
          // Required attributes
          GA_RWAttributeRef Cd;
          GA_RWAttributeRef Alpha;
          GA_RWAttributeRef v;
          GA_RWAttributeRef N;
          GA_RWAttributeRef material;
          GA_RWAttributeRef id;
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
#endif

#if HOUDINI_MAJOR_RELEASE==9
          // Required attributes
          int Cd;
          int Alpha;
          int v;
          int N;
          int material;
          int id;
          int pscale;
          int weight;
          int width;

          int pointCd;
          int pointAlpha;
          int pointV;
          int pointN;
          int pointMaterial;
          int pointId;
          int pointInstId;
          int pointPscale;

          // Optional attributes
          /*        int num_neighbors;
                  int force;
                  int vorticity;
                  int uv;
                  int age;
                  int viscosity;
                  int density;
                  int pressure;
                  int mass;
                  int temperature;*/
#endif
     } myInstAttrOffsets;

     struct inst_mb_attr_offset_struct {

#if HOUDINI_MAJOR_RELEASE>=11
          // Required attributes
          GA_RWAttributeRef Cd;
          GA_RWAttributeRef Alpha;
          GA_RWAttributeRef v;
          GA_RWAttributeRef N;
          GA_RWAttributeRef material;
          GA_RWAttributeRef id;
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
#endif

#if HOUDINI_MAJOR_RELEASE==9
          // Required attributes
          int Cd;
          int Alpha;
          int v;
          int N;
          int material;
          int id;
          int pscale;
          int weight;
          int width;

          int pointCd;
          int pointAlpha;
          int pointV;
          int pointN;
          int pointMaterial;
          int pointId;
          int pointInstId;
          int pointPscale;

          // Optional attributes
          /*        int num_neighbors;
                  int force;
                  int vorticity;
                  int uv;
                  int age;
                  int viscosity;
                  int density;
                  int pressure;
                  int mass;
                  int temperature;*/
#endif

     } myInstMBAttrOffsets;


     VRAY_clusterThis *myParent;

     // Parameters
     int     myPrimType;
     fpreal  mySize[3];
     fpreal  myRadius;
     int     myDoMotionBlur;
     fpreal  myShutter;
     UT_String myGeoFile;
     UT_String myObjectName;
     uint32   myNumCopies;
     uint32   myRecursion;
     fpreal   myBirthProb;

     // Other vars ...
//   UT_String myMaterial;
//   fpreal myTheta;

     UT_BoundingBox myBox;
     fpreal bb_x1, bb_x2, bb_y1, bb_y2, bb_z1, bb_z2;

     GU_Detail *myFileGDP;
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

};



#endif


/**********************************************************************************/
//  $Log:
//
/**********************************************************************************/


