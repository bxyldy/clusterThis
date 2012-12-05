/* ******************************************************************************
*
*  VRAY_clusterThisUtil
*
*
* Description :
*
*
***************************************************************************** */


#ifndef __VRAY_clusterThisUtil_C__
#define __VRAY_clusterThisUtil_C__



/* ******************************************************************************
*  Function Name : getBoundingBox
*
*  Description :  Get the bounding box for this VRAY_clusterThis object
*
*  Input Arguments : None
*
*  Return Value : None
*
***************************************************************************** */
void VRAY_clusterThis::getBoundingBox(UT_BoundingBox & box)
{

   box = myVelBox;

#ifdef DEBUG
   std::cout << "VRAY_clusterThis::getBoundingBox() box: " << box << std::endl;
#endif

}


/* ******************************************************************************
*  Function Name : checkRequiredAttributes
*
*  Description :  Check that all the required attributes are in the point cloud
*
*  Input Arguments : None
*
*  Return Value : None
*
***************************************************************************** */
void VRAY_clusterThis::checkRequiredAttributes()
{

   // TODO: Confirm that all required attrs are being checked for each instance type

//   std::cout << "VRAY_clusterThis::checkRequiredAttributes()" << std::endl;
   // Check for required attributes
   if(myPointAttrRefs.Cd.isInvalid()) {
         cout << "Incoming points must have Cd attribute! Throwing exception ..." << endl;
         throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have Cd attribute! ", 1);
      }

   if(myPointAttrRefs.Alpha.isInvalid()) {
         cout << "Incoming points must have Alpha attribute! Throwing exception ..." << endl;
         throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have Alpha attribute! ", 1);
      }

   if(myPointAttrRefs.v.isInvalid()) {
         cout << "Incoming points must have v attribute! Throwing exception ..." << endl;
         throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have v attribute! ", 1);
      }

   if(myPointAttrRefs.N.isInvalid()) {
         cout << "Incoming points must have N attribute! Throwing exception ..." << endl;
         throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have N attribute! ", 1);
      }

   if(myPointAttrRefs.pscale.isInvalid()) {
         cout << "Incoming points must have pscale attribute! Throwing exception ..." << endl;
         throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have pscale attribute! ", 1);
      }

   if(myPointAttrRefs.id.isInvalid()) {
         cout << "Incoming points must have id attribute Throwing exception ..." << endl;
         throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have id attribute! ", 1);
      }

}



/* ******************************************************************************
*  Function Name : convert()
*
*  Description : convert point cloud to VDB level set or fog volume
*
*  Input Arguments : None
*
*  Return Value :
*
* ***************************************************************************** */
void VRAY_clusterThis::convert(
   openvdb::ScalarGrid::Ptr outputGrid,
   ParticleList & paList,
   const Settings & settings,
   hvdb::Interrupter & boss)
{

   openvdb::tools::ParticlesToLevelSet<openvdb::ScalarGrid, ParticleList, hvdb::Interrupter> raster(*outputGrid, boss);

//   std::cout << "VRAY_clusterThis::convert() " << std::endl;


   raster.setRmin(settings.mRadiusMin);

   if(myVerbose == CLUSTER_MSG_DEBUG) {
         std::cout << "VRAY_clusterThis::convert(): raster.getVoxelSize(): " << raster.getVoxelSize() << std::endl;
         std::cout << "VRAY_clusterThis::convert(): raster.getRmin(): " << raster.getRmin() << std::endl;
         std::cout << "VRAY_clusterThis::convert(): raster.getHalfWidth(): " << raster.getHalfWidth() << std::endl;
      }

   if(raster.getHalfWidth() < openvdb::Real(2)) {
         std::cout << "VRAY_clusterThis::convert(): Half width of narrow-band < 2 voxels which creates holes when meshed!" << std::endl;
      }
   else
      if(raster.getHalfWidth() > openvdb::Real(1000)) {
            throw std::runtime_error(
               "VRAY_clusterThis::convert(): Half width of narrow-band > 1000 voxels which exceeds memory limitations!");
         }

   if(settings.mRasterizeTrails && paList.hasVelocity()) {
         if(myVerbose == CLUSTER_MSG_DEBUG)
            std::cout << "VRAY_clusterThis::convert(): rasterizing trails"  << std::endl;
         raster.rasterizeTrails(paList, settings.mDx);
      }
   else {
         if(myVerbose == CLUSTER_MSG_DEBUG)
            std::cout << "VRAY_clusterThis::convert(): rasterizing spheres"  << std::endl;
         raster.rasterizeSpheres(paList);
      }

   if(boss.wasInterrupted()) {
         if(myVerbose == CLUSTER_MSG_DEBUG)
            std::cout << "VRAY_clusterThis::convert(): Process was interrupted"  << std::endl;
         return;
      }

   // Convert the level-set into a fog volume.
   if(settings.mFogVolume) {
         if(myVerbose == CLUSTER_MSG_DEBUG)
            std::cout << "VRAY_clusterThis::convert(): converting to fog volume"  << std::endl;
         float cutOffDist = std::numeric_limits<float>::max();
         if(settings.mGradientWidth > 1e-6)
            cutOffDist = settings.mGradientWidth;
         openvdb::tools::levelSetToFogVolume(*outputGrid, cutOffDist, false);
      }

// print stats of the vdb grid
   if(myVerbose == CLUSTER_MSG_DEBUG)
      outputGrid->print();

}




