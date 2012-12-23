/* ******************************************************************************
*
*  VRAY_clusterThisPreProcess
*
*
* Description :
*
*
***************************************************************************** */

#ifndef __VRAY_clusterThisPreProcess_C__
#define __VRAY_clusterThisPreProcess_C__


/* ******************************************************************************
*  Function Name : buildVDBGrids()
*
*  Description :   Build the VDB grids to be used for pre & post voxel processing
*
*  Input Arguments : GU_Detail * gdp
*
*  Return Value : None
*
***************************************************************************** */
void VRAY_clusterThis::buildVDBGrids(GU_Detail * gdp)
{
   long int stat_interval = (long int)(myNumSourcePoints * 0.10) + 1;

   GEO_Point * src_ppt;
   GEO_PointPtrArray src_list;
   UT_Vector3 tmp_v, new_v;
   fpreal dist;

   // Perform Open VDB pre processing
//   if(myVDBPreProcess || myVDBPostProcess) {
   if(myVDBPostProcess) {

         if(myVerbose > CLUSTER_MSG_INFO)
            cout << "VRAY_clusterThis::preProcess() Processing Voxels" << std::endl;

         ParticleList paList(gdp, myPostVDBRadiusMult, myPostVDBVelocityMult);
         openvdb::tools::PointSampler mySampler, gradSampler;
//                     openvdb::tools::GridSampling<openvdb::FloatTree>  myGridSampler;

         if(myVerbose == CLUSTER_MSG_DEBUG)
            std::cout << "VRAY_clusterThis::preProcess() paList.size() ... "  << paList.size() << std::endl;

         if(paList.size() != 0) {

               hvdb::Interrupter boss("VRAY_clusterThis::preProcess() Converting particles to level set");

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
               myGeoGrid = openvdb::ScalarGrid::create(background);
               myGeoGrid->setTransform(transform);
               myGeoGrid->setGridClass(openvdb::GRID_LEVEL_SET);

               // Perform the particle conversion.
               this->convert(myGeoGrid, paList, settings, boss);

               if(myVerbose == CLUSTER_MSG_DEBUG) {
                     std::cout << "VRAY_clusterThis::preProcess() - activeVoxelCount(): "
                               << myGeoGrid->activeVoxelCount() << std::endl;
                     std::cout << "VRAY_clusterThis::preProcess() - background: "
                               << myGeoGrid->background() << std::endl;
                  }


//                           openvdb::FloatGrid::Accessor accessor = myGeoGrid->getAccessor();
//
//                           openvdb::Vec3i anIndex = myGeoGrid->worldToIndex(openvdb::Vec3d(-0.0168841, -2.81901, 0.0316872));
//
//                           openvdb::Coord xyz(1000, -200000000, 30000000);
//                           openvdb::Coord xyz(anIndex);
//                           accessor.setValue(xyz, 1.0);
//                           -0.0168841, -2.81901, 0.0316872
//                           std::cout << "myGeoGrid" << xyz << " = " << accessor.getValue(xyz) << std::endl;


//
//                     long int aCounter = 0;
//
//                           for(openvdb::ScalarGrid::ValueOnCIter iter = myGeoGrid->cbeginValueOn(); iter; ++iter)
//                              {
//                                 openvdb::Vec3d worldPos = myGeoGrid->indexToWorld(iter.getCoord());
//                                 openvdb::CoordBBox bbox;
//                                 iter.getBoundingBox(bbox);
//                                 if(((long int)aCounter++ % 100) == 0) {
//                                       std::cout << "myGeoGrid" << iter.getCoord() << " = " << *iter << "\t"
//                                                 << "\tbbox: " << bbox
//                                                 << "\tworldPos: " << worldPos << std::endl;
//                                    }
//                              }
//

               // Insert the new grid into the ouput detail.
               UT_String gridNameStr = "ClusterGrid";
               myGeoGrid->insertMeta("float type", openvdb::StringMetadata("averaged_velocity"));
               myGeoGrid->insertMeta("name", openvdb::StringMetadata((const char *)gridNameStr));
               myGeoGrid->insertMeta("VoxelSize", openvdb::FloatMetadata(myPostVoxelSize));
               myGeoGrid->insertMeta("background", openvdb::FloatMetadata(background));

//    hvdb::createVdbPrimitive(*gdp, myGeoGrid, gridNameStr.toStdString().c_str());
//    GU_PrimVDB (GU_Detail *gdp, GA_Offset offset=GA_INVALID_OFFSET)
//    GU_PrimVDB (const GA_MergeMap &map, GA_Detail &detail, GA_Offset offset, const GA_Primitive &src_prim)


               UT_Vector3 inst_pos, seed_pos, currVel;
//         const GA_PointGroup * sourceGroup = NULL;
               long int pt_counter = 0;
               float vdb_radius = 2.0f;

               if(myVerbose > CLUSTER_MSG_INFO)
                  std::cout << "VRAY_clusterThis::preProcess() - Massaging data ... " << std::endl;

               long int pointsFound = 0;

               GEO_AttributeHandle inst_vel_gah = inst_gdp->getPointAttribute("v");
               GEO_AttributeHandle source_vel_gah = gdp->getPointAttribute("v");
               GEO_AttributeHandle inst_N_gah = inst_gdp->getPointAttribute("N");
               GEO_AttributeHandle source_N_gah = gdp->getPointAttribute("N");
               GEO_AttributeHandle inst_Cd_gah = inst_gdp->getPointAttribute("Cd");
               GEO_AttributeHandle source_Cd_gah = gdp->getPointAttribute("Cd");
               GEO_AttributeHandle inst_Alpha_gah = inst_gdp->getPointAttribute("Alpha");
               GEO_AttributeHandle source_Alpha_gah = gdp->getPointAttribute("Alpha");

//                     openvdb::ScalarGrid::Accessor accessor;
//                     openvdb::FloatTree myTree;
//         openvdb::FloatTree myTree;
               openvdb::FloatTree::ConstPtr myTreePtr;
               openvdb::VectorTree::ConstPtr myGradTreePtr;
               openvdb::FloatTree::ValueType sampleResult;
               openvdb::VectorGrid::ValueType gradResult;
               openvdb::FloatTree & myTree = myGeoGrid->treeRW();

               openvdb::tools::Filter<openvdb::FloatGrid> preFilter(*myGeoGrid);
//                           openvdb::tools::Filter<openvdb::FloatGrid> barFilter(myGeoGrid);



               if(myPreVDBMedianFilter) {
                     if(myVerbose > CLUSTER_MSG_INFO)
                        std::cout << "VRAY_clusterThis::preProcess() - Filtering with median filter ... " << std::endl;
                     for(int n = 0; n < myPostVDBMedianIterations && !boss.wasInterrupted(); ++n)
                        preFilter.median();
                  }


               if(myPostVDBMeanFilter) {
                     std::cout << "VRAY_clusterThis::preProcess() - Filtering with mean filter ... " << std::endl;
                     for(int n = 0; n < myPostVDBMeanIterations && !boss.wasInterrupted(); ++n)
                        preFilter.mean();
                  }

               if(myPostVDBMeanCurvatureFilter) {
                     std::cout << "VRAY_clusterThis::preProcess() - Filtering with mean curvature filter ... " << std::endl;
                     for(int n = 0; n < myPostVDBMeanCurvatureIterations && !boss.wasInterrupted(); ++n)
                        preFilter.meanCurvature();
                  }


               if(myPostVDBLaplacianFilter) {
                     std::cout << "VRAY_clusterThis::preProcess() - Filtering with laplacian filter ... " << std::endl;
                     for(int n = 0; n < myPostVDBLaplacianIterations && !boss.wasInterrupted(); ++n)
                        preFilter.laplacian();
                  }


//                           if(myVDBReNormalizeFilter)
//                              float r = barFilter.renormalize(3, 0.1);

               if(myPostVDBOffsetFilter) {
                     std::cout << "VRAY_clusterThis::preProcess() - Filtering with offset filter ... " << std::endl;
                     preFilter.offset(myPostVDBOffsetFilterAmount);
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

               openvdb::tools::Gradient<openvdb::ScalarGrid> myGradient(*myGeoGrid);
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



               if(myPostVDBWriteDebugFiles || myPostVDBWriteDebugFiles) {

                     openvdb::GridPtrVec outgrids;
                     openvdb::GridPtrVec gradgrids;

                     openvdb::io::File outFile("/tmp/cluster_in_grid.vdb");
                     outgrids.push_back(myGeoGrid);
                     outFile.write(outgrids);
                     outFile.close();

                     openvdb::io::File gradientFile("/tmp/cluster_in_gradient_grid.vdb");
                     gradgrids.push_back(gradientGrid);
                     gradientFile.write(gradgrids);
                     gradientFile.close();
                  }


            }   //  if(paList.size() != 0)


      } // if(myVDBPreProcess)






}



/* ******************************************************************************
*  Function Name : preProcess()
*
*  Description :   pre process the geo
*
*  Input Arguments : GU_Detail * gdp
*
*  Return Value : None
*
***************************************************************************** */

void VRAY_clusterThis::preProcess(GU_Detail * gdp)

{

   long int stat_interval = (long int)(myNumSourcePoints * 0.10) + 1;

   GEO_Point * src_ppt;
   GEO_PointPtrArray src_list;
   UT_Vector3 tmp_v, new_v;
   fpreal dist;

   myPreProcTime = std::clock();
   std::time(&myPreProcStartTime);


   // Perform Open VDB pre processing
   if(myVDBPreProcess) {

         if(myVerbose > CLUSTER_MSG_INFO)
            cout << "VRAY_clusterThis::preProcess() Processing Voxels" << std::endl;

         ParticleList paList(gdp, myPostVDBRadiusMult, myPostVDBVelocityMult);
         openvdb::tools::PointSampler mySampler, gradSampler;
//                     openvdb::tools::GridSampling<openvdb::FloatTree>  myGridSampler;

         if(myVerbose == CLUSTER_MSG_DEBUG)
            std::cout << "VRAY_clusterThis::preProcess() paList.size() ... "  << paList.size() << std::endl;

         if(paList.size() != 0) {

               hvdb::Interrupter boss("VRAY_clusterThis::preProcess() Converting particles to level set");

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
               myGeoGrid = openvdb::ScalarGrid::create(background);
               myGeoGrid->setTransform(transform);
               myGeoGrid->setGridClass(openvdb::GRID_LEVEL_SET);

               // Perform the particle conversion.
               this->convert(myGeoGrid, paList, settings, boss);

               if(myVerbose == CLUSTER_MSG_DEBUG) {
                     std::cout << "VRAY_clusterThis::preProcess() - activeVoxelCount(): "
                               << myGeoGrid->activeVoxelCount() << std::endl;
                     std::cout << "VRAY_clusterThis::preProcess() - background: "
                               << myGeoGrid->background() << std::endl;
                  }


//                           openvdb::FloatGrid::Accessor accessor = myGeoGrid->getAccessor();
//
//                           openvdb::Vec3i anIndex = myGeoGrid->worldToIndex(openvdb::Vec3d(-0.0168841, -2.81901, 0.0316872));
//
//                           openvdb::Coord xyz(1000, -200000000, 30000000);
//                           openvdb::Coord xyz(anIndex);
//                           accessor.setValue(xyz, 1.0);
//                           -0.0168841, -2.81901, 0.0316872
//                           std::cout << "myGeoGrid" << xyz << " = " << accessor.getValue(xyz) << std::endl;


//
//                     long int aCounter = 0;
//
//                           for(openvdb::ScalarGrid::ValueOnCIter iter = myGeoGrid->cbeginValueOn(); iter; ++iter)
//                              {
//                                 openvdb::Vec3d worldPos = myGeoGrid->indexToWorld(iter.getCoord());
//                                 openvdb::CoordBBox bbox;
//                                 iter.getBoundingBox(bbox);
//                                 if(((long int)aCounter++ % 100) == 0) {
//                                       std::cout << "myGeoGrid" << iter.getCoord() << " = " << *iter << "\t"
//                                                 << "\tbbox: " << bbox
//                                                 << "\tworldPos: " << worldPos << std::endl;
//                                    }
//                              }
//

               // Insert the new grid into the ouput detail.
               UT_String gridNameStr = "ClusterGrid";
               myGeoGrid->insertMeta("float type", openvdb::StringMetadata("averaged_velocity"));
               myGeoGrid->insertMeta("name", openvdb::StringMetadata((const char *)gridNameStr));
               myGeoGrid->insertMeta("VoxelSize", openvdb::FloatMetadata(myPostVoxelSize));
               myGeoGrid->insertMeta("background", openvdb::FloatMetadata(background));

//    hvdb::createVdbPrimitive(*gdp, myGeoGrid, gridNameStr.toStdString().c_str());
//    GU_PrimVDB (GU_Detail *gdp, GA_Offset offset=GA_INVALID_OFFSET)
//    GU_PrimVDB (const GA_MergeMap &map, GA_Detail &detail, GA_Offset offset, const GA_Primitive &src_prim)


               UT_Vector3 inst_pos, seed_pos, currVel;
//         const GA_PointGroup * sourceGroup = NULL;
               long int pt_counter = 0;
               float vdb_radius = 2.0f;

               if(myVerbose > CLUSTER_MSG_INFO)
                  std::cout << "VRAY_clusterThis::preProcess() - Massaging data ... " << std::endl;

               long int pointsFound = 0;

               GEO_AttributeHandle inst_vel_gah = inst_gdp->getPointAttribute("v");
               GEO_AttributeHandle source_vel_gah = gdp->getPointAttribute("v");
               GEO_AttributeHandle inst_N_gah = inst_gdp->getPointAttribute("N");
               GEO_AttributeHandle source_N_gah = gdp->getPointAttribute("N");
               GEO_AttributeHandle inst_Cd_gah = inst_gdp->getPointAttribute("Cd");
               GEO_AttributeHandle source_Cd_gah = gdp->getPointAttribute("Cd");
               GEO_AttributeHandle inst_Alpha_gah = inst_gdp->getPointAttribute("Alpha");
               GEO_AttributeHandle source_Alpha_gah = gdp->getPointAttribute("Alpha");

//                     openvdb::ScalarGrid::Accessor accessor;
//                     openvdb::FloatTree myTree;
//         openvdb::FloatTree myTree;
               openvdb::FloatTree::ConstPtr myTreePtr;
               openvdb::VectorTree::ConstPtr myGradTreePtr;
               openvdb::FloatTree::ValueType sampleResult;
               openvdb::VectorGrid::ValueType gradResult;
               openvdb::FloatTree & myTree = myGeoGrid->treeRW();

               openvdb::tools::Filter<openvdb::FloatGrid> preFilter(*myGeoGrid);
//                           openvdb::tools::Filter<openvdb::FloatGrid> barFilter(myGeoGrid);



               if(myPostVDBMedianFilter) {
                     if(myVerbose > CLUSTER_MSG_INFO)
                        std::cout << "VRAY_clusterThis::preProcess() - Filtering with median filter ... " << std::endl;
                     for(int n = 0; n < myPostVDBMedianIterations && !boss.wasInterrupted(); ++n)
                        preFilter.median();
                  }


               if(myPostVDBMeanFilter) {
                     std::cout << "VRAY_clusterThis::preProcess() - Filtering with mean filter ... " << std::endl;
                     for(int n = 0; n < myPostVDBMeanIterations && !boss.wasInterrupted(); ++n)
                        preFilter.mean();
                  }

               if(myPostVDBMeanCurvatureFilter) {
                     std::cout << "VRAY_clusterThis::preProcess() - Filtering with mean curvature filter ... " << std::endl;
                     for(int n = 0; n < myPostVDBMeanCurvatureIterations && !boss.wasInterrupted(); ++n)
                        preFilter.meanCurvature();
                  }


               if(myPostVDBLaplacianFilter) {
                     std::cout << "VRAY_clusterThis::preProcess() - Filtering with laplacian filter ... " << std::endl;
                     for(int n = 0; n < myPostVDBLaplacianIterations && !boss.wasInterrupted(); ++n)
                        preFilter.laplacian();
                  }


//                           if(myVDBReNormalizeFilter)
//                              float r = barFilter.renormalize(3, 0.1);

               if(myPostVDBOffsetFilter) {
                     std::cout << "VRAY_clusterThis::preProcess() - Filtering with offset filter ... " << std::endl;
                     preFilter.offset(myPostVDBOffsetFilterAmount);
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

               openvdb::tools::Gradient<openvdb::ScalarGrid> myGradient(*myGeoGrid);
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



               if(myPostVDBWriteDebugFiles) {

                     openvdb::GridPtrVec outgrids;
                     openvdb::GridPtrVec gradgrids;

                     openvdb::io::File outFile("/tmp/cluster_in_grid.vdb");
                     outgrids.push_back(myGeoGrid);
                     outFile.write(outgrids);
                     outFile.close();

                     openvdb::io::File gradientFile("/tmp/cluster_in_gradient_grid.vdb");
                     gradgrids.push_back(gradientGrid);
                     gradientFile.write(gradgrids);
                     gradientFile.close();
                  }


            }   //  if(paList.size() != 0)

         std::time(&myPreProcEndTime);
         myPreProcExecTime = std::clock() - myPreProcTime;

      } // if(myVDBPreProcess)



}


#endif


