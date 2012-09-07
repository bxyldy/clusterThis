/* ******************************************************************************
*
*  VRAY_clusterThisVolume class
*
* $RCSfile: VRAY_clusterThisVolume.h,v $
*
* Description :
*
* $Revision: 1.10 $
*
* $Source: /dca/cvsroot/houdini/VRAY_clusterThis/VRAY_clusterThisVolume.h,v $
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

#ifndef __VRAY_clusterThisVolume_h__
#define __VRAY_clusterThisVolume_h__


/* ******************************************************************************
*  Class Name : VRAY_clusterThisVolume()
*
*  Description :  Class for VRAY_clusterThisVolume
*
*
*  Input Arguments : None
*
*  Return Value : None
*
***************************************************************************** */

class VRAY_clusterThisVolume : public VRAY_Volume {
public:

     VRAY_clusterThisVolume(const VRAY_clusterThis *theClusterObj)
          :
          myRadius(theClusterObj->myRadius),
          myDoMotionBlur(theClusterObj->myDoMotionBlur),
          myShutter(theClusterObj->myShutter)

     {
#ifdef DEBUG
          std::cout << "VRAY_clusterThisVolume::VRAY_clusterThisVolume() - Constructor" << std::endl;
#endif

          myBox     = theClusterObj->myBox;
          bb_x1     = theClusterObj->bb_x1;
          bb_x2     = theClusterObj->bb_x2;
          bb_y1     = theClusterObj->bb_y1;
          bb_y2     = theClusterObj->bb_y2;
          bb_z1     = theClusterObj->bb_z1;
          bb_z2     = theClusterObj->bb_z2;

          mySize[0] = theClusterObj->mySize[0];
          mySize[1] = theClusterObj->mySize[1];
          mySize[2] = theClusterObj->mySize[2];

//    myPointAttributes.myPos = theClusterObj->myPointAttributes.myPos;
          myPointAttributes.myNewPos = theClusterObj->myPointAttributes.myNewPos;

          myPointAttributes.Cd = theClusterObj->myPointAttributes.Cd;
          myPointAttributes.Alpha = theClusterObj->myPointAttributes.Alpha;
          myPointAttributes.v = theClusterObj->myPointAttributes.v;
          myPointAttributes.N = theClusterObj->myPointAttributes.N;
          myPointAttributes.id = theClusterObj->myPointAttributes.id;
          myPointAttributes.pscale = theClusterObj->myPointAttributes.pscale;
          myPointAttributes.weight = theClusterObj->myPointAttributes.weight;
          myPointAttributes.material = theClusterObj->myPointAttributes.material;

          myNoiseType = static_cast<UT_Noise::UT_NoiseType> ( 0 );
          myNoise.initialize ( 7, myNoiseType );
          myNoise.setSeed(myPointAttributes.id);

//   UT_FloatArray *weights;
//   UT_RefArray< UT_BoundingBox > boxes;
//   fpreal foo = 1.0;
//   boxes.append(myBox);
////   weights->append(foo);
//
//   UT_VoxelArrayF *myVoxelArray;
//    myVoxelArray = new UT_VoxelArrayF;
//    myVoxelArray->size(100,100,100);
//
//    // We want out of bound values to evaluate to wall voxels.
//    myVoxelArray->setBorder(UT_VOXELBORDER_CONSTANT, foo);
//
//  addVoxelBoxes (*myVoxelArray, boxes, *weights, mySize[0], 0);
//
//  for(int x = 0; x < myVoxelArray->getXRes(); x++)
//     for(int y = 0; x < myVoxelArray->getYRes(); x++)
//         for(int z = 0; x < myVoxelArray->getZRes(); x++)
//            myVoxelArray->setValue ( x,  y,  z, 10.0F);
//
//
//std::cout << "VRAY_clusterThisVolume::VRAY_clusterThisVolume() - numVoxels () " << myVoxelArray->numVoxels () << std::endl;
//std::cout << "VRAY_clusterThisVolume::VRAY_clusterThisVolume() - numTiles () " << myVoxelArray->numTiles () << std::endl;
//std::cout << "VRAY_clusterThisVolume::VRAY_clusterThisVolume() - getXRes () " << myVoxelArray->getXRes () << std::endl;
//std::cout << "VRAY_clusterThisVolume::VRAY_clusterThisVolume() - getYRes () " << myVoxelArray->getYRes () << std::endl;
//std::cout << "VRAY_clusterThisVolume::VRAY_clusterThisVolume() - getZRes () " << myVoxelArray->getZRes () << std::endl;
//std::cout << "VRAY_clusterThisVolume::VRAY_clusterThisVolume() - getMemoryUsage () " << myVoxelArray->getMemoryUsage () << std::endl;


#ifdef DEBUG
          cout << "VRAY_clusterThisVolume::VRAY_clusterThisVolume() myPointAttributes.myNewPos: " << myPointAttributes.myNewPos << endl;
          cout << "VRAY_clusterThisVolume::VRAY_clusterThisVolume() myPointAttributes.material: " << myPointAttributes.material << endl;
          cout << "VRAY_clusterThisVolume::VRAY_clusterThisVolume() myPointAttributes.id: " << myPointAttributes.id << endl;
          cout << "VRAY_clusterThisVolume::VRAY_clusterThisVolume() myPointAttributes.pscale: " << myPointAttributes.pscale << endl;
          cout << "VRAY_clusterThisVolume::VRAY_clusterThisVolume() mySize: " << mySize << endl;
#endif

     }


