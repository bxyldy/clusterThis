/* ******************************************************************************
*
*  VRAY_clusterThisAttributeUtils
*
* $RCSfile: VRAY_clusterThisAttributeUtils.C,v $
*
* Description :
*
* $Revision: 1.13 $
*
* $Source: /dca/cvsroot/houdini/VRAY_clusterThis/VRAY_clusterThisAttributeUtils.C,v $
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

#ifndef __VRAY_clusterThisAttributeUtils_C__
#define __VRAY_clusterThisAttributeUtils_C__


/* ******************************************************************************
*  Function Name : createAttributeOffsets()
*
*  Description :   Create the attribute the offsets for the instanced objects
*
*  Input Arguments : GU_Detail *inst_gdp
*
*  Return Value : int status
*
***************************************************************************** */
void VRAY_clusterThis::createAttributeOffsets(GU_Detail * inst_gdp, GU_Detail * mb_gdp)
{

#ifdef DEBUG
   cout << "VRAY_clusterThis::createAttributeOffsets() " << endl;
#endif


// TODO: Check all these references with *.isValid() and throw exeception if it fails !!!


   if((myPrimType != CLUSTER_POINT) && (myMethod == CLUSTER_INSTANCE_NOW)) {

#ifdef DEBUG
         cout << "VRAY_clusterThis::createAttributeOffsets() Creating primitive attributes" << endl;
#endif

         myInstAttrRefs.Cd = inst_gdp->addDiffuseAttribute(GEO_PRIMITIVE_DICT);
         myInstAttrRefs.Alpha = inst_gdp->addAlphaAttribute(GEO_PRIMITIVE_DICT);
         myInstAttrRefs.v = inst_gdp->addVelocityAttribute(GEO_PRIMITIVE_DICT);
         myInstAttrRefs.N = inst_gdp->addNormalAttribute(GEO_PRIMITIVE_DICT);
         myInstAttrRefs.pscale = inst_gdp->addFloatTuple(GA_ATTRIB_PRIMITIVE, "pscale", 1);
         myInstAttrRefs.id = inst_gdp->addIntTuple(GA_ATTRIB_PRIMITIVE, "id", 1);
         myInstAttrRefs.inst_id = inst_gdp->addIntTuple(GA_ATTRIB_PRIMITIVE, "inst_id", 1);
         myInstAttrRefs.material = inst_gdp->addStringTuple(GA_ATTRIB_PRIMITIVE, "shop_materialpath", 1);

         myInstAttrRefs.pointCd = inst_gdp->addDiffuseAttribute(GEO_POINT_DICT);
         myInstAttrRefs.pointAlpha = inst_gdp->addAlphaAttribute(GEO_POINT_DICT);
         myInstAttrRefs.pointV = inst_gdp->addVelocityAttribute(GEO_POINT_DICT);
         myInstAttrRefs.pointN = inst_gdp->addNormalAttribute(GEO_POINT_DICT);
         myInstAttrRefs.pointPscale = inst_gdp->addFloatTuple(GA_ATTRIB_POINT, "pscale", 1);
         myInstAttrRefs.pointId = inst_gdp->addIntTuple(GA_ATTRIB_POINT, "id", 1);
         myInstAttrRefs.pointInstId = inst_gdp->addIntTuple(GA_ATTRIB_POINT, "inst_id", 1);
         myInstAttrRefs.pointMaterial = inst_gdp->addStringTuple(GA_ATTRIB_POINT, "shop_materialpath", 1);

         if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
               myInstMBAttrOffsets.Cd = mb_gdp->addDiffuseAttribute(GEO_PRIMITIVE_DICT);
               myInstMBAttrOffsets.Alpha = mb_gdp->addAlphaAttribute(GEO_PRIMITIVE_DICT);
               myInstMBAttrOffsets.v = mb_gdp->addVelocityAttribute(GEO_PRIMITIVE_DICT);
               myInstMBAttrOffsets.N = mb_gdp->addNormalAttribute(GEO_PRIMITIVE_DICT);
               myInstMBAttrOffsets.pscale = mb_gdp->addFloatTuple(GA_ATTRIB_PRIMITIVE, "pscale", 1);
               myInstMBAttrOffsets.id = mb_gdp->addIntTuple(GA_ATTRIB_PRIMITIVE, "id", 1);
               myInstMBAttrOffsets.inst_id = mb_gdp->addIntTuple(GA_ATTRIB_PRIMITIVE, "inst_id", 1);
               myInstMBAttrOffsets.material = mb_gdp->addStringTuple(GA_ATTRIB_PRIMITIVE, "shop_materialpath", 1);

               myInstMBAttrOffsets.pointCd = mb_gdp->addDiffuseAttribute(GEO_POINT_DICT);
               myInstMBAttrOffsets.pointAlpha = mb_gdp->addAlphaAttribute(GEO_POINT_DICT);
               myInstMBAttrOffsets.pointV = mb_gdp->addVelocityAttribute(GEO_POINT_DICT);
               myInstMBAttrOffsets.pointN = mb_gdp->addNormalAttribute(GEO_POINT_DICT);
               myInstMBAttrOffsets.pointPscale = mb_gdp->addFloatTuple(GA_ATTRIB_POINT, "pscale", 1);
               myInstMBAttrOffsets.pointId = mb_gdp->addIntTuple(GA_ATTRIB_POINT, "id", 1);
               myInstMBAttrOffsets.pointInstId = mb_gdp->addIntTuple(GA_ATTRIB_POINT, "inst_id", 1);
               myInstMBAttrOffsets.pointMaterial = mb_gdp->addStringTuple(GA_ATTRIB_POINT, "shop_materialpath", 1);
            }
      }



   // Do the point attribute creation

   if((myPrimType == CLUSTER_POINT) && (myMethod == CLUSTER_INSTANCE_NOW)) {
#ifdef DEBUG
         cout << "VRAY_clusterThis::createAttributeOffsets() Creating Point attributes" << endl;
#endif

         myInstAttrRefs.pointCd = inst_gdp->addDiffuseAttribute(GEO_POINT_DICT);
         myInstAttrRefs.pointAlpha = inst_gdp->addAlphaAttribute(GEO_POINT_DICT);
         myInstAttrRefs.pointV = inst_gdp->addVelocityAttribute(GEO_POINT_DICT);
         myInstAttrRefs.pointN = inst_gdp->addNormalAttribute(GEO_POINT_DICT);
         myInstAttrRefs.pointPscale = inst_gdp->addFloatTuple(GA_ATTRIB_POINT, "pscale", 1);
         myInstAttrRefs.pointId = inst_gdp->addIntTuple(GA_ATTRIB_POINT, "id", 1);
         myInstAttrRefs.pointInstId = inst_gdp->addIntTuple(GA_ATTRIB_POINT, "inst_id", 1);
         myInstAttrRefs.pointMaterial = inst_gdp->addStringTuple(GA_ATTRIB_POINT, "shop_materialpath", 1);

         if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
               myInstMBAttrOffsets.pointCd = mb_gdp->addDiffuseAttribute(GEO_POINT_DICT);
               myInstMBAttrOffsets.pointAlpha = mb_gdp->addAlphaAttribute(GEO_POINT_DICT);
               myInstMBAttrOffsets.pointV = mb_gdp->addVelocityAttribute(GEO_POINT_DICT);
               myInstMBAttrOffsets.pointN = mb_gdp->addNormalAttribute(GEO_POINT_DICT);
               myInstMBAttrOffsets.pointPscale = mb_gdp->addFloatTuple(GA_ATTRIB_POINT, "pscale", 1);
               myInstMBAttrOffsets.pointId = mb_gdp->addIntTuple(GA_ATTRIB_POINT, "id", 1);
               myInstMBAttrOffsets.pointInstId = mb_gdp->addIntTuple(GA_ATTRIB_POINT, "inst_id", 1);
               myInstMBAttrOffsets.pointMaterial = mb_gdp->addStringTuple(GA_ATTRIB_POINT, "shop_materialpath", 1);
            }
      }
}



