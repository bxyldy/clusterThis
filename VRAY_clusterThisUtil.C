#ifndef __VRAY_clusterThisUtil_C__
#define __VRAY_clusterThisUtil_C__


static void getRoughBBox(UT_BoundingBox & box, UT_BoundingBox & vbox,
                              const GEO_Point * point, const UT_Vector3 & scale,
                              const GA_ROAttributeRef & voff,
                              fpreal tscale, const UT_Matrix4 & xform)
{
   fpreal     maxradius;
   static fpreal isin45 = 1.0F / SYSsin(M_PI / 4);  // isin45 = 1.41421
   UT_Vector3    pt;

//   std::cout << "getRoughBBox() \nbox: " << box << "vbox: " << vbox << std::endl;

   maxradius = SYSmax(scale.x(), scale.y()) * isin45 * 0.5F;

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

   if(voff.isValid()) {
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

      }


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

#ifdef DEBUG
   cout << "VRAY_clusterThis::calculateNewInstPosition() " << "delta: " << delta << endl;
   cout << "VRAY_clusterThis::calculateNewInstPosition() " << "dx: " << dx << " dy: " << dy << " dz: " << dz << endl;
#endif

//   myNoise.setSeed(myPointAttributes.id);

   // Calculate a bit of noise to add to the new position ...
   // TODO:
//   fpreal noise_bias = (myNoise.turbulence(myPointAttributes.myPos, myFractalDepth, myRough, myNoiseAtten) * myNoiseAmp) + 1.0;

   // myNoise.turbulence(myPos, myFractalDepth, myNoiseVec, myRough, myNoiseAtten);
   // cout << "VRAY_clusterThis::render() " << "myNoiseVec: " << myNoiseVec.x() << " " << myNoiseVec.x() << " " << myNoiseVec.x() << endl;

#ifdef DEBUG
   cout << "VRAY_clusterThis::calculateNewInstPosition() " << "noise_bias: " << noise_bias << endl;
#endif

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

#ifdef DEBUG
   cout << "VRAY_clusterThis::calculateNewInstPosition() myPos:   "
        << myPointAttributes.myPos.x() << " " << myPointAttributes.myPos.y() << " " << myPointAttributes.myPos.z() << endl;
   cout << "VRAY_clusterThis::calculateNewInstPosition() newPos: "
        << myPointAttributes.myNewPos[0] << " " << myPointAttributes.myNewPos[1] << " " << myPointAttributes.myNewPos[2] << endl;
#endif

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


static inline void getInstBBox(UT_BoundingBox & box, UT_BoundingBox & vbox,
                               const GEO_Point * point, const UT_Vector3 & sprite_scale,
                               const GA_ROAttributeRef & voff,
                               fpreal tscale, const UT_Matrix4 & xform)
{
   fpreal     maxradius;
   static fpreal isin45 = 1.0F / SYSsin(M_PI / 4);
   UT_Vector3    pt;

   maxradius = SYSmax(sprite_scale.x(), sprite_scale.y()) * isin45 * 0.5F;

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

   if(voff.isValid()) {
         UT_Vector3  vel;
         int      i;
         fpreal      amount;

         vel = point->getValue<UT_Vector3>(voff);
         for(i = 0; i < 3; i++) {
               amount = vel(i) * tscale;
               if(amount < 0)
                  vbox.vals[i][1] -= amount;
               else
                  vbox.vals[i][0] -= amount;

            }
      }
}


#endif