int VRAY_clusterThis::convertVDBUnits()
{
//     const bool toWSUnits = static_cast<bool>(evalInt("worldSpaceUnits", 0, 0));
//
//     if (toWSUnits) {
//         setFloat("bandWidthWS", 0, 0, evalFloat("bandWidth", 0, 0) * mVoxelSize);
//         return 1;
//     }
//
//     setFloat("bandWidth", 0, 0, evalFloat("bandWidthWS", 0, 0) / mVoxelSize);
//
   return 1;
}




/* ******************************************************************************
*  Function Name : exitClusterThis()
*
*  Description :  Start the exit process
*
*  Input Arguments : void *data
*
*  Return Value :
*
***************************************************************************** */
void VRAY_clusterThis::exitClusterThis(void * data)
{
   VRAY_clusterThis * me = (VRAY_clusterThis *)data;

   if(me->myVerbose > CLUSTER_MSG_INFO)
      std::cout << std::endl << std::endl << "VRAY_clusterThis::exitClusterThis() - Preparing to exit!" << std::endl;


//   if(me->myVerbose > CLUSTER_MSG_INFO)
//      cout << "VRAY_clusterThis::exitClusterThis() myTempFname: " << (const char *)me->myTempFname << endl;


//   if(me->tempFileDeleted) {
//         me->tempFileDeleted = true;
//         cout << "VRAY_clusterThis::exitClusterThis(): " << me->tempFileDeleted << endl;
////         me->exitClusterThisReal((const char *)me->myTempFname);
//         me->exitClusterThisReal(data);
//      }


   if(me->myVerbose > CLUSTER_MSG_INFO)
      cout << "VRAY_clusterThis::exitClusterThis() - Running exit processing" << endl;

//   cout << "VRAY_clusterThis::exitClusterThis(): " << me->tempFileDeleted << endl;
//
//   cout << "VRAY_clusterThis::exitClusterThis() - temp filename " << me->myTempFname << endl;

//   const char * fname = me->myTempFname;
//
////   ofstream myStream;
//
////   myStream.open("exit_data.txt", ios_base::app);
////   myStream << this->exitData.exitTime << std::endl;
////   myStream << this->exitData.exitCode << std::endl;
////   myStream.flush();
////   myStream.close();
////   cout << "VRAY_clusterThis::exitClusterThisReal() : " << this->exitData.exitTime << endl;
////
//
//   struct stat fileResults;
//
//   if(me->myUseTempFile && !me->mySaveTempFile) {
//         if((UT_String(fname)).isstring() && stat(fname, &fileResults) == 0) {
//               if(me->myVerbose > CLUSTER_MSG_INFO)
//                  cout << "VRAY_clusterThis::exitClusterThis() - Found temp file " << fname << endl;
//               if(!remove(fname) && (me->myVerbose > CLUSTER_MSG_INFO))
//                  cout << "VRAY_clusterThis::exitClusterThis() - Removed geometry temp file: " << fname << endl;
//            }
//         else
//            if(me->myVerbose > CLUSTER_MSG_INFO)
//               cout << "VRAY_clusterThis::exitClusterThis() - Did not find temp file " << fname << endl << endl;
//      }
//
//

   const char * fname = me->myTempFname;

   me->exitClusterThisReal(fname);
//         me->exitClusterThisReal(data);

}



