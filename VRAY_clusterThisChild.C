/* ******************************************************************************
*
*  VRAY_clusterThisChild class for VRAY_clusterThis mantra DSO
*
* $RCSfile: VRAY_clusterThisChild.C,v $
*
* Description : This class will instance geomtery from the addProcedural calls of it's "parent" class, VRAY_clusterThis
*
* $Revision: 1.25 $
*
* $Source: /dca/cvsroot/houdini/VRAY_clusterThis/VRAY_clusterThisChild.C,v $
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

#ifndef __VRAY_clusterThisChild_C__
#define __VRAY_clusterThisChild_C__

#include "VRAY_clusterThisChild.h"


/* ******************************************************************************
*  Function Name : getClassName()
*
*  Description :  getClassName
*
*  Input Arguments : None
*
*  Return Value : const char *
*
***************************************************************************** */
const char * VRAY_clusterThisChild::getClassName()
{
#ifdef DEBUG
   std::cout << "VRAY_clusterThisChild::getClassName()" << std::endl;
#endif
   return "VRAY_clusterThisChild";
}


/* ******************************************************************************
*  Function Name : initialize()
*
*  Description :
*
*  Input Arguments : const UT_BoundingBox *
*
*  Return Value : int
*
***************************************************************************** */
int VRAY_clusterThisChild::initialize(const UT_BoundingBox *)
{
#ifdef DEBUG
   std::cout << "VRAY_clusterThisChild::initialize()" << std::endl;
#endif
   fprintf(stderr, "VRAY_clusterThisChild::initialize() - This method should never be called!\n");
   return 0;
}


/* ******************************************************************************
*  Function Name : getBoundingBox()
*
*  Description :
*
*  Input Arguments : UT_BoundingBox &box
*
*  Return Value :
*
***************************************************************************** */
void VRAY_clusterThisChild::getBoundingBox(UT_BoundingBox & box)
{
   box.initBounds(myPointAttributes.myNewPos);

//       box.enlargeBounds(mySize[0], mySize[1], mySize[2]);

   box.enlargeBounds(mySize[0] * 2, mySize[1] * 2, mySize[2] * 2);

//   fpreal size = mySize[0];
//   if (size < mySize[1])
//      size = mySize[1];
//   if (size < mySize[2])
//      size = mySize[2];

//   box.enlargeBounds(0, (size));


//#ifdef DEBUG
//   std::cout << "VRAY_clusterThisChild::getBoundingBox() box: " << box << std::endl;
//#endif

// FROM THE VRAY_DemoMountain example:
//
   // Invoke the measuring code on the bounding box to determine the level of
   // detail.  The level of detail is the square root of the number of pixels
   // covered by the bounding box (with all shading factors considered).
   // However, for computing LOD, we do *not* want to include displacement
   // bounds
//    computeBounds(box, false);
//    lod = getLevelOfDetail(box);

// set the LOD for this child instance
   myLOD = getLevelOfDetail(box);
}


/* ******************************************************************************
*  Function Name : render()
*
*  Description :  Render this child cluster object
*
*  Input Arguments : None
*
*  Return Value :
*
***************************************************************************** */
void VRAY_clusterThisChild::render()
{
#ifdef DEBUG
   std::cout << "VRAY_clusterThisChild::render()" << std::endl;
#endif

   if(myLOD < 0.1) {
//      std::cout << "VRAY_clusterThisChild::render() - LOD culled" << std::endl;
         return;
      }


   // // Create a primitive based upon user's selection
   // // TODO: can later be driven by a point attribute
   switch(myPrimType) {
         case CLUSTER_POINT:
            VRAY_clusterThisChild::instancePoint();
            break;
         case CLUSTER_PRIM_SPHERE:
            VRAY_clusterThisChild::instanceSphere();
            break;
         case CLUSTER_PRIM_CUBE:
            VRAY_clusterThisChild::instanceCube();
            break;
         case CLUSTER_PRIM_GRID:
            VRAY_clusterThisChild::instanceGrid();
            break;
         case CLUSTER_PRIM_TUBE:
            VRAY_clusterThisChild::instanceTube();
            break;
         case CLUSTER_PRIM_CIRCLE:
            VRAY_clusterThisChild::instanceCircle();
            break;
         case CLUSTER_PRIM_CURVE:
            VRAY_clusterThisChild::instanceCurve();
            break;
         case CLUSTER_PRIM_METABALL:
            VRAY_clusterThisChild::instanceMetaball();
            break;
         case CLUSTER_FILE:
            VRAY_clusterThisChild::instanceFile();
            break;
            // In case a prim type comes through that's not "legal", throw exception
         default:
            throw VRAY_clusterThis_Exception("VRAY_clusterThisChild::render() Illegal primitive type, exiting ...", 1);
            break;
      }

}



