/* ******************************************************************************
*
*  clusterThis mantra DSO for render time geo clustering
*
* $RCSfile: VRAY_clusterThisInstance.C,v $
*
* Description :
*
* $Revision: 1.20 $
*
* $Source: /dca/cvsroot/houdini/VRAY_clusterThis/VRAY_clusterThisInstance.C,v $
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


#ifndef __VRAY_clusterThisInstance_C__
#define __VRAY_clusterThisInstance_C__


/* ******************************************************************************
*  Function Name : instancePoint()
*
*  Description :
*
*  Input Arguments : GU_Detail *inst_gdp, GU_Detail *mb_gdp
*
*  Return Value : int
*
***************************************************************************** */
int VRAY_clusterThis::instancePoint(GU_Detail * inst_gdp, GU_Detail * mb_gdp)
{

#ifdef DEBUG
   std::cout << "VRAY_clusterThis::instancePoint()" << std::endl;
#endif

   GEO_Point * ppt;

   ppt = inst_gdp->appendPointElement();
   ppt->setPos((float)myPointAttributes.myNewPos[0],
         (float)myPointAttributes.myNewPos[1],
         (float)myPointAttributes.myNewPos[2], 1.0);

   VRAY_clusterThis::setPointInstanceAttributes(inst_gdp, ppt);

   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
         ppt = mb_gdp->appendPointElement();
         ppt->setPos((float)myPointAttributes.myMBPos[0],
                                                 (float)myPointAttributes.myMBPos[1],
                                                 (float)myPointAttributes.myMBPos[2], 1.0);

         VRAY_clusterThis::setPointInstanceAttributes(mb_gdp, ppt);
      }

   if(myCVEX_Exec)
      VRAY_clusterThis::runCVEX(inst_gdp, mb_gdp, myCVEXFname, CLUSTER_CVEX_POINT);

#ifdef DEBUG
   cout << "VRAY_clusterThis::instancePoint() Instanced a point " << endl;
#endif

   return 0;
}



/* ******************************************************************************
*  Function Name : instanceSphere()
*
*  Description :
*
*  Input Arguments : GU_Detail *inst_gdp, GU_Detail *mb_gdp
*
*  Return Value : int
*
***************************************************************************** */
int VRAY_clusterThis::instanceSphere(GU_Detail * inst_gdp, GU_Detail * mb_gdp)
{

#ifdef DEBUG
   std::cout << "VRAY_clusterThis::instanceSphere()" << std::endl;
#endif

   UT_Matrix4 xform(1.0);
   GU_PrimSphere * sphere;
   GU_PrimSphereParms sphere_parms;
   UT_XformOrder xformOrder;

   sphere_parms.gdp = inst_gdp;
   sphere_parms.xform = xform;
   sphere_parms.xform.scale(mySize[0] * myPointAttributes.pscale, mySize[1] * myPointAttributes.pscale, mySize[2] * myPointAttributes.pscale);
   sphere_parms.xform.rotate(myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2], xformOrder);
   sphere_parms.xform.translate(myPointAttributes.myNewPos[0], myPointAttributes.myNewPos[1], myPointAttributes.myNewPos[2]);
   sphere = (GU_PrimSphere *) GU_PrimSphere::build(sphere_parms);

   VRAY_clusterThis::setInstanceAttributes(inst_gdp, sphere);


   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
         sphere_parms.gdp = mb_gdp;
         sphere_parms.xform.scale(mySize[0] * myPointAttributes.pscale, mySize[1] * myPointAttributes.pscale, mySize[2] * myPointAttributes.pscale);
         sphere_parms.xform.rotate(myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2], xformOrder);
         sphere_parms.xform.translate(myPointAttributes.myNewPos[0], myPointAttributes.myNewPos[1], myPointAttributes.myNewPos[2]);
         sphere = (GU_PrimSphere *) GU_PrimSphere::build(sphere_parms);

         VRAY_clusterThis::setInstanceAttributes(mb_gdp, sphere);
      }

#ifdef DEBUG
   cout << "VRAY_clusterThis::instanceSphere() Instanced a sphere " << endl;