/* ******************************************************************************
*  Function Name : getAttributeOffsets()
*
*  Description :   Get the attribute the offsets of the incoming geometry
*
*  Input Arguments : GU_Detail *inst_gdp
*
*  Return Value : int status
*
***************************************************************************** */
int VRAY_clusterThis::getAttributeOffsets(GU_Detail * inst_gdp)
{

#ifdef DEBUG
   cout << "VRAY_clusterThis::getAttributeOffsets() " << endl;
#endif

   try {

         myPointAttrOffsets.Cd = inst_gdp->findDiffuseAttribute(GEO_POINT_DICT);
         myPointAttrOffsets.Alpha = inst_gdp->findAlphaAttribute(GEO_POINT_DICT);

         myPointAttrOffsets.v = inst_gdp->findFloatTuple(GA_ATTRIB_POINT, "v", 3);
         myPointAttrOffsets.N = inst_gdp->findFloatTuple(GA_ATTRIB_POINT, "N", 3);
         myPointAttrOffsets.orient = inst_gdp->findFloatTuple(GA_ATTRIB_POINT, "orient", 4);
         myPointAttrOffsets.pscale = inst_gdp->findFloatTuple(GA_ATTRIB_POINT, "pscale", 1);
         myPointAttrOffsets.id = inst_gdp->findIntTuple(GA_ATTRIB_POINT, "id", 1);

         if(myPrimType == CLUSTER_PRIM_METABALL)
            myPointAttrOffsets.weight = inst_gdp->findFloatTuple(GA_ATTRIB_POINT, "weight", 1);

         myPointAttrOffsets.material = inst_gdp->findStringTuple(GA_ATTRIB_POINT, "shop_materialpath");


         /*    myPointAttrOffsets.uv = inst_gdp->findTextureAttribute ( GEO_POINT_DICT );
             myPointAttrOffsets.force = inst_gdp->pointAttribs().getOffset ( "force", GB_ATTRIB_VECTOR );
             myPointAttrOffsets.vorticity = inst_gdp->pointAttribs().getOffset ( "vorticity", GB_ATTRIB_VECTOR );
             myPointAttrOffsets.age = inst_gdp->pointAttribs().getOffset ( "age", GB_ATTRIB_FLOAT );
             myPointAttrOffsets.viscosity = inst_gdp->pointAttribs().getOffset ( "viscosity", GB_ATTRIB_FLOAT );
             myPointAttrOffsets.pressure = inst_gdp->pointAttribs().getOffset ( "pressure", GB_ATTRIB_FLOAT );
             myPointAttrOffsets.density = inst_gdp->pointAttribs().getOffset ( "density", GB_ATTRIB_FLOAT );
             myPointAttrOffsets.mass = inst_gdp->pointAttribs().getOffset ( "mass", GB_ATTRIB_FLOAT );
             myPointAttrOffsets.temperature = inst_gdp->pointAttribs().getOffset ( "temperature", GB_ATTRIB_FLOAT );
             myPointAttrOffsets.num_neighbors = inst_gdp->pointAttribs().getOffset ( "num_neighbors", GB_ATTRIB_INT );*/

      }
   // Process exceptions ...
   catch(VRAY_clusterThis_Exception e) {
         e.what();
         cout << "VRAY_clusterThis::getAttributeOffsets() - Exception encountered, exiting" << endl << endl;
         return 1;
      }


#ifdef DEBUG
   cout << "Geometry attribute offsets:" << endl;
   cout << "Cd: " << myPointAttrOffsets.Cd.isValid() << endl;
   cout << "Alpha: " << myPointAttrOffsets.Alpha.isValid() << endl;
   cout << "v: " << myPointAttrOffsets.v.isValid() << endl;
   cout << "N: " << myPointAttrOffsets.N.isValid() << endl;
//    cout << "orient: " << myPointAttrOffsets.orient.isValid() << endl;
   cout << "pscale: " << myPointAttrOffsets.pscale.isValid() << endl;
   cout << "id: " << myPointAttrOffsets.id.isValid() << endl;
   cout << "weight: " << myPointAttrOffsets.weight.isValid() << endl;
   cout << "material: " << myPointAttrOffsets.material.isValid() << endl;

//     cout << "uv: " << myPointAttrOffsets.uv.isValid() << endl;
//     cout << "age: " << myPointAttrOffsets.age.isValid() << endl;
//     cout << "force: " << myPointAttrOffsets.force.isValid() << endl;
//     cout << "viscosity: " << myPointAttrOffsets.viscosity.isValid() << endl;
//     cout << "vorticity: " << myPointAttrOffsets.vorticity.isValid() << endl;
//     cout << "pressure: " << myPointAttrOffsets.pressure.isValid() << endl;
//     cout << "density: " << myPointAttrOffsets.density.isValid() << endl;
//     cout << "mass: " << myPointAttrOffsets.mass.isValid() << endl;
//     cout << "temperature: " << myPointAttrOffsets.temperature.isValid() << endl;
//     cout << "num_neighbors: " << myPointAttrOffsets.num_neighbors.isValid() << endl;
#endif

//    cout << "VRAY_clusterThis::getAttributeOffsets() exiting" << endl;

   return 0;
}


