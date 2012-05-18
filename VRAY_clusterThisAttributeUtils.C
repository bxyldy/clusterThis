/* ******************************************************************************
*
*  VRAY_clusterThisAttributeUtils
*
* $RCSfile: VRAY_clusterThisAttributeUtils.C,v $
*
* Description :
*
* $Revision: 1.7 $
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
*  Input Arguments : GU_Detail *gdp
*
*  Return Value : int status
*
***************************************************************************** */
void VRAY_clusterThis::createAttributeOffsets(GU_Detail *inst_gdp, GU_Detail *mb_gdp) {

#ifdef DEBUG
    cout << "VRAY_clusterThis::createAttributeOffsets() " << endl;
#endif

      if ((myPrimType != CLUSTER_POINT) && (myMethod == CLUSTER_INSTANCE_NOW)) {

#ifdef DEBUG
    cout << "VRAY_clusterThis::createAttributeOffsets() Creating primitive attributes" << endl;
#endif

          myInstAttrOffsets.Cd = inst_gdp->addDiffuseAttribute ( GEO_PRIMITIVE_DICT );
          myInstAttrOffsets.Alpha = inst_gdp->addAlphaAttribute ( GEO_PRIMITIVE_DICT );
          myInstAttrOffsets.v = inst_gdp->addVelocityAttribute ( GEO_PRIMITIVE_DICT );
          myInstAttrOffsets.N = inst_gdp->addNormalAttribute ( GEO_PRIMITIVE_DICT );
          myInstAttrOffsets.pscale = inst_gdp->addPrimAttrib ( "pscale", sizeof ( fpreal ), GB_ATTRIB_FLOAT, 0 );
          myInstAttrOffsets.id = inst_gdp->addPrimAttrib ( "id", sizeof ( int ), GB_ATTRIB_INT, 0 );
          myInstAttrOffsets.inst_id = inst_gdp->addPrimAttrib ( "inst_id", sizeof ( int ), GB_ATTRIB_INT, 0 );

#if HOUDINI_MAJOR_RELEASE==9                
          myInstAttrOffsets.material = inst_gdp->addPrimAttrib ( "shop_materialpath", sizeof ( UT_String ), GB_ATTRIB_STRING, 0 );
          myInstAttrOffsets.material = inst_gdp->addPrimAttrib ( "shop_vm_surface", sizeof ( UT_String ), GB_ATTRIB_STRING, 0 );
#endif
#if HOUDINI_MAJOR_RELEASE>=11               
          myInstAttrOffsets.material = inst_gdp->addPrimAttrib ( "shop_materialpath", sizeof ( int ), GB_ATTRIB_INDEX, 0 );
          myInstAttrOffsets.material = inst_gdp->addPrimAttrib ( "shop_vm_surface", sizeof ( int ), GB_ATTRIB_INDEX, 0 );
#endif

          myInstAttrOffsets.pointCd = inst_gdp->addDiffuseAttribute(GEO_POINT_DICT);
          myInstAttrOffsets.pointAlpha = inst_gdp->addAlphaAttribute(GEO_POINT_DICT);
          myInstAttrOffsets.pointV = inst_gdp->addVelocityAttribute(GEO_POINT_DICT);
          myInstAttrOffsets.pointN = inst_gdp->addNormalAttribute(GEO_POINT_DICT);
          myInstAttrOffsets.pointPscale = inst_gdp->addPointAttrib( "pscale", sizeof ( fpreal ), GB_ATTRIB_FLOAT, 0 );
          myInstAttrOffsets.pointId = inst_gdp->addPointAttrib( "id", sizeof ( int ), GB_ATTRIB_INT, 0 );
          myInstAttrOffsets.pointInstId = inst_gdp->addPointAttrib( "inst_id", sizeof ( int ), GB_ATTRIB_INT, 0 );
          
#if HOUDINI_MAJOR_RELEASE==9                
          myInstAttrOffsets.pointMaterial = inst_gdp->addPrimAttrib ( "shop_vm_surface", sizeof ( UT_String ), GB_ATTRIB_STRING, 0 );
          myInstAttrOffsets.pointMaterial = inst_gdp->addPrimAttrib ( "shop_materialpath", sizeof ( UT_String ), GB_ATTRIB_STRING, 0 );
#endif
#if HOUDINI_MAJOR_RELEASE>=11               
          myInstAttrOffsets.pointMaterial = inst_gdp->addPrimAttrib ( "shop_vm_surface", sizeof ( int ), GB_ATTRIB_INDEX, 0 );
          myInstAttrOffsets.pointMaterial = inst_gdp->addPrimAttrib ( "shop_materialpath", sizeof ( int ), GB_ATTRIB_INDEX, 0 );
#endif


          if (myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
              myInstMBAttrOffsets.Cd = mb_gdp->addDiffuseAttribute ( GEO_PRIMITIVE_DICT );
              myInstMBAttrOffsets.Alpha = mb_gdp->addAlphaAttribute ( GEO_PRIMITIVE_DICT );
              myInstMBAttrOffsets.v = mb_gdp->addVelocityAttribute ( GEO_PRIMITIVE_DICT );
              myInstMBAttrOffsets.N = mb_gdp->addNormalAttribute ( GEO_PRIMITIVE_DICT );
              myInstMBAttrOffsets.pscale = mb_gdp->addPrimAttrib ( "pscale", sizeof ( fpreal ), GB_ATTRIB_FLOAT, 0 );
              myInstMBAttrOffsets.id = mb_gdp->addPrimAttrib ( "id", sizeof ( int ), GB_ATTRIB_INT, 0 );
              myInstMBAttrOffsets.inst_id = mb_gdp->addPrimAttrib ( "inst_id", sizeof ( int ), GB_ATTRIB_INT, 0 );
              
#if HOUDINI_MAJOR_RELEASE==9                
              myInstMBAttrOffsets.material = mb_gdp->addPrimAttrib ( "shop_vm_surface", sizeof ( UT_String ), GB_ATTRIB_STRING, 0 );
              myInstMBAttrOffsets.material = mb_gdp->addPrimAttrib ( "shop_materialpath", sizeof ( UT_String ), GB_ATTRIB_STRING, 0 );
#endif
#if HOUDINI_MAJOR_RELEASE>=11               
              myInstMBAttrOffsets.material = mb_gdp->addPrimAttrib ( "shop_vm_surface", sizeof ( int ), GB_ATTRIB_INDEX, 0 );
              myInstMBAttrOffsets.material = mb_gdp->addPrimAttrib ( "shop_materialpath", sizeof ( int ), GB_ATTRIB_INDEX, 0 );
#endif
              
              myInstMBAttrOffsets.pointCd = mb_gdp->addDiffuseAttribute(GEO_POINT_DICT);
              myInstMBAttrOffsets.pointAlpha = mb_gdp->addAlphaAttribute(GEO_POINT_DICT);
              myInstMBAttrOffsets.pointV = mb_gdp->addVelocityAttribute(GEO_POINT_DICT);
              myInstMBAttrOffsets.pointN = mb_gdp->addNormalAttribute(GEO_POINT_DICT);
              myInstMBAttrOffsets.pointPscale = mb_gdp->addPointAttrib( "pscale", sizeof ( fpreal ), GB_ATTRIB_FLOAT, 0 );
              myInstMBAttrOffsets.pointId = mb_gdp->addPointAttrib( "id", sizeof ( int ), GB_ATTRIB_INT, 0 );
              myInstMBAttrOffsets.pointInstId = mb_gdp->addPointAttrib( "inst_id", sizeof ( int ), GB_ATTRIB_INT, 0 );
#if HOUDINI_MAJOR_RELEASE==9                
          myInstMBAttrOffsets.pointMaterial = mb_gdp->addPrimAttrib ( "shop_vm_surface", sizeof ( UT_String ), GB_ATTRIB_STRING, 0 );
          myInstMBAttrOffsets.pointMaterial = mb_gdp->addPrimAttrib ( "shop_materialpath", sizeof ( UT_String ), GB_ATTRIB_STRING, 0 );
#endif
#if HOUDINI_MAJOR_RELEASE>=11               
          myInstMBAttrOffsets.pointMaterial = mb_gdp->addPrimAttrib ( "shop_vm_surface", sizeof ( int ), GB_ATTRIB_INDEX, 0 );
          myInstMBAttrOffsets.pointMaterial = mb_gdp->addPrimAttrib ( "shop_materialpath", sizeof ( int ), GB_ATTRIB_INDEX, 0 );
#endif
          }

      }



      // Do the point attribute creation 

      if ((myPrimType == CLUSTER_POINT) && (myMethod == CLUSTER_INSTANCE_NOW)) {

#ifdef DEBUG
    cout << "VRAY_clusterThis::createAttributeOffsets() Creating Point attributes" << endl;
#endif

          myInstAttrOffsets.pointCd = inst_gdp->addDiffuseAttribute(GEO_POINT_DICT);
          myInstAttrOffsets.pointAlpha = inst_gdp->addAlphaAttribute(GEO_POINT_DICT);
          myInstAttrOffsets.pointV = inst_gdp->addVelocityAttribute(GEO_POINT_DICT);
          myInstAttrOffsets.pointN = inst_gdp->addNormalAttribute(GEO_POINT_DICT);
          myInstAttrOffsets.pointPscale = inst_gdp->addPointAttrib( "pscale", sizeof ( fpreal ), GB_ATTRIB_FLOAT, 0 );
          myInstAttrOffsets.pointId = inst_gdp->addPointAttrib( "id", sizeof ( int ), GB_ATTRIB_INT, 0 );
          myInstAttrOffsets.pointInstId = inst_gdp->addPointAttrib ( "inst_id", sizeof ( int ), GB_ATTRIB_INT, 0 );
          
#if HOUDINI_MAJOR_RELEASE==9                
          myInstAttrOffsets.pointMaterial = inst_gdp->addPointAttrib ( "shop_vm_surface", sizeof ( UT_String ), GB_ATTRIB_STRING, 0 );
          myInstAttrOffsets.pointMaterial = inst_gdp->addPointAttrib ( "shop_materialpath", sizeof ( UT_String ), GB_ATTRIB_STRING, 0 );
#endif
#if HOUDINI_MAJOR_RELEASE>=11               
          myInstAttrOffsets.pointMaterial = inst_gdp->addPointAttrib ( "shop_vm_surface", sizeof ( int ), GB_ATTRIB_INDEX, 0 );
          myInstAttrOffsets.pointMaterial = inst_gdp->addPointAttrib ( "shop_materialpath", sizeof ( int ), GB_ATTRIB_INDEX, 0 );
#endif

          if (myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
              myInstMBAttrOffsets.pointCd = mb_gdp->addDiffuseAttribute(GEO_POINT_DICT);
              myInstMBAttrOffsets.pointAlpha = mb_gdp->addAlphaAttribute(GEO_POINT_DICT);
              myInstMBAttrOffsets.pointV = mb_gdp->addVelocityAttribute(GEO_POINT_DICT);
              myInstMBAttrOffsets.pointN = mb_gdp->addNormalAttribute(GEO_POINT_DICT);
              myInstMBAttrOffsets.pointPscale = mb_gdp->addPointAttrib( "pscale", sizeof ( fpreal ), GB_ATTRIB_FLOAT, 0 );
              myInstMBAttrOffsets.pointId = mb_gdp->addPointAttrib( "id", sizeof ( int ), GB_ATTRIB_INT, 0 );
              myInstMBAttrOffsets.pointInstId = mb_gdp->addPrimAttrib ( "inst_id", sizeof ( int ), GB_ATTRIB_INT, 0 );
              
#if HOUDINI_MAJOR_RELEASE==9                
              myInstAttrOffsets.pointMaterial = mb_gdp->addPointAttrib ( "shop_vm_surface", sizeof ( UT_String ), GB_ATTRIB_STRING, 0 );
              myInstAttrOffsets.pointMaterial = mb_gdp->addPointAttrib ( "shop_materialpath", sizeof ( UT_String ), GB_ATTRIB_STRING, 0 );
#endif
#if HOUDINI_MAJOR_RELEASE>=11               
              myInstAttrOffsets.pointMaterial = mb_gdp->addPointAttrib ( "shop_vm_surface", sizeof ( int ), GB_ATTRIB_INDEX, 0 );
              myInstAttrOffsets.pointMaterial = mb_gdp->addPointAttrib ( "shop_materialpath", sizeof ( int ), GB_ATTRIB_INDEX, 0 );
#endif

              
          }
      }


}






