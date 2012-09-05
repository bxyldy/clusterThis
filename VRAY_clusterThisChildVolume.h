/* ******************************************************************************
*
*  VRAY_clusterThisChildVolume class
*
* $RCSfile: VRAY_clusterThisChildVolume.h,v $
*
* Description :
*
* $Revision: 1.2 $
*
* $Source: /dca/cvsroot/houdini/VRAY_clusterThis/VRAY_clusterThisChildVolume.h,v $
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

#ifndef __VRAY_clusterThisChildVolume_h__
#define __VRAY_clusterThisChildVolume_h__


/* ******************************************************************************
*  Class Name : VRAY_clusterThisChildVolume()
*
*  Description :  Class for VRAY_clusterThisChildVolume
*
*
*  Input Arguments : None
*
*  Return Value : None
*
***************************************************************************** */

class VRAY_clusterThisChildVolume : public VRAY_Volume {
public:

    VRAY_clusterThisChildVolume(const VRAY_clusterThisChild *theClusterObj)
            :
            myRadius(theClusterObj->myRadius),
            myDoMotionBlur(theClusterObj->myDoMotionBlur),
            myShutter(theClusterObj->myShutter)

    {
#ifdef DEBUG
        std::cout << "VRAY_clusterThisChildVolume::VRAY_clusterThisChildVolume() - Constructor" << std::endl;
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


        //UT_FloatArray *weights;
        //UT_RefArray< UT_BoundingBox > boxes;
        //fpreal foo = 1.0;
        //boxes.append(myBox);

        //UT_VoxelArrayF *myVoxelArray;
        // myVoxelArray = new UT_VoxelArrayF;
        // myVoxelArray->size(100,100,100);

        // We want out of bound values to evaluate to wall voxels.
//    myVoxelArray->setBorder(UT_VOXELBORDER_CONSTANT, foo);

//   VRAY_clusterThisVolume *aVol = new VRAY_clusterThisVolume(this);

//  addVoxelBoxes (*myVoxelArray, boxes, *weights, mySize[0], 0);

        //int  numVoxels () const
        //   int  numTiles () const

#ifdef DEBUG
        cout << "VRAY_clusterThisChildVolume::VRAY_clusterThisChildVolume() myPointAttributes.myNewPos: " << myPointAttributes.myNewPos << endl;
        cout << "VRAY_clusterThisChildVolume::VRAY_clusterThisChildVolume() myPointAttributes.material: " << myPointAttributes.material << endl;
        cout << "VRAY_clusterThisChildVolume::VRAY_clusterThisChildVolume() myPointAttributes.id: " << myPointAttributes.id << endl;
        cout << "VRAY_clusterThisChildVolume::VRAY_clusterThisChildVolume() myPointAttributes.pscale: " << myPointAttributes.pscale << endl;
        cout << "VRAY_clusterThisChildVolume::VRAY_clusterThisChildVolume() mySize: " << mySize << endl;
#endif

    }


    virtual void getWeightedBoxes(UT_RefArray<UT_BoundingBox> &boxes, UT_FloatArray &weights, fpreal radius, fpreal dbound) const;
    virtual void getAttributeBinding(UT_StringArray &names, UT_IntArray &sizes) const;
    virtual void evaluate(const UT_Vector3 &pos, const UT_Filter &filter, fpreal radius, fpreal time, int idx, fpreal *data) const;
    virtual UT_Vector3 gradient(const UT_Vector3 &pos, const UT_Filter &filter, fpreal radius, fpreal time, int idx) const;

    UT_BoundingBox myBox;
    fpreal bb_x1, bb_x2, bb_y1, bb_y2, bb_z1, bb_z2;

    fpreal  myRadius;
    fpreal  mySize[3];
    fpreal  myShutter;
    int     myDoMotionBlur;

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
//  $Log:
//
//
/**********************************************************************************/


