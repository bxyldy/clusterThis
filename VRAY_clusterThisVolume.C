/* ******************************************************************************
*
*  VRAY_clusterThisVolume class
*
* $RCSfile: VRAY_clusterThisVolume.C,v $
*
* Description :
*
* $Revision: 1.9 $
*
* $Source: /dca/cvsroot/houdini/VRAY_clusterThis/VRAY_clusterThisVolume.C,v $
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

#ifndef __VRAY_clusterThisVolume_C__
#define __VRAY_clusterThisVolume_C__

#include "VRAY_clusterThisVolume.h"


//  virtual void  getWeightedBoxes (UT_RefArray< UT_BoundingBox > &boxes, UT_FloatArray &weights, fpreal radius, fpreal dbound) const =0
//    UT_BoundingBox (fpreal axmin, fpreal aymin, fpreal azmin, fpreal axmax, fpreal aymax, fpreal azmax)

void VRAY_clusterThisVolume::getWeightedBoxes(UT_RefArray<UT_BoundingBox> &boxes, UT_FloatArray &weights, fpreal, fpreal dbound) const
{
// std::cout << "VRAY_clusterThisVolume::getWeightedBoxes() - dbound: " << dbound << std::endl;

    UT_BoundingBox aBox;
    aBox.initBounds ( bb_x1, bb_y1, bb_z1);
    aBox.enlargeBounds ( bb_x2, bb_y2, bb_z2);
    boxes.append(aBox);

    weights.append(1.0F);

}

void VRAY_clusterThisVolume::getAttributeBinding(UT_StringArray &names, UT_IntArray &sizes) const
{
    names.append(UT_String("density"));
    sizes.append(1);
    names.append(UT_String("radius"));
    sizes.append(1);
}


// virtual void   evaluate (const UT_Vector3 &pos, const UT_Filter &filter, fpreal radius, fpreal time, int idx, fpreal *data) const =0


void VRAY_clusterThisVolume::evaluate(const UT_Vector3 &pos, const UT_Filter &filter, fpreal radius, fpreal time, int idx, fpreal *data) const
{

// cout << "VRAY_clusterThisVolume::evaluate() myPointAttributes.myNewPos: " << myPointAttributes.myNewPos << endl;
// cout << "distance: " << distance3d(pos,myPointAttributes.myPos) << endl;
//    fpreal noise_bias = ( myNoise.turbulence ( myPointAttributes.myPos, myFractalDepth, myRough, myNoiseAtten ) * myNoiseAmp ) + 1.0;


    switch (idx) {
// case 0: data[0] = (pos.length2() < 1.0F) ? 1.0F : 0.0F; break;
// case 1: data[0] = pos.length(); break;

//    case 0: data[0] = (distance3d(pos, myPointAttributes.myNewPos) < (mySize[0] / 2) * noise_bias) ? 1.0F : 0.0F; break;
    case 0:
        data[0] = (distance3d(pos, myPointAttributes.myNewPos) < mySize[0]) ? 1.0F : 0.0F;
        break;

    case 1:
        data[0] = mySize[0];
        break;

    default:
        UT_ASSERT(0 && "Invalid attribute evaluation");
    }
}

// virtual UT_Vector3   gradient (const UT_Vector3 &pos, const UT_Filter &filter, fpreal radius, fpreal time, int idx) const
UT_Vector3 VRAY_clusterThisVolume::gradient(const UT_Vector3 &pos, const UT_Filter &, fpreal, fpreal, int) const
{
    return UT_Vector3(myPointAttributes.myNewPos - pos);
//    return UT_Vector3(0, 0, 0);
}


#endif





/**********************************************************************************/
//  $Log: VRAY_clusterThisVolume.C,v $
//  Revision 1.9  2009-11-20 14:59:57  mstory
//  Release 1.4.7 ready.
//
//  Revision 1.8  2009-04-06 16:40:58  mstory
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
//  Revision 1.7  2009-02-10 21:55:59  mstory
//  Added all attributes for the CVEX processing of instanced geo.
//  Added OTL version checking.
//
//  Revision 1.6  2008-12-04 05:37:41  mstory
//  .
//
//  Revision 1.5  2008-11-19 01:11:43  mstory
//  Added point instancing.  Fixed the file instancing problem.
//  Most of the shader assignment issues straightened out.
//
//  Revision 1.4  2008-10-20 19:35:00  mstory
//  Added a switch to be able to choose using the addProcedural() method of allocating procedurals.
//
//  Revision 1.2  2008-10-11 18:15:06  mstory
//  .
//
//
/**********************************************************************************/