#endif

   if(myCVEX_Exec)
      VRAY_clusterThis::runCVEX(inst_gdp, mb_gdp, myCVEXFname, CLUSTER_CVEX_POINT);

   return 0;
}



/* ******************************************************************************
*  Function Name : instanceCube()
*
*  Description :
*
*  Input Arguments : GU_Detail *inst_gdp, GU_Detail *mb_gdp
*
*  Return Value : int
*
***************************************************************************** */
int VRAY_clusterThis::instanceCube(GU_Detail * inst_gdp, GU_Detail * mb_gdp)
{
#ifdef DEBUG
   std::cout << "VRAY_clusterThis::instanceCube()" << std::endl;
#endif

   GEO_Primitive * myCube;
   GEO_Point * ppt;
   UT_Matrix4 xform(1.0);
   UT_XformOrder xformOrder;
   UT_Matrix3 rot_xform(1.0);
   UT_Vector3 myDir = myPointAttributes.N;
   UT_Vector3 myUp = UT_Vector3(0,1,0);
   rot_xform.orient(myDir, myUp);
   xform = rot_xform;

   myCube = (GEO_Primitive *) inst_gdp->cube(
               myPointAttributes.myNewPos[0] - ((mySize[0] * myPointAttributes.pscale) / 2),
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

   VRAY_clusterThis::setInstanceAttributes(inst_gdp, myCube);

   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {

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

         VRAY_clusterThis::setInstanceAttributes(mb_gdp, myCube);
      }

#ifdef DEBUG
   cout << "VRAY_clusterThis::instanceCube() Instanced a cube " << endl;
#endif

   if(myCVEX_Exec)
      VRAY_clusterThis::runCVEX(inst_gdp, mb_gdp, myCVEXFname, CLUSTER_CVEX_POINT);

   return 0;
}



/* ******************************************************************************
*  Function Name : instanceGrid()
*
*  Description :
*
*  Input Arguments : GU_Detail *inst_gdp, GU_Detail *mb_gdp
*
*  Return Value : int
*
***************************************************************************** */
int VRAY_clusterThis::instanceGrid(GU_Detail * inst_gdp, GU_Detail * mb_gdp)
{
#ifdef DEBUG
   std::cout << "VRAY_clusterThis::instanceGrid()" << std::endl;
#endif

   GEO_Primitive * myGrid;
   GU_GridParms grid_parms;
   GEO_Point * ppt;
   UT_Matrix4 xform(1.0);
   UT_XformOrder xformOrder;
   UT_Matrix3 rot_xform(1.0);
   UT_Vector3 myDir = myPointAttributes.N;
   UT_Vector3 myUp = UT_Vector3(0,1,0);
   rot_xform.orient(myDir, myUp);
   xform = rot_xform;

   grid_parms.rows = 2;
   grid_parms.cols = 2;
   grid_parms.xsize = mySize[0] * myPointAttributes.pscale;
   grid_parms.ysize = mySize[1] * myPointAttributes.pscale;
   grid_parms.xcenter = myPointAttributes.myNewPos[0];
   grid_parms.ycenter = myPointAttributes.myNewPos[1];
   grid_parms.zcenter = myPointAttributes.myNewPos[2];
   grid_parms.plane = GU_PLANE_XY;
   myGrid = inst_gdp->buildGrid(grid_parms, GU_GRID_POLY);

   for(int i=0; i < myGrid->getVertexCount(); i++) {
         ppt = myGrid->getVertexElement(i).getPt();
         UT_Vector4  P = ppt->getPos();
         P *= xform;
         ppt->setPos(P);
      }

   VRAY_clusterThis::setInstanceAttributes(inst_gdp, myGrid);

   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {

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

         VRAY_clusterThis::setInstanceAttributes(mb_gdp, myGrid);
      }

#ifdef DEBUG
   cout << "VRAY_clusterThis::instanceGrid() Instanced a grid " << endl;
#endif

   if(myCVEX_Exec)
      VRAY_clusterThis::runCVEX(inst_gdp, mb_gdp, myCVEXFname, CLUSTER_CVEX_POINT);

   return 0;
}


/* ******************************************************************************
*  Function Name : instanceTube()
*
*  Description :
*
*  Input Arguments : GU_Detail *inst_gdp, GU_Detail *mb_gdp
*
*  Return Value : int
*
***************************************************************************** */
int VRAY_clusterThis::instanceTube(GU_Detail * inst_gdp, GU_Detail * mb_gdp)
{
#ifdef DEBUG
   std::cout << "VRAY_clusterThis::instanceTube()" << std::endl;
#endif

   GU_PrimTube * tube;
   GU_PrimTubeParms tube_parms;
   GU_CapOptions tube_cap_options;
   UT_Matrix4 xform(1.0);
   UT_XformOrder xformOrder;

//    GU_CapOptions:
//GU_CapType   firstUCap
//GU_CapType   lastUCap
//GU_CapType   firstVCap
//GU_CapType   lastVCap
//int    pshapeU
//int    numfirstUCaps
//int    numlastUCaps
//int    numfirstVCaps
//int    numlastVCaps
//int    pshapeV
//float  firstUScale
//float  lastUScale
//float  firstVScale
//float  lastVScale

// enum     GU_CapType:
//GU_CAP_NONE = 0, GU_CAP_FACETED = 1, GU_CAP_SHARED = 2, GU_CAP_ROUNDED = 3, GU_CAP_TANGENT = 4
//
// GU_PrimTubeParms:
//int    rows
//int    cols
//int    orderu
//int    orderv
//int    imperfect
//float  taper
//GEO_SurfaceType    type
//
// GEO_SurfaceType:
//GEO_PATCH_ROWS
//GEO_PATCH_COLS
//GEO_PATCH_ROWCOL
//GEO_PATCH_TRIANGLE
//GEO_PATCH_QUADS
//GEO_PATCH_ALTTRIANGLE
//GEO_PATCH_REVTRIANGLE

   tube_parms.gdp = inst_gdp;
//   tube_cap_options.numfirstUCaps = 1;
//   tube_cap_options.numlastUCaps = 1;
//   tube_cap_options.firstUCap = GU_CAP_SHARED;
//   tube_cap_options.lastUCap = GU_CAP_SHARED;
//   tube_cap_options.numfirstVCaps = 1;
//   tube_cap_options.numlastVCaps = 1;
//   tube_cap_options.firstVCap = GU_CAP_ROUNDED;
//   tube_cap_options.lastVCap = GU_CAP_ROUNDED;
//   tube_parms.rows = 4;
//   tube_parms.cols = 10;
//   tube_parms.taper = 1.0;
//   tube_parms.type = GEO_PATCH_QUADS;

   tube_parms.xform = xform;
   tube_parms.xform.scale(mySize[0] * myPointAttributes.pscale, mySize[1] * myPointAttributes.pscale, mySize[2] * myPointAttributes.pscale);
   tube_parms.xform.rotate(myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2], xformOrder);
   tube_parms.xform.translate(myPointAttributes.myNewPos[0], myPointAttributes.myNewPos[1], myPointAttributes.myNewPos[2]);
   tube = (GU_PrimTube *) GU_PrimTube::build(tube_parms, tube_cap_options);

   VRAY_clusterThis::setInstanceAttributes(inst_gdp, tube);

   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
         tube_parms.gdp = mb_gdp;
         tube_parms.xform.scale(mySize[0] * myPointAttributes.pscale, mySize[1] * myPointAttributes.pscale, mySize[2] * myPointAttributes.pscale);
         tube_parms.xform.rotate(myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2], xformOrder);
         tube_parms.xform.translate(myPointAttributes.myMBPos[0], myPointAttributes.myMBPos[1], myPointAttributes.myMBPos[2]);
         tube = (GU_PrimTube *) GU_PrimTube::build(tube_parms, tube_cap_options);

         VRAY_clusterThis::setInstanceAttributes(mb_gdp, tube);
      }

