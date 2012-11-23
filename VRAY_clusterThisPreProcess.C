/* ******************************************************************************
*
*  VRAY_clusterThisPreProcess
*
* $RCSfile: VRAY_clusterThisPreProcess.C,v $
*
* Description :
*
*
***************************************************************************** */

#ifndef __VRAY_clusterThisPreProcess_C__
#define __VRAY_clusterThisPreProcess_C__


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

   GEO_Point * ppt;

   long int num_points = (long int) gdp->points().entries();
   long int stat_interval = (long int)(num_points * 0.10) + 1;


   for(uint32 i = gdp->points().entries(); i-- > 0;) {
         ppt = gdp->points()(i);

         myPointList.append(i);

      }



   // If the user wants to build grids for pre processing
   // TODO: Should this be an option?  There may be functions/features that will depend on this ... discuss!
   if(!myPreProcess)
      return;

   if(myVerbose > CLUSTER_MSG_INFO)
      cout << "VRAY_clusterThis::preProcess() Pre Processing Voxels" << std::endl;

//                     openvdb::ScalarGrid::Accessor accessor;
//                     openvdb::FloatTree myTree;
   openvdb::FloatTree::ConstPtr myGeoTreePtr;
   openvdb::VectorTree::ConstPtr myGeoGradTreePtr;

   ParticleList paGeoList(gdp, myPreVDBRadiusMult, myPreVDBVelocityMult);
   openvdb::tools::PointSampler myGeoSampler, geoGradSampler;