     void 	getBoxes (UT_RefArray< UT_BoundingBox > &boxes, float radius, float dbound, float zerothreshold) const;
//     void 	evaluateMulti (const UT_Vector3 *pos, const UT_Filter &filter, float radius, const float *time, int idx, float *data, int size, int stride) ;
//     bool 	evaluateInterval (const UT_BoundingBox &box, const UT_Filter &filter, float radius, float time, int idx, float *minval, float *maxval) ;
//     float 	getGradientDelta () ;
//     int 	getNormalAttribute () ;
//     float 	getVoxelSize () ;


     void getWeightedBoxes(UT_RefArray<UT_BoundingBox> &boxes, UT_FloatArray &weights, fpreal radius, fpreal dbound) const;
     void getAttributeBinding(UT_StringArray &names, UT_IntArray &sizes) const;
     void evaluate(const UT_Vector3 &pos, const UT_Filter &filter, fpreal radius, fpreal time, int idx, fpreal *data) const;
     UT_Vector3 gradient(const UT_Vector3 &pos, const UT_Filter &filter, fpreal radius, fpreal time, int idx) const;

     UT_BoundingBox myBox;
     fpreal bb_x1, bb_x2, bb_y1, bb_y2, bb_z1, bb_z2;

     fpreal  myRadius;
     fpreal  mySize[3];
     fpreal  myShutter;
     int     myDoMotionBlur;

     UT_Noise::UT_NoiseType  myNoiseType;
     fpreal  myRough;
     fpreal  myNoiseAmp;
     fpreal  myNoiseAtten;

     UT_Noise myNoise;

     // Point attribute structure
     struct attribute_struct {
          // Required attributes
//      UT_Vector4 myPos;
          UT_Vector4 myNewPos;
          UT_Vector4 myMBPos;
          UT_Vector3 Cd;
          fpreal Alpha;
          UT_Vector3 v;
          UT_Vector3 N;
          int id;
          fpreal pscale;
          fpreal weight;
          UT_String material;
     } myPointAttributes;


};



#endif


/**********************************************************************************/
//  $Log: VRAY_clusterThisVolume.h,v $
//  Revision 1.10  2011-02-06 19:49:15  mstory
//  Modified for Houdini version 11.
//
//  Refactored a lot of the attribute code, cleaned up odds and ends.
//
//  Revision 1.9  2009-11-20 14:59:57  mstory
//  Release 1.4.7 ready.
//
//  Revision 1.8  2009-04-06 17:13:44  mstory
//  Clean up code a bit.
//
//  Revision 1.7  2009-04-06 16:40:58  mstory
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
//  Revision 1.6  2009-02-10 21:55:59  mstory
//  Added all attributes for the CVEX processing of instanced geo.
//  Added OTL version checking.
//
//  Revision 1.5  2008-12-04 05:37:41  mstory
//  .
//
//  Revision 1.4  2008-11-19 01:11:43  mstory
//  Added point instancing.  Fixed the file instancing problem.
//  Most of the shader assignment issues straightened out.
//
//  Revision 1.3  2008-10-20 19:35:00  mstory
//  Added a switch to be able to choose using the addProcedural() method of allocating procedurals.
//
//  Revision 1.2  2008-10-11 18:15:06  mstory
//  .
//
//
/**********************************************************************************/


