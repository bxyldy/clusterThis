/* ******************************************************************************
*
*  VRAY_clusterThisRender
*
* $RCSfile: VRAY_clusterThisRender.C,v $
*
* Description :
*
* $Revision: 1.17 $
*
* $Source: /dca/cvsroot/houdini/VRAY_clusterThis/VRAY_clusterThisRender.C,v $
*
* $Author: mstory $
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

#ifndef __VRAY_clusterThisRender_C__
#define __VRAY_clusterThisRender_C__


/* ******************************************************************************
*  Function Name : render()
*
*  Description :   Render VRAY_clusterThis object
*
*  Input Arguments : None
*
*  Return Value : None
*
***************************************************************************** */
void VRAY_clusterThis::render()
{
   GU_Detail * gdp, *inst_gdp, *mb_gdp, *file_gdp;
   GEO_Point * ppt;

//   GEO_AttributeHandle attrHandleVelocity, attrHandleForce, attrHandleVorticity, attrHandleNormal, attrHandleNumNeighbors,
//   attrHandleTextureUV, attrHandleMass, attrHandleAge, attrHandleTemperature, attrHandleID,
//   attrHandleDensity, attrHandleViscosity, attrHandlePressure, attrHandlePscale;

   long int point_num = 0;
   static bool rendered = false;

   if(myVerbose > CLUSTER_MSG_QUIET) {
         std::cout << "VRAY_clusterThis::render() - Version: " << DCA_VERSION << std::endl;
         std::cout << "VRAY_clusterThis::render() - Built for Houdini Version: " << UT_MAJOR_VERSION
                   << "." << UT_MINOR_VERSION << "." << UT_BUILD_VERSION_INT << std::endl;
         std::cout << "VRAY_clusterThis::render() - Instancing ..." <<  endl;
      }


   try {

//          cout << "VM_GEO_clusterThis OTL version: " <<  myOTLVersion << endl;

//       if(myOTLVersion != DCA_VERSION) {
//          cout << "VM_GEO_clusterThis OTL is wrong version: " <<  myOTLVersion << ", should be version: " << DCA_VERSION << ", please install correct version." << endl;
//          throw VRAY_clusterThis_Exception ( "VRAY_clusterThis::render() VM_GEO_clusterThis OTL is wrong version!", 1 );
//       }


         if(!rendered || !myUseTempFile) {

               void * handle = VRAY_Procedural::queryObject(0);
               gdp = VRAY_Procedural::allocateGeometry();

               if(myUseGeoFile) {
                     // If the file failed to load, throw an exception
                     if(!(gdp->load((const char *)mySrcGeoFname).success()))
                        throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() - Failed to read source geometry file ", 1);

                     if(myVerbose > CLUSTER_MSG_INFO)
                        cout << "VRAY_clusterThis::render() - Successfully loaded source geo file: " << mySrcGeoFname << endl;
                  }
               else {
                     gdp->copy(*VRAY_Procedural::queryGeometry(handle, 0));
                     if(myVerbose > CLUSTER_MSG_INFO)
                        cout << "VRAY_clusterThis::render() - Copied incoming geometry" << endl;
                  }


               gdp->getBBox(&myBox);

//               std::cout << "VRAY_clusterThis::render() - gdp->getBBox(&myBox): " << myBox << std::endl;

               VRAY_Procedural::querySurfaceShader(handle, myMaterial);
               myMaterial.harden();
//         myPointAttributes.material = myMaterial;

//         const char **        getSParm (int token) const
//         cout << "VRAY_clusterThis::render() getSParm: " << *getSParm (0) << endl;


#ifdef DEBUG
               cout << "VRAY_clusterThis::render() myMaterial: " << myMaterial << endl;
#endif

               myLOD = getLevelOfDetail(myBox);
               if(myVerbose > CLUSTER_MSG_INFO)
                  cout << "VRAY_clusterThis::render() myLOD: " << myLOD << endl;


               // Get the number if points of the incoming geometery, calculate an interval for reporting the status of the instancing to the user
               long int num_points = (long int) gdp->points().entries();
               long int stat_interval = (long int)(num_points * 0.10) + 1;

               if(myVerbose > CLUSTER_MSG_QUIET)
                  cout << "VRAY_clusterThis::render() Number of points of incoming geometry: " << num_points << endl;

               myObjectName = VRAY_Procedural::queryObjectName(handle);

//      cout << "VRAY_clusterThis::render() Object Name: " << myObjectName << endl;
//      cout << "VRAY_clusterThis::render() Root Name: " << queryRootName() << endl;

// DEBUG stuff ...

//   changeSetting("object:geo_velocityblur", "on");

               UT_String      str;
               int     vblur = 0;
               import("object:velocityblur", &vblur, 0);

               if(vblur) {
                     str = 0;
                     import("velocity", str);
                     if(str.isstring()) {
//               const char  *  name;
//               name = queryObjectName(handle);
                           VRAYwarning("%s[%s] cannot get %s",
                                       VRAY_Procedural::getClassName(), (const char *)myObjectName, " motion blur attr");

                        }
                  }


               myXformInverse = queryTransform(handle, 0);
               myXformInverse.invert();




#ifdef DEBUG
               cout << "Geometry Samples: " << queryGeometrySamples(handle) << endl;
// cout << "scale: " << getFParm ( "scale" ) << endl;
#endif

               // Dump the user parameters to the console
               if(myVerbose == CLUSTER_MSG_DEBUG)
                  VRAY_clusterThis::dumpParameters();

               switch(myMethod) {
                     case CLUSTER_INSTANCE_NOW:
                        inst_gdp = VRAY_Procedural::allocateGeometry();
                        if(myDoMotionBlur == CLUSTER_MB_DEFORMATION)
                           mb_gdp = VRAY_Procedural::allocateGeometry();
                        if(myVerbose > CLUSTER_MSG_QUIET)
                           cout << "VRAY_clusterThis::render() - Using \"instance all the geometry at once\" method" << endl;
                        break;
                     case CLUSTER_INSTANCE_DEFERRED:
                        if(myVerbose > CLUSTER_MSG_QUIET)
                           cout << "VRAY_clusterThis::render() - Using \"addProcedural()\" method" << endl;
                        break;
                  }

               rendered = true;

               // Get the point's attribute offsets
               VRAY_clusterThis::getAttributeOffsets(gdp);

               // Check for required attributes
               VRAY_clusterThis::checkRequiredAttributes();

               // Check for weight attribute if the user wants metaballs
               if((myPrimType == CLUSTER_PRIM_METABALL) && (myPointAttrOffsets.weight.isInvalid())) {

                     cout << "Incoming points must have weight attribute if instancing metaballs! Throwing exception ..." << endl;
                     throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Incoming points must have weight attribute if instancing metaballs!", 1);
                  }


               if(myPrimType == CLUSTER_FILE) {
                     file_gdp = VRAY_Procedural::allocateGeometry();
                     int file_load_stat = VRAY_clusterThis::preLoadGeoFile(file_gdp);

                     if(!file_load_stat) {
//                           myFileGDP = file_gdp;
                           if(myVerbose > CLUSTER_MSG_INFO)
                              cout << "VRAY_clusterThis::render() Successfully loaded geometry file: " << myGeoFile << endl;
                        }
                     else {
                           throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Failed to load geometry file ", 1);
                        }
                  }

               // init some vars ...
               myPointAttributes.Cd = (UT_Vector3(1, 1, 1));
               myPointAttributes.v = (UT_Vector3(0, 0, 0));
               myPointAttributes.N = (UT_Vector3(0, 1, 0));
               myPointAttributes.Alpha = 1.0;
               myPointAttributes.pscale = 1.0;
               myPointAttributes.id = 0;

               myNoise.initialize(myNoiseSeed, myNoiseType);

               // Create the attribute "offsets" for the geometry to be instanced
               VRAY_clusterThis::createAttributeOffsets(inst_gdp, mb_gdp);

//changeSetting("surface", "constant Cd ( 1 0 0 )", "object");


               fpreal theta = (2.0 * M_PI) / myNumCopies;
               myInstanceNum = 0;

               if(myCVEX_Exec_pre) {
                     if(myVerbose > CLUSTER_MSG_INFO)
                        cout << "VRAY_clusterThis::render() Executing Pre Process CVEX code" << endl;
                     VRAY_clusterThis::runCVEX(gdp, gdp, myCVEXFname_pre, CLUSTER_CVEX_POINT);
                  }

               /// For each point of the incoming geometry
               GA_FOR_ALL_GPOINTS(gdp, ppt) {
                  myPointAttributes.myPos = ppt->getPos();

                  // get the point's attributes
                  VRAY_clusterThis::getAttributes(ppt, gdp);

#ifdef DEBUG
                  cout << "VRAY_clusterThis::render() " << "theta: " << theta << endl;
#endif

                  uint seed = 37;
                  fpreal dice;
                  bool skip = false;

                  if((myPrimType != CLUSTER_PRIM_CURVE) ||
                        ((myMethod == CLUSTER_INSTANCE_DEFERRED && (myPrimType == CLUSTER_PRIM_CURVE)))) {


                        // For each point, make a number of copies of and recurse a number of times for each copy ...
                        for(int copyNum = 0; copyNum < myNumCopies; copyNum++) {
                              for(int recursionNum = 0; recursionNum < myRecursion; recursionNum++) {

                                    // generate random number to determine to instance or not

                                    dice = SYSfastRandom(seed);
                                    (dice > myBirthProb)?skip = true:skip = false;
//                  cout << dice << " " << skip << endl;
                                    seed = uint(dice * 100);

                                    // Calculate the position for the next instanced object ...
                                    VRAY_clusterThis::calculateNewPosition(theta, copyNum, recursionNum);

                                    if(!skip) {

                                          // Instance an object ...
                                          switch(myMethod) {
                                                   // For the "create all geometry at once" method, instance the object now ...
                                                case CLUSTER_INSTANCE_NOW:
                                                   // Create a primitive based upon user's selection
                                                   // TODO: can later be driven by a point attribute
                                                   switch(myPrimType) {
                                                         case CLUSTER_POINT:
                                                            VRAY_clusterThis::instancePoint(inst_gdp, mb_gdp);
                                                            break;
                                                         case CLUSTER_PRIM_SPHERE:
                                                            VRAY_clusterThis::instanceSphere(inst_gdp, mb_gdp);
                                                            break;
                                                         case CLUSTER_PRIM_CUBE:
                                                            VRAY_clusterThis::instanceCube(inst_gdp, mb_gdp);
                                                            break;
                                                         case CLUSTER_PRIM_GRID:
                                                            VRAY_clusterThis::instanceGrid(inst_gdp, mb_gdp);
                                                            break;
                                                         case CLUSTER_PRIM_TUBE:
                                                            VRAY_clusterThis::instanceTube(inst_gdp, mb_gdp);
                                                            break;
                                                         case CLUSTER_PRIM_CIRCLE:
                                                            VRAY_clusterThis::instanceCircle(inst_gdp, mb_gdp);
                                                            break;
                                                         case CLUSTER_PRIM_METABALL:
                                                            VRAY_clusterThis::instanceMetaball(inst_gdp, mb_gdp);
                                                            break;
                                                         case CLUSTER_FILE:
                                                            VRAY_clusterThis::instanceFile(file_gdp, inst_gdp, mb_gdp);
                                                            break;
                                                            // In case a instance type comes through that's not "legal", throw exception
                                                         default:
                                                            throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() Illegal instance type, exiting ...", 1);
                                                            break;
                                                      }
                                                   break;

                                                   // For the "deferred instance" method, add the procedural now ...
                                                case CLUSTER_INSTANCE_DEFERRED:
                                                   VRAY_Procedural::openProceduralObject();
                                                   VRAY_clusterThisChild * child = new VRAY_clusterThisChild::VRAY_clusterThisChild(this);
                                                   VRAY_Procedural::addProcedural(child);
                                                   VRAY_Procedural::changeSetting("object:geo_velocityblur", "on");
                                                   VRAY_Procedural::closeObject();
//changeSetting("surface", "constant Cd ( 1 0 0 )", "object");
//   changeSetting("object:geo_velocityblur", "on");

                                                   break;
                                             }

                                          myInstanceNum++;

#ifdef DEBUG
                                          cout << "VRAY_clusterThis::render() - myInstanceNum: " << myInstanceNum << std::endl;
#endif
                                       }

                                 } // for number of recursions ...
                           } // for number of copies ...

                     }

                  // User wants a curve instanced on this point
                  if((myPrimType == CLUSTER_PRIM_CURVE) && (myMethod == CLUSTER_INSTANCE_NOW) && (!skip))
                     VRAY_clusterThis::instanceCurve(inst_gdp, mb_gdp, theta, point_num);

                  // Increment our point counter
                  point_num++;

                  // Print out stats to the console
                  if(myVerbose > CLUSTER_MSG_INFO && (myPrimType != CLUSTER_PRIM_CURVE))
                     if((long int)(point_num % stat_interval) == 0)
                        cout << "VRAY_clusterThis::render() Number of points processed: " << point_num << " Number of instances: " << myInstanceNum << endl;

               } // for all points ...


               if(myVerbose > CLUSTER_MSG_QUIET)
                  if(myMethod == CLUSTER_INSTANCE_NOW)
                     if(myDoMotionBlur == CLUSTER_MB_DEFORMATION)
                        cout << "VRAY_clusterThis::render() - Memory usage(MB): " <<
                             (fpreal)(inst_gdp->getMemoryUsage() + mb_gdp->getMemoryUsage() / (1024.0 * 1024.0)) << endl;
                     else
                        cout << "VRAY_clusterThis::render() - Memory usage(MB): " <<
                             (fpreal)(inst_gdp->getMemoryUsage() / (1024.0 * 1024.0)) << endl;

               // If the "instance all the geo at once method" is used, add the the instanced geo for mantra to render ...
               switch(myMethod) {
                     case CLUSTER_INSTANCE_NOW:

                        if(myCVEX_Exec_post) {
                              if(myVerbose > CLUSTER_MSG_INFO)
                                 cout << "VRAY_clusterThis::render() Executing Post Process CVEX code" << endl;
                              VRAY_clusterThis::runCVEX(inst_gdp, mb_gdp, myCVEXFname_post, CLUSTER_CVEX_POINT);
                           }
                        VRAY_Procedural::openGeometryObject();
                        VRAY_Procedural::addGeometry(inst_gdp, 0.0);

                        if(myDoMotionBlur == CLUSTER_MB_VELOCITY)
                           VRAY_Procedural::addVelocityBlurGeometry(inst_gdp, myShutter, myShutter2);

// float    addVelocityBlurGeometry (GU_Detail *gdp, fpreal pre_blur, fpreal post_blur, const char *velocity_attribute="v")

                        if(myDoMotionBlur == CLUSTER_MB_DEFORMATION)
                           VRAY_Procedural::addGeometry(mb_gdp, myShutter);

                        VRAY_Procedural::setComputeN(1);
//                        setSurface(myMaterial);
                        VRAY_Procedural::closeObject();

                        break;
                     case CLUSTER_INSTANCE_DEFERRED:
                        break;
                  }

               if(myVerbose > CLUSTER_MSG_QUIET && (myPrimType != CLUSTER_PRIM_CURVE))
                  cout << "VRAY_clusterThis::render() Total number of instances: " << myInstanceNum << endl;

               // Save the geo to temp location so it doesn't have to be regenerated for a deep shadow pass, etc.
               if(myMethod == CLUSTER_INSTANCE_NOW && myUseTempFile) {
                     ofstream myGeoStream;
                     // myGeoStream.open("/tmp/thisGeo.bgeo");
                     myGeoStream.open((const char *)myTempFname, ios_base::binary);
                     UT_Options myOptions;
                     inst_gdp->save(myGeoStream, 1, &myOptions);
                     myGeoStream.flush();
                     myGeoStream.close();
                     if(myVerbose > CLUSTER_MSG_QUIET)
                        cout << "VRAY_clusterThis::render() - Saved geometry to temp file: " << myTempFname << endl;
                  }

               if(myPrimType == CLUSTER_FILE)
                  VRAY_Procedural::freeGeometry(file_gdp);

               // We're done, free the original geometry
               VRAY_Procedural::freeGeometry(gdp);

            } /// if (rendered) ...


         // Geo has already been generated ...
         else {
               if(myVerbose > CLUSTER_MSG_QUIET)
                  cout << "VRAY_clusterThis::render() - Already generated geometry, reading temp geo file: " << myTempFname << endl;
               inst_gdp = VRAY_Procedural::allocateGeometry();
               UT_Options myOptions;

               // If the file failed to load, throw an exception
               if((inst_gdp->load((const char *)myTempFname).success())) {
                     if(myVerbose > CLUSTER_MSG_QUIET)
                        cout << "VRAY_clusterThis::render() - Successfully loaded temp geo file: " << myTempFname << endl;
                  }
               else
                  throw VRAY_clusterThis_Exception("VRAY_clusterThis::render() - Failed to read temp geometry file ", 1);

               VRAY_Procedural::openGeometryObject();
               VRAY_Procedural::addGeometry(inst_gdp, 0);

               if(myDoMotionBlur == CLUSTER_MB_VELOCITY)
                  VRAY_Procedural::addVelocityBlurGeometry(inst_gdp, myShutter, myShutter2);

// float    addVelocityBlurGeometry (GU_Detail *gdp, fpreal pre_blur, fpreal post_blur, const char *velocity_attribute="v")

               //if(myDoMotionBlur == CLUSTER_MB_DEFORMATION) {
               //   mb_gdp = allocateGeometry();

               // if(!mb_gdp->load((const char *)myTempFname, &myOptions))
               //  cout << "VRAY_clusterThis::render() - Successfully loaded temp geo file for deformation motion blur: " << myTempFname << endl;
               // else
               //      throw VRAY_clusterThis_Exception ("VRAY_clusterThis::render() - Failed to read temp geometry file ", 1 );

               //   addGeometry(mb_gdp, myShutter);
               //   }

               VRAY_Procedural::setComputeN(1);
               VRAY_Procedural::setSurface(myMaterial);
               VRAY_Procedural::closeObject();
            }

      } // try ...



   // Process exceptions ...
   catch(VRAY_clusterThis_Exception e) {
         e.what();
         cout << "VRAY_clusterThis::render() - Exception encountered, copying incoming geometry" << endl << endl;
         if(gdp)
            VRAY_Procedural::freeGeometry(gdp);
         if(inst_gdp)
            VRAY_Procedural::freeGeometry(inst_gdp);
         if(myDoMotionBlur == CLUSTER_MB_DEFORMATION)
            if(mb_gdp)
               VRAY_Procedural::freeGeometry(mb_gdp);
         void * handle = queryObject(0);
         gdp = VRAY_Procedural::allocateGeometry();
         gdp->copy(*queryGeometry(handle, 0));
         VRAY_Procedural::openGeometryObject();
         VRAY_Procedural::addGeometry(gdp, 0);
         closeObject();
         return;
      }


   catch(...) {
         cout << "VRAY_clusterThis::render() - Unknown exception encountered, freeing geometry and exiting" << endl << endl;
         freeGeometry(gdp);
         freeGeometry(inst_gdp);
         if(myDoMotionBlur == CLUSTER_MB_DEFORMATION)
            freeGeometry(mb_gdp);
         return;
      }


   if(myVerbose > CLUSTER_MSG_QUIET)
      cout << "VRAY_clusterThis::render() - Leaving render() method" << endl;

   return;

}