#ifdef DEBUG
   cout << "VRAY_clusterThis::instanceTube() Instanced a tube " << endl;
#endif

   if(myCVEX_Exec)
      VRAY_clusterThis::runCVEX(inst_gdp, mb_gdp, myCVEXFname, CLUSTER_CVEX_POINT);

   return 0;
}


/* ******************************************************************************
*  Function Name : instanceCircle()
*
*  Description :
*
*  Input Arguments : GU_Detail *inst_gdp, GU_Detail *mb_gdp
*
*  Return Value : int
*
***************************************************************************** */
int VRAY_clusterThis::instanceCircle(GU_Detail * inst_gdp, GU_Detail * mb_gdp)
{
#ifdef DEBUG
   std::cout << "VRAY_clusterThis::instanceCircle()" << std::endl;
#endif

   UT_Matrix4 xform(1.0);
   GU_PrimCircle * circle;
   GU_PrimCircleParms circle_parms;
   UT_XformOrder xformOrder;

   circle_parms.gdp = inst_gdp;
   circle_parms.xform = xform;
   circle_parms.xform.scale(mySize[0] * myPointAttributes.pscale, mySize[1] * myPointAttributes.pscale, mySize[2] * myPointAttributes.pscale);
   circle_parms.xform.rotate(myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2], xformOrder);
   circle_parms.xform.translate(myPointAttributes.myNewPos[0], myPointAttributes.myNewPos[1], myPointAttributes.myNewPos[2]);
   circle = (GU_PrimCircle *) GU_PrimCircle::build(circle_parms);

   VRAY_clusterThis::setInstanceAttributes(inst_gdp, circle);

   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
         circle_parms.gdp = mb_gdp;
         circle_parms.xform = xform;
         circle_parms.xform.scale(mySize[0] * myPointAttributes.pscale, mySize[1] * myPointAttributes.pscale, mySize[2] * myPointAttributes.pscale);
         circle_parms.xform.rotate(myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2], xformOrder);
         circle_parms.xform.translate(myPointAttributes.myMBPos[0], myPointAttributes.myMBPos[1], myPointAttributes.myMBPos[2]);
         circle = (GU_PrimCircle *) GU_PrimCircle::build(circle_parms);

         VRAY_clusterThis::setInstanceAttributes(mb_gdp, circle);
      }

