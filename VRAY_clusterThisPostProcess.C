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

   GEO_Point * ppt;

   long int num_points = (long int) gdp->points().entries();
   long int stat_interval = (long int)(num_points * 0.10) + 1;



   if(myVerbose > CLUSTER_MSG_INFO)
      cout << "VRAY_clusterThis::postProcess() Processing Voxels" << std::endl;

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
         const GA_PointGroup * sourceGroup = NULL;
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

         openvdb::FloatTree::ValueType sampleResult;
         openvdb::VectorGrid::ValueType gradResult;
         const openvdb::FloatTree aTree;
         myTreePtr = outputGrid->tree();

         openvdb::tools::Filter<openvdb::FloatGrid> fooFilter(outputGrid);
//                           openvdb::tools::Filter<openvdb::FloatGrid> barFilter(outputGrid);

         if(myPostVDBMedianFilter)
            fooFilter.median();

         if(myPostVDBMeanFilter)
            fooFilter.mean();

         if(myPostVDBMeanCurvatureFilter)
            fooFilter.meanCurvature();

         if(myPostVDBLaplacianFilter)
            fooFilter.laplacian();

//                           if(myVDBReNormalizeFilter)
//                              float r = barFilter.renormalize(3, 0.1);

         if(myPostVDBOffsetFilter)
            fooFilter.offset(myPostVDBOffsetFilterAmount);


         openvdb::VectorGrid::Ptr gradientGrid = openvdb::VectorGrid::create();
         gradientGrid->setTransform(transform);
//               gradientGrid->setGridClass(openvdb::GRID_FOG_VOLUME );
         gradientGrid->setGridClass(openvdb::GRID_LEVEL_SET);

         openvdb::tools::Gradient<openvdb::ScalarGrid> myGradient(outputGrid);
         gradientGrid = myGradient.process();
         myGradTreePtr = gradientGrid->tree();


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


         GA_FOR_ALL_GROUP_POINTS(inst_gdp, sourceGroup, ppt) {
//                              myCurrPtOff = ppt->getMapOffset();
//                              std::cout << "myCurrPtOff: " << myCurrPtOff << std::endl;

            inst_pos = ppt->getPos();

// Vec3d worldToIndex   (  const Vec3d &     xyz    )    const

//                              openvdb::Vec3R theIndex =
//                                 (openvdb::Vec3R(inst_pos[0], inst_pos[1], inst_pos[2]));
            openvdb::Vec3R theIndex =
               outputGrid->worldToIndex(openvdb::Vec3R(inst_pos[0], inst_pos[1], inst_pos[2]));

            vdb_radius = static_cast<fpreal>(ppt->getValue<fpreal>(myInstAttrRefs.pointVDBRadius, 0));
//                                    std::cout << "vdb_radius: " << vdb_radius << std::endl;

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

            if(myVerbose == CLUSTER_MSG_DEBUG) {
                  pt_counter++;
                  if((long int)(pt_counter % (stat_interval * myNumCopies * myRecursion)) == 0) {
                        cout << "VRAY_clusterThis::postProcess() Number of points post processed: " << pt_counter
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

}


#endif