/* ******************************************************************************
*  Function Name : getAttributes()
*
*  Description :
*
*  Input Arguments : GEO_Point *ppt, GU_Detail *inst_gdp
*
*  Return Value : int
*
***************************************************************************** */
inline int VRAY_clusterThis::getAttributes(GEO_Point * ppt, GU_Detail * inst_gdp)
{

#ifdef DEBUG
   cout << "VRAY_clusterThis::getAttributes() " << endl;
#endif

   myPointAttributes.Cd = static_cast<UT_Vector3>(ppt->getValue<UT_Vector3>(myPointAttrOffsets.Cd, 0));
   myPointAttributes.Alpha = static_cast<fpreal>(ppt->getValue<fpreal>(myPointAttrOffsets.Alpha, 0));
   myPointAttributes.v = static_cast<UT_Vector3>(ppt->getValue<UT_Vector3>(myPointAttrOffsets.v, 0));
   myPointAttributes.N = static_cast<UT_Vector3>(ppt->getValue<UT_Vector3>(myPointAttrOffsets.N, 0));
   myPointAttributes.N.normalize();

   myPointAttributes.pscale = static_cast<fpreal>(ppt->getValue<fpreal>(myPointAttrOffsets.pscale, 0));
   myPointAttributes.id = static_cast<int>(ppt->getValue<int>(myPointAttrOffsets.id, 0));

   if(myPrimType == CLUSTER_PRIM_METABALL)
      myPointAttributes.weight = static_cast<fpreal>(ppt->getValue<fpreal>(myPointAttrOffsets.weight, 0));

   myPointAttributes.material = ppt->getString(myPointAttrOffsets.material) ;
//     cout << "VRAY_clusterThis::getAttributes() myPointAttributes.material: " << myPointAttributes.material << endl;

#ifdef DEBUG
   cout << "VRAY_clusterThis::getAttributes() " << "Cd: " << myPointAttributes.Cd << endl;
   cout << "VRAY_clusterThis::getAttributes() " << "Alpha: " << myPointAttributes.Alpha << endl;
   cout << "VRAY_clusterThis::getAttributes() " << "v: " << myPointAttributes.v << endl;
   cout << "VRAY_clusterThis::getAttributes() " << "N: " << myPointAttributes.N << endl;
   cout << "VRAY_clusterThis::getAttributes() " << "orient: " << myPointAttributes.orient << endl;
   cout << "VRAY_clusterThis::getAttributes() " << "pscale: " << myPointAttributes.pscale << endl;
   cout << "VRAY_clusterThis::getAttributes() " << "id: " << myPointAttributes.id << endl;
   cout << "VRAY_clusterThis::getAttributes() " << "weight: " << myPointAttributes.weight << endl;
   cout << "VRAY_clusterThis::get_attributes() " << "material: " << myPointAttributes.material << endl;
// cout << "VRAY_clusterThis::getAttributes() " << "myMaterial: " << myMaterial << endl;
#endif

   return 0;

}