#ifdef DEBUG
   cout << "VRAY_clusterThis::instanceCircle() Instanced a circle " << endl;
#endif

   if(myCVEX_Exec)
      VRAY_clusterThis::runCVEX(inst_gdp, mb_gdp, myCVEXFname, CLUSTER_CVEX_POINT);

   return 0;
}



/* ******************************************************************************
*  Function Name : instanceCurve()
*
*  Description :
*
*  Input Arguments : GU_Detail *inst_gdp, GU_Detail *mb_gdp
*
*  Return Value : int
*
***************************************************************************** */
int VRAY_clusterThis::instanceCurve(GU_Detail * inst_gdp, GU_Detail * mb_gdp, fpreal theta, long int point_num)
{
#ifdef DEBUG
   std::cout << "VRAY_clusterThis::instanceCurve()" << std::endl;
#endif

   GEO_Point * ppt;
   GU_PrimNURBCurve * myCurve, * myMBCurve;
   uint32 num_vtx;
   int myCurvePointNum = 0;
   UT_Vector4 pt_pos;

   UT_Vector3 myCd(0.0, 1.0, 0.0);
   UT_Vector3 myV(0.1, 1.1, 0.1);
   UT_Vector3 myN(0.0, 0.0, 1.0);

   GA_RWAttributeRef pt_Cd = inst_gdp->addDiffuseAttribute(GEO_POINT_DICT);
   GA_RWAttributeRef pt_Alpha = inst_gdp->addAlphaAttribute(GEO_POINT_DICT);
   GA_RWAttributeRef pt_v = inst_gdp->addVelocityAttribute(GEO_POINT_DICT);
   GA_RWAttributeRef pt_N = inst_gdp->addNormalAttribute(GEO_POINT_DICT);
   GA_RWAttributeRef pt_width = inst_gdp->addFloatTuple(GA_ATTRIB_POINT, "width", 1);
   GA_RWAttributeRef pt_material = inst_gdp->addStringTuple(GA_ATTRIB_POINT, "shop_materialpath", 1);

   num_vtx = ((myNumCopies * myRecursion) > 4)?(myNumCopies * myRecursion):4;
   myCurve = (GU_PrimNURBCurve *)GU_PrimNURBCurve::build((GU_Detail *)inst_gdp, num_vtx, 4, 0, 1, 1);

//   cout << "VRAY_clusterThis::instanceCurve() - num vertices: " << myCurve->getVertexCount() << endl;;
//   cout << "id: "  << myCurve->getPrimitiveId() << endl;
//   cout << "breakCount: "  << myCurve->breakCount() << endl;


   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
         myMBCurve = (GU_PrimNURBCurve *)GU_PrimNURBCurve::build((GU_Detail *)mb_gdp, num_vtx, 4, 0, 1, 1);
         VRAY_clusterThis::setInstanceAttributes(mb_gdp, myMBCurve);
      }


   for(int copyNum = 0; copyNum < myNumCopies; copyNum++)
      for(int recursionNum = 0; recursionNum < myRecursion; recursionNum++) {

            calculateNewPosition(theta, copyNum, recursionNum);
            ppt = myCurve->getVertexElement(myCurvePointNum).getPt();
            ppt->setPos((float)myPointAttributes.myNewPos[0],
                        (float)myPointAttributes.myNewPos[1],
                        (float)myPointAttributes.myNewPos[2], 1.0);

            // Assign attributes to each point
            ppt->setValue<UT_Vector3>(pt_Cd, (const UT_Vector3)myCd);
            ppt->setValue<fpreal>(pt_Alpha, (const fpreal)myPointAttributes.Alpha);
            ppt->setValue<UT_Vector3>(pt_v, (const UT_Vector3)myPointAttributes.v);
            ppt->setValue<UT_Vector3>(pt_N, (const UT_Vector3)myPointAttributes.N);
            ppt->setValue<fpreal>(pt_width, (const fpreal)myPointAttributes.width);
//            ppt->setValue<int>(pt_id, (const int)myPointAttributes.id);
//            ppt->setValue<int>(pt_inst_id, (const int)myCurvePointNum);
            ppt->setString(pt_material, myPointAttributes.material);

//            if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
//                  ppt = myMBCurve->getVertexElement(myCurvePointNum).getPt();
//                  ppt->setPos((float)myPointAttributes.myNewPos[0],
//                              (float)myPointAttributes.myMBPos[1],
//                              (float)myPointAttributes.myMBPos[2], 1.0);
//
//                  // Assign attributes to each point
//                  ppt->setValue<UT_Vector3>(pt_Cd, (const UT_Vector3)myPointAttributes.Cd);
//                  ppt->setValue<fpreal>(pt_Alpha, (const fpreal)myPointAttributes.Alpha);
//                  ppt->setValue<UT_Vector3>(pt_v, (const UT_Vector3)myPointAttributes.v);
////                  ppt->setValue<UT_Vector3>(pt_N, (const UT_Vector3)myPointAttributes.N);
//                  ppt->setValue<fpreal>(pt_width, (const fpreal)myPointAttributes.width);
////            ppt->setValue<int>(myInstAttrRefs.pointId, (const int)myPointAttributes.id);
////            ppt->setValue<int>(myInstAttrRefs.pointInstId, (const int)myInstanceNum);
//                  ppt->setString(pt_material, myPointAttributes.material);
//
//               }
//
//            std::cout << "VRAY_clusterThis::instanceCurve() myCurvePointNum: " << myCurvePointNum << std::endl;

            myCurvePointNum++;
         }