/* ******************************************************************************
*  Function Name : instancePoint()
*
*  Description :
*
*  Input Arguments :
*
*  Return Value : int
*
***************************************************************************** */
int VRAY_clusterThisChild::instancePoint()
{

#ifdef DEBUG
   std::cout << "VRAY_clusterThisChild::instancePoint()" << std::endl;
#endif

   GU_Detail * gdp, *mb_gdp;
   GEO_Point * ppt;
   GA_RWAttributeRef attrRef;
   GA_RWHandleI attrIntHandle;
   GA_RWHandleF attrFloatHandle;
   GA_RWHandleV3 attrVector3Handle;

   gdp = allocateGeometry();

// TODO:
// TODO:  Why oh why ... do I have to create the attributes here ... INVESTIGATE!
// TODO:

   GA_RWAttributeRef pt_Cd = gdp->addDiffuseAttribute(GEO_POINT_DICT);
   GA_RWAttributeRef pt_Alpha = gdp->addAlphaAttribute(GEO_POINT_DICT);
   GA_RWAttributeRef pt_vel = gdp->addVelocityAttribute(GEO_POINT_DICT);
//   GA_RWAttributeRef pt_N = gdp->addNormalAttribute(GEO_POINT_DICT);
   GA_RWAttributeRef pt_pscale = gdp->addFloatTuple(GA_ATTRIB_POINT, "pscale", 1);
//   GA_RWAttributeRef pt_id = gdp->addIntTuple(GA_ATTRIB_POINT, "id", 1);
//   GA_RWAttributeRef pt_inst_id = gdp->addIntTuple(GA_ATTRIB_POINT, "inst_id", 1);
//   GA_RWAttributeRef pt_material = gdp->addStringTuple(GA_ATTRIB_POINT, "shop_materialpath", 1);

   ppt = gdp->appendPointElement();
   ppt->setPos((float)myPointAttributes.myNewPos[0],
               (float)myPointAttributes.myNewPos[1],
               (float)myPointAttributes.myNewPos[2], 1.0);

   ppt->setValue<UT_Vector3>(pt_Cd, (const UT_Vector3)myPointAttributes.Cd);
   ppt->setValue<fpreal>(pt_Alpha, (const fpreal)myPointAttributes.Alpha);
   ppt->setValue<UT_Vector3>(pt_vel, (const UT_Vector3)myPointAttributes.v);
//   ppt->setValue<UT_Vector3>(pt_N, (const UT_Vector3)myPointAttributes.N);
   ppt->setValue<fpreal>(pt_pscale, (const fpreal)myPointAttributes.pscale);
//   ppt->setValue<int>(pt_id, (const int)myPointAttributes.id);
//   ppt->setValue<int>(pt_inst_id, (const int)myInstanceNum);
//   ppt->setString(pt_material, myPointAttributes.material);


   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
         mb_gdp = allocateGeometry();

         pt_Cd = mb_gdp->addDiffuseAttribute(GEO_POINT_DICT);
         pt_Alpha = mb_gdp->addAlphaAttribute(GEO_POINT_DICT);
         pt_vel = mb_gdp->addVelocityAttribute(GEO_POINT_DICT);
//         pt_N = mb_gdp->addNormalAttribute(GEO_POINT_DICT);
//         pt_pscale = mb_gdp->addFloatTuple(GA_ATTRIB_POINT, "pscale", 1);
//         pt_id = mb_gdp->addIntTuple(GA_ATTRIB_POINT, "id", 1);
//         pt_inst_id = mb_gdp->addIntTuple(GA_ATTRIB_POINT, "inst_id", 1);

//      GA_RWAttributeRef pt_material = mb_gdp->addStringTuple(GA_ATTRIB_POINT, "shop_materialpath", 1);

         ppt = mb_gdp->appendPointElement();
         ppt->setPos((float)myPointAttributes.myMBPos[0],
                     (float)myPointAttributes.myMBPos[1],
                     (float)myPointAttributes.myMBPos[2], 1.0);
         ppt->setValue<UT_Vector3>(pt_Cd, (const UT_Vector3)myPointAttributes.Cd);
         ppt->setValue<fpreal>(pt_Alpha, (const fpreal)myPointAttributes.Alpha);
         ppt->setValue<UT_Vector3>(pt_vel, (const UT_Vector3)myPointAttributes.v);
//   ppt->setValue<UT_Vector3>(pt_N, (const UT_Vector3)myPointAttributes.N);
         ppt->setValue<fpreal>(pt_pscale, (const fpreal)myPointAttributes.pscale);
//   ppt->setValue<int>(pt_id, (const int)myPointAttributes.id);
//   ppt->setValue<int>(pt_inst_id, (const int)myInstanceNum);
//   ppt->setString(pt_material, myPointAttributes.material);

      }

   openGeometryObject();
   addGeometry(gdp, 0.0);
   if(myDoMotionBlur == CLUSTER_MB_VELOCITY)
      addVelocityBlurGeometry(gdp, myShutter, myShutter2);
   else if(myDoMotionBlur == CLUSTER_MB_DEFORMATION)
      addGeometry(mb_gdp, myShutter);
   setSurface((const char *)myPointAttributes.material);
   closeObject();

//   std::cout << "VRAY_clusterThisChild::instancePoint() : myInstanceNum: " << myInstanceNum << std::endl;

   myInstanceNum++;

#ifdef DEBUG
   cout << "VRAY_clusterThisChild::instancePoint() Instanced a point " << endl;
#endif

   return 0;

}



/* ******************************************************************************
*  Function Name : instanceSphere()
*
*  Description :
*
*  Input Arguments :
*
*  Return Value : int
*
***************************************************************************** */
int VRAY_clusterThisChild::instanceSphere()
{

#ifdef DEBUG
   std::cout << "VRAY_clusterThisChild::instanceSphere()" << std::endl;
#endif

   GU_Detail * gdp, *mb_gdp;
   UT_Matrix4 xform(1.0);
   UT_XformOrder xformOrder;
//   GEO_Point * ppt;
   GU_PrimSphere * sphere;
   GU_PrimSphereParms sphere_parms;

   gdp = allocateGeometry();

   sphere_parms.gdp = gdp;
   sphere_parms.xform = xform;
   sphere_parms.xform.scale(mySize[0] * myPointAttributes.pscale, mySize[1] * myPointAttributes.pscale, mySize[2] * myPointAttributes.pscale);
   sphere_parms.xform.rotate(myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2], xformOrder);
   sphere_parms.xform.translate(myPointAttributes.myNewPos[0], myPointAttributes.myNewPos[1], myPointAttributes.myNewPos[2]);
   sphere = (GU_PrimSphere *) GU_PrimSphere::build(sphere_parms);

   VRAY_clusterThisChild::setInstanceAttributes(gdp, sphere);


//   for (int i=0; i < sphere->getVertexCount(); i++) {
//      ppt = sphere->getVertexElement(i).getPt();
//      ppt->setValue<UT_Vector3>(myInstAttrRefs.pointCd, (const UT_Vector3)myPointAttributes.Cd);
//      ppt->setValue<float>(myInstAttrRefs.pointAlpha, (const float)myPointAttributes.Alpha);
//      ppt->setValue<UT_Vector3>(myInstAttrRefs.pointV, (const UT_Vector3)myPointAttributes.v);
//      ppt->setValue<UT_Vector3>(myInstAttrRefs.pointN, (const UT_Vector3)myPointAttributes.N);
//      ppt->setValue<float>(myInstAttrRefs.pointPscale, (const float)myPointAttributes.pscale);
//      ppt->setValue<int>(myInstAttrRefs.pointId, (const int)myPointAttributes.id);
//      ppt->setValue<int>(myInstAttrRefs.pointInstId, (const int)myInstanceNum);
//      ppt->setString(myInstAttrRefs.pointMaterial, myPointAttributes.material);
//
////          cout << "VRAY_clusterThis::setInstanceAttributes: Point Instance Alpha: " << ppt->getValue<float>(myInstAttrRefs.pointAlpha) << endl;
//   }


   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
         mb_gdp = allocateGeometry();

         sphere_parms.gdp = mb_gdp;
         sphere_parms.xform.scale(mySize[0] * myPointAttributes.pscale, mySize[1] * myPointAttributes.pscale, mySize[2] * myPointAttributes.pscale);
         sphere_parms.xform.rotate(myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2], xformOrder);
         sphere_parms.xform.translate(myPointAttributes.myMBPos[0], myPointAttributes.myMBPos[1], myPointAttributes.myMBPos[2]);
         sphere = (GU_PrimSphere *) GU_PrimSphere::build(sphere_parms);

         VRAY_clusterThisChild::setInstanceAttributes(mb_gdp, sphere);

      }

   openGeometryObject();
   addGeometry(gdp, 0.0);
   if(myDoMotionBlur == CLUSTER_MB_VELOCITY)
      addVelocityBlurGeometry(gdp, myShutter, myShutter2);
   else if(myDoMotionBlur == CLUSTER_MB_DEFORMATION)
      if(myDoMotionBlur == CLUSTER_MB_DEFORMATION)
         addGeometry(mb_gdp, myShutter);
   setComputeN(1);
   setSurface((const char *)myPointAttributes.material);
   closeObject();

   myInstanceNum++;

