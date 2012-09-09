/* ******************************************************************************
*
*  clusterThis mantra DSO for render time geo clustering
*
* $RCSfile: VRAY_clusterCVEXUtil.C,v $
*
* Description :
*
* $Revision: 1.8 $
*
* $Source: /dca/cvsroot/houdini/VRAY_clusterThis/VRAY_clusterCVEXUtil.C,v $
*
* $Author: mstory $
*f
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

#ifndef __VRAY_clusterThisCVEXUtil_C__
#define __VRAY_clusterThisCVEXUtil_C__


/*

static void dumpInt(int *v, int n)
{
    int     i;
    printf("%d", v[0]);
    for (i = 1; i < n; i++)
        printf(", %d", v[i]);
}

static void dumpFloat(const fpreal32 *v, int n)
{
    int     i;
    printf("%g", v[0]);
    for (i = 1; i < n; i++)
        printf(", %g", v[i]);
}

static void dumpVector(const UT_Vector3 *v, int n)
{
    int     i;
    printf("{%g,%g,%g}", v[0].x(), v[0].y(), v[0].z());
    for (i = 1; i < n; i++)
        printf(", {%g,%g,%g}", v[i].x(), v[i].y(), v[i].z());
}




static void dumpValue(CVEX_Value *value)
{
    if (!value) {
        cout << "Not a valid CVEX value" << endl;
        return;
    }

//     if (!value->isExport()) {
//         cout << value->getName() << " CVEX value is not exported" << endl;
//         return;
//     }

    printf("%s = [", value->getName());
    switch (value->getType())
    {
    case CVEX_TYPE_INTEGER:
        dumpInt((int *)value->getData(), value->getArraySize());
        break;
    case CVEX_TYPE_FLOAT:
        dumpFloat((fpreal32 *)value->getData(), value->getArraySize());
        break;
    case CVEX_TYPE_VECTOR3:
        dumpVector((UT_Vector3 *)value->getData(), value->getArraySize());
        break;
    default:
        printf("No output supported currently\n");
    }
    printf("]\n");
}

*/



static void dumpValueList(const char * label, CVEX_ValueList & list)
{
   int      i;
   CVEX_Value * value;
   printf("%s:\n", label);
   for (i = 0; i < list.entries(); i++) {
      value = list.getValue(i);
      printf("\t%2d.", i+1);
      if (value->isExport())
         printf("export ");
      switch (value->getType()) {
      case CVEX_TYPE_INTEGER:
         printf("int");
         break;
      case CVEX_TYPE_FLOAT:
         printf("float");
         break;
      case CVEX_TYPE_VECTOR3:
         printf("vector");
         break;
      case CVEX_TYPE_VECTOR4:
         printf("vector4");
         break;
      case CVEX_TYPE_MATRIX3:
         printf("matrix3");
         break;
      case CVEX_TYPE_MATRIX4:
         printf("matrix");
         break;
      case CVEX_TYPE_STRING:
         printf("string");
         break;
      default:
         printf("unknown");
         break;
      }
      printf(" %s[%d]\n", value->getName(), value->getArraySize());
   }
}


#endif


/**********************************************************************************/
//  $Log: VRAY_clusterCVEXUtil.C,v $
//  Revision 1.8  2012-09-09 05:00:55  mstory
//  More cleanup and testing.
//
//  Revision 1.7  2012-09-07 15:39:23  mstory
//   Removed all volume instancing (used in different project) and continu… …
//
//  …ed H12 modifications.
//
//  --mstory
//
//  Revision 1.6  2012-09-05 23:02:39  mstory
//  Modifications for H12.
//
//  Revision 1.5  2011-02-06 19:49:15  mstory
//  Modified for Houdini version 11.
//
//  Refactored a lot of the attribute code, cleaned up odds and ends.
//
//  Revision 1.4  2010-02-23 08:36:22  mstory
//  Fixed most of the CVEX problems with primtive instancng.  Fixed seg faults from uninitilialized pointers in the CVEX variables,
//
//  Revision 1.3  2009-11-20 14:59:57  mstory
//  Release 1.4.7 ready.
//
//  Revision 1.2  2009-02-11 04:17:53  mstory
//  Added velocity blur for point instancing.
//
//
/**********************************************************************************/