/* ******************************************************************************
*  Function Name : getAttributeOffsets()
*
*  Description :   Get the attribute the offsets of the incoming geometry
*
*  Input Arguments : GU_Detail *gdp
*
*  Return Value : int status
*
***************************************************************************** */
int VRAY_clusterThis::getAttributeOffsets(GU_Detail *gdp) {

#ifdef DEBUG
    cout << "VRAY_clusterThis::getAttributeOffsets() " << endl;
#endif

  try {

    myPointAttrOffsets.Cd = gdp->findDiffuseAttribute ( GEO_POINT_DICT );
    myPointAttrOffsets.Alpha = gdp->findAlphaAttribute ( GEO_POINT_DICT );
    myPointAttrOffsets.v = gdp->pointAttribs().getOffset ( "v", GB_ATTRIB_VECTOR );
    myPointAttrOffsets.N = gdp->pointAttribs().getOffset ( "N", GB_ATTRIB_VECTOR );
    myPointAttrOffsets.orient = gdp->pointAttribs().getOffset ( "orient", GB_ATTRIB_VECTOR );
    myPointAttrOffsets.pscale = gdp->pointAttribs().getOffset ( "pscale", GB_ATTRIB_FLOAT );
    myPointAttrOffsets.id = gdp->pointAttribs().getOffset ( "id", GB_ATTRIB_INT );
    if (myPrimType == CLUSTER_PRIM_METABALL)
        myPointAttrOffsets.weight = gdp->pointAttribs().getOffset ( "weight", GB_ATTRIB_FLOAT );

#if HOUDINI_MAJOR_RELEASE==9
//   myPointAttrOffsets.material = gdp->pointAttribs().getOffset ( "shop_materialpath", GB_ATTRIB_INDEX );
  myPointAttrOffsets.material = gdp->pointAttribs().getOffset ( "shop_materialpath", GB_ATTRIB_STRING );
#endif
#if HOUDINI_MAJOR_RELEASE>=11
  myPointAttrOffsets.material = gdp->pointAttribs().getOffset ( "shop_materialpath", GB_ATTRIB_INDEX );
//    myPointAttrOffsets.material = gdp->pointAttribs().getOffset ( "shop_vm_surface", GB_ATTRIB_INDEX );
#endif


//int texOffset = gdp->findPointAttrib( "texname", GB_ATTRIB_INDEX );
//GB_Attribute *texAttr = gdp->pointAttribs().find( "texname", GB_ATTRIB_INDEX );
//int *idx = static_cast< int* >( point->getAttribData( texOffset ) );
//UT_String texVal = texAttr->getIndex( *idx );


/*    myPointAttrOffsets.uv = gdp->findTextureAttribute ( GEO_POINT_DICT );
    myPointAttrOffsets.force = gdp->pointAttribs().getOffset ( "force", GB_ATTRIB_VECTOR );
    myPointAttrOffsets.vorticity = gdp->pointAttribs().getOffset ( "vorticity", GB_ATTRIB_VECTOR );
    myPointAttrOffsets.age = gdp->pointAttribs().getOffset ( "age", GB_ATTRIB_FLOAT );
    myPointAttrOffsets.viscosity = gdp->pointAttribs().getOffset ( "viscosity", GB_ATTRIB_FLOAT );
    myPointAttrOffsets.pressure = gdp->pointAttribs().getOffset ( "pressure", GB_ATTRIB_FLOAT );
    myPointAttrOffsets.density = gdp->pointAttribs().getOffset ( "density", GB_ATTRIB_FLOAT );
    myPointAttrOffsets.mass = gdp->pointAttribs().getOffset ( "mass", GB_ATTRIB_FLOAT );
    myPointAttrOffsets.temperature = gdp->pointAttribs().getOffset ( "temperature", GB_ATTRIB_FLOAT );
    myPointAttrOffsets.num_neighbors = gdp->pointAttribs().getOffset ( "num_neighbors", GB_ATTRIB_INT );*/
    
    }
    // Process exceptions ...
    catch ( VRAY_clusterThis_Exception e ) {
        e.what();
        cout << "VRAY_clusterThis::getAttributeOffsets() - Exception encountered, exiting" << endl << endl;
        return 1;
    }


#ifdef DEBUG
#if HOUDINI_MAJOR_RELEASE==9
    cout << "Geometry attribute offsets:" << endl;
    cout << "Cd: " << myPointAttrOffsets.Cd << endl;
    cout << "Alpha: " << myPointAttrOffsets.Alpha << endl;
    cout << "v: " << myPointAttrOffsets.v << endl;
    cout << "N: " << myPointAttrOffsets.N << endl;
//    cout << "orient: " << myPointAttrOffsets.orient << endl;
    cout << "pscale: " << myPointAttrOffsets.pscale << endl;
    cout << "id: " << myPointAttrOffsets.id << endl;
    cout << "weight: " << myPointAttrOffsets.weight << endl;
    cout << "material: " << myPointAttrOffsets.material << endl;

    cout << "uv: " << myPointAttrOffsets.uv << endl;
    cout << "age: " << myPointAttrOffsets.age << endl;
    cout << "force: " << myPointAttrOffsets.force << endl;
    cout << "viscosity: " << myPointAttrOffsets.viscosity << endl;
    cout << "vorticity: " << myPointAttrOffsets.vorticity << endl;
    cout << "pressure: " << myPointAttrOffsets.pressure << endl;
    cout << "density: " << myPointAttrOffsets.density << endl;
    cout << "mass: " << myPointAttrOffsets.mass << endl;
    cout << "temperature: " << myPointAttrOffsets.temperature << endl;
    cout << "num_neighbors: " << myPointAttrOffsets.num_neighbors << endl;
#endif
#endif


#ifdef DEBUG
#if HOUDINI_MAJOR_RELEASE>=11

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

    cout << "uv: " << myPointAttrOffsets.uv.isValid() << endl;
    cout << "age: " << myPointAttrOffsets.age.isValid() << endl;
    cout << "force: " << myPointAttrOffsets.force.isValid() << endl;
    cout << "viscosity: " << myPointAttrOffsets.viscosity.isValid() << endl;
    cout << "vorticity: " << myPointAttrOffsets.vorticity.isValid() << endl;
    cout << "pressure: " << myPointAttrOffsets.pressure.isValid() << endl;
    cout << "density: " << myPointAttrOffsets.density.isValid() << endl;
    cout << "mass: " << myPointAttrOffsets.mass.isValid() << endl;
    cout << "temperature: " << myPointAttrOffsets.temperature.isValid() << endl;
    cout << "num_neighbors: " << myPointAttrOffsets.num_neighbors.isValid() << endl;
#endif
#endif

//    cout << "VRAY_clusterThis::getAttributeOffsets() exiting" << endl;

    return 0;
}