/* ******************************************************************************
*  Function Name : addFileAttributeOffsets()
*
*  Description :   Add attribute offsets for the instanced file
*
*  Input Arguments : GU_Detail *inst_gdp
*
*  Return Value : int status
*
***************************************************************************** */
inline int VRAY_clusterThis::addFileAttributeOffsets(GU_Detail * inst_gdp)
{

#ifdef DEBUG
   cout << "VRAY_clusterThis::addFileAttributeOffsets() " << endl;
#endif

   // Primitive attributes
   myFileAttrOffsets.Cd = inst_gdp->addDiffuseAttribute(GEO_PRIMITIVE_DICT);
   myFileAttrOffsets.Alpha = inst_gdp->addAlphaAttribute(GEO_PRIMITIVE_DICT);
   myFileAttrOffsets.v = inst_gdp->addVelocityAttribute(GEO_PRIMITIVE_DICT);
   myFileAttrOffsets.N = inst_gdp->addNormalAttribute(GEO_PRIMITIVE_DICT);
   myFileAttrOffsets.pscale = inst_gdp->addFloatTuple(GA_ATTRIB_PRIMITIVE, "pscale", 1);
   myFileAttrOffsets.id = inst_gdp->addIntTuple(GA_ATTRIB_PRIMITIVE, "id", 1);
   myFileAttrOffsets.orient = inst_gdp->addFloatTuple(GA_ATTRIB_PRIMITIVE, "orient", 4);


//   myFileAttrOffsets.lod = inst_gdp->addPrimAttrib ( "lod", sizeof ( int ), GB_ATTRIB_INT, 0 );

   //   myInstAttrRefs.material = inst_gdp->addPrimAttrib ( "shop_materialpath", sizeof ( UT_String ), GB_ATTRIB_STRING, 0 );

   // Point attributes
   myFileAttrOffsets.pointCd = inst_gdp->addDiffuseAttribute(GEO_POINT_DICT);
   myFileAttrOffsets.Alpha = inst_gdp->addAlphaAttribute(GEO_POINT_DICT);
   myFileAttrOffsets.pointV = inst_gdp->addVelocityAttribute(GEO_POINT_DICT);

//   myFileAttrOffsets.pointN = inst_gdp->pointAttribs().getOffset ( "N", GB_ATTRIB_VECTOR );
//   if(myFileAttrOffsets.pointN == -1)
//      myFileAttrOffsets.pointN = inst_gdp->addNormalAttribute(GEO_POINT_DICT);

   myFileAttrOffsets.pointPscale = inst_gdp->addFloatTuple(GA_ATTRIB_POINT, "pscale", 1);
   myFileAttrOffsets.pointId = inst_gdp->addIntTuple(GA_ATTRIB_POINT, "id", 1);
   myFileAttrOffsets.lod = inst_gdp->addIntTuple(GA_ATTRIB_POINT, "lod", 1);

   return 0;
}


