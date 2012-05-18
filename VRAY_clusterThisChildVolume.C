/* ******************************************************************************
*
*  VRAY_clusterThisChildVolume class
*
* $RCSfile: VRAY_clusterThisChildVolume.C,v $
*
* Description :
*
* $Revision: 1.2 $
*
* $Source: /dca/cvsroot/houdini/VRAY_clusterThis/VRAY_clusterThisChildVolume.C,v $
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

#ifndef __VRAY_clusterThisChildVolume_C__
#define __VRAY_clusterThisChildVolume_C__

#include "VRAY_clusterThisChildVolume.h"


void VRAY_clusterThisChildVolume::getWeightedBoxes(UT_RefArray<UT_BoundingBox> &boxes, UT_FloatArray &weights, fpreal, fpreal dbound) const
{
// std::cout << "VRAY_clusterThisChildVolume::getWeightedBoxes() - dbound: " << dbound << std::endl;

//    boxes.append(myBox);

    UT_BoundingBox aBox;
    aBox.initBounds ( bb_x1, bb_y1, bb_z1);
    aBox.enlargeBounds ( bb_x2, bb_y2, bb_z2);
    boxes.append(aBox);

    weights.append(1.0F);
}

void VRAY_clusterThisChildVolume::getAttributeBinding(UT_StringArray &names, UT_IntArray &sizes) const
{
    names.append(UT_String("density"));
    sizes.append(1);
    names.append(UT_String("radius"));
    sizes.append(1);
}


// virtual void   evaluate (const UT_Vector3 &pos, const UT_Filter &filter, fpreal radius, fpreal time, int idx, fpreal *data) const =0


void VRAY_clusterThisChildVolume::evaluate(const UT_Vector3 &pos, const UT_Filter &filter, fpreal radius, fpreal time, int idx, fpreal *data) const
{

/// cout << "VRAY_clusterThisChildVolume::evaluate() myPointAttributes.myNewPos: " << myPointAttributes.myNewPos << endl;

// cout << "distance: " << distance3d(pos,myPointAttributes.myPos) << endl;

    switch (idx)
    {
// case 0: data[0] = (pos.length2() < 1.0F) ? 1.0F : 0.0F; break;
// case 1: data[0] = pos.length(); break;

    case 0:
        // data[0] = (distance3d(pos, myPointAttributes.myNewPos) < (mySize[0] / 2)) ? 1.0F : 0.0F;
        data[0] = (distance3d(pos, myPointAttributes.myNewPos) < mySize[0]) ? 1.0F : 0.0F;
        break;

    case 1:
        data[0] = mySize[0];
        break;

    default:
        UT_ASSERT(0 && "Invalid attribute evaluation");
    }
}

UT_Vector3 VRAY_clusterThisChildVolume::gradient(const UT_Vector3 &, const UT_Filter &, fpreal, fpreal, int) const
{
    return UT_Vector3(0, 0, 0);
}


#endif





/**********************************************************************************/
//  $Log:
//
//
/**********************************************************************************/