#ifdef DEBUG
   cout << "VRAY_clusterThisChild::instanceSphere() Instanced a sphere " << endl;
#endif

   return 0;
}



/* ******************************************************************************
*  Function Name : instanceCube()
*
*  Description :
*
*  Input Arguments :
*
*  Return Value : int
*
***************************************************************************** */
int VRAY_clusterThisChild::instanceCube()
{
#ifdef DEBUG
   std::cout << "VRAY_clusterThisChild::instanceCube()" << std::endl;
#endif

   GU_Detail * gdp, *mb_gdp;
   UT_Matrix4 xform(1.0);
   GEO_Primitive * myCube;
   GEO_Point * ppt;
   UT_XformOrder xformOrder;
//   UT_Vector3F r, s, t, p;
   UT_Matrix3 rot_xform(1.0);
   UT_Vector3 myDir = myPointAttributes.N;
   UT_Vector3 myUp = UT_Vector3(0,1,0);
//   xform.identity();
//   rot_xform.identity();
   rot_xform.orient(myDir, myUp);
   xform = rot_xform;

   gdp = allocateGeometry();


   myCube = (GEO_Primitive *) gdp->cube(myPointAttributes.myNewPos[0] - ((mySize[0] * myPointAttributes.pscale) / 2),
                                        myPointAttributes.myNewPos[0] + ((mySize[0] * myPointAttributes.pscale) / 2),
                                        myPointAttributes.myNewPos[1] - ((mySize[1] * myPointAttributes.pscale) / 2),
                                        myPointAttributes.myNewPos[1] + ((mySize[1] * myPointAttributes.pscale) / 2),
                                        myPointAttributes.myNewPos[2] - ((mySize[2] * myPointAttributes.pscale) / 2),
                                        myPointAttributes.myNewPos[2] + ((mySize[2] * myPointAttributes.pscale) / 2));

   for(int i=0; i < myCube->getVertexCount(); i++) {
         ppt = myCube->getVertexElement(i).getPt();
         UT_Vector4  P = ppt->getPos();
         P *= xform;
         ppt->setPos(P);
      }


   VRAY_clusterThisChild::setInstanceAttributes(gdp, myCube);

   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
         mb_gdp = allocateGeometry();

         myCube = (GEO_Primitive *) mb_gdp->cube(myPointAttributes.myMBPos[0] - ((mySize[0] * myPointAttributes.pscale) / 2),
                                                 myPointAttributes.myMBPos[0] + ((mySize[0] * myPointAttributes.pscale) / 2),
                                                 myPointAttributes.myMBPos[1] - ((mySize[1] * myPointAttributes.pscale) / 2),
                                                 myPointAttributes.myMBPos[1] + ((mySize[1] * myPointAttributes.pscale) / 2),
                                                 myPointAttributes.myMBPos[2] - ((mySize[2] * myPointAttributes.pscale) / 2),
                                                 myPointAttributes.myMBPos[2] + ((mySize[2] * myPointAttributes.pscale) / 2));

         for(int i=0; i < myCube->getVertexCount(); i++) {
               ppt = myCube->getVertexElement(i).getPt();
               UT_Vector4  P = ppt->getPos();
               P *= xform;
               ppt->setPos(P);
            }

         VRAY_clusterThisChild::setInstanceAttributes(mb_gdp, myCube);

      }

   openGeometryObject();
   addGeometry(gdp, 0.0);
   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION)
      addGeometry(mb_gdp, myShutter);
   setComputeN(1);
   setSurface(myPointAttributes.material);
   closeObject();

   myInstanceNum++;

#ifdef DEBUG
   cout << "VRAY_clusterThisChild::instanceCube() Instanced a cube " << endl;
#endif

   return 0;
}


/* ******************************************************************************
*  Function Name : instanceGrid()
*
*  Description :
*
*  Input Arguments :
*
*  Return Value : int
*
***************************************************************************** */
int VRAY_clusterThisChild::instanceGrid()
{
#ifdef DEBUG
   std::cout << "VRAY_clusterThisChild::instanceGrid()" << std::endl;
#endif

   GEO_Primitive * myGrid;
   GU_Detail * gdp, *mb_gdp;
   UT_Matrix4 xform(1.0);
   GU_GridParms grid_parms;
   GEO_Point * ppt;
   UT_XformOrder xformOrder;
//   UT_Vector3F r, s, t, p;
   UT_Matrix3 rot_xform(1.0);
   UT_Vector3 myDir = myPointAttributes.N;
   UT_Vector3 myUp = UT_Vector3(0,1,0);
   myDir.normalize();
//   xform.identity();
//   rot_xform.identity();
   rot_xform.orient(myDir, myUp);
   xform = rot_xform;

   gdp = allocateGeometry();

   grid_parms.rows = 2;
   grid_parms.cols = 2;
   grid_parms.xsize = mySize[0] * myPointAttributes.pscale;
   grid_parms.ysize = mySize[1] * myPointAttributes.pscale;
   grid_parms.xcenter = myPointAttributes.myNewPos[0];
   grid_parms.ycenter = myPointAttributes.myNewPos[1];
   grid_parms.zcenter = myPointAttributes.myNewPos[2];
   grid_parms.plane = GU_PLANE_XY;
   myGrid = gdp->buildGrid(grid_parms, GU_GRID_POLY);

   for(int i=0; i < myGrid->getVertexCount(); i++) {
         ppt = myGrid->getVertexElement(i).getPt();
         UT_Vector4  P = ppt->getPos();
         P *= xform;
         ppt->setPos(P);
      }

   VRAY_clusterThisChild::setInstanceAttributes(gdp, myGrid);

   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
         mb_gdp = allocateGeometry();

         grid_parms.xcenter = myPointAttributes.myMBPos[0];
         grid_parms.ycenter = myPointAttributes.myMBPos[1];
         grid_parms.zcenter = myPointAttributes.myMBPos[2];
         myGrid = mb_gdp->buildGrid(grid_parms, GU_GRID_POLY);

         for(int i=0; i < myGrid->getVertexCount(); i++) {
               ppt = myGrid->getVertexElement(i).getPt();
               UT_Vector4  P = ppt->getPos();
               P *= xform;
               ppt->setPos(P);
            }

         VRAY_clusterThisChild::setInstanceAttributes(mb_gdp, myGrid);

      }

   openGeometryObject();
   addGeometry(gdp, 0.0);
   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION)
      addGeometry(mb_gdp, myShutter);
   setComputeN(1);
   setSurface(myPointAttributes.material);
   closeObject();

   myInstanceNum++;