#endif


/**********************************************************************************/
//  $Log: VRAY_clusterThisRender.C,v $
//  Revision 1.17  2012-09-09 05:00:55  mstory
//  More cleanup and testing.
//
//  Revision 1.16  2012-09-07 15:39:23  mstory
//   Removed all volume instancing (used in different project) and continu… …
//
//  …ed H12 modifications.
//
//  --mstory
//
//  Revision 1.15  2012-09-05 23:02:39  mstory
//  Modifications for H12.
//
//  Revision 1.14  2012-09-04 03:25:28  mstory
//  .
//
//  Revision 1.11  2011-02-15 00:59:15  mstory
//  Refactored out rededundant attribute code in the child (deferred) instancicng mode.
//  Made remaining changes for H11 (and beyond) versions way of handiling attributes.
//
//
//  --mstory
//
//  Revision 1.10  2011-02-06 22:35:15  mstory
//  Fixed the exit processing function.
//
//  Ready for release 1.5.1
//
//  Revision 1.9  2011-02-06 19:49:15  mstory
//  Modified for Houdini version 11.
//
//  Refactored a lot of the attribute code, cleaned up odds and ends.
//
//  Revision 1.8  2010-04-12 06:39:42  mstory
//  Finished CVEX modifications.
//
//  Revision 1.7  2010-04-10 10:11:42  mstory
//  Added additional CVEX processing.  Fixed a few annoying bugs.  Adding external disk geo source.
//
//  Revision 1.6  2010-02-26 08:06:32  mstory
//  Adding more CVEX options.
//
//  Revision 1.5  2010-02-23 08:36:22  mstory
//  Fixed most of the CVEX problems with primtive instancng.  Fixed seg faults from uninitilialized pointers in the CVEX variables,
//
//  Revision 1.4  2009-11-20 14:59:57  mstory
//  Release 1.4.7 ready.
//
//  Revision 1.3  2009-11-19 16:26:51  mstory
//  Adding point inst id to child objects (for deferred instancing), need to add to prims as well.
//
//  Revision 1.2  2009-11-16 17:47:12  mstory
//  Fixed the curve instancing, still need to determine all attribites required for the curve (i.e. width)
//
//  Revision 1.1  2009-11-16 08:35:03  mstory
//  Created seperate source files for some of the functions in VRAY_clusterThis.C.
//

/**********************************************************************************/