/* ******************************************************************************
*  Function Name : exitClusterThisReal()
*
*  Description : Clean up temp file if used, save stats to file and DB
*
*  Input Arguments : None
*
*  Return Value :
*
* ***************************************************************************** */
//void VRAY_clusterThis::exitClusterThisReal(void * data)
void VRAY_clusterThis::exitClusterThisReal(const char * fname)
{
   struct stat fileResults;
//   VRAY_clusterThis * me = (VRAY_clusterThis *)data;


   if(myVerbose > CLUSTER_MSG_INFO)
      cout << "VRAY_clusterThis::exitClusterThisReal() - Running exit processing" << endl;


//   cout << "VRAY_clusterThis::exitClusterThisReal(): " << tempFileDeleted << endl;
//
//   cout << "VRAY_clusterThis::exitClusterThisReal() - temp filename " << myTempFname << endl;
//
//   const char * fname = me->myTempFname;

//   ofstream myStream;

//   myStream.open("exit_data.txt", ios_base::app);
//   myStream << this->exitData.exitTime << std::endl;
//   myStream << this->exitData.exitCode << std::endl;
//   myStream.flush();
//   myStream.close();
//   cout << "VRAY_clusterThis::exitClusterThisReal() : " << this->exitData.exitTime << endl;
//


   if(this->myUseTempFile && !this->mySaveTempFile) {
         if((UT_String(fname)).isstring() && stat(fname, &fileResults) == 0) {
               if(myVerbose > CLUSTER_MSG_INFO)
                  cout << "VRAY_clusterThis::exitClusterThisReal() - Found temp file " << fname << endl;
               if(!remove(fname) && (myVerbose > CLUSTER_MSG_INFO))
                  cout << "VRAY_clusterThis::exitClusterThisReal() - Removed geometry temp file: " << fname << endl;
            }
         else
            if(myVerbose > CLUSTER_MSG_INFO)
               cout << "VRAY_clusterThis::exitClusterThisReal() - Did not find temp file " << fname << endl << endl;
      }



   if(myVerbose > CLUSTER_MSG_INFO)
      std::cout << "VRAY_clusterThis::exitClusterThisReal() - Exiting" << std::endl;
}





/// ********************** MISC. STATIC FUNCTIONS *************************************




static void getRoughBBox(UT_BoundingBox & box, UT_BoundingBox & vbox,
                         const GEO_Point * point, const fpreal scale,
//                         const GEO_Point * point, const UT_Vector3 & scale,
                         const GA_ROAttributeRef & voff,
                         fpreal tscale, const UT_Matrix4 & xform)
{
   fpreal     maxradius;
//   static fpreal isin45 = 1.0F / SYSsin(M_PI / 4);  // isin45 = 1.41421
   UT_Vector3    pt;

   maxradius = scale * 0.5F;
//   maxradius = SYSmax(scale.x(), scale.y()) * isin45 * 0.5F;

   pt = UT_Vector3(-maxradius, -maxradius, 0) * xform;
   box.initBounds(pt);
   pt = UT_Vector3(-maxradius,  maxradius, 0) * xform;
   box.enlargeBounds(pt);
   pt = UT_Vector3(maxradius, -maxradius, 0) * xform;
   box.enlargeBounds(pt);
   pt = UT_Vector3(maxradius,  maxradius, 0) * xform;
   box.enlargeBounds(pt);

   box.translate(point->getPos());
   vbox = box;

   UT_Vector3  vel;
   int      i;
   fpreal      amount;

   vel = point->getValue<UT_Vector3>(voff);
//         std::cout << "getRoughBBox() vel: " << vel << std::endl;
   for(i = 0; i < 3; i++) {
         amount = vel(i) * tscale;
         if(amount < 0)
            vbox.vals[i][1] -= amount;
         else
            vbox.vals[i][0] -= amount;

      }
//         std::cout << "getRoughBBox() amount: " << amount << std::endl;
//   std::cout << "getRoughBBox() \nbox: " << box << "vbox: " << vbox << std::endl;


}

static inline void clampBox(UT_BoundingBox & from, const UT_BoundingBox & to)
{
   if(from.vals[0][0] < to.vals[0][0])
      from.vals[0][0] = to.vals[0][0];
   if(from.vals[1][0] < to.vals[1][0])
      from.vals[1][0] = to.vals[1][0];
   if(from.vals[2][0] < to.vals[2][0])
      from.vals[2][0] = to.vals[2][0];
   if(from.vals[0][1] > to.vals[0][1])
      from.vals[0][1] = to.vals[0][1];
   if(from.vals[1][1] > to.vals[1][1])
      from.vals[1][1] = to.vals[1][1];
   if(from.vals[2][1] > to.vals[2][1])
      from.vals[2][1] = to.vals[2][1];
}