//   myCurve->close ();
//   myMBCurve->close ();


#ifdef DEBUG
   cout << "VRAY_clusterThis::instanceCurve() - Instanced a curve " << endl;
#endif

   if(myCVEX_Exec)
      VRAY_clusterThis::runCVEX(inst_gdp, mb_gdp, myCVEXFname, CLUSTER_CVEX_POINT);

   return 0;
}



/* ******************************************************************************
*  Function Name : instanceMetaball()
*
*  Description :
*
*  Input Arguments : GU_Detail *inst_gdp, GU_Detail *mb_gdp
*
*  Return Value : int
*
***************************************************************************** */
int VRAY_clusterThis::instanceMetaball(GU_Detail * inst_gdp, GU_Detail * mb_gdp)
{

#ifdef DEBUG
   std::cout << "VRAY_clusterThis::instanceMetaball()" << std::endl;
#endif

   UT_Matrix4 xform(1.0);
   UT_XformOrder xformOrder;
   GU_PrimMetaBall * metaball;
   GU_PrimMetaBallParms metaball_parms;

   metaball_parms.gdp = inst_gdp;
   metaball_parms.xform = xform;
   metaball_parms.weight = myPointAttributes.weight;
   metaball_parms.xform.scale(mySize[0] * myPointAttributes.pscale, mySize[1] * myPointAttributes.pscale, mySize[2] * myPointAttributes.pscale);
   metaball_parms.xform.rotate(myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2], xformOrder);
   metaball_parms.xform.translate(myPointAttributes.myNewPos[0], myPointAttributes.myNewPos[1], myPointAttributes.myNewPos[2]);
   metaball = (GU_PrimMetaBall *) GU_PrimMetaBall::build(metaball_parms);


   VRAY_clusterThis::setInstanceAttributes(inst_gdp, metaball);

   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {

         metaball_parms.gdp = mb_gdp;
         metaball_parms.xform = xform;
         metaball_parms.weight = myPointAttributes.weight;
         metaball_parms.xform.scale(mySize[0] * myPointAttributes.pscale, mySize[1] * myPointAttributes.pscale, mySize[2] * myPointAttributes.pscale);
         metaball_parms.xform.rotate(myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2], xformOrder);
         metaball_parms.xform.translate(myPointAttributes.myMBPos[0], myPointAttributes.myMBPos[1], myPointAttributes.myMBPos[2]);
         metaball = (GU_PrimMetaBall *) GU_PrimMetaBall::build(metaball_parms);

         VRAY_clusterThis::setInstanceAttributes(mb_gdp, metaball);

      }