#ifdef DEBUG
   cout << "VRAY_clusterThisChild::instanceGrid() Instanced a grid " << endl;
#endif

   return 0;
}


/* ******************************************************************************
*  Function Name : instanceTube()
*
*  Description :
*
*  Input Arguments :
*
*  Return Value : int
*
***************************************************************************** */
int VRAY_clusterThisChild::instanceTube()
{
#ifdef DEBUG
   std::cout << "VRAY_clusterThisChild::instanceTube()" << std::endl;
#endif

   GU_Detail * gdp, *mb_gdp;
   UT_Matrix4 xform(1.0);
   UT_XformOrder xformOrder;

   gdp = allocateGeometry();

   GU_PrimTube * tube;
   GU_PrimTubeParms tube_parms;
   GU_CapOptions tube_cap_options;

   tube_parms.gdp = gdp;
//   tube_cap_options.firstUCap = GU_CAP_ROUNDED;
//   tube_cap_options.firstVCap = GU_CAP_ROUNDED;
//   tube_cap_options.lastUCap = GU_CAP_ROUNDED;
//   tube_cap_options.lastVCap = GU_CAP_ROUNDED;
   tube_parms.xform = xform;
   tube_parms.xform.scale(mySize[0] * myPointAttributes.pscale, mySize[1] * myPointAttributes.pscale, mySize[2] * myPointAttributes.pscale);
   tube_parms.xform.rotate(myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2], xformOrder);
   tube_parms.xform.translate(myPointAttributes.myNewPos[0], myPointAttributes.myNewPos[1], myPointAttributes.myNewPos[2]);
   tube = (GU_PrimTube *) GU_PrimTube::build(tube_parms, tube_cap_options);

   VRAY_clusterThisChild::setInstanceAttributes(gdp, tube);

   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
         mb_gdp = allocateGeometry();

         tube_parms.gdp = mb_gdp;
//         tube_cap_options.firstUCap = GU_CAP_ROUNDED;
//         tube_cap_options.firstVCap = GU_CAP_ROUNDED;
//         tube_cap_options.lastUCap = GU_CAP_ROUNDED;
//         tube_cap_options.lastVCap = GU_CAP_ROUNDED;
         tube_parms.xform = xform;
         tube_parms.xform.scale(mySize[0] * myPointAttributes.pscale, mySize[1] * myPointAttributes.pscale, mySize[2] * myPointAttributes.pscale);
         tube_parms.xform.rotate(myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2], xformOrder);
         tube_parms.xform.translate(myPointAttributes.myNewPos[0], myPointAttributes.myNewPos[1], myPointAttributes.myNewPos[2]);
         tube = (GU_PrimTube *) GU_PrimTube::build(tube_parms, tube_cap_options);

         VRAY_clusterThisChild::setInstanceAttributes(mb_gdp, tube);
      }

   openGeometryObject();
   addGeometry(gdp, 0.0);
   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION)
      addGeometry(mb_gdp, myShutter);
   setComputeN(1);
   setSurface(myPointAttributes.material);
   closeObject();

   myInstanceNum++;

#ifdef DEBUG
   cout << "VRAY_clusterThisChild::instanceTube() Instanced a tube " << endl;
#endif

   return 0;
}


/* ******************************************************************************
*  Function Name : instanceCircle()
*
*  Description :
*
*  Input Arguments :
*
*  Return Value : int
*
***************************************************************************** */
int VRAY_clusterThisChild::instanceCircle()
{
#ifdef DEBUG
   std::cout << "VRAY_clusterThisChild::instanceCircle()" << std::endl;
#endif

   std::cout << "VRAY_clusterThisChild::instanceCircle(): myInstanceNum = " << myInstanceNum<< std::endl;

   GU_Detail * gdp, *mb_gdp;
   UT_Matrix4 xform(1.0);
   UT_XformOrder xformOrder;
   GU_PrimCircle * circle;
   GU_PrimCircleParms circle_parms;

   gdp = allocateGeometry();

   circle_parms.gdp = gdp;
   circle_parms.xform = xform;
   circle_parms.xform.scale(mySize[0] * myPointAttributes.pscale, mySize[1] * myPointAttributes.pscale, mySize[2] * myPointAttributes.pscale);
   circle_parms.xform.rotate(myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2], xformOrder);
   circle_parms.xform.translate(myPointAttributes.myNewPos[0], myPointAttributes.myNewPos[1], myPointAttributes.myNewPos[2]);
   circle = (GU_PrimCircle *) GU_PrimCircle::build(circle_parms);

   VRAY_clusterThisChild::setInstanceAttributes(gdp, circle);

   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
         mb_gdp = allocateGeometry();

         circle_parms.gdp = mb_gdp;
         circle_parms.xform = xform;
         circle_parms.xform.scale(mySize[0] * myPointAttributes.pscale, mySize[1] * myPointAttributes.pscale, mySize[2] * myPointAttributes.pscale);
         circle_parms.xform.rotate(myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2], xformOrder);
         circle_parms.xform.translate(myPointAttributes.myMBPos[0], myPointAttributes.myMBPos[1], myPointAttributes.myMBPos[2]);
         circle = (GU_PrimCircle *) GU_PrimCircle::build(circle_parms);

         VRAY_clusterThisChild::setInstanceAttributes(mb_gdp, circle);

      }

   openGeometryObject();
   addGeometry(gdp, 0.0);
   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION)
      addGeometry(mb_gdp, myShutter);
   setComputeN(1);
   setSurface(myPointAttributes.material);
   closeObject();

   myInstanceNum++;

#ifdef DEBUG
   cout << "VRAY_clusterThisChild::instanceCircle() Instanced a circle " << endl;
#endif

   return 0;
}