/* ******************************************************************************
*  Function Name : setPointInstanceAttributes()
*
*  Description :
*
*  Input Arguments : GU_Detail *inst_gdp, GEO_Point *ppt
*
*  Return Value : void
*
***************************************************************************** */
inline void VRAY_clusterThis::setPointInstanceAttributes(GU_Detail * inst_gdp, GEO_Point * ppt)
{

   GA_RWHandleV3 attrVector3Handle;

#ifdef DEBUG
   cout << "VRAY_clusterThis::setPointInstanceAttributes() " << endl;
#endif


   ppt->setValue<UT_Vector3>(myInstAttrRefs.pointCd, (const UT_Vector3)myPointAttributes.Cd);
   ppt->setValue<fpreal>(myInstAttrRefs.pointAlpha, (const fpreal)myPointAttributes.Alpha);
   ppt->setValue<UT_Vector3>(myInstAttrRefs.pointV, (const UT_Vector3)myPointAttributes.v);
   ppt->setValue<UT_Vector3>(myInstAttrRefs.pointN, (const UT_Vector3)myPointAttributes.N);
   ppt->setValue<fpreal>(myInstAttrRefs.pointPscale, (const fpreal)myPointAttributes.pscale);
   ppt->setValue<int>(myInstAttrRefs.pointId, (const int)myPointAttributes.id);
   ppt->setValue<int>(myInstAttrRefs.pointInstId, (const int)myInstanceNum);

   ppt->setString(myInstAttrRefs.pointMaterial, myPointAttributes.material);

//     UT_String mat_str;
//     GEO_AttributeHandle matAttribHandle = inst_gdp->getPointAttribute("shop_materialpath");
//
//     if ( matAttribHandle.isAttributeValid() ) {
//          matAttribHandle.setElement(ppt);
//          matAttribHandle.getString(mat_str);
//          cout << "point shop_materialpath: " << mat_str << endl;
//     }


}



