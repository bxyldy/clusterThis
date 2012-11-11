/* ******************************************************************************
*
*  VRAY_clusterThisRender
*
* $RCSfile: VRAY_clusterThisRender.C,v $
*
* Description :
*
* $Revision: 1.17 $
*
* $Source: /dca/cvsroot/houdini/VRAY_clusterThis/VRAY_clusterThisRender.C,v $
*
* $Author: mstory $
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

#ifndef __VRAY_clusterThisRender_C__
#define __VRAY_clusterThisRender_C__


/* ******************************************************************************
*  Function Name : render()
*
*  Description :   Render VRAY_clusterThis object
*
*  Input Arguments : None
*
*  Return Value : None
*
***************************************************************************** */
void VRAY_clusterThis::render()
{
   GU_Detail * gdp, *inst_gdp, *mb_gdp, *file_gdp;
   GEO_Point * ppt;

//   GEO_AttributeHandle attrHandleVelocity, attrHandleForce, attrHandleVorticity, attrHandleNormal, attrHandleNumNeighbors,
//   attrHandleTextureUV, attrHandleMass, attrHandleAge, attrHandleTemperature, attrHandleID,
//   attrHandleDensity, attrHandleViscosity, attrHandlePressure, attrHandlePscale;

   long int point_num = 0;
   static bool rendered = false;


   tempFileDeleted = false;
//   cout << "VRAY_clusterThis::render() tempFileDeleted: " << tempFileDeleted << endl;

   dca::myPasses(1);
//   std::cout << "VRAY_clusterThis::render() - num_passes: " << dca::myPasses(0) <<  std::endl;

   if(myVerbose > CLUSTER_MSG_QUIET) {
         std::cout << "VRAY_clusterThis::render() - Version: " << MAJOR_VER << "." << MINOR_VER << "." << BUILD_VER << std::endl;
         std::cout << "VRAY_clusterThis::render() - Built for Houdini Version: " << UT_MAJOR_VERSION
                   << "." << UT_MINOR_VERSION << "." << UT_BUILD_VERSION_INT << std::endl;
         std::cout << "VRAY_clusterThis::render() - Instancing ..." <<  std::endl;
      }


   try {

//          cout << "VM_GEO_clusterThis OTL version: " <<  myOTLVersion << std::endl;

//       if(myOTLVersion != DCA_VERSION) {
//          cout << "VM_GEO_clusterThis OTL is wrong version: " <<  myOTLVersion << ", should be version: " << DCA_VERSION << ", please install correct version." << std::endl;
//          throw VRAY_clusterThis_Exception ( "VRAY_clusterThis::render() VM_GEO_clusterThis OTL is wrong version!", 1 );
//       }


         if(!rendered || !myUseTempFile) {

               void * handle = VRAY_Procedural::queryObject(0);
               gdp = VRAY_Procedural::allocateGeometry();
               if(!gdp) {
                     throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() - object has no geometry ", 1);
                  }

               if(myUseGeoFile) {
                     // If the file failed to load, throw an exception
                     if(!(gdp->load((const char *)mySrcGeoFname).success()))
                        throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() - Failed to read source geometry file ", 1);

                     if(myVerbose > CLUSTER_MSG_INFO)
                        cout << "VRAY_clusterThis::render() - Successfully loaded source geo file: " << mySrcGeoFname << std::endl;
                  }
               else {
                     gdp->copy(*VRAY_Procedural::queryGeometry(handle, 0));
                     if(myVerbose > CLUSTER_MSG_INFO)
                        cout << "VRAY_clusterThis::render() - Copied incoming geometry" << std::endl;
                  }


               gdp->getBBox(&myBox);

//               std::cout << "VRAY_clusterThis::render() - gdp->getBBox(&myBox): " << myBox << std::endl;


               VRAY_Procedural::querySurfaceShader(handle, myMaterial);
               myMaterial.harden();
//         myPointAttributes.material = myMaterial;

//         const char **        getSParm (int token) const
//         cout << "VRAY_clusterThis::render() getSParm: " << *getSParm (0) << std::endl;


#ifdef DEBUG
               cout << "VRAY_clusterThis::render() myMaterial: " << myMaterial << std::endl;
#endif

               myLOD = getLevelOfDetail(myBox);
               if(myVerbose > CLUSTER_MSG_INFO)
                  cout << "VRAY_clusterThis::render() myLOD: " << myLOD << std::endl;


               // Get the number if points of the incoming geometery, calculate an interval for reporting the status of the instancing to the user
               long int num_points = (long int) gdp->points().entries();
               long int stat_interval = (long int)(num_points * 0.10) + 1;

               if(myVerbose > CLUSTER_MSG_QUIET)
                  cout << "VRAY_clusterThis::render() Number of points of incoming geometry: " << num_points << std::endl;

               myObjectName = VRAY_Procedural::queryObjectName(handle);

//      cout << "VRAY_clusterThis::render() Object Name: " << myObjectName << std::endl;
//      cout << "VRAY_clusterThis::render() Root Name: " << queryRootName() << std::endl;

// DEBUG stuff ...

//   changeSetting("object:geo_velocityblur", "on");

               UT_String      str;
               int     vblur = 0;
               import("object:velocityblur", &vblur, 0);

               if(vblur) {
                     str = 0;
                     import("velocity", str);
                     if(str.isstring()) {
//               const char  *  name;
//               name = queryObjectName(handle);
                           VRAYwarning("%s[%s] cannot get %s",
                                       VRAY_Procedural::getClassName(), (const char *)myObjectName, " motion blur attr");

                        }
                  }


               myXformInverse = queryTransform(handle, 0);
               myXformInverse.invert();




#ifdef DEBUG
               cout << "Geometry Samples: " << queryGeometrySamples(handle) << std::endl;
// cout << "scale: " << getFParm ( "scale" ) << std::endl;
#endif

               // Dump the user parameters to the console
               if(myVerbose == CLUSTER_MSG_DEBUG)
                  VRAY_clusterThis::dumpParameters();

               switch(myMethod) {
                     case CLUSTER_INSTANCE_NOW:
                        inst_gdp = VRAY_Procedural::allocateGeometry();
                        if(myDoMotionBlur == CLUSTER_MB_DEFORMATION)
                           mb_gdp = VRAY_Procedural::allocateGeometry();
                        if(myVerbose > CLUSTER_MSG_QUIET)
                           cout << "VRAY_clusterThis::render() - Using \"instance all the geometry at once\" method" << std::endl;
                        break;
                     case CLUSTER_INSTANCE_DEFERRED:
                        if(myVerbose > CLUSTER_MSG_QUIET)
                           cout << "VRAY_clusterThis::render() - Using \"addProcedural()\" method" << std::endl;
                        break;
                  }

               rendered = true;

               // Get the point's attribute offsets
               VRAY_clusterThis::getAttributeOffsets(gdp);

               // Check for required attributes
               VRAY_clusterThis::checkRequiredAttributes();

               // Check for weight attribute if the user wants metaballs
               if((myPrimType == CLUSTER_PRIM_METABALL) && (myPointAttrOffsets.weight.isInvalid())) {

                     cout << "Incoming points must have weight attribute if instancing metaballs! Throwing exception ..." << std::endl;
                     throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have weight attribute if instancing metaballs!", 1);
                  }


               if(myPrimType == CLUSTER_FILE) {
                     file_gdp = VRAY_Procedural::allocateGeometry();
                     int file_load_stat = VRAY_clusterThis::preLoadGeoFile(file_gdp);

                     if(!file_load_stat) {
//                           myFileGDP = file_gdp;
                           if(myVerbose > CLUSTER_MSG_INFO)
                              cout << "VRAY_clusterThis::render() Successfully loaded geometry file: " << myGeoFile << std::endl;
                        }
                     else {
                           throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Failed to load geometry file ", 1);
                        }
                  }

               // init some vars ...
               myPointAttributes.Cd = (UT_Vector3(1, 1, 1));
               myPointAttributes.v = (UT_Vector3(0, 0, 0));
               myPointAttributes.N = (UT_Vector3(0, 1, 0));
               myPointAttributes.Alpha = 1.0;
               myPointAttributes.pscale = 1.0;
               myPointAttributes.id = 0;

               myNoise.initialize(myNoiseSeed, myNoiseType);

               // Create the attribute "offsets" for the geometry to be instanced
               VRAY_clusterThis::createAttributeOffsets(inst_gdp, mb_gdp);

//changeSetting("surface", "constant Cd ( 1 0 0 )", "object");


               fpreal theta = (2.0 * M_PI) / myNumCopies;
               myInstanceNum = 0;

               if(myCVEX_Exec_pre) {
                     if(myVerbose > CLUSTER_MSG_INFO)
                        cout << "VRAY_clusterThis::render() Executing Pre Process CVEX code" << std::endl;
                     VRAY_clusterThis::runCVEX(gdp, gdp, myCVEXFname_pre, CLUSTER_CVEX_POINT);
                  }

               /// For each point of the incoming geometry
               GA_FOR_ALL_GPOINTS(gdp, ppt) {
                  myPointAttributes.myPos = ppt->getPos();

                  // get the point's attributes
                  VRAY_clusterThis::getAttributes(ppt, gdp);

#ifdef DEBUG
                  cout << "VRAY_clusterThis::render() " << "theta: " << theta << std::endl;
#endif

                  uint seed = 37;
                  fpreal dice;
                  bool skip = false;

                  if((myPrimType != CLUSTER_PRIM_CURVE) ||
                        ((myMethod == CLUSTER_INSTANCE_DEFERRED && (myPrimType == CLUSTER_PRIM_CURVE)))) {


                        // For each point, make a number of copies of and recurse a number of times for each copy ...
                        for(int copyNum = 0; copyNum < myNumCopies; copyNum++) {
                              for(int recursionNum = 0; recursionNum < myRecursion; recursionNum++) {

                                    // generate random number to determine to instance or not

                                    dice = SYSfastRandom(seed);
                                    (dice > myBirthProb) ? skip = true : skip = false;
//                  cout << dice << " " << skip << std::endl;
                                    seed = uint(dice * 100);

                                    // Calculate the position for the next instanced object ...
                                    VRAY_clusterThis::calculateNewPosition(theta, copyNum, recursionNum);

                                    if(!skip) {

                                          // Instance an object ...
                                          switch(myMethod) {
                                                   // For the "create all geometry at once" method, instance the object now ...
                                                case CLUSTER_INSTANCE_NOW:
                                                   // Create a primitive based upon user's selection
                                                   // TODO: can later be driven by a point attribute
                                                   switch(myPrimType) {
                                                         case CLUSTER_POINT:
                                                            VRAY_clusterThis::instancePoint(inst_gdp, mb_gdp);
                                                            break;
                                                         case CLUSTER_PRIM_SPHERE:
                                                            VRAY_clusterThis::instanceSphere(inst_gdp, mb_gdp);
                                                            break;
                                                         case CLUSTER_PRIM_CUBE:
                                                            VRAY_clusterThis::instanceCube(inst_gdp, mb_gdp);
                                                            break;
                                                         case CLUSTER_PRIM_GRID:
                                                            VRAY_clusterThis::instanceGrid(inst_gdp, mb_gdp);
                                                            break;
                                                         case CLUSTER_PRIM_TUBE:
                                                            VRAY_clusterThis::instanceTube(inst_gdp, mb_gdp);
                                                            break;
                                                         case CLUSTER_PRIM_CIRCLE:
                                                            VRAY_clusterThis::instanceCircle(inst_gdp, mb_gdp);
                                                            break;
                                                         case CLUSTER_PRIM_METABALL:
                                                            VRAY_clusterThis::instanceMetaball(inst_gdp, mb_gdp);
                                                            break;
                                                         case CLUSTER_FILE:
                                                            VRAY_clusterThis::instanceFile(file_gdp, inst_gdp, mb_gdp);
                                                            break;
                                                            // In case a instance type comes through that's not "legal", throw exception
                                                         default:
                                                            throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Illegal instance type, exiting ...", 1);
                                                            break;
                                                      }
                                                   break;

                                                   // For the "deferred instance" method, add the procedural now ...
                                                case CLUSTER_INSTANCE_DEFERRED:
                                                   VRAY_Procedural::openProceduralObject();
                                                   VRAY_clusterThisChild * child = new VRAY_clusterThisChild::VRAY_clusterThisChild(this);
                                                   VRAY_Procedural::addProcedural(child);
                                                   VRAY_Procedural::changeSetting("object:geo_velocityblur", "on");
                                                   VRAY_Procedural::closeObject();
//changeSetting("surface", "constant Cd ( 1 0 0 )", "object");
//   changeSetting("object:geo_velocityblur", "on");

                                                   break;
                                             }

                                          myInstanceNum++;

#ifdef DEBUG
                                          cout << "VRAY_clusterThis::render() - myInstanceNum: " << myInstanceNum << std::endl;
#endif
                                       }

                                 } // for number of recursions ...
                           } // for number of copies ...

                     }

                  // User wants a curve instanced on this point
                  if((myPrimType == CLUSTER_PRIM_CURVE) && (myMethod == CLUSTER_INSTANCE_NOW) && (!skip))
                     VRAY_clusterThis::instanceCurve(inst_gdp, mb_gdp, theta, point_num);

                  // Increment our point counter
                  point_num++;

                  // Print out stats to the console
                  if(myVerbose > CLUSTER_MSG_INFO && (myPrimType != CLUSTER_PRIM_CURVE))
                     if((long int)(point_num % stat_interval) == 0)
                        cout << "VRAY_clusterThis::render() Number of points processed: " << point_num << " Number of instances: " << myInstanceNum << std::endl;

               } // for all points ...





               if(myPostProcess) {

                     cout << "VRAY_clusterThis::render() Processing Voxels" << std::endl;

//   Vec3d voxelDimensions() const { return mTransform->voxelDimensions(); }
//00257     Vec3d voxelDimensions(const Vec3d& xyz) const { return mTransform->voxelDimensions(xyz); }
//00259     bool hasUniformVoxels() const { return mTransform->hasUniformScale(); }
//00261
//00262     Vec3d indexToWorld(const Vec3d& xyz) const { return mTransform->indexToWorld(xyz); }
//00263     Vec3d indexToWorld(const Coord& ijk) const { return mTransform->indexToWorld(ijk); }
//00265
//00266     Vec3d worldToIndex(const Vec3d& xyz) const { return mTransform->worldToIndex(xyz); }

// Vec3d worldToIndex   (  const Vec3d &     xyz    )    const

//                     openvdb::ScalarGrid::Accessor accessor;
//                     openvdb::FloatTree myTree;
                     openvdb::FloatTree::ConstPtr myTreePtr;
                     openvdb::VectorTree::ConstPtr myGradTreePtr;

                     ParticleList paList(gdp, /* "dR" */ 1.0, /* "dV" */ 1.0);
                     openvdb::tools::PointSampler mySampler, gradSampler;
//                     openvdb::tools::GridSampling<openvdb::FloatTree>  myGridSampler;

                     std::cout << "paList.size() ... "  << paList.size() << std::endl;

                     if(paList.size() != 0) {

                           hvdb::Interrupter boss("Converting particles to level set");

                           Settings settings;
                           settings.mRasterizeTrails = myRasterType;
                           settings.mDx = myDx;  // only used for rasterizeTrails()
                           settings.mFogVolume = myFogVolume;
                           settings.mGradientWidth = myGradientWidth;  // only used for fog volume

                           float background;

                           // background in WS units
                           if(myWSUnits)
                              background = myBandWidth;
                           // background NOT in WS units
                           else
                              background = myVoxelSize * myBandWidth;

                           // Construct a new scalar grid with the specified background value.
                           openvdb::math::Transform::Ptr transform =
                              openvdb::math::Transform::createLinearTransform(myVoxelSize);
                           openvdb::ScalarGrid::Ptr outputGrid = openvdb::ScalarGrid::create(background);
                           outputGrid->setTransform(transform);
                           outputGrid->setGridClass(openvdb::GRID_LEVEL_SET);

                           // Perform the particle conversion.
                           this->convert(outputGrid, paList, settings, boss);

                           std::cout << "VRAY_clusterThis::render() - activeVoxelCount () "
                                     << outputGrid->activeVoxelCount() << std::endl;
                           std::cout << "VRAY_clusterThis::render() - background: "
                                     << outputGrid->background() << std::endl;

//                           openvdb::FloatGrid::Accessor accessor = outputGrid->getAccessor();
//
//                           openvdb::Vec3i anIndex = outputGrid->worldToIndex(openvdb::Vec3d(-0.0168841, -2.81901, 0.0316872));
//
//                           openvdb::Coord xyz(1000, -200000000, 30000000);
//                           openvdb::Coord xyz(anIndex);
//                           accessor.setValue(xyz, 1.0);
//                           -0.0168841, -2.81901, 0.0316872
//                           std::cout << "outputGrid" << xyz << " = " << accessor.getValue(xyz) << std::endl;


//
//                     long int aCounter = 0;
//
//                           for(openvdb::ScalarGrid::ValueOnCIter iter = outputGrid->cbeginValueOn(); iter; ++iter)
//                              {
//                                 openvdb::Vec3d worldPos = outputGrid->indexToWorld(iter.getCoord());
//                                 openvdb::CoordBBox bbox;
//                                 iter.getBoundingBox(bbox);
//                                 if(((long int)aCounter++ % 100) == 0) {
//                                       std::cout << "outputGrid" << iter.getCoord() << " = " << *iter << "\t"
//                                                 << "\tbbox: " << bbox
//                                                 << "\tworldPos: " << worldPos << std::endl;
//                                    }
//                              }
//

                           // Insert the new grid into the ouput detail.
                           UT_String gridNameStr = "My Cluster Grid";
                           outputGrid->insertMeta("vector type", openvdb::StringMetadata("covariant (gradient)"));
                           outputGrid->insertMeta("name", openvdb::StringMetadata((const char *)gridNameStr));
                           outputGrid->insertMeta("VoxelSize", openvdb::FloatMetadata(myVoxelSize));
                           outputGrid->insertMeta("background", openvdb::FloatMetadata(background));

//    hvdb::createVdbPrimitive(*gdp, outputGrid, gridNameStr.toStdString().c_str());
//    GU_PrimVDB (GU_Detail *gdp, GA_Offset offset=GA_INVALID_OFFSET)
//    GU_PrimVDB (const GA_MergeMap &map, GA_Detail &detail, GA_Offset offset, const GA_Primitive &src_prim)


                           UT_Vector3 inst_pos, seed_pos, currVel;
                           const GA_PointGroup * sourceGroup = NULL;
                           long int pt_counter = 0;
                           float radius = 5.0f;

                           std::cout << "VRAY_clusterThis::render() - Massaging data ... " << std::endl;

                           long int pointsFound = 0;
                           GEO_AttributeHandle inst_vel_gah = inst_gdp->getPointAttribute("v");
                           GEO_AttributeHandle source_vel_gah = gdp->getPointAttribute("v");
                           GEO_AttributeHandle inst_Cd_gah = inst_gdp->getPointAttribute("Cd");
                           GEO_AttributeHandle source_Cd_gah = gdp->getPointAttribute("Cd");
                           GEO_AttributeHandle inst_Alpha_gah = inst_gdp->getPointAttribute("Alpha");
                           GEO_AttributeHandle source_Alpha_gah = gdp->getPointAttribute("Alpha");

                           if(!inst_vel_gah.isAttributeValid())
                              throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Instance velocity handle invalid, exiting ...", 1);
                           if(!source_vel_gah.isAttributeValid())
                              throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Source velocity handle invalid, exiting ...", 1);
                           if(!inst_Cd_gah.isAttributeValid())
                              throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Instance color handle invalid, exiting ...", 1);
                           if(!source_Cd_gah.isAttributeValid())
                              throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Source color handle invalid, exiting ...", 1);
                           if(!inst_Alpha_gah.isAttributeValid())
                              throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Instance alpha handle invalid, exiting ...", 1);
                           if(!source_Alpha_gah.isAttributeValid())
                              throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Source alpha handle invalid, exiting ...", 1);

                           openvdb::FloatTree::ValueType sampleResult;
                           openvdb::VectorGrid::ValueType gradResult;
                           const openvdb::FloatTree aTree;
                           myTreePtr = outputGrid->tree();

                           openvdb::VectorGrid::Ptr gradientGrid = openvdb::VectorGrid::create();

                           openvdb::tools::Gradient<openvdb::ScalarGrid> myGradient(outputGrid);

                           gradientGrid = myGradient.process();
                           myGradTreePtr = gradientGrid->tree();


                           GA_FOR_ALL_GROUP_POINTS(inst_gdp, sourceGroup, ppt) {
//                              myCurrPtOff = ppt->getMapOffset();
//                              std::cout << "myCurrPtOff: " << myCurrPtOff << std::endl;

                              inst_pos = ppt->getPos();

// Vec3d worldToIndex   (  const Vec3d &     xyz    )    const

//                              openvdb::Vec3R theIndex =
//                                 (openvdb::Vec3R(inst_pos[0], inst_pos[1], inst_pos[2]));
                              openvdb::Vec3R theIndex =
                                 outputGrid->worldToIndex(openvdb::Vec3R(inst_pos[0], inst_pos[1], inst_pos[2]));

                              radius = static_cast<fpreal>(ppt->getValue<fpreal>(myInstAttrRefs.pointRadius, 0));
//                                    std::cout << "radius: " << radius << std::endl;

// static bool    sample (const TreeT &inTree, const Vec3R &inCoord, typename TreeT::ValueType &sampleResult)
                              const openvdb::Vec3R  inst_sample_pos(theIndex[0], theIndex[1], theIndex[2]);

                              bool success = mySampler.sample(*myTreePtr, inst_sample_pos, sampleResult);

                              gradSampler.sample(*myGradTreePtr, inst_sample_pos, gradResult);
//
//                              std::cout << "success: " << success << "\tinst_pos: " << inst_pos
//                                        << "\tinst_sample_pos: " << inst_sample_pos
//                                        << "\tsampleResult: " << sampleResult << std::endl;


//ValueType    sampleWorld (const Vec3R &pt) const
//ValueType    sampleWorld (Real x, Real y, Real z) const

                              // if the instanced point is within the vdb volume
                              if(success) {
//                                    std::cout << "inst_pos: " << inst_pos << " inst_sample_pos: "
//                                              << inst_sample_pos << " sampleResult: " << sampleResult
//                                              << " gradResult: " << gradResult << std::endl;

//                                    float weight;
                                    pointsFound++;

                                    inst_vel_gah.setElement(ppt);
                                    currVel = inst_vel_gah.getV3();

                                    UT_Vector3 gradVect = UT_Vector3(gradResult[0], gradResult[1], gradResult[2]);

                                    ppt->setPos(inst_pos + (myPosInfluence * (sampleResult * gradVect)));
//                                    ppt->setPos(inst_pos + (sampleResult * myPosInfluence *(currVel / myFPS)));

//                                    inst_vel_gah.setV3(currVel * ((1 / sampleResult) * radius));
                                    inst_vel_gah.setV3(currVel + (sampleResult * gradVect));

//                                    std::cout << "currVel: " << currVel << " sampleResult " << sampleResult
//                                              << " new vel: " <<  currVel * sampleResult << std::endl;

//                                    inst_Cd_gah.setElement(ppt);
//                                    inst_Cd_gah.setV3(inst_Cd_gah.getV3() * abs(sampleResult));
//
//
//                                    inst_Alpha_gah.setElement(ppt);
//                                    inst_Alpha_gah.setF(inst_Alpha_gah.getF() * abs(sampleResult));

                                 } // if the instanced point is within the vdb volume

                              pt_counter++;
                              if((long int)(pt_counter % (stat_interval * myNumCopies * myRecursion)) == 0) {
                                    cout << "VRAY_clusterThis::render() Number of points post processed: " << pt_counter
                                         << "\t - Number of points found in vdb grid: " << pointsFound << std::endl;
                                 }
                           }


                           if(!pointsFound)
                              cout << "VRAY_clusterThis::render() NO POINTS POST PROCESSED!!! " << std::endl;

                        }   //  if(paList.size() != 0)


                  }




               if(myVerbose > CLUSTER_MSG_QUIET)
                  if(myMethod == CLUSTER_INSTANCE_NOW)
                     if(myDoMotionBlur == CLUSTER_MB_DEFORMATION)
                        cout << "VRAY_clusterThis::render() - Memory usage(MB): " <<
                             (fpreal)(inst_gdp->getMemoryUsage() + mb_gdp->getMemoryUsage() / (1024.0 * 1024.0)) << std::endl;
                     else
                        cout << "VRAY_clusterThis::render() - Memory usage(MB): " <<
                             (fpreal)(inst_gdp->getMemoryUsage() / (1024.0 * 1024.0)) << std::endl;

               // If the "instance all the geo at once method" is used, add the the instanced geo for mantra to render ...
               switch(myMethod) {
                     case CLUSTER_INSTANCE_NOW:

                        if(myCVEX_Exec_post) {
                              if(myVerbose > CLUSTER_MSG_INFO)
                                 cout << "VRAY_clusterThis::render() Executing Post Process CVEX code" << std::endl;
                              VRAY_clusterThis::runCVEX(inst_gdp, mb_gdp, myCVEXFname_post, CLUSTER_CVEX_POINT);
                           }
                        VRAY_Procedural::openGeometryObject();
                        VRAY_Procedural::addGeometry(inst_gdp, 0.0);

                        if(myDoMotionBlur == CLUSTER_MB_VELOCITY)
                           VRAY_Procedural::addVelocityBlurGeometry(inst_gdp, myShutter, myShutter2);

// float    addVelocityBlurGeometry (GU_Detail *gdp, fpreal pre_blur, fpreal post_blur, const char *velocity_attribute="v")

                        if(myDoMotionBlur == CLUSTER_MB_DEFORMATION)
                           VRAY_Procedural::addGeometry(mb_gdp, myShutter);

                        VRAY_Procedural::setComputeN(1);
//                        setSurface(myMaterial);
                        VRAY_Procedural::closeObject();

                        break;
                     case CLUSTER_INSTANCE_DEFERRED:
                        break;
                  }

               if(myVerbose > CLUSTER_MSG_QUIET && (myPrimType != CLUSTER_PRIM_CURVE))
                  cout << "VRAY_clusterThis::render() Total number of instances: " << myInstanceNum << std::endl;

               // Save the geo to temp location so it doesn't have to be regenerated for a deep shadow pass, etc.
               if(myMethod == CLUSTER_INSTANCE_NOW && myUseTempFile) {
                     ofstream myGeoStream;
                     // myGeoStream.open("/tmp/thisGeo.bgeo");
                     myGeoStream.open((const char *)myTempFname, ios_base::binary);
                     UT_Options myOptions;
                     inst_gdp->save(myGeoStream, 1, &myOptions);
                     myGeoStream.flush();
                     myGeoStream.close();
                     if(myVerbose > CLUSTER_MSG_QUIET)
                        cout << "VRAY_clusterThis::render() - Saved geometry to temp file: " << myTempFname << std::endl;
                  }

               if(myPrimType == CLUSTER_FILE)
                  VRAY_Procedural::freeGeometry(file_gdp);

               // We're done, free the original geometry
               VRAY_Procedural::freeGeometry(gdp);

            } /// if (rendered) ...


         // Geo has already been generated ...
         else {
               if(myVerbose > CLUSTER_MSG_QUIET)
                  cout << "VRAY_clusterThis::render() - Already generated geometry, reading temp geo file: " << myTempFname << std::endl;
               inst_gdp = VRAY_Procedural::allocateGeometry();
               UT_Options myOptions;

               // If the file failed to load, throw an exception
               if((inst_gdp->load((const char *)myTempFname).success())) {
                     if(myVerbose > CLUSTER_MSG_QUIET)
                        cout << "VRAY_clusterThis::render() - Successfully loaded temp geo file: " << myTempFname << std::endl;
                  }
               else
                  throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() - Failed to read temp geometry file ", 1);

               VRAY_Procedural::openGeometryObject();
               VRAY_Procedural::addGeometry(inst_gdp, 0);

               if(myDoMotionBlur == CLUSTER_MB_VELOCITY)
                  VRAY_Procedural::addVelocityBlurGeometry(inst_gdp, myShutter, myShutter2);

// float    addVelocityBlurGeometry (GU_Detail *gdp, fpreal pre_blur, fpreal post_blur, const char *velocity_attribute="v")

               //if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
               //   mb_gdp = allocateGeometry();

               // if(!mb_gdp->load((const char *)myTempFname, &myOptions))
               //  cout << "VRAY_clusterThis::render() - Successfully loaded temp geo file for deformation motion blur: " << myTempFname << std::endl;
               // else
               //      throw VRAY_clusterThis_Exception ("VRAY_clusterThis::render() - Failed to read temp geometry file ", 1 );

               //   addGeometry(mb_gdp, myShutter);
               //   }

               VRAY_Procedural::setComputeN(1);
               VRAY_Procedural::setSurface(myMaterial);
               VRAY_Procedural::closeObject();
            }

      } // try ...



   // Process exceptions ...
   catch(VRAY_clusterThis_Exception e) {
         e.what();
         cout << "VRAY_clusterThis::render() - Exception encountered, copying incoming geometry" << std::endl << std::endl;
         if(gdp)
            VRAY_Procedural::freeGeometry(gdp);
         if(inst_gdp)
            VRAY_Procedural::freeGeometry(inst_gdp);
         if(myDoMotionBlur == CLUSTER_MB_DEFORMATION)
            if(mb_gdp)
               VRAY_Procedural::freeGeometry(mb_gdp);
         void * handle = queryObject(0);
         gdp = VRAY_Procedural::allocateGeometry();
         gdp->copy(*queryGeometry(handle, 0));
         VRAY_Procedural::openGeometryObject();
         VRAY_Procedural::addGeometry(gdp, 0);
         closeObject();
         return;
      }


   catch(...) {
         cout << "VRAY_clusterThis::render() - Unknown exception encountered, freeing geometry and exiting" << std::endl << std::endl;
         freeGeometry(gdp);
         freeGeometry(inst_gdp);
         if(myDoMotionBlur == CLUSTER_MB_DEFORMATION)
            freeGeometry(mb_gdp);
         return;
      }


   if(myVerbose > CLUSTER_MSG_QUIET)
      cout << "VRAY_clusterThis::render() - Leaving render() method" << std::endl;

   return;

}



#endif