/* ******************************************************************************
*  Function Name : instanceCurve()
*
*  Description :
*
*  Input Arguments :
*
*  Return Value : int
*
***************************************************************************** */
int VRAY_clusterThisChild::instanceCurve()
{
#ifdef DEBUG
   std::cout << "VRAY_clusterThisChild::instanceCurve()" << std::endl;
#endif

   return 0;


   GU_Detail * inst_gdp, *mb_gdp;
   UT_Matrix4 xform(1.0);

   inst_gdp = allocateGeometry();

   GEO_Point * ppt;
   GA_RWAttributeRef pt_Cd = inst_gdp->addDiffuseAttribute(GEO_POINT_DICT);
   GA_RWAttributeRef pt_Alpha = inst_gdp->addAlphaAttribute(GEO_POINT_DICT);
   GA_RWAttributeRef pt_vel = inst_gdp->addVelocityAttribute(GEO_POINT_DICT);
   GA_RWAttributeRef pt_N = inst_gdp->addNormalAttribute(GEO_POINT_DICT);

   GA_RWAttributeRef pt_pscale = inst_gdp->addFloatTuple(GA_ATTRIB_POINT, "pscale", 1);
   GA_RWAttributeRef pt_id = inst_gdp->addIntTuple(GA_ATTRIB_POINT, "id", 1);
   GA_RWAttributeRef pt_inst_id = inst_gdp->addIntTuple(GA_ATTRIB_POINT, "inst_id", 1);

//    GA_RWAttributeRef pt_material = inst_gdp->addPointAttrib ( "shop_materialpath", sizeof ( int ), GB_ATTRIB_INDEX, 0 );

   GA_RWAttributeRef pt_mb_Cd;
   GA_RWAttributeRef pt_mb_Alpha;
   GA_RWAttributeRef pt_mb_vel;
   GA_RWAttributeRef pt_mb_N;
   GA_RWAttributeRef pt_mb_pscale;
   GA_RWAttributeRef pt_mb_id;
   GA_RWAttributeRef pt_mb_inst_id;
   GA_RWAttributeRef pt_mb_material;

   // static GU_PrimNURBCurve *  build (GU_Detail *gudp, int nelems, int order=4, int closed=0, int interpEnds=1, int appendPoints=1)
   // myCurve = GU_PrimNURBCurve::build(gdp, num_divs, 4, 0, 1, 0);
   // virtual int  appendVertex (GEO_Point *ppt=0)

   uint32 num_vtx = ((myNumCopies * myRecursion) > 4)?(myNumCopies * myRecursion):4;
   GU_PrimNURBCurve * myCurve = (GU_PrimNURBCurve *)GU_PrimNURBCurve::build((GU_Detail *)inst_gdp, num_vtx, 4, 0, 1, 1);

   GU_PrimNURBCurve * myMBCurve;

//   cout << "VRAY_clusterThis::instanceCurve() - num vertices: " << myCurve->getVertexCount () << endl;;
//   cout << "id: "  << myCurve->getPrimitiveId() << endl;
//   cout << "breakCount: "  << myCurve->breakCount () << endl;
//   myCurve->raiseOrder (4);
//   cout << "getOrder: "  << myCurve->getOrder () << endl;


   if(pt_Cd.isValid())
      myCurve->setValue<UT_Vector3>(pt_Cd, (const UT_Vector3)myPointAttributes.Cd);
   myCurve->setValue<fpreal>(pt_Alpha, (const fpreal)myPointAttributes.Alpha);
   myCurve->setValue<UT_Vector3>(pt_vel, (const UT_Vector3)myPointAttributes.v);
   myCurve->setValue<UT_Vector3>(pt_N, (const UT_Vector3)myPointAttributes.N);

   myCurve->setValue<int>(pt_id, (const int)myPointAttributes.id);

//               UT_String* mat = myCurve->castAttribData<UT_String> ( myInstAttrRefs.material );
//               *mat = myPointAttributes.material;

   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
         mb_gdp = allocateGeometry();
         myMBCurve = (GU_PrimNURBCurve *)GU_PrimNURBCurve::build((GU_Detail *)mb_gdp, num_vtx, 4, 0, 1, 1);
         pt_mb_Cd = mb_gdp->addDiffuseAttribute(GEO_POINT_DICT);
         pt_mb_Alpha = mb_gdp->addAlphaAttribute(GEO_POINT_DICT);
         pt_mb_vel = mb_gdp->addVelocityAttribute(GEO_POINT_DICT);
         pt_mb_N = mb_gdp->addNormalAttribute(GEO_POINT_DICT);
         pt_pscale = mb_gdp->addFloatTuple(GA_ATTRIB_POINT, "pscale", 1);
         pt_id = mb_gdp->addIntTuple(GA_ATTRIB_POINT, "id", 1);
         pt_inst_id = mb_gdp->addIntTuple(GA_ATTRIB_POINT, "inst_id", 1);

//        pt_mb_material = mb_gdp->addPointAttrib ( "shop_materialpath", sizeof ( int ), GB_ATTRIB_INDEX, 0);

         myCurve->setValue<UT_Vector3>(pt_mb_Cd, (const UT_Vector3)myPointAttributes.Cd);
         myCurve->setValue<fpreal>(pt_mb_Alpha, (const fpreal)myPointAttributes.Alpha);
         myCurve->setValue<UT_Vector3>(pt_mb_vel, (const UT_Vector3)myPointAttributes.v);
         myCurve->setValue<UT_Vector3>(pt_mb_N, (const UT_Vector3)myPointAttributes.N);

         myCurve->setValue<int>(pt_mb_id, (const int)myPointAttributes.id);

//        UT_String* mat = myMBCurve->castAttribData<UT_String> ( myInstMBAttrOffsets.material );
//        *mat = myPointAttributes.material;

      }

   fpreal theta = (2.0 * M_PI) / myNumCopies;
   uint seed = 23;
   fpreal dice;
   bool skip = false;

   myInstanceNum = 0;

   for(int copyNum = 0; copyNum < myNumCopies; copyNum++)
      for(int recursionNum = 0; recursionNum < myRecursion; recursionNum++) {
            // insert random number to determine to instance or not

            dice = SYSfastRandom(seed);
            (dice > myBirthProb)?skip = true:skip = false;
//                  cout << dice << " " << skip << endl;
            seed = uint(dice * 100);

//            if(!skip) {
            if(true) {

                  myParent->calculateNewPosition(theta, copyNum, recursionNum);
                  ppt = myCurve->getVertexElement(myInstanceNum).getPt();
                  inst_gdp->points()[myInstanceNum]->setPos((float)myPointAttributes.myNewPos[0],
                        (float)myPointAttributes.myNewPos[1],
                        (float)myPointAttributes.myNewPos[2], 1.0);

                  // Assign color to each point
//              Cd = ppt->castAttribData<UT_Vector3> ( pt_Cd );
//              Cd->assign ( ( float ) myPointAttributes.Cd.x(), ( float ) myPointAttributes.Cd.y(), ( float ) myPointAttributes.Cd.z() );

//                 Alpha_ptr = ppt->castAttribData<fpreal> ( pt_Alpha );
//                 *Alpha_ptr = myPointAttributes.Alpha;
//                 vel = ppt->castAttribData<UT_Vector3> ( pt_vel );
//                 vel->assign ( ( float ) myPointAttributes.v.x(), ( float ) myPointAttributes.v.y(), ( float ) myPointAttributes.v.z() );
//                 N = ppt->castAttribData<UT_Vector3> ( pt_N );
//                 N->assign ( ( float ) myPointAttributes.N.x(), ( float ) myPointAttributes.N.y(), ( float ) myPointAttributes.N.z() );
//                 pscale_ptr = ppt->castAttribData<fpreal> ( pt_pscale );
//                 *pscale_ptr = myPointAttributes.pscale;
//                 id_ptr = ppt->castAttribData<int> ( pt_id );
//                 *id_ptr = myPointAttributes.id;
//                 inst_id_ptr = ppt->castAttribData<int> ( pt_inst_id );
//                 *inst_id_ptr = myInstanceNum;
//                 UT_String* mat = ppt->castAttribData<UT_String> ( pt_material );
//                 *mat = myPointAttributes.material;


                  if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
                        ppt = myMBCurve->getVertexElement(myInstanceNum).getPt();
                        mb_gdp->points()[myInstanceNum]->setPos((float)myPointAttributes.myMBPos[0],
                                                                (float)myPointAttributes.myMBPos[1],
                                                                (float)myPointAttributes.myMBPos[2], 1.0);


                        // Assign color to each point
//                  Cd = ppt->castAttribData<UT_Vector3> ( pt_mb_Cd );
//                  Cd->assign ( ( float ) myPointAttributes.Cd.x(), ( float ) myPointAttributes.Cd.y(), ( float ) myPointAttributes.Cd.z() );

//                     Alpha_ptr = ppt->castAttribData<fpreal> ( pt_mb_Alpha );
//                     *Alpha_ptr = myPointAttributes.Alpha;
//                     vel = ppt->castAttribData<UT_Vector3> ( pt_mb_vel );
//                     vel->assign ( ( float ) myPointAttributes.v.x(), ( float ) myPointAttributes.v.y(), ( float ) myPointAttributes.v.z() );
//                     N = ppt->castAttribData<UT_Vector3> ( pt_mb_N );
//                     N->assign ( ( float ) myPointAttributes.N.x(), ( float ) myPointAttributes.N.y(), ( float ) myPointAttributes.N.z() );
//                     pscale_ptr = ppt->castAttribData<fpreal> ( pt_mb_pscale );
//                     *pscale_ptr = myPointAttributes.pscale;
//                     id_ptr = ppt->castAttribData<int> ( pt_mb_id );
//                     *id_ptr = myPointAttributes.id;
//                     inst_id_ptr = ppt->castAttribData<int> ( pt_mb_inst_id );
//                     *inst_id_ptr = myInstanceNum;
//                     UT_String* mat = ppt->castAttribData<UT_String> ( pt_mb_material );
//                     *mat = myPointAttributes.material;

                     }

                  myInstanceNum++;

               }
         }

