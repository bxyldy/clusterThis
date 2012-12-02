/* ******************************************************************************
*
*  VRAY_clusterThisPostProcess
*
* $RCSfile: VRAY_clusterThisPostProcess.C,v $
*
* Description :
*
*
***************************************************************************** */



#ifndef __VRAY_clusterThisPostProcess_C__
#define __VRAY_clusterThisPostProcess_C__


/* ******************************************************************************
*  Function Name : postProcess()
*
*  Description :   post process the geo
*
*  Input Arguments : GU_Detail * gdp, GU_Detail * inst_gdp, GU_Detail * mb_gdp
*
*  Return Value : None
*
***************************************************************************** */

void VRAY_clusterThis::postProcess(GU_Detail * gdp, GU_Detail * inst_gdp, GU_Detail * mb_gdp)

{

   long int num_points = (long int) gdp->points().entries();
   long int stat_interval = (long int)(num_points * 0.10) + 1;


   GEO_Point * src_ppt, * inst_ppt;
   GEO_PointPtrArray src_list;
   UT_Vector3 tmp_v, new_v;
   fpreal dist;

   // Perform "nearest neighbor" post processing
   if(myNNPostProcess) {
         if(myVerbose > CLUSTER_MSG_INFO)
            cout << "VRAY_clusterThis::postProcess() Performing nearest neighbor processing " << std::endl;

         GA_FOR_ALL_GPOINTS(inst_gdp, inst_ppt) {

            UT_Vector3 inst_pos = inst_ppt->getPos();

            fpreal inst_radius = static_cast<fpreal>(inst_ppt->getValue<fpreal>(myInstAttrRefs.pointRadius, 0));
            UT_Vector3 inst_v = static_cast<UT_Vector3>(inst_ppt->getValue<UT_Vector3>(myInstAttrRefs.pointV, 0));

            int num_src_pts_found = mySRCPointTree.findAllClosePt(inst_pos, inst_radius, src_list);
//      cout << "VRAY_clusterThis::postProcess() num_src_pts_found: " << num_src_pts_found << " inst_radius: " << inst_radius << std::endl;

            new_v = 0.0;
            if(num_src_pts_found > 0)
               for(uint i = 0; i < src_list.entries(); i++) {
                     src_ppt = src_list(i);
                     tmp_v = static_cast<UT_Vector3>(src_ppt->getValue<UT_Vector3>(myPointAttrRefs.v, 0));
//                     dist = distance2(inst_pos, static_cast<UT_Vector3>(src_ppt->getPos()));
                     dist = distance3d(inst_pos, static_cast<UT_Vector3>(src_ppt->getPos()));
//                     new_v = new_v + (tmp_v * (1 + (inst_radius - dist)));
//                     new_v = new_v + (tmp_v * (1 + SYSsqrt((inst_radius * inst_radius) - dist)));
//                     new_v = new_v + (tmp_v * (1 + (inst_radius * inst_radius) - dist));
                     new_v = new_v + (tmp_v * (1 + inst_radius - dist));
                  }

//  SYSavg(a,b,c)
// SYSclamp(a, min, max)
//std::numeric_limits<float>::max();
//std::numeric_limits<float>::min();
//std::numeric_limits<float>::infinity();

//            const UT_Vector3 v =  static_cast<const UT_Vector3>(
//                                 (((new_v / myFPS) * myNNPostVelInfluence)));

//            const UT_Vector3 v =  static_cast<const UT_Vector3>(
//                                 (((new_v / myFPS) * myNNPostVelInfluence) / static_cast<float>(num_src_pts_found)));

            const UT_Vector3 v =
               static_cast<const UT_Vector3>((new_v / static_cast<float>(num_src_pts_found)));
//            const UT_Vector3 v =
//               static_cast<const UT_Vector3>(((new_v / myFPS) / static_cast<float>(num_src_pts_found)));

            SYSclamp(v[0], std::numeric_limits<float>::min(), std::numeric_limits<float>::max());
            SYSclamp(v[1], std::numeric_limits<float>::min(), std::numeric_limits<float>::max());
            SYSclamp(v[2], std::numeric_limits<float>::min(), std::numeric_limits<float>::max());

//            cout << "VRAY_clusterThis::postProcess() v: " << v << std::endl;

            inst_ppt->setValue<UT_Vector3>(myInstAttrRefs.pointV, static_cast<const UT_Vector3>(v * myNNPostVelInfluence));

            inst_ppt->setPos((inst_pos + static_cast<const UT_Vector3>(v * myNNPostPosInfluence)));

         }

      } // if(myNNPostProcess)



   // Perform Open VDB post processing
   if(myVDBPostProcess) {


         if(myVerbose > CLUSTER_MSG_INFO)
            cout << "VRAY_clusterThis::postProcess() Processing Voxels" << std::endl;


         ParticleList paList(gdp, myPostVDBRadiusMult, myPostVDBVelocityMult);
         openvdb::tools::PointSampler mySampler, gradSampler;
//                     openvdb::tools::GridSampling<openvdb::FloatTree>  myGridSampler;

         if(myVerbose == CLUSTER_MSG_DEBUG)
            std::cout << "VRAY_clusterThis::postProcess() paList.size() ... "  << paList.size() << std::endl;

         if(paList.size() != 0) {

               hvdb::Interrupter boss("VRAY_clusterThis::postProcess() Converting particles to level set");

               Settings settings;
               settings.mRadiusMin = myPostRadiusMin;
               settings.mRasterizeTrails = myPostRasterType;
               settings.mDx = myPostDx;  // only used for rasterizeTrails()
               settings.mFogVolume = myPostFogVolume;
               settings.mGradientWidth = myPostGradientWidth;  // only used for fog volume

               float background;

               // background in WS units
               if(myPostWSUnits)
                  background = myPostBandWidth;
               // background NOT in WS units
               else
                  background = myPostVoxelSize * myPostBandWidth;

               // Construct a new scalar grid with the specified background value.
               openvdb::math::Transform::Ptr transform =
                  openvdb::math::Transform::createLinearTransform(myPostVoxelSize);
               openvdb::ScalarGrid::Ptr outputGrid = openvdb::ScalarGrid::create(background);
               outputGrid->setTransform(transform);
               outputGrid->setGridClass(openvdb::GRID_LEVEL_SET);

               // Perform the particle conversion.
               this->convert(outputGrid, paList, settings, boss);

               if(myVerbose == CLUSTER_MSG_DEBUG) {
                     std::cout << "VRAY_clusterThis::postProcess() - activeVoxelCount(): "
                               << outputGrid->activeVoxelCount() << std::endl;
                     std::cout << "VRAY_clusterThis::postProcess() - background: "
                               << outputGrid->background() << std::endl;
                  }


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
               UT_String gridNameStr = "ClusterGrid";
               outputGrid->insertMeta("float type", openvdb::StringMetadata("averaged_velocity"));
               outputGrid->insertMeta("name", openvdb::StringMetadata((const char *)gridNameStr));
               outputGrid->insertMeta("VoxelSize", openvdb::FloatMetadata(myPostVoxelSize));
               outputGrid->insertMeta("background", openvdb::FloatMetadata(background));

//    hvdb::createVdbPrimitive(*gdp, outputGrid, gridNameStr.toStdString().c_str());
//    GU_PrimVDB (GU_Detail *gdp, GA_Offset offset=GA_INVALID_OFFSET)
//    GU_PrimVDB (const GA_MergeMap &map, GA_Detail &detail, GA_Offset offset, const GA_Primitive &src_prim)


               UT_Vector3 inst_pos, seed_pos, currVel;
//         const GA_PointGroup * sourceGroup = NULL;
               long int pt_counter = 0;
               float vdb_radius = 2.0f;

               if(myVerbose > CLUSTER_MSG_INFO)
                  std::cout << "VRAY_clusterThis::postProcess() - Massaging data ... " << std::endl;

               long int pointsFound = 0;
               GEO_AttributeHandle inst_vel_gah = inst_gdp->getPointAttribute("v");
               GEO_AttributeHandle source_vel_gah = gdp->getPointAttribute("v");
               GEO_AttributeHandle inst_N_gah = inst_gdp->getPointAttribute("N");
               GEO_AttributeHandle source_N_gah = gdp->getPointAttribute("N");
               GEO_AttributeHandle inst_Cd_gah = inst_gdp->getPointAttribute("Cd");
               GEO_AttributeHandle source_Cd_gah = gdp->getPointAttribute("Cd");
               GEO_AttributeHandle inst_Alpha_gah = inst_gdp->getPointAttribute("Alpha");
               GEO_AttributeHandle source_Alpha_gah = gdp->getPointAttribute("Alpha");

               if(!inst_vel_gah.isAttributeValid())
                  throw VRAY_clusterThis_Exception("VRAY_clusterThis::postProcess() Instance velocity handle invalid, exiting ...", 1);
               if(!source_vel_gah.isAttributeValid())
                  throw VRAY_clusterThis_Exception("VRAY_clusterThis::postProcess() Source velocity handle invalid, exiting ...", 1);
               if(!inst_N_gah.isAttributeValid())
                  throw VRAY_clusterThis_Exception("VRAY_clusterThis::postProcess() Instance normal handle invalid, exiting ...", 1);
               if(!source_N_gah.isAttributeValid())
                  throw VRAY_clusterThis_Exception("VRAY_clusterThis::postProcess() Source normal handle invalid, exiting ...", 1);
               if(!inst_Cd_gah.isAttributeValid())
                  throw VRAY_clusterThis_Exception("VRAY_clusterThis::postProcess() Instance color handle invalid, exiting ...", 1);
               if(!source_Cd_gah.isAttributeValid())
                  throw VRAY_clusterThis_Exception("VRAY_clusterThis::postProcess() Source color handle invalid, exiting ...", 1);
               if(!inst_Alpha_gah.isAttributeValid())
                  throw VRAY_clusterThis_Exception("VRAY_clusterThis::postProcess() Instance alpha handle invalid, exiting ...", 1);
               if(!source_Alpha_gah.isAttributeValid())
                  throw VRAY_clusterThis_Exception("VRAY_clusterThis::postProcess() Source alpha handle invalid, exiting ...", 1);

//                     openvdb::ScalarGrid::Accessor accessor;
//                     openvdb::FloatTree myTree;
//         openvdb::FloatTree myTree;
               openvdb::FloatTree::ConstPtr myTreePtr;
               openvdb::VectorTree::ConstPtr myGradTreePtr;
               openvdb::FloatTree::ValueType sampleResult;
               openvdb::VectorGrid::ValueType gradResult;
               const openvdb::FloatTree aTree;
               openvdb::FloatTree & myTree = outputGrid->treeRW();

               openvdb::tools::Filter<openvdb::FloatGrid> fooFilter(*outputGrid);
//                           openvdb::tools::Filter<openvdb::FloatGrid> barFilter(outputGrid);



               if(myPostVDBMedianFilter) {
               if(myVerbose > CLUSTER_MSG_INFO)
                  std::cout << "VRAY_clusterThis::postProcess() - Filtering with median filter ... " << std::endl;
                  for(int n = 0; n < myPostVDBMedianIterations && !boss.wasInterrupted(); ++n)
                     fooFilter.median();
               }


               if(myPostVDBMeanFilter) {
                  std::cout << "VRAY_clusterThis::postProcess() - Filtering with mean filter ... " << std::endl;
                  for(int n = 0; n < myPostVDBMeanIterations && !boss.wasInterrupted(); ++n)
                     fooFilter.mean();
               }

               if(myPostVDBMeanCurvatureFilter) {
                  std::cout << "VRAY_clusterThis::postProcess() - Filtering with mean curvature filter ... " << std::endl;
                  for(int n = 0; n < myPostVDBMeanCurvatureIterations && !boss.wasInterrupted(); ++n)
                     fooFilter.meanCurvature();
               }


               if(myPostVDBLaplacianFilter) {
                  std::cout << "VRAY_clusterThis::postProcess() - Filtering with laplacian filter ... " << std::endl;
                  for(int n = 0; n < myPostVDBLaplacianIterations && !boss.wasInterrupted(); ++n)
                     fooFilter.laplacian();
               }


//                           if(myVDBReNormalizeFilter)
//                              float r = barFilter.renormalize(3, 0.1);

               if(myPostVDBOffsetFilter) {
                  std::cout << "VRAY_clusterThis::postProcess() - Filtering with offset filter ... " << std::endl;
                  fooFilter.offset(myPostVDBOffsetFilterAmount);
               }

//00838 {
//00839
//00840     if (verbose) {
//00841         std::cout << "Morphological " << (offset>0 ? "erosion" : "dilation")
//00842             << " by the offset " << offset << std::endl;
//00843     }
//00844
//00845     filter.offset(offset);
//00846 }




               openvdb::VectorGrid::Ptr gradientGrid = openvdb::VectorGrid::create();
               gradientGrid->setTransform(transform);
//               gradientGrid->setGridClass(openvdb::GRID_FOG_VOLUME );
               gradientGrid->setGridClass(openvdb::GRID_LEVEL_SET);

               openvdb::tools::Gradient<openvdb::ScalarGrid> myGradient(*outputGrid);
               gradientGrid = myGradient.process();
               openvdb::VectorTree & myGradTree = gradientGrid->treeRW();


//void   median (int width=1, bool serial=false)
//void   mean (int width=1, bool serial=false)
//void   meanCurvature (bool serial=false)
//void   laplacian (bool serial=false)
//void   offset (float offset, bool serial=false)

//template<bool UseWENO>
//float  renormalize (size_t maxSteps, float epsilon, bool verbose=false, bool serial=false)
//    Iterative re-normalization.

               gridNameStr = "ClusterGradientGrid";
               gradientGrid->insertMeta("vector type", openvdb::StringMetadata("covariant (gradient)"));
               gradientGrid->insertMeta("name", openvdb::StringMetadata((const char *)gridNameStr));
               gradientGrid->insertMeta("VoxelSize", openvdb::FloatMetadata(myPostVoxelSize));
               gradientGrid->insertMeta("background", openvdb::FloatMetadata(background));

               GEO_Point * ppt;

               GA_FOR_ALL_GPOINTS(inst_gdp, ppt) {
//            int     myCurrPtOff = ppt->getMapOffset();
//                              std::cout << "myCurrPtOff: " << myCurrPtOff << std::endl;
//   for(uint32 i = inst_gdp->points().entries(); i-- > 0;) {
//         GEO_Point * ppt = inst_gdp->points()(i);

                  inst_pos = ppt->getPos();

                  openvdb::Vec3R theIndex =
                     outputGrid->worldToIndex(openvdb::Vec3R(inst_pos[0], inst_pos[1], inst_pos[2]));

                  vdb_radius = static_cast<fpreal>(ppt->getValue<fpreal>(myInstAttrRefs.pointVDBRadius, 0));
//                                    std::cout << "vdb_radius: " << vdb_radius << std::endl;

                  const openvdb::Vec3R  inst_sample_pos(theIndex[0], theIndex[1], theIndex[2]);

                  bool success = mySampler.sample(myTree, inst_sample_pos, sampleResult);

                  bool grad_success = gradSampler.sample(myGradTree, inst_sample_pos, gradResult);



//
//                              std::cout << "success: " << success << "\tinst_pos: " << inst_pos
//                                        << "\tinst_sample_pos: " << inst_sample_pos
//                                        << "\tsampleResult: " << sampleResult << std::endl;

//ValueType    sampleWorld (const Vec3R &pt) const
//ValueType    sampleWorld (Real x, Real y, Real z) const

                  // if the instanced point is within the vdb volume
//            if(success) {
                  if(success && grad_success) {
//                                    std::cout << "inst_pos: " << inst_pos << " inst_sample_pos: "
//                                              << inst_sample_pos << " sampleResult: " << sampleResult
//                                              << " gradResult: " << gradResult << std::endl;
//                                    float weight;
                        pointsFound++;

                        inst_vel_gah.setElement(ppt);
                        currVel = inst_vel_gah.getV3();

                        UT_Vector3 gradVect = UT_Vector3(gradResult[0], gradResult[1], gradResult[2]);

                        ppt->setPos(inst_pos + (myPostPosInfluence *(sampleResult * gradVect)));
//                                    ppt->setPos(inst_pos + (sampleResult * myPosInfluence *(currVel / myFPS)));

//                                    inst_vel_gah.setV3(currVel * ((1 / sampleResult) * radius));
                        inst_vel_gah.setV3(currVel + (myPostVelInfluence *(sampleResult * gradVect)));

//                                    std::cout << "currVel: " << currVel << " sampleResult " << sampleResult
//                                              << " new vel: " <<  currVel * sampleResult << std::endl;

                        inst_N_gah.setV3(inst_N_gah.getV3() + (myPostNormalInfluence *(sampleResult * gradVect)));

//                        inst_Cd_gah.setElement(ppt);
//                        inst_Cd_gah.setV3(inst_Cd_gah.getV3() * abs(sampleResult));
//
//
//                        inst_Alpha_gah.setElement(ppt);
//                        inst_Alpha_gah.setF(inst_Alpha_gah.getF() * abs(sampleResult));

                     } // if the instanced point is within the vdb volume

//            else {
//                           int id = static_cast<int>(ppt->getValue<int>(myInstAttrRefs.pointId, 0));
//
//                  std::cout << "VRAY_clusterThis::postProcess() didn't hit sample. pt idx: " << myCurrPtOff << " id: " << id << std::endl;
//
//               }

                  if(myVerbose == CLUSTER_MSG_DEBUG) {
                        pt_counter++;
                        if((long int)(pt_counter % (stat_interval * myNumCopies * myRecursion)) == 0) {
                              std::cout << "VRAY_clusterThis::postProcess() Number of points post processed: " << pt_counter
                                        << "\t - Number of points found in vdb grid: " << pointsFound << std::endl;
                           }
                     }

               }


               if(myVerbose == CLUSTER_MSG_DEBUG) {
                     if(!pointsFound)
                        cout << "VRAY_clusterThis::postProcess() NO POINTS POST PROCESSED!!! " << std::endl;
                     else
                        cout << "VRAY_clusterThis::postProcess() Average instanced points post processed: "
                             << float((float(pointsFound) / float(pt_counter) * 100.0f)) << "%" << std::endl;
                  }


               if(myPostVDBWriteDebugFiles) {

                     openvdb::GridPtrVec outgrids;
                     openvdb::GridPtrVec gradgrids;

                     openvdb::io::File outFile("/tmp/cluster_out_grid.vdb");
                     outgrids.push_back(outputGrid);
                     outFile.write(outgrids);
                     outFile.close();

                     openvdb::io::File gradientFile("/tmp/cluster_gradient_grid.vdb");
                     gradgrids.push_back(gradientGrid);
                     gradientFile.write(gradgrids);
                     gradientFile.close();
                  }


            }   //  if(paList.size() != 0)


      } // if(myVDBPostProcess)


}


#endif