/* ******************************************************************************
*  Function Name : getAttributes()
*
*  Description :
*
*  Input Arguments : GEO_Point *ppt, GU_Detail *gdp
*
*  Return Value : int
*
***************************************************************************** */
inline int VRAY_clusterThis::getAttributes(GEO_Point *ppt, GU_Detail *gdp) {

#ifdef DEBUG
    cout << "VRAY_clusterThis::getAttributes() " << endl;
#endif

//        myGeoPrim->setValue<UT_Vector3>(myInstAttrOffsets.Cd, (const UT_Vector3)myPointAttributes.Cd);
//       static_cast<int>(ppt->getValue<int>(attrRef, 0))))

#if HOUDINI_MAJOR_RELEASE>=11
    myPointAttributes.Cd = static_cast<UT_Vector3>(ppt->getValue<UT_Vector3>(myPointAttrOffsets.Cd, 0));
#endif
#if HOUDINI_MAJOR_RELEASE==9
    myPointAttributes.Cd = *ppt->castAttribData<UT_Vector3> ( myPointAttrOffsets.Cd );
#endif

#if HOUDINI_MAJOR_RELEASE>=11
    myPointAttributes.Alpha = static_cast<fpreal>(ppt->getValue<fpreal>(myPointAttrOffsets.Alpha, 0));
#endif
#if HOUDINI_MAJOR_RELEASE==9
    myPointAttributes.Alpha = *ppt->castAttribData<fpreal> ( myPointAttrOffsets.Alpha );
#endif

#if HOUDINI_MAJOR_RELEASE>=11
    myPointAttributes.v = static_cast<UT_Vector3>(ppt->getValue<UT_Vector3>(myPointAttrOffsets.v, 0));
#endif
#if HOUDINI_MAJOR_RELEASE==9
    myPointAttributes.v = *ppt->castAttribData<UT_Vector3> ( myPointAttrOffsets.v );
#endif

#if HOUDINI_MAJOR_RELEASE>=11
    myPointAttributes.N = static_cast<UT_Vector3>(ppt->getValue<UT_Vector3>(myPointAttrOffsets.N, 0));
#endif
#if HOUDINI_MAJOR_RELEASE==9
    myPointAttributes.N = *ppt->castAttribData<UT_Vector3> ( myPointAttrOffsets.N );
#endif

    myPointAttributes.N.normalize();

//    myPointAttributes.orient = *ppt->castAttribData<UT_Vector4> ( myPointAttrOffsets.orient );

#if HOUDINI_MAJOR_RELEASE>=11
    myPointAttributes.pscale = static_cast<fpreal>(ppt->getValue<fpreal>(myPointAttrOffsets.pscale, 0));
#endif
#if HOUDINI_MAJOR_RELEASE==9
    myPointAttributes.pscale = *ppt->castAttribData<fpreal> ( myPointAttrOffsets.pscale );
#endif

#if HOUDINI_MAJOR_RELEASE>=11
    myPointAttributes.id = static_cast<int>(ppt->getValue<int>(myPointAttrOffsets.id, 0));
#endif
#if HOUDINI_MAJOR_RELEASE==9
    myPointAttributes.id = *ppt->castAttribData<int> ( myPointAttrOffsets.id );
#endif

    if (myPrimType == CLUSTER_PRIM_METABALL)
#if HOUDINI_MAJOR_RELEASE>=11
        myPointAttributes.weight = static_cast<fpreal>(ppt->getValue<fpreal>(myPointAttrOffsets.weight, 0));
#endif
#if HOUDINI_MAJOR_RELEASE==9
        myPointAttributes.weight = *ppt->castAttribData<fpreal> ( myPointAttrOffsets.weight );
#endif


#if HOUDINI_MAJOR_RELEASE==9
    if (myPointAttrOffsets.material == -1) {
        myPointAttributes.material = myMaterial;
    }

    else {
//   GB_Attribute *myAttr =  gdp->pointAttribs().find( "shop_materialpath", GB_ATTRIB_INDEX );
        GB_Attribute *myAttr =  gdp->pointAttribs().find( "shop_vm_surface", GB_ATTRIB_INDEX );
        int myIndex =  *ppt->castAttribData<int> ( myPointAttrOffsets.material );
        myPointAttributes.material = (const char*) myAttr->getIndex( myIndex );

    cout << "VRAY_clusterThis::getAttributes() material: " << myPointAttributes.material << endl;
    }

#endif


#if HOUDINI_MAJOR_RELEASE>=11
    //if (myPointAttrOffsets.material.isValid()) {
    //    myPointAttributes.material = myMaterial;
    //
    //    //GB_Attribute *myAttr =  gdp->pointAttribs().find( "shop_vm_surface", GB_ATTRIB_INDEX );
    //    //int myIndex =  *ppt->castAttribData<int> ( myPointAttrOffsets.material );
    //    //myPointAttributes.material = (const char*) myAttr->getIndex( myIndex );
    //}
    //else {
        GEO_AttributeHandle matAttribHandle = gdp->getPointAttribute("shop_materialpath");

        if ( matAttribHandle.isAttributeValid() ) {
            matAttribHandle.setElement(ppt);
            matAttribHandle.getString(myPointAttributes.material);
//        }

  cout << "VRAY_clusterThis::getAttributes() material: " << myPointAttributes.material << endl;
    }
#endif


// GEO_AttributeHandle VM_Surface_Attr;
// VM_Surface_Attr = gdp->getAttribute(GEO_POINT_DICT, "shop_vm_surface");
// VM_Surface_Attr = gdp->getAttribute(GEO_POINT_DICT, "shop_materialpath");

//UT_String ShaderString;
//VM_Surface_Attr.getString(ShaderString);
//myPointAttributes.material = ShaderString.steal();

//cout << "VRAY_clusterThis::getAttributes() material: " << myPointAttributes.material << endl;

    /*
    char *pStolenCharString = ShaderString.steal();
    char *pStrTokChar;
    pStrTokChar = strtok(pStolenCharString," ");

    while(pStrTokChar != NULL)
    {
    pStrTokChar = strtok(NULL, " ");
    // cout << "pStrTokChar: " << pStrTokChar << endl;
    }
    */

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
*  Input Arguments : GU_Detail *gdp
*
*  Return Value : int status
*
***************************************************************************** */
inline int VRAY_clusterThis::addFileAttributeOffsets(GU_Detail *gdp) {

#ifdef DEBUG
    cout << "VRAY_clusterThis::addFileAttributeOffsets() " << endl;
#endif

  // Primitive attributes
  myFileAttrOffsets.Cd = gdp->addDiffuseAttribute ( GEO_PRIMITIVE_DICT );
  myFileAttrOffsets.Alpha = gdp->addAlphaAttribute ( GEO_PRIMITIVE_DICT );
  myFileAttrOffsets.v = gdp->addVelocityAttribute ( GEO_PRIMITIVE_DICT );
  myFileAttrOffsets.N = gdp->addNormalAttribute(GEO_PRIMITIVE_DICT);
  myFileAttrOffsets.orient = gdp->addPrimAttrib ( "orient", sizeof ( UT_Vector4 ), GB_ATTRIB_VECTOR, 0 );
  myFileAttrOffsets.pscale = gdp->addPrimAttrib ( "pscale", sizeof ( fpreal ), GB_ATTRIB_FLOAT, 0 );
  myFileAttrOffsets.id = gdp->addPrimAttrib ( "id", sizeof ( int ), GB_ATTRIB_INT, 0 );
//   myFileAttrOffsets.lod = gdp->addPrimAttrib ( "lod", sizeof ( int ), GB_ATTRIB_INT, 0 );

  //   myInstAttrOffsets.material = gdp->addPrimAttrib ( "shop_materialpath", sizeof ( UT_String ), GB_ATTRIB_STRING, 0 );

#if HOUDINI_MAJOR_RELEASE==9                
                myInstAttrOffsets.material = gdp->addPrimAttrib ( "shop_vm_surface", sizeof ( UT_String ), GB_ATTRIB_STRING, 0 );
#endif
#if HOUDINI_MAJOR_RELEASE>=11               
                myInstAttrOffsets.material = gdp->addPrimAttrib ( "shop_vm_surface", sizeof ( UT_String ), GB_ATTRIB_INDEX, 0 );
#endif

  // Point attributes
  myFileAttrOffsets.pointCd = gdp->addDiffuseAttribute ( GEO_POINT_DICT );
  myFileAttrOffsets.Alpha = gdp->addAlphaAttribute ( GEO_POINT_DICT );
  myFileAttrOffsets.pointV = gdp->addVelocityAttribute(GEO_POINT_DICT);

//   myFileAttrOffsets.pointN = gdp->pointAttribs().getOffset ( "N", GB_ATTRIB_VECTOR );
//   if(myFileAttrOffsets.pointN == -1)
//      myFileAttrOffsets.pointN = gdp->addNormalAttribute(GEO_POINT_DICT);

  myFileAttrOffsets.pointPscale = gdp->addPointAttrib( "pscale", sizeof ( fpreal ), GB_ATTRIB_FLOAT, 0 );
  myFileAttrOffsets.pointId = gdp->addPointAttrib( "id", sizeof ( int ), GB_ATTRIB_INT, 0 );
  myFileAttrOffsets.pointLOD = gdp->addPointAttrib( "lod", sizeof ( int ), GB_ATTRIB_INT, 0 );

  return 0;
}


/* ******************************************************************************
*  Function Name : setPointInstanceAttributes()
*
*  Description :
*
*  Input Arguments : GU_Detail *gdp, GEO_Point *ppt
*
*  Return Value : void
*
***************************************************************************** */
inline void VRAY_clusterThis::setPointInstanceAttributes(GU_Detail *gdp, GEO_Point *ppt) {

#ifdef DEBUG
    cout << "VRAY_clusterThis::setPointInstanceAttributes() " << endl;
#endif

#if HOUDINI_MAJOR_RELEASE==9
    UT_Vector3  *Cd, *vel, *N;
    fpreal *pscale_ptr, *Alpha_ptr;
    int *id_ptr, *inst_id_ptr;
#endif

#if HOUDINI_MAJOR_RELEASE==9
    Cd = ppt->castAttribData<UT_Vector3> ( myInstAttrOffsets.pointCd );
    Cd->assign ( ( float ) myPointAttributes.Cd.x(), ( float ) myPointAttributes.Cd.y(), ( float ) myPointAttributes.Cd.z() );
#endif
#if HOUDINI_MAJOR_RELEASE>=11
    ppt->setValue<UT_Vector3>(myInstAttrOffsets.pointCd, (const UT_Vector3)myPointAttributes.Cd);
#endif

#if HOUDINI_MAJOR_RELEASE==9
    Alpha_ptr = ppt->castAttribData<fpreal> ( myInstAttrOffsets.pointAlpha );
    *Alpha_ptr = myPointAttributes.Alpha;
#endif
#if HOUDINI_MAJOR_RELEASE>=11
    ppt->setValue<fpreal>(myInstAttrOffsets.pointAlpha, (const fpreal)myPointAttributes.Alpha);
#endif

#if HOUDINI_MAJOR_RELEASE==9
    vel = ppt->castAttribData<UT_Vector3> ( myInstAttrOffsets.pointV );
    vel->assign ( ( float ) myPointAttributes.v.x(), ( float ) myPointAttributes.v.y(), ( float ) myPointAttributes.v.z() );
#endif
#if HOUDINI_MAJOR_RELEASE>=11
    ppt->setValue<UT_Vector3>(myInstAttrOffsets.pointV, (const UT_Vector3)myPointAttributes.v);
#endif

#if HOUDINI_MAJOR_RELEASE==9
    N = ppt->castAttribData<UT_Vector3> ( myInstAttrOffsets.pointN );
    N->assign ( ( float ) myPointAttributes.N.x(), ( float ) myPointAttributes.N.y(), ( float ) myPointAttributes.N.z() );
#endif
#if HOUDINI_MAJOR_RELEASE>=11
    ppt->setValue<UT_Vector3>(myInstAttrOffsets.pointN, (const UT_Vector3)myPointAttributes.N);
#endif

#if HOUDINI_MAJOR_RELEASE==9
    pscale_ptr = ppt->castAttribData<fpreal> ( myInstAttrOffsets.pointPscale );
    *pscale_ptr = myPointAttributes.pscale;
#endif
#if HOUDINI_MAJOR_RELEASE>=11
    ppt->setValue<fpreal>(myInstAttrOffsets.pointPscale, (const fpreal)myPointAttributes.pscale);
#endif

#if HOUDINI_MAJOR_RELEASE==9
    id_ptr = ppt->castAttribData<int> ( myInstAttrOffsets.pointId );
    *id_ptr = myPointAttributes.id;
#endif
#if HOUDINI_MAJOR_RELEASE>=11
    ppt->setValue<int>(myInstAttrOffsets.pointId, (const int)myPointAttributes.id);
#endif

#if HOUDINI_MAJOR_RELEASE==9
    inst_id_ptr = ppt->castAttribData<int> ( myInstAttrOffsets.pointInstId );
    *inst_id_ptr = myInstanceNum;
#endif
#if HOUDINI_MAJOR_RELEASE>=11
    ppt->setValue<int>(myInstAttrOffsets.pointInstId, (const int)myInstanceNum);
#endif

#if HOUDINI_MAJOR_RELEASE==9
    UT_String* mat = ppt->castAttribData<UT_String> ( myInstAttrOffsets.pointMaterial );
    *mat = myPointAttributes.material;
#endif
#if HOUDINI_MAJOR_RELEASE>=11
GEO_AttributeHandle matAttribHandle = gdp->getPointAttribute("shop_materialpath");

if ( matAttribHandle.isAttributeValid() ) {
    matAttribHandle.setElement(ppt);
    matAttribHandle.setString(myPointAttributes.material);
}
#endif



}



/* ******************************************************************************
*  Function Name : setInstanceAttributes()
*
*  Description : Set the attributes of the instanced primitive
*
*  Input Arguments : GU_Detail *gdp, GEO_Primitive *myGeoPrim
*
*  Return Value : viod
*
***************************************************************************** */
inline void VRAY_clusterThis::setInstanceAttributes(GU_Detail *gdp, GEO_Primitive *myGeoPrim) {

#ifdef DEBUG
    cout << "VRAY_clusterThis::setInstanceAttributes() " << endl;
#endif

#if HOUDINI_MAJOR_RELEASE==9
    UT_Vector3  *Cd, *vel, *N;
//    UT_Vector3  *orient;
    fpreal *pscale_ptr, *Alpha_ptr;
    int *id_ptr, *inst_id_ptr;
#endif

    
#if HOUDINI_MAJOR_RELEASE==9
    Cd = myGeoPrim->castAttribData<UT_Vector3> ( myInstAttrOffsets.Cd );
    Cd->assign ( ( fpreal ) myPointAttributes.Cd.x(), ( fpreal ) myPointAttributes.Cd.y(), ( fpreal ) myPointAttributes.Cd.z() );
#endif
#if HOUDINI_MAJOR_RELEASE>=11
        myGeoPrim->setValue<UT_Vector3>(myInstAttrOffsets.Cd, (const UT_Vector3)myPointAttributes.Cd);
#endif

#if HOUDINI_MAJOR_RELEASE==9
    Alpha_ptr = myGeoPrim->castAttribData<fpreal> ( myInstAttrOffsets.Alpha );
    *Alpha_ptr = myPointAttributes.Alpha;
#endif
#if HOUDINI_MAJOR_RELEASE>=11
        myGeoPrim->setValue<fpreal>(myInstAttrOffsets.Alpha, (const fpreal)myPointAttributes.Alpha);
#endif
    
#if HOUDINI_MAJOR_RELEASE==9
    vel = myGeoPrim->castAttribData<UT_Vector3> ( myInstAttrOffsets.v );
    vel->assign ( ( fpreal ) myPointAttributes.v.x(), ( fpreal ) myPointAttributes.v.y(), ( fpreal ) myPointAttributes.v.z() );
#endif
#if HOUDINI_MAJOR_RELEASE>=11
        myGeoPrim->setValue<UT_Vector3>(myInstAttrOffsets.v, (const UT_Vector3)myPointAttributes.v);
#endif

#if HOUDINI_MAJOR_RELEASE==9
    N = myGeoPrim->castAttribData<UT_Vector3> ( myInstAttrOffsets.N );
    N->assign ( ( fpreal ) myPointAttributes.N.x(), ( fpreal ) myPointAttributes.N.y(), ( fpreal ) myPointAttributes.N.z() );
#endif
#if HOUDINI_MAJOR_RELEASE>=11
        myGeoPrim->setValue<UT_Vector3>(myInstAttrOffsets.N, (const UT_Vector3)myPointAttributes.N);
#endif


//    orient = myGeoPrim->castAttribData<UT_Vector4> ( myInstAttrOffsets.orient );
//    orient->assign ( ( fpreal ) myPointAttributes.orient.x(), ( fpreal ) myPointAttributes.orient.y(), ( fpreal ) myPointAttributes.orient.z() );


#if HOUDINI_MAJOR_RELEASE==9
    pscale_ptr = myGeoPrim->castAttribData<fpreal> ( myInstAttrOffsets.pscale );
    *pscale_ptr = myPointAttributes.pscale;
#endif
#if HOUDINI_MAJOR_RELEASE>=11
        myGeoPrim->setValue<fpreal>(myInstAttrOffsets.pscale, (const fpreal)myPointAttributes.pscale);
#endif

#if HOUDINI_MAJOR_RELEASE==9
    id_ptr = myGeoPrim->castAttribData<int> ( myInstAttrOffsets.id );
    *id_ptr = myPointAttributes.id;
#endif
#if HOUDINI_MAJOR_RELEASE>=11
        myGeoPrim->setValue<int>(myInstAttrOffsets.id, (const int)myPointAttributes.id);
#endif

#if HOUDINI_MAJOR_RELEASE==9
    inst_id_ptr = myGeoPrim->castAttribData<int> ( myInstAttrOffsets.inst_id );
    *inst_id_ptr = myInstanceNum;
#endif
#if HOUDINI_MAJOR_RELEASE>=11
        myGeoPrim->setValue<int>(myInstAttrOffsets.inst_id, (const int)myInstanceNum);
#endif


#if HOUDINI_MAJOR_RELEASE==9
    UT_String* mat = myGeoPrim->castAttribData<UT_String> ( myInstAttrOffsets.material );
    *mat = myPointAttributes.material;
#endif
#if HOUDINI_MAJOR_RELEASE>=11
GEO_AttributeHandle matAttribHandle = gdp->getPointAttribute("shop_materialpath");

if ( matAttribHandle.isAttributeValid() ) {
    matAttribHandle.setElement(myGeoPrim);
    matAttribHandle.setString(myPointAttributes.material);
}
#endif

    
//    cout << "VRAY_clusterThis::setInstanceAttributes() " << endl;


}




/* ******************************************************************************
*  Function Name : setFileAttributes()
*
*  Description :
*
*  Input Arguments : GEO_Point *ppt, GU_Detail *gdp
*
*  Return Value : int
*
***************************************************************************** */
inline int VRAY_clusterThis::setFileAttributes(GU_Detail *gdp) {

#ifdef DEBUG
    cout << "VRAY_clusterThis::setFileAttributes() " << endl;
#endif

#if HOUDINI_MAJOR_RELEASE==9
    UT_Vector3  *Cd, *vel;
//    UT_Vector3  *Cd, *vel, *N;
    fpreal *Alpha_ptr;
//    fpreal *pscale_ptr;
    int *id_ptr;
#endif
    GEO_Point *ppt;

#ifdef DEBUG
  long int num_points = ( long int ) gdp->points().entries();
  cout << "VRAY_clusterThis::setFileAttributes() - num points :" << num_points << endl;
#endif

// NOTE: For now. set only velocity (for motion blur), and id attrs for each point.

  FOR_ALL_GPOINTS ( gdp, ppt ) {
/*
      Cd = ppt->castAttribData<UT_Vector3> ( myInstAttrOffsets.pointCd );
      Cd->assign ( ( fpreal ) myPointAttributes.Cd.x(), ( fpreal ) myPointAttributes.Cd.y(), ( fpreal ) myPointAttributes.Cd.z() );
      Alpha_ptr = ppt->castAttribData<fpreal> ( myInstAttrOffsets.pointAlpha );
      *Alpha_ptr = myPointAttributes.Alpha;
*/

#if HOUDINI_MAJOR_RELEASE==9
      vel = ppt->castAttribData<UT_Vector3> ( myFileAttrOffsets.pointV );
      vel->assign ( ( fpreal ) myPointAttributes.v.x(), ( fpreal ) myPointAttributes.v.y(), ( fpreal ) myPointAttributes.v.z() );
#endif
#if HOUDINI_MAJOR_RELEASE>=11
      ppt->setValue<UT_Vector3>(myFileAttrOffsets.pointV, (const UT_Vector3)myPointAttributes.v);
#endif


/*
      N = ppt->castAttribData<UT_Vector3> ( myInstAttrOffsets.pointN );
      N->assign ( ( fpreal ) myPointAttributes.N.x(), ( fpreal ) myPointAttributes.N.y(), ( fpreal ) myPointAttributes.N.z() );

      pscale_ptr = ppt->castAttribData<fpreal> ( myFileAttrOffsets.pointPscale );
      *pscale_ptr = myPointAttributes.pscale;
*/

#if HOUDINI_MAJOR_RELEASE==9
      id_ptr = ppt->castAttribData<int> ( myFileAttrOffsets.pointId );
      *id_ptr = myPointAttributes.id;
#endif
#if HOUDINI_MAJOR_RELEASE>=11
      ppt->setValue<int>(myFileAttrOffsets.pointId, (const int)myPointAttributes.id);
#endif


//      UT_String* mat = ppt->castAttribData<UT_String> ( myInstAttrOffsets.pointMaterial );
//      *mat = myPointAttributes.material;

#ifdef DEBUG
    cout << "VRAY_clusterThis::setFileAttributes() " << "Cd: " << *Cd << endl;
    cout << "VRAY_clusterThis::setFileAttributes() " << "Alpha: " << *Alpha_ptr << endl;
    cout << "VRAY_clusterThis::setFileAttributes() " << "v: " << *vel << endl;
    cout << "VRAY_clusterThis::setFileAttributes() " << "N: " << myPointAttributes.N << endl;
    cout << "VRAY_clusterThis::setFileAttributes() " << "pscale: " << myPointAttributes.pscale << endl;
    cout << "VRAY_clusterThis::setFileAttributes() " << "id: " << myPointAttributes.id << endl;
//   cout << "VRAY_clusterThis::setFileAttributes() " << "material: " << myPointAttributes.material << endl;
// cout << "VRAY_clusterThis::getAttributes() " << "myMaterial: " << myMaterial << endl;
#endif

  }

  GEO_Primitive  *prim;

  FOR_ALL_PRIMITIVES(gdp, prim)  {

#if HOUDINI_MAJOR_RELEASE==9
      Cd = prim->castAttribData<UT_Vector3> ( myFileAttrOffsets.Cd );
      Cd->assign ( ( fpreal ) myPointAttributes.Cd.x(), ( fpreal ) myPointAttributes.Cd.y(), ( fpreal ) myPointAttributes.Cd.z() );
#endif
#if HOUDINI_MAJOR_RELEASE>=11
      prim->setValue<UT_Vector3>(myFileAttrOffsets.Cd, (const UT_Vector3)myPointAttributes.Cd);
#endif

#if HOUDINI_MAJOR_RELEASE==9
      Alpha_ptr = prim->castAttribData<fpreal> ( myFileAttrOffsets.Alpha );
      *Alpha_ptr = myPointAttributes.Alpha;
#endif
#if HOUDINI_MAJOR_RELEASE>=11
      prim->setValue<fpreal>(myFileAttrOffsets.Alpha, (const fpreal)myPointAttributes.Alpha);
#endif

#if HOUDINI_MAJOR_RELEASE==9
      vel = prim->castAttribData<UT_Vector3> ( myFileAttrOffsets.v );
      vel->assign ( ( fpreal ) myPointAttributes.v.x(), ( fpreal ) myPointAttributes.v.y(), ( fpreal ) myPointAttributes.v.z() );
#endif
#if HOUDINI_MAJOR_RELEASE>=11
      prim->setValue<UT_Vector3>(myFileAttrOffsets.v, (const UT_Vector3)myPointAttributes.v);
#endif


// NOTE: File instanced geo should have normals already, do not set the normals to the source point normals 

// NOTE: No need to set pscale attr on prims (already in a point attr)

/*
      N = prim->castAttribData<UT_Vector3> ( myFileAttrOffsets.N );
      N->assign ( ( fpreal ) myPointAttributes.N.x(), ( fpreal ) myPointAttributes.N.y(), ( fpreal ) myPointAttributes.N.z() );
//      prim->normal(1);
      pscale_ptr = prim->castAttribData<fpreal> ( myFileAttrOffsets.pscale );
      *pscale_ptr = myPointAttributes.pscale;
*/

#if HOUDINI_MAJOR_RELEASE==9
      id_ptr = prim->castAttribData<int> ( myFileAttrOffsets.id );
      *id_ptr = myPointAttributes.id;
#endif
#if HOUDINI_MAJOR_RELEASE>=11
      prim->setValue<int>(myFileAttrOffsets.id, (const int)myPointAttributes.id);
#endif

#if HOUDINI_MAJOR_RELEASE==9
//      UT_String* mat = prim->castAttribData<UT_String> ( myInstAttrOffsets.material );
//      *mat = myPointAttributes.material;
#endif
#if HOUDINI_MAJOR_RELEASE>=11
GEO_AttributeHandle matAttribHandle = gdp->getPrimAttribute("shop_materialpath");

if ( matAttribHandle.isAttributeValid() ) {
    matAttribHandle.setElement(prim);
    matAttribHandle.setString(myPointAttributes.material);
}
#endif


  }


  return 0;

}



#endif


/**********************************************************************************/
//  $Log: VRAY_clusterThisAttributeUtils.C,v $
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