static inline int testClampBox(const UT_BoundingBox & from, const UT_BoundingBox & to)
{
   if(from.vals[0][0] < to.vals[0][0])
      return 1;
   if(from.vals[1][0] < to.vals[1][0])
      return 1;
   if(from.vals[2][0] < to.vals[2][0])
      return 1;
   if(from.vals[0][1] > to.vals[0][1])
      return 1;
   if(from.vals[1][1] > to.vals[1][1])
      return 1;
   if(from.vals[2][1] > to.vals[2][1])
      return 1;
   return 0;
}


static short int myPasses(int mode)
{
   static short int num_passes; // keep track of how many times the DSO gets called
   if(mode)
      return num_passes;
   else
      num_passes++;
   return 0;

}



static inline int calculateNewInstPosition(fpreal theta, uint32 i, uint32 j)
{
#ifdef DEBUG
   cout << "VRAY_clusterThis::calculateNewInstPosition() i: " << i << " j: " << j << endl;
#endif

   // Calculate a new position for the object ...
//   fpreal delta = theta * i;
//   fpreal dx, dy, dz = 0.0;
//   dx = SYSsin(delta * myFreqX + myOffsetX);
//   dy = SYScos(delta * myFreqY + myOffsetY);
//   dz = SYScos(delta * myFreqZ + myOffsetZ);

//#ifdef DEBUG
//   cout << "VRAY_clusterThis::calculateNewInstPosition() " << "delta: " << delta << endl;
//   cout << "VRAY_clusterThis::calculateNewInstPosition() " << "dx: " << dx << " dy: " << dy << " dz: " << dz << endl;
//#endif

//   myNoise.setSeed(myPointAttributes.id);

   // Calculate a bit of noise to add to the new position ...
   // TODO:
//   fpreal noise_bias = (myNoise.turbulence(myPointAttributes.myPos, myFractalDepth, myRough, myNoiseAtten) * myNoiseAmp) + 1.0;

//      UT_Vector3 myNoiseVec;
   // myNoise.turbulence(myPos, myFractalDepth, myNoiseVec, myRough, myNoiseAtten);
   // cout << "VRAY_clusterThis::render() " << "myNoiseVec: " << myNoiseVec.x() << " " << myNoiseVec.x() << " " << myNoiseVec.x() << endl;

//#ifdef DEBUG
//   cout << "VRAY_clusterThis::calculateNewInstPosition() " << "noise_bias: " << noise_bias << endl;
//#endif

//   // Calculate the new object's position
//   myPointAttributes.myNewPos[0] = (fpreal) myPointAttributes.myPos.x() +
//                                   ((dx * myRadius) * noise_bias * SYSsin(static_cast<fpreal>(j + i)));
//   myPointAttributes.myNewPos[1] = (fpreal) myPointAttributes.myPos.y() +
//                                   ((dy * myRadius) * noise_bias * SYScos(static_cast<fpreal>(j + i)));
//   myPointAttributes.myNewPos[2] = (fpreal) myPointAttributes.myPos.z() +
//                                   ((dz * myRadius) * noise_bias * (SYSsin(static_cast<fpreal>(j + i)) + SYScos(static_cast<fpreal>(j + i))));
////   myPointAttributes.myNewPos[2] = ( fpreal ) myPointAttributes.myPos.z() +
////                                    ( ( dz * myRadius ) * noise_bias * ( SYScos ( static_cast<fpreal>(j + i)) ) );
//
//   if (myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
//      myPointAttributes.myMBPos[0] = myPointAttributes.myNewPos[0] - myPointAttributes.v.x();
//      myPointAttributes.myMBPos[1] = myPointAttributes.myNewPos[1] - myPointAttributes.v.y();
//      myPointAttributes.myMBPos[2] = myPointAttributes.myNewPos[2] - myPointAttributes.v.z();
//   }

//#ifdef DEBUG
//   cout << "VRAY_clusterThis::calculateNewInstPosition() myPos:   "
//        << myPointAttributes.myPos.x() << " " << myPointAttributes.myPos.y() << " " << myPointAttributes.myPos.z() << endl;
//   cout << "VRAY_clusterThis::calculateNewInstPosition() newPos: "
//        << myPointAttributes.myNewPos[0] << " " << myPointAttributes.myNewPos[1] << " " << myPointAttributes.myNewPos[2] << endl;
//#endif

   return 0;

}


static inline int computeDivs(fpreal inc, fpreal min)
{
   int  divs = (int)SYSceil(inc / min);
   if(divs < 1)
      divs = 1;
   else
      if(divs > 4)
         divs = 4;
   return divs;
}



#endif