//                     openvdb::tools::GridSampling<openvdb::FloatTree>  myGridSampler;

   if(myVerbose == CLUSTER_MSG_DEBUG)
      std::cout << "VRAY_clusterThis::preProcess() paGeoList.size() ... "  << paGeoList.size() << std::endl;

   if(paGeoList.size() != 0) {

         hvdb::Interrupter boss("VRAY_clusterThis::preProcess() Converting particles to level set");

         Settings settings;
         settings.mRadiusMin = myPreRadiusMin;
         settings.mRasterizeTrails = myPreRasterType;
         settings.mDx = myPreDx;  // only used for rasterizeTrails()
         settings.mFogVolume = myPreFogVolume;
         settings.mGradientWidth = myPreGradientWidth;  // only used for fog volume

         float background;

         // background in WS units
         if(myPreWSUnits)
            background = myPreBandWidth;
         // background NOT in WS units
         else
            background = myPreVoxelSize * myPreBandWidth;

         // Construct a new scalar grid with the specified background value.
         openvdb::math::Transform::Ptr transform =
            openvdb::math::Transform::createLinearTransform(myPreVoxelSize);
//         openvdb::ScalarGrid::Ptr myGeoGrid = openvdb::ScalarGrid::create(background);
         myGeoGrid = openvdb::ScalarGrid::create(background);

         myGeoGrid->setTransform(transform);
         myGeoGrid->setGridClass(openvdb::GRID_LEVEL_SET);

         // Perform the particle conversion.
         this->convert(myGeoGrid, paGeoList, settings, boss);

         if(myVerbose == CLUSTER_MSG_DEBUG) {
               std::cout << "VRAY_clusterThis::preProcess() - activeVoxelCount(): "
                         << myGeoGrid->activeVoxelCount() << std::endl;
               std::cout << "VRAY_clusterThis::preProcess() - background: "
                         << myGeoGrid->background() << std::endl;
            }

         // Insert the new grid into the ouput detail.
         UT_String gridNameStr = "ClusterGrid";
         myGeoGrid->insertMeta("float type", openvdb::StringMetadata("averaged_velocity"));
         myGeoGrid->insertMeta("name", openvdb::StringMetadata((const char *)gridNameStr));
         myGeoGrid->insertMeta("VoxelSize", openvdb::FloatMetadata(myPreVoxelSize));
         myGeoGrid->insertMeta("background", openvdb::FloatMetadata(background));


         UT_Vector3 pos, seed_pos, currVel;
         const GA_PointGroup * sourceGroup = NULL;
         long int pt_counter = 0;
         float radius = 5.0f;

         if(myVerbose > CLUSTER_MSG_INFO)
            std::cout << "VRAY_clusterThis::preProcess() - Massaging data ... " << std::endl;

         long int pointsFound = 0;
         GEO_AttributeHandle inst_vel_gah = gdp->getPointAttribute("v");
         GEO_AttributeHandle source_vel_gah = gdp->getPointAttribute("v");
         GEO_AttributeHandle inst_N_gah = gdp->getPointAttribute("N");
         GEO_AttributeHandle source_N_gah = gdp->getPointAttribute("N");
         GEO_AttributeHandle inst_Cd_gah = gdp->getPointAttribute("Cd");
         GEO_AttributeHandle source_Cd_gah = gdp->getPointAttribute("Cd");
         GEO_AttributeHandle inst_Alpha_gah = gdp->getPointAttribute("Alpha");
         GEO_AttributeHandle source_Alpha_gah = gdp->getPointAttribute("Alpha");

         if(!inst_vel_gah.isAttributeValid())
            throw VRAY_clusterThis_Exception("VRAY_clusterThis::preProcess() Instance velocity handle invalid, exiting ...", 1);
         if(!source_vel_gah.isAttributeValid())
            throw VRAY_clusterThis_Exception("VRAY_clusterThis::preProcess() Source velocity handle invalid, exiting ...", 1);
         if(!inst_N_gah.isAttributeValid())
            throw VRAY_clusterThis_Exception("VRAY_clusterThis::preProcess() Instance normal handle invalid, exiting ...", 1);
         if(!source_N_gah.isAttributeValid())
            throw VRAY_clusterThis_Exception("VRAY_clusterThis::preProcess() Source normal handle invalid, exiting ...", 1);
         if(!inst_Cd_gah.isAttributeValid())
            throw VRAY_clusterThis_Exception("VRAY_clusterThis::preProcess() Instance color handle invalid, exiting ...", 1);
         if(!source_Cd_gah.isAttributeValid())
            throw VRAY_clusterThis_Exception("VRAY_clusterThis::preProcess() Source color handle invalid, exiting ...", 1);
         if(!inst_Alpha_gah.isAttributeValid())
            throw VRAY_clusterThis_Exception("VRAY_clusterThis::preProcess() Instance alpha handle invalid, exiting ...", 1);
         if(!source_Alpha_gah.isAttributeValid())
            throw VRAY_clusterThis_Exception("VRAY_clusterThis::preProcess() Source alpha handle invalid, exiting ...", 1);

         openvdb::FloatTree::ValueType sampleResult;
         openvdb::VectorGrid::ValueType gradResult;
         const openvdb::FloatTree aTree;
         myGeoTreePtr = myGeoGrid->tree();

         openvdb::tools::Filter<openvdb::FloatGrid> preProcessFilter(myGeoGrid);
//                           openvdb::tools::Filter<openvdb::FloatGrid> barFilter(myGeoGrid);

         if(myPreVDBMedianFilter)
            preProcessFilter.median();

         if(myPreVDBMeanFilter)
            preProcessFilter.mean();

         if(myPreVDBMeanCurvatureFilter)
            preProcessFilter.meanCurvature();

         if(myPreVDBLaplacianFilter)
            preProcessFilter.laplacian();

//                           if(myVDBReNormalizeFilter)
//                              float r = barFilter.renormalize(3, 0.1);

         if(myPreVDBOffsetFilter)
            preProcessFilter.offset(myPreVDBOffsetFilterAmount);


         myGradientGrid = openvdb::VectorGrid::create();
//         openvdb::VectorGrid::Ptr myGradientGrid = openvdb::VectorGrid::create();
         myGradientGrid->setTransform(transform);
//               myGradientGrid->setGridClass(openvdb::GRID_FOG_VOLUME );
         myGradientGrid->setGridClass(openvdb::GRID_LEVEL_SET);

         openvdb::tools::Gradient<openvdb::ScalarGrid> myGradient(myGeoGrid);
         myGradientGrid = myGradient.process();
         myGeoGradTreePtr = myGradientGrid->tree();

         gridNameStr = "ClusterGradientGrid";
         myGradientGrid->insertMeta("vector type", openvdb::StringMetadata("covariant (gradient)"));
         myGradientGrid->insertMeta("name", openvdb::StringMetadata((const char *)gridNameStr));
         myGradientGrid->insertMeta("VoxelSize", openvdb::FloatMetadata(myPreVoxelSize));
         myGradientGrid->insertMeta("background", openvdb::FloatMetadata(background));


         GA_FOR_ALL_GROUP_POINTS(gdp, sourceGroup, ppt) {
//                              myCurrPtOff = ppt->getMapOffset();
//                              std::cout << "myCurrPtOff: " << myCurrPtOff << std::endl;

            pos = ppt->getPos();

// Vec3d worldToIndex   (  const Vec3d &     xyz    )    const

//                              openvdb::Vec3R theIndex =
//                                 (openvdb::Vec3R(pos[0], pos[1], pos[2]));
            openvdb::Vec3R theIndex =
               myGeoGrid->worldToIndex(openvdb::Vec3R(pos[0], pos[1], pos[2]));

            radius = static_cast<fpreal>(ppt->getValue<fpreal>(myInstAttrRefs.pointVDBRadius, 0));
//                                    std::cout << "radius: " << radius << std::endl;

// static bool    sample (const TreeT &inTree, const Vec3R &inCoord, typename TreeT::ValueType &sampleResult)
            const openvdb::Vec3R  inst_sample_pos(theIndex[0], theIndex[1], theIndex[2]);

            bool success = myGeoSampler.sample(*myGeoTreePtr, inst_sample_pos, sampleResult);

            geoGradSampler.sample(*myGeoGradTreePtr, inst_sample_pos, gradResult);
//
//                              std::cout << "success: " << success << "\tpos: " << pos
//                                        << "\tinst_sample_pos: " << inst_sample_pos
//                                        << "\tsampleResult: " << sampleResult << std::endl;

//ValueType    sampleWorld (const Vec3R &pt) const
//ValueType    sampleWorld (Real x, Real y, Real z) const

            // if the instanced point is within the vdb volume
            if(success) {
//                                    std::cout << "pos: " << pos << " inst_sample_pos: "
//                                              << inst_sample_pos << " sampleResult: " << sampleResult
//                                              << " gradResult: " << gradResult << std::endl;
//                                    float weight;
                  pointsFound++;

                  inst_vel_gah.setElement(ppt);
                  currVel = inst_vel_gah.getV3();

                  UT_Vector3 gradVect = UT_Vector3(gradResult[0], gradResult[1], gradResult[2]);

                  ppt->setPos(pos + (myPrePosInfluence *(sampleResult * gradVect)));
//                                    ppt->setPos(pos + (sampleResult * myPosInfluence *(currVel / myFPS)));

//                                    inst_vel_gah.setV3(currVel * ((1 / sampleResult) * radius));
                  inst_vel_gah.setV3(currVel + (myPreVelInfluence *(sampleResult * gradVect)));

//                                    std::cout << "currVel: " << currVel << " sampleResult " << sampleResult
//                                              << " new vel: " <<  currVel * sampleResult << std::endl;

                  inst_N_gah.setV3(inst_N_gah.getV3() + (myPreNormalInfluence *(sampleResult * gradVect)));

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
                        cout << "VRAY_clusterThis::preProcess() Number of points pre processed: " << pt_counter
                             << "\t - Number of points found in vdb grid: " << pointsFound << std::endl;
                     }
               }

         }


         if(myVerbose == CLUSTER_MSG_DEBUG) {
               if(!pointsFound)
                  cout << "VRAY_clusterThis::preProcess() NO POINTS PRE PROCESSED!!! " << std::endl;
               else
                  cout << "VRAY_clusterThis::preProcess() Average instanced points pre processed: "
                       << float((float(pointsFound) / float(pt_counter) * 100.0f)) << "%" << std::endl;
            }


         if(myPreVDBWriteDebugFiles) {

               openvdb::GridPtrVec outgrids;
               openvdb::GridPtrVec gradgrids;

               openvdb::io::File outFile("/tmp/cluster_in_grid.vdb");
               outgrids.push_back(myGeoGrid);
               outFile.write(outgrids);
               outFile.close();

               openvdb::io::File gradientFile("/tmp/cluster_in_gradient_grid.vdb");
               gradgrids.push_back(myGradientGrid);
               gradientFile.write(gradgrids);
               gradientFile.close();
            }


      }   //  if(paGeoList.size() != 0)


   return;



}


#endif