#ifdef DEBUG
   cout << "VRAY_clusterThis::instanceMetaball() Instanced a metaball " << endl;
#endif

   if(myCVEX_Exec)
      VRAY_clusterThis::runCVEX(inst_gdp, mb_gdp, myCVEXFname, CLUSTER_CVEX_POINT);

   return 0;
}


/* ******************************************************************************
*  Function Name : instanceFile()
*
*  Description :  Instance the geometry from a BGEO file
*
*  Input Arguments : GU_Detail *file_gdp, GU_Detail *inst_gdp, GU_Detail *mb_gdp
*
*  Return Value : int
*
***************************************************************************** */
int VRAY_clusterThis::instanceFile(GU_Detail * file_gdp, GU_Detail * inst_gdp, GU_Detail * mb_gdp)
{
#ifdef DEBUG
   std::cout << "VRAY_clusterThis::instanceFile()" << std::endl;
#endif

   GU_Detail temp_gdp(file_gdp), null_gdp;
   UT_Matrix4 xform(1.0);
   UT_Matrix3 rot_xform(1.0);
   UT_XformOrder xformOrder;

   UT_Vector3 myDir = myPointAttributes.N;
   myDir.normalize();
   UT_Vector3 myUp = UT_Vector3(0,1,0);

// Transform the geo to the new position
   rot_xform.orient(myDir, myUp);
   xform = rot_xform;

   xform.scale(mySize[0] * myPointAttributes.pscale, mySize[1] * myPointAttributes.pscale, mySize[2] * myPointAttributes.pscale);
//    xform.rotate(myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2], xformOrder);
   xform.translate(myPointAttributes.myNewPos[0], myPointAttributes.myNewPos[1], myPointAttributes.myNewPos[2]);
//   xform.xform(xformOrder, myPointAttributes.myNewPos[0], myPointAttributes.myNewPos[1], myPointAttributes.myNewPos[2],
//               myPointAttributes.N[0], myPointAttributes.N[1], myPointAttributes.N[2],
//               mySize[0], mySize[1], mySize[2]);

   temp_gdp.transform(xform);

//   GU_Detail theFileGDP(theFiles[myPointAttributes.lod][myPointAttributes.anim_frame]);
   addFileAttributeOffsets(&temp_gdp);
   setFileAttributes(&temp_gdp);

   // Run CVEX function on this instance
   if(myCVEX_Exec)
      VRAY_clusterThis::runCVEX(&temp_gdp, &null_gdp, myCVEXFname, CLUSTER_CVEX_POINT);

   if(myCVEX_Exec_prim)
      VRAY_clusterThis::runCVEX(&temp_gdp, &null_gdp, myCVEXFname_prim, CLUSTER_CVEX_PRIM);

   inst_gdp->merge(temp_gdp);

   if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
         GU_Detail temp_gdp(file_gdp);

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

         // Run CVEX function on this instance
         if(myCVEX_Exec)
            VRAY_clusterThis::runCVEX(&temp_gdp, &null_gdp, myCVEXFname, CLUSTER_CVEX_POINT);

         if(myCVEX_Exec_prim)
            VRAY_clusterThis::runCVEX(&temp_gdp, &null_gdp, myCVEXFname_prim, CLUSTER_CVEX_PRIM);

         temp_gdp.transform(xform);
         mb_gdp->merge(temp_gdp);
      }