/* ******************************************************************************
*  Function Name : setInstanceAttributes()
*
*  Description : Set the attributes of the instanced primitive
*
*  Input Arguments : GU_Detail *inst_gdp, GEO_Primitive *myGeoPrim
*
*  Return Value : viod
*
***************************************************************************** */
inline void VRAY_clusterThis::setInstanceAttributes(GU_Detail * inst_gdp, GEO_Primitive * myGeoPrim)
{

#ifdef DEBUG
   cout << "VRAY_clusterThis::setInstanceAttributes() " << endl;
#endif
   GEO_Point * ppt;

   myGeoPrim->setValue<UT_Vector3>(myInstAttrRefs.Cd, (const UT_Vector3)myPointAttributes.Cd);
   myGeoPrim->setValue<fpreal>(myInstAttrRefs.Alpha, (const fpreal)myPointAttributes.Alpha);
   myGeoPrim->setValue<UT_Vector3>(myInstAttrRefs.v, (const UT_Vector3)myPointAttributes.v);
   myGeoPrim->setValue<UT_Vector3>(myInstAttrRefs.N, (const UT_Vector3)myPointAttributes.N);
//    myGeoPrim->setValue<UT_Vector4>(myInstAttrRefs.orient (const UT_Vector4)myPointAttributes.orient);
   myGeoPrim->setValue<fpreal>(myInstAttrRefs.pscale, (const fpreal)myPointAttributes.pscale);
   myGeoPrim->setValue<int>(myInstAttrRefs.id, (const int)myPointAttributes.id);
   myGeoPrim->setValue<int>(myInstAttrRefs.inst_id, (const int)myInstanceNum);
   myGeoPrim->setValue<fpreal>(myInstAttrRefs.weight, (const fpreal)myPointAttributes.weight);
   myGeoPrim->setValue<fpreal>(myInstAttrRefs.width, (const fpreal)myPointAttributes.width);

//     std::cout << "VRAY_clusterThis::setInstanceAttributes: myPointAttributes.Alpha: "  << myPointAttributes.Alpha << std::endl;
//
//     GEO_AttributeHandle alphaAttribHandle = inst_gdp->getPrimAttribute("Alpha");
//
//     if ( alphaAttribHandle.isAttributeValid() ) {
//          alphaAttribHandle.setElement(myGeoPrim);
//          cout << "VRAY_clusterThis::setInstanceAttributes: Primitive Instance Alpha: " << alphaAttribHandle.getF() << endl;
//     }

   // apply attribues to each vertex
   for(int i=0; i < myGeoPrim->getVertexCount(); i++) {
         ppt = myGeoPrim->getVertexElement(i).getPt();
         ppt->setValue<UT_Vector3>(myInstAttrRefs.pointCd, (const UT_Vector3)myPointAttributes.Cd);
         ppt->setValue<float>(myInstAttrRefs.pointAlpha, (const float)myPointAttributes.Alpha);
         ppt->setValue<UT_Vector3>(myInstAttrRefs.pointV, (const UT_Vector3)myPointAttributes.v);
         ppt->setValue<UT_Vector3>(myInstAttrRefs.pointN, (const UT_Vector3)myPointAttributes.N);
         ppt->setValue<float>(myInstAttrRefs.pointPscale, (const float)myPointAttributes.pscale);
         ppt->setValue<int>(myInstAttrRefs.pointId, (const int)myPointAttributes.id);
         ppt->setValue<int>(myInstAttrRefs.pointInstId, (const int)myInstanceNum);
         ppt->setString(myInstAttrRefs.pointMaterial, myPointAttributes.material);


//          cout << "VRAY_clusterThis::setInstanceAttributes: Point Instance Alpha: " << ppt->getValue<float>(myInstAttrRefs.pointAlpha) << endl;
      }


//     if (myInstAttrRefs.material.isValid()) {
   myGeoPrim->setString(myInstAttrRefs.material, myPointAttributes.material);
//          std::cout << "prim material set: "  << myPointAttributes.material << std::endl;
//     }

//     UT_String mat_str;
//     GEO_AttributeHandle matAttribHandle = inst_gdp->getPrimAttribute("shop_materialpath");
//
//     if ( matAttribHandle.isAttributeValid() ) {
//          matAttribHandle.setElement(myGeoPrim);
//          matAttribHandle.getString(mat_str);
//          cout << "shop_materialpath: " << mat_str << endl;
//     }

}