//   myCurve->close ();
//   myMBCurve->close ();



#ifdef DEBUG
   cout << "VRAY_clusterThisChild::instanceCurve() Instanced a curve " << endl;
#endif

   openGeometryObject();
   addGeometry(inst_gdp, 0.0);
   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION)
      addGeometry(mb_gdp, myShutter);
   setComputeN(1);
   setSurface(myPointAttributes.material);
   closeObject();

   return 0;
}



/* ******************************************************************************
*  Function Name : instanceMetaball()
*
*  Description :
*
*  Input Arguments :
*
*  Return Value : int
*
***************************************************************************** */
int VRAY_clusterThisChild::instanceMetaball()
{

#ifdef DEBUG
   std::cout << "VRAY_clusterThis::instanceMetaball()" << std::endl;
#endif

   GU_Detail * gdp, *mb_gdp;
   UT_Matrix4 xform(1.0);
   UT_XformOrder xformOrder;
   GU_PrimMetaBall * metaball;
   GU_PrimMetaBallParms metaball_parms;

   gdp = allocateGeometry();

   metaball_parms.gdp = gdp;
   metaball_parms.xform = xform;
   metaball_parms.weight = myPointAttributes.weight;
   metaball_parms.xform.scale(mySize[0] * myPointAttributes.pscale, mySize[1] * myPointAttributes.pscale, mySize[2] * myPointAttributes.pscale);
   metaball_parms.xform.rotate(myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2], xformOrder);
   metaball_parms.xform.translate(myPointAttributes.myNewPos[0], myPointAttributes.myNewPos[1], myPointAttributes.myNewPos[2]);
   metaball = (GU_PrimMetaBall *) GU_PrimMetaBall::build(metaball_parms);

   VRAY_clusterThisChild::setInstanceAttributes(gdp, metaball);

   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
         mb_gdp = allocateGeometry();

         metaball_parms.gdp = mb_gdp;
         metaball_parms.xform = xform;
         metaball_parms.weight = myPointAttributes.weight;
         metaball_parms.xform.scale(mySize[0] * myPointAttributes.pscale, mySize[1] * myPointAttributes.pscale, mySize[2] * myPointAttributes.pscale);
         metaball_parms.xform.rotate(myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2], xformOrder);
         metaball_parms.xform.translate(myPointAttributes.myMBPos[0], myPointAttributes.myMBPos[1], myPointAttributes.myMBPos[2]);
         metaball = (GU_PrimMetaBall *) GU_PrimMetaBall::build(metaball_parms);

         VRAY_clusterThisChild::setInstanceAttributes(mb_gdp, metaball);

      }

   openGeometryObject();
   addGeometry(gdp, 0.0);
   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION)
      addGeometry(mb_gdp, myShutter);
   setComputeN(1);
   setSurface(myPointAttributes.material);
   closeObject();

   myInstanceNum++;

#ifdef DEBUG
   cout << "VRAY_clusterThis::instanceMetaball() Instanced a metaball " << endl;
#endif

   return 0;
}