#ifdef DEBUG
   cout << "VRAY_clusterThis::instanceFile() Instanced a file " << endl;
#endif

   return 0;
}


#endif


/**********************************************************************************/
//  $Log: VRAY_clusterThisInstance.C,v $
//  Revision 1.20  2012-09-09 05:00:54  mstory
//  More cleanup and testing.
//
//  Revision 1.19  2012-09-07 15:39:23  mstory
//   Removed all volume instancing (used in different project) and continu… …
//
//  …ed H12 modifications.
//
//  --mstory
//
//  Revision 1.18  2012-09-05 23:02:38  mstory
//  Modifications for H12.
//
//  Revision 1.17  2012-09-04 03:25:28  mstory
//  .
//
//  Revision 1.14  2011-02-15 00:59:15  mstory
//  Refactored out rededundant attribute code in the child (deferred) instancicng mode.
//  Made remaining changes for H11 (and beyond) versions way of handiling attributes.
//
//
//  --mstory
//
//  Revision 1.13  2011-02-06 22:35:15  mstory
//  Fixed the exit processing function.
//
//  Ready for release 1.5.1
//
//  Revision 1.12  2011-02-06 19:49:15  mstory
//  Modified for Houdini version 11.
//
//  Refactored a lot of the attribute code, cleaned up odds and ends.
//
//  Revision 1.11  2010-04-12 06:39:42  mstory
//  Finished CVEX modifications.
//
//  Revision 1.10  2010-04-10 10:11:42  mstory
//  Added additional CVEX processing.  Fixed a few annoying bugs.  Adding external disk geo source.
//
//  Revision 1.9  2010-02-26 08:06:32  mstory
//  Adding more CVEX options.
//
//  Revision 1.8  2010-02-23 08:36:22  mstory
//  Fixed most of the CVEX problems with primtive instancng.  Fixed seg faults from uninitilialized pointers in the CVEX variables,
//
//  Revision 1.7  2009-11-20 14:59:57  mstory
//  Release 1.4.7 ready.
//
//  Revision 1.6  2009-11-19 16:26:51  mstory
//  Adding point inst id to child objects (for deferred instancing), need to add to prims as well.
//
//  Revision 1.5  2009-11-16 17:47:12  mstory
//  Fixed the curve instancing, still need to determine all attribites required for the curve (i.e. width)
//
//  Revision 1.4  2009-11-16 08:32:45  mstory
//  Added instance ID for each instance passed to CVEX processing.
//
//  Revision 1.3  2009-04-06 17:13:44  mstory
//  Clean up code a bit.
//
//  Revision 1.2  2009-04-06 16:40:58  mstory
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
//  Revision 1.1  2009-02-10 21:56:55  mstory
//  Added seperate files for the CVEX utilities and the instancing methods.
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