/* ******************************************************************************
*  Function Name : setFileAttributes()
*
*  Description :
*
*  Input Arguments : GEO_Point *ppt, GU_Detail *inst_gdp
*
*  Return Value : int
*
***************************************************************************** */
inline int VRAY_clusterThis::setFileAttributes(GU_Detail * inst_gdp)
{

#ifdef DEBUG
   cout << "VRAY_clusterThis::setFileAttributes() " << endl;
#endif

   GEO_Point * ppt;

#ifdef DEBUG
   long int num_points = (long int) inst_gdp->points().entries();
   cout << "VRAY_clusterThis::setFileAttributes() - num points :" << num_points << endl;
#endif

// NOTE: For now. set only velocity (for motion blur), and id attrs for each point.

   GA_FOR_ALL_GPOINTS(inst_gdp, ppt) {
      /*
            Cd = ppt->castAttribData<UT_Vector3> ( myInstAttrRefs.pointCd );
            Cd->assign ( ( fpreal ) myPointAttributes.Cd.x(), ( fpreal ) myPointAttributes.Cd.y(), ( fpreal ) myPointAttributes.Cd.z() );
            Alpha_ptr = ppt->castAttribData<fpreal> ( myInstAttrRefs.pointAlpha );
            *Alpha_ptr = myPointAttributes.Alpha;
      */

      ppt->setValue<UT_Vector3>(myFileAttrOffsets.pointV, (const UT_Vector3)myPointAttributes.v);


      /*
            N = ppt->castAttribData<UT_Vector3> ( myInstAttrRefs.pointN );
            N->assign ( ( fpreal ) myPointAttributes.N.x(), ( fpreal ) myPointAttributes.N.y(), ( fpreal ) myPointAttributes.N.z() );

            pscale_ptr = ppt->castAttribData<fpreal> ( myFileAttrOffsets.pointPscale );
            *pscale_ptr = myPointAttributes.pscale;
      */

      ppt->setValue<int>(myFileAttrOffsets.pointId, (const int)myPointAttributes.id);


//      UT_String* mat = ppt->castAttribData<UT_String> ( myInstAttrRefs.pointMaterial );
//      *mat = myPointAttributes.material;

#ifdef DEBUG
//      cout << "VRAY_clusterThis::setFileAttributes() " << "Cd: " << *Cd << endl;
//      cout << "VRAY_clusterThis::setFileAttributes() " << "Alpha: " << *Alpha_ptr << endl;
//      cout << "VRAY_clusterThis::setFileAttributes() " << "v: " << *vel << endl;
//      cout << "VRAY_clusterThis::setFileAttributes() " << "N: " << myPointAttributes.N << endl;
//      cout << "VRAY_clusterThis::setFileAttributes() " << "pscale: " << myPointAttributes.pscale << endl;
//      cout << "VRAY_clusterThis::setFileAttributes() " << "id: " << myPointAttributes.id << endl;
//   cout << "VRAY_clusterThis::setFileAttributes() " << "material: " << myPointAttributes.material << endl;
// cout << "VRAY_clusterThis::getAttributes() " << "myMaterial: " << myMaterial << endl;
#endif

   }


   GEO_Primitive * prim;

   GA_FOR_ALL_PRIMITIVES(inst_gdp, prim)  {

      prim->setValue<UT_Vector3>(myFileAttrOffsets.Cd, (const UT_Vector3)myPointAttributes.Cd);
      prim->setValue<fpreal>(myFileAttrOffsets.Alpha, (const fpreal)myPointAttributes.Alpha);
      prim->setValue<UT_Vector3>(myFileAttrOffsets.v, (const UT_Vector3)myPointAttributes.v);


// NOTE: File instanced geo should have normals already, do not set the normals to the source point normals

// NOTE: No need to set pscale attr on prims (already in a point attr)

      /*
            N = prim->castAttribData<UT_Vector3> ( myFileAttrOffsets.N );
            N->assign ( ( fpreal ) myPointAttributes.N.x(), ( fpreal ) myPointAttributes.N.y(), ( fpreal ) myPointAttributes.N.z() );
      //      prim->normal(1);
            pscale_ptr = prim->castAttribData<fpreal> ( myFileAttrOffsets.pscale );
            *pscale_ptr = myPointAttributes.pscale;
      */

      prim->setValue<int>(myFileAttrOffsets.id, (const int)myPointAttributes.id);
      GEO_AttributeHandle matAttribHandle = inst_gdp->getPrimAttribute("shop_materialpath");

      if(matAttribHandle.isAttributeValid()) {
            matAttribHandle.setElement(prim);
            matAttribHandle.setString(myPointAttributes.material);
         }


   }


   return 0;

}