/* ******************************************************************************
*  Function Name : instanceFile()
*
*  Description :
*
*  Input Arguments :
*
*  Return Value : int
*
***************************************************************************** */
int VRAY_clusterThisChild::instanceFile()
{
#ifdef DEBUG
   std::cout << "VRAY_clusterThisChild::instanceFile()" << std::endl;
#endif

//   std::cout << "VRAY_clusterThisChild::instanceFile() - myGeoFile: " << (const char *)myGeoFile << std::endl;

   GU_Detail * inst_gdp, *mb_gdp, *file_gdp;
   GEO_Point * ppt;

   inst_gdp = allocateGeometry();
   file_gdp = allocateGeometry();

   if(!file_gdp->load((const char *)myGeoFile).success())
      throw VRAY_clusterThis_Exception("VRAY_clusterThisChild::instanceFile() Failed to load geometry file ", 1);

#ifdef DEBUG
   long int num_points = (long int) file_gdp->points().entries();
   cout << "VRAY_clusterThisChild::instanceFile() - num points :" << num_points << endl;
#endif

   // Primitive attributes
   GA_RWAttributeRef Cd = file_gdp->addDiffuseAttribute(GEO_PRIMITIVE_DICT);
   GA_RWAttributeRef Alpha = file_gdp->addAlphaAttribute(GEO_PRIMITIVE_DICT);
   GA_RWAttributeRef v = file_gdp->addVelocityAttribute(GEO_PRIMITIVE_DICT);
//   GA_RWAttributeRef N = file_gdp->addNormalAttribute(GEO_PRIMITIVE_DICT);
//   GA_RWAttributeRef id = file_gdp->addIntTuple(GA_ATTRIB_PRIMITIVE, "id", 1);
//   GA_RWAttributeRef inst_id = file_gdp->addIntTuple(GA_ATTRIB_PRIMITIVE, "inst_id", 1);

   GA_RWAttributeRef material = file_gdp->addStringTuple(GA_ATTRIB_PRIMITIVE, "shop_materialpath", 1);

   // Point attributes
   GA_RWAttributeRef pointCd = file_gdp->addDiffuseAttribute(GEO_POINT_DICT);
   GA_RWAttributeRef pointAlpha = file_gdp->addAlphaAttribute(GEO_POINT_DICT);
   GA_RWAttributeRef pointV = file_gdp->addVelocityAttribute(GEO_POINT_DICT);

//   GA_RWAttributeRef pointPscale = file_gdp->addFloatTuple(GA_ATTRIB_POINT, "pscale", 1);
//   GA_RWAttributeRef pointId = file_gdp->addIntTuple(GA_ATTRIB_POINT, "id", 1);
//   GA_RWAttributeRef pointInstId = file_gdp->addIntTuple(GA_ATTRIB_POINT, "inst_id", 1);
//   GA_RWAttributeRef lod = file_gdp->addIntTuple(GA_ATTRIB_POINT, "lod", 1);

//   if(!pointCd.isValid())
//      throw VRAY_clusterThis_Exception("VRAY_clusterThisChild::instanceFile() point Cd attr failed ", 1);

// NOTE: File instanced geo should have normals already, do not set the normals to the source point normals

   GA_FOR_ALL_GPOINTS(file_gdp, ppt) {
      ppt->setValue<UT_Vector3>(pointCd, (const UT_Vector3)myPointAttributes.Cd);
      ppt->setValue<float>(pointAlpha, (const float)myPointAttributes.Alpha);
      ppt->setValue<UT_Vector3>(pointV, (const UT_Vector3)myPointAttributes.v);
//      ppt->setValue<int>(pointId, (const int)myPointAttributes.id);
   }


   GEO_Primitive * prim;

   GA_FOR_ALL_PRIMITIVES(file_gdp, prim)  {

      prim->setValue<UT_Vector3>(Cd, (const UT_Vector3)myPointAttributes.Cd);
      prim->setValue<fpreal>(Alpha, (const fpreal)myPointAttributes.Alpha);
      prim->setValue<UT_Vector3>(v, (const UT_Vector3)myPointAttributes.v);
//      prim->setValue<int>(id, (const int)myPointAttributes.id);

      prim->setString(material, myPointAttributes.material);
   }


   GU_Detail temp_gdp(file_gdp);

   UT_XformOrder xformOrder;
   UT_Matrix4 xform(1.0);
   UT_Matrix3 rot_xform(1.0);
   UT_Vector3 myDir = myPointAttributes.N;
   UT_Vector3 myUp = UT_Vector3(0,1,0);

// Transform the geo to the new position
   xform.identity();
   rot_xform.identity();
   rot_xform.orient(myDir, myUp);
   xform = rot_xform;

   xform.scale(mySize[0] * myPointAttributes.pscale, mySize[1] * myPointAttributes.pscale, mySize[2] * myPointAttributes.pscale);
//    xform.rotate(myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2], xformOrder);
   xform.translate(myPointAttributes.myNewPos[0], myPointAttributes.myNewPos[1], myPointAttributes.myNewPos[2]);
//   xform.xform(xformOrder, myPointAttributes.myNewPos[0], myPointAttributes.myNewPos[1], myPointAttributes.myNewPos[2],
//               myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2],
//               mySize[0], mySize[1], mySize[2]);

   temp_gdp.transform(xform);


   inst_gdp->merge(temp_gdp);

   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
         mb_gdp = allocateGeometry();

         xform.identity();
         rot_xform.identity();
         rot_xform.orient(myDir, myUp);
         xform = rot_xform;
         xform.scale(mySize[0] * myPointAttributes.pscale, mySize[1] * myPointAttributes.pscale, mySize[2] * myPointAttributes.pscale);
//        xform.rotate(myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2], xformOrder);
         xform.translate(myPointAttributes.myMBPos[0], myPointAttributes.myMBPos[1], myPointAttributes.myMBPos[2]);
         //   xform.xform(xformOrder, myPointAttributes.myMBPos[0], myPointAttributes.myMBPos[1], myPointAttributes.myMBPos[2],
         //               myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2],
         //               mySize[0], mySize[1], mySize[2]);

         temp_gdp.transform(xform);
         mb_gdp->merge(temp_gdp);
      }

   openGeometryObject();
   addGeometry(inst_gdp, 0.0);
   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION)
      addGeometry(mb_gdp, myShutter);
   setComputeN(1);
   setSurface(myPointAttributes.material);
   closeObject();

   myInstanceNum++;

#ifdef DEBUG
   cout << "VRAY_clusterThisChild::instanceFile() Instanced a file " << endl;
#endif

   return 0;
}