#endif


/**********************************************************************************/
//  $Log: VRAY_clusterThisAttributeUtils.C,v $
//  Revision 1.13  2012-09-09 05:00:54  mstory
//  More cleanup and testing.
//
//  Revision 1.12  2012-09-07 15:39:22  mstory
//   Removed all volume instancing (used in different project) and continu… …
//
//  …ed H12 modifications.
//
//  --mstory
//
//  Revision 1.11  2012-09-05 23:02:38  mstory
//  Modifications for H12.
//
//  Revision 1.10  2012-09-04 03:25:28  mstory
//  .
//
//  Revision 1.7  2011-02-15 00:59:15  mstory
//  Refactored out rededundant attribute code in the child (deferred) instancicng mode.
//  Made remaining changes for H11 (and beyond) versions way of handiling attributes.
//
//
//  --mstory
//
//  Revision 1.6  2011-02-06 19:49:15  mstory
//  Modified for Houdini version 11.
//
//  Refactored a lot of the attribute code, cleaned up odds and ends.
//
//  Revision 1.5  2010-04-10 10:11:42  mstory
//  Added additional CVEX processing.  Fixed a few annoying bugs.  Adding external disk geo source.
//
//  Revision 1.4  2010-02-23 08:36:22  mstory
//  Fixed most of the CVEX problems with primtive instancng.  Fixed seg faults from uninitilialized pointers in the CVEX variables,
//
//  Revision 1.3  2009-11-20 14:59:57  mstory
//  Release 1.4.7 ready.
//
//  Revision 1.2  2009-11-16 17:47:12  mstory
//  Fixed the curve instancing, still need to determine all attribites required for the curve (i.e. width)
//
//  Revision 1.1  2009-11-16 08:35:03  mstory
//  Created seperate source files for some of the functions in VRAY_clusterThis.C.
//

/**********************************************************************************/