/* ******************************************************************************
*  Function Name : setInstanceAttributes()
*
*  Description :
*
*  Input Arguments : GEO_Point *ppt, GU_Detail *gdp
*
*  Return Value : int
*
***************************************************************************** */
inline void VRAY_clusterThisChild::setInstanceAttributes(GU_Detail * gdp, GEO_Primitive * myGeoPrim)
{

#ifdef DEBUG
   cout << "VRAY_clusterThisChild::setInstanceAttributes() " << endl;
#endif

   GEO_Point * ppt;

   GA_RWAttributeRef Cd = gdp->addDiffuseAttribute(GEO_PRIMITIVE_DICT);
   GA_RWAttributeRef Alpha = gdp->addAlphaAttribute(GEO_PRIMITIVE_DICT);
   GA_RWAttributeRef vel = gdp->addVelocityAttribute(GEO_PRIMITIVE_DICT);
//   GA_RWAttributeRef N = gdp->addNormalAttribute(GEO_PRIMITIVE_DICT);
//   GA_RWAttributeRef pscale = gdp->addFloatTuple(GEO_PRIMITIVE_DICT, "pscale", 1);
//   GA_RWAttributeRef id = gdp->addIntTuple(GEO_PRIMITIVE_DICT, "id", 1);
//   GA_RWAttributeRef inst_id = gdp->addIntTuple(GEO_PRIMITIVE_DICT, "inst_id", 1);
   GA_RWAttributeRef material = gdp->addStringTuple(GEO_PRIMITIVE_DICT, "shop_materialpath", 1);

   GA_RWAttributeRef pt_Cd = gdp->addDiffuseAttribute(GEO_POINT_DICT);
   GA_RWAttributeRef pt_Alpha = gdp->addAlphaAttribute(GEO_POINT_DICT);
   GA_RWAttributeRef pt_vel = gdp->addVelocityAttribute(GEO_POINT_DICT);
   GA_RWAttributeRef pt_N = gdp->addNormalAttribute(GEO_POINT_DICT);
//   GA_RWAttributeRef pt_pscale = gdp->addFloatTuple(GA_ATTRIB_POINT, "pscale", 1);
//   GA_RWAttributeRef pt_id = gdp->addIntTuple(GA_ATTRIB_POINT, "id", 1);
//   GA_RWAttributeRef pt_inst_id = gdp->addIntTuple(GA_ATTRIB_POINT, "inst_id", 1);
//   GA_RWAttributeRef pt_material = gdp->addStringTuple(GA_ATTRIB_POINT, "shop_materialpath", 1);


   myGeoPrim->setValue<UT_Vector3>(Cd, (const UT_Vector3)myPointAttributes.Cd);
   myGeoPrim->setValue<fpreal>(Alpha, (const fpreal)myPointAttributes.Alpha);
   myGeoPrim->setValue<UT_Vector3>(vel, (const UT_Vector3)myPointAttributes.v);
//   myGeoPrim->setValue<UT_Vector3>(N, (const UT_Vector3)myPointAttributes.N);
//    myGeoPrim->setValue<UT_Vector4>(orient (const UT_Vector4)myPointAttributes.orient);
//   myGeoPrim->setValue<fpreal>(pscale, (const fpreal)myPointAttributes.pscale);
//   myGeoPrim->setValue<int>(id, (const int)myPointAttributes.id);
//   myGeoPrim->setValue<int>(inst_id, (const int)myInstanceNum);
   myGeoPrim->setString(material, myPointAttributes.material);


   // apply attribues to each vertex
   for(int i=0; i < myGeoPrim->getVertexCount(); i++) {
         ppt = myGeoPrim->getVertexElement(i).getPt();
         ppt->setValue<UT_Vector3>(pt_Cd, (const UT_Vector3)myPointAttributes.Cd);
         ppt->setValue<float>(pt_Alpha, (const float)myPointAttributes.Alpha);
         ppt->setValue<UT_Vector3>(pt_vel, (const UT_Vector3)myPointAttributes.v);
         ppt->setValue<UT_Vector3>(pt_N, (const UT_Vector3)myPointAttributes.N);
//         ppt->setValue<float>(pt_pscale, (const float)myPointAttributes.pscale);
//         ppt->setValue<int>(pt_id, (const int)myPointAttributes.id);
//         ppt->setValue<int>(pt_inst_id, (const int)myInstanceNum);
//         ppt->setString(pt_material, myPointAttributes.material);
      }

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
void VRAY_clusterThisChild::dumpParameters()
{

   cout << "VRAY_clusterThisChild::dumpParameters() myPrimType: " << myPrimType << endl;
   cout << "VRAY_clusterThisChild::dumpParameters() myRadius: " << myRadius << endl;
   cout << "VRAY_clusterThisChild::dumpParameters() mySize: " << mySize[0] << mySize[1] << mySize[2] << " " << endl;
   cout << "VRAY_clusterThisChild::dumpParameters() myGeoFile: " << myGeoFile << endl;
}


#endif



/**********************************************************************************/
//  $Log: VRAY_clusterThisChild.C,v $
//  Revision 1.25  2012-09-09 05:00:54  mstory
//  More cleanup and testing.
//
//  Revision 1.24  2012-09-07 15:39:22  mstory
//   Removed all volume instancing (used in different project) and continu… …
//
//  …ed H12 modifications.
//
//  --mstory
//
//  Revision 1.23  2012-09-05 23:02:38  mstory
//  Modifications for H12.
//
//  Revision 1.22  2012-09-04 03:25:28  mstory
//  .
//
//  Revision 1.19  2011-02-15 00:59:15  mstory
//  Refactored out rededundant attribute code in the child (deferred) instancicng mode.
//  Made remaining changes for H11 (and beyond) versions way of handiling attributes.
//
//
//  --mstory
//
//  Revision 1.18  2011-02-06 19:49:15  mstory
//  Modified for Houdini version 11.
//
//  Refactored a lot of the attribute code, cleaned up odds and ends.
//
//  Revision 1.17  2010-02-23 08:36:22  mstory
//  Fixed most of the CVEX problems with primtive instancng.  Fixed seg faults from uninitilialized pointers in the CVEX variables,
//
//  Revision 1.16  2009-11-20 14:59:57  mstory
//  Release 1.4.7 ready.
//
//  Revision 1.15  2009-11-19 16:26:51  mstory
//  Adding point inst id to child objects (for deferred instancing), need to add to prims as well.
//
//  Revision 1.14  2009-04-06 17:13:44  mstory
//  Clean up code a bit.
//
//  Revision 1.13  2009-04-06 16:40:58  mstory
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
//  Revision 1.12  2009-02-10 21:55:59  mstory
//  Added all attributes for the CVEX processing of instanced geo.
//  Added OTL version checking.
//
//  Revision 1.11  2008-12-04 05:37:41  mstory
//  .
//
//  Revision 1.10  2008-11-27 05:32:39  mstory
//  Added Alpha attribute and fixed bug where it crashes mantra if the weight attr wasn't in the input geo.
//
//  Revision 1.9  2008-11-19 01:11:43  mstory
//  Added point instancing.  Fixed the file instancing problem.
//  Most of the shader assignment issues straightened out.
//
//  Revision 1.8  2008-10-30 19:51:54  mstory
//  Added file instancing (still needs work).
//
//  Revision 1.7  2008-10-30 07:03:06  mstory
//  Added deformation motion blur and metaball instancing.
//
//  Revision 1.6  2008-10-20 22:43:57  mstory
//  *** empty log message ***
//
//  Revision 1.5  2008-10-20 22:12:14  mstory
//  Cleaned up unused vars, etc.  Ready for enxt release.
//
//  Revision 1.4  2008-10-20 19:35:00  mstory
//  Added a switch to be able to choose using the addProcedural() method of allocating procedurals.
//
//  Revision 1.2  2008-10-11 18:15:06  mstory
//  .
//
//
/**********************************************************************************/


