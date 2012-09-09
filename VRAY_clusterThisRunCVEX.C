/* ******************************************************************************
*
*  VRAY_clusterThisRunCVEX
*
* $RCSfile: VRAY_clusterThisRunCVEX.C,v $
*
* Description :
*
* $Revision: 1.12 $
*
* $Source: /dca/cvsroot/houdini/VRAY_clusterThis/VRAY_clusterThisRunCVEX.C,v $
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

#ifndef __VRAY_clusterThisRunCVEX_C__
#define __VRAY_clusterThisRunCVEX_C__



/* ******************************************************************************
*  Function Name : runCVEX()
*
*  Description :  Run VEX code on the instanced geometry from an external .vex file
*
*  Input Arguments : GU_Detail *inst_gdp, GU_Detail *mb_gdp
*
*  Return Value : int
*
***************************************************************************** */
int VRAY_clusterThis::runCVEX(GU_Detail * inst_gdp, GU_Detail * mb_gdp, UT_String theCVEXFname, uint method)
{

   if(myVerbose > CLUSTER_MSG_INFO) {
         std::cout << "VRAY_clusterThis::runCVEX() - Processing CVEX arrays - points: " << inst_gdp->points().entries()
                   << " primitives: " << inst_gdp->primitives().entries() << std::endl;
      }


   UT_Vector3 * P = NULL, *POut = NULL, *N = NULL, *NOut = NULL, *v = NULL, *vOut = NULL, *Cd = NULL, *CdOut = NULL;
   fpreal * weight = NULL, *weightOut = NULL, *Alpha = NULL, *AlphaOut = NULL, *pscale = NULL, *pscaleOut = NULL;
   int * id = NULL, *inst_id = NULL;

   // helper local class to clean up memory
   class cleanUpCVEXMem {
   public:
      void cleanUp(UT_Vector3 * P, UT_Vector3 * POut, UT_Vector3 * N, UT_Vector3 * NOut,
                   UT_Vector3 * v, UT_Vector3 * vOut, UT_Vector3 * Cd, UT_Vector3 * CdOut,
                   fpreal * weight, fpreal * weightOut, fpreal * Alpha, fpreal * AlphaOut,
                   fpreal * pscale, fpreal * pscaleOut,
                   int * id, int * inst_id) {

         if(P) delete []P;
         if(POut) delete []POut;
         if(Cd) delete []Cd;
         if(CdOut) delete []CdOut;
         if(Alpha) delete []Alpha;
         if(AlphaOut) delete []AlphaOut;
         if(N) delete []N;
         if(NOut) delete []NOut;
         if(v) delete []v;
         if(vOut) delete []vOut;
         if(pscale) delete []pscale;
         if(pscaleOut) delete []pscaleOut;
         if(weight) delete []weight;
         if(weightOut) delete []weightOut;
         if(id) delete []id;
         if(inst_id) delete []inst_id;

         return;
      };

   } memCleaner;


   try {
         uint32         num_inst = 0, num_points = 0, num_prim = 0;
         long int       num = 0;
         GEO_Point   *   ppt;
         GEO_Primitive * prim;
         UT_Vector4     pos;
         CVEX_Context   cvex;
         GU_Detail   *   cvex_gdp;
         uint32         num_passes = 1;
         uint32         mb_pass = 0;
         fpreal         time = myCurrentTime;

// std::cout << "VRAY_clusterThis::runCVEX() - time: " << time << endl;

         if(myDoMotionBlur == CLUSTER_MB_DEFORMATION)
            num_passes = 2;

         if(myPrimType == CLUSTER_POINT) {
               num_inst = myInstanceNum;
//            num_inst = inst_gdp->points().entries();
               num_points = inst_gdp->points().entries();
//           std::cout << "VRAY_clusterThis::runCVEX() - num_inst: " << num_inst << std::endl;
            }
         else {
               num_inst = myInstanceNum;
               num_prim = inst_gdp->primitives().entries();
               num_points = inst_gdp->points().entries();
               num_inst = num_points;
//           std::cout << "VRAY_clusterThis::runCVEX() - num_inst: " << num_inst << " num_points: " << num_points << " num_prim: " << num_prim << std::endl;
            }


         UT_Vector3 primAttr, ptAttr;

         GA_RWAttributeRef primN_ref, primV_ref, primCd_ref, primAlpha_ref, primPscale_ref, primId_ref, primWeight_ref;
         GA_RWAttributeRef ptN_ref, ptV_ref, ptCd_ref, ptAlpha_ref, ptPscale_ref, ptId_ref, ptInstId_ref;

         if(myPrimType == CLUSTER_POINT) {

               // Allocate the space for the point positions
               P = new UT_Vector3[num_points];
               POut = new UT_Vector3[num_points];

               if(myCVEXPointVars.cvex_Cd_pt) {
                     Cd = new UT_Vector3[num_points];
                     CdOut = new UT_Vector3[num_points];
                  }

               if(myCVEXPointVars.cvex_Alpha_pt) {
                     Alpha = new fpreal[num_points];
                     AlphaOut = new fpreal[num_points];
                  }

               if(myCVEXPointVars.cvex_N_pt) {
                     N = new UT_Vector3[num_points];
                     NOut = new UT_Vector3[num_points];
                  }

               if(myCVEXPointVars.cvex_v_pt) {
                     v = new UT_Vector3[num_points];
                     vOut = new UT_Vector3[num_points];
                  }

               if(myCVEXPointVars.cvex_pscale_pt) {
                     pscale = new fpreal[num_points];
                     pscaleOut = new fpreal[num_points];
                  }

               id = new int[num_points];
               inst_id = new int[num_points];

            }
         else {
               // If we're processing the points of the prims, allocate the space
               if(method == CLUSTER_CVEX_POINT) {
                     P = new UT_Vector3[num_points];
                     POut = new UT_Vector3[num_points];
                     // Allocate space for the id and inst_id attr's
                     id = new int[num_points];
                     inst_id = new int[num_points];
                  }
               // Else we're working on primitives, so allocate sace for the attrs the user wants to CVEX!
               else {

                     if(myCVEXPrimVars.cvex_Cd_prim) {
                           Cd = new UT_Vector3[num_prim];
                           CdOut = new UT_Vector3[num_prim];
                        }

                     if(myCVEXPrimVars.cvex_Alpha_prim) {
                           Alpha = new fpreal[num_prim];
                           AlphaOut = new fpreal[num_prim];
                        }

                     if(myCVEXPrimVars.cvex_N_prim) {
                           N = new UT_Vector3[num_prim];
                           NOut = new UT_Vector3[num_prim];
                        }

                     if(myCVEXPrimVars.cvex_v_prim) {
                           v = new UT_Vector3[num_prim];
                           vOut = new UT_Vector3[num_prim];
                        }

                     if(myCVEXPrimVars.cvex_pscale_prim) {
                           pscale = new fpreal[num_prim];
                           pscaleOut = new fpreal[num_prim];
                        }

                     if(myPrimType == VRAY_clusterThis::CLUSTER_PRIM_METABALL && myCVEXPrimVars.cvex_weight_prim) {
                           weight = new fpreal[num_prim];
                           weightOut = new fpreal[num_prim];
                        }
                     // Allocate space for the id and inst_id attr's
                     id = new int[num_prim];
                     inst_id = new int[num_prim];

                  }

            }



         // Retrieve the attribute offsets
         for(uint32 pass = 0; pass < num_passes; pass++) {

               if(pass == 0) {
                     cvex_gdp = inst_gdp;
                     mb_pass = 0;
                  }
               else {
                     cvex_gdp = mb_gdp;
                     mb_pass = 1;
                  }

               if(myPrimType == CLUSTER_POINT) {

                     if(myCVEXPointVars.cvex_Cd_pt) {
                           ptCd_ref = cvex_gdp->findDiffuseAttribute(GEO_POINT_DICT);
                        }

                     if(myCVEXPointVars.cvex_Alpha_pt) {
                           ptAlpha_ref = cvex_gdp->findFloatTuple(GA_ATTRIB_POINT, "Alpha", 1);
                        }

                     if(myCVEXPointVars.cvex_N_pt) {
                           ptN_ref = cvex_gdp->findNormalAttribute(GEO_POINT_DICT);
                        }

                     if(myCVEXPointVars.cvex_v_pt) {
                           ptV_ref = cvex_gdp->findVelocityAttribute(GEO_POINT_DICT);
                        }

                     if(myCVEXPointVars.cvex_pscale_pt) {
                           ptPscale_ref = cvex_gdp->findFloatTuple(GA_ATTRIB_POINT, "pscale", 1);
                        }

                     ptId_ref = cvex_gdp->findIntTuple(GA_ATTRIB_POINT, "id", 1);
                     ptInstId_ref = cvex_gdp->findIntTuple(GA_ATTRIB_POINT, "inst_id", 1);

                  }
               else {
                     // If we're not processing the points of the prims, get the attr's offset for the attrs the user wants to use
                     if(method != CLUSTER_CVEX_POINT) {

                           if(myCVEXPrimVars.cvex_Cd_prim) {
                                 primCd_ref = cvex_gdp->findDiffuseAttribute(GEO_PRIMITIVE_DICT);
                              }

                           if(myCVEXPrimVars.cvex_Alpha_prim) {
                                 primAlpha_ref = cvex_gdp->findFloatTuple(GA_ATTRIB_POINT, "Alpha", 1);
                              }

                           if(myCVEXPrimVars.cvex_N_prim) {
                                 primN_ref = cvex_gdp->findFloatTuple(GA_ATTRIB_POINT, "N", 3);
                                 //                  primN_ref = cvex_gdp->findNormalAttribute ( GEO_PRIMITIVE_DICT );
                              }

                           if(myCVEXPrimVars.cvex_v_prim) {
                                 primV_ref = cvex_gdp->findVelocityAttribute(GEO_PRIMITIVE_DICT);
                              }

                           if(myCVEXPrimVars.cvex_pscale_prim) {
                                 ptPscale_ref = cvex_gdp->findFloatTuple(GA_ATTRIB_POINT, "pscale", 1);
                              }

                           if(myPrimType == VRAY_clusterThis::CLUSTER_PRIM_METABALL && myCVEXPrimVars.cvex_weight_prim) {
                                 primWeight_ref = cvex_gdp->findFloatTuple(GA_ATTRIB_POINT, "weight", 1);
                              }

                        }

                     // Get the offset for id attr
                     ptId_ref = cvex_gdp->findIntTuple(GA_ATTRIB_POINT, "id", 1);
                     ptInstId_ref = cvex_gdp->findIntTuple(GA_ATTRIB_POINT, "inst_id", 1);
                  }



#ifdef DEBUG
               std::cout << "VRAY_clusterThis::runCVEX() - Retrieved attribute offsets" << std::endl;
#endif

               // Set the values of the incoming arrays

               if(myPrimType == CLUSTER_POINT) {

                     num = 0;
                     GA_FOR_ALL_GPOINTS(cvex_gdp, ppt) {
                        pos = ppt->getPos();
                        P[num][0] = pos.x();
                        P[num][1] = pos.y();
                        P[num][2] = pos.z();
//                    P[num] = ppt->getPos();
                        id[num] = static_cast<int>(ppt->getValue<int>(ptId_ref, 0));

                        if(ptInstId_ref.isValid())
                           inst_id[num] = static_cast<int>(ppt->getValue<int>(ptInstId_ref, 0));
                        else
                           inst_id[num] = id[num];

//                    cout << "IN - id : " << id[num] << "\tinst_id " << inst_id[num] << "\tnum " << num << endl;
                        ++num;
                     }

                     if(myCVEXPointVars.cvex_Cd_pt) {
                           num = 0;
                           GA_FOR_ALL_GPOINTS(cvex_gdp, ppt) {
                              ptAttr = static_cast<UT_Vector3>(ppt->getValue<UT_Vector3>(ptCd_ref, 0));
                              Cd[num][0] = ptAttr.x();
                              Cd[num][1] = ptAttr.y();
                              Cd[num][2] = ptAttr.z();
//                     cout << "IN - Cd: " << Cd[num] << endl;
                              ++num;
                           }
                        }

                     if(myCVEXPointVars.cvex_Alpha_pt) {
                           num = 0;
                           GA_FOR_ALL_GPOINTS(cvex_gdp, ppt)
                           Alpha[num++] = static_cast<fpreal>(ppt->getValue<fpreal>(ptAlpha_ref, 0));
                        }

                     if(myCVEXPointVars.cvex_N_pt) {
                           num = 0;
                           GA_FOR_ALL_GPOINTS(cvex_gdp, ppt) {
                              ptAttr = static_cast<UT_Vector3>(ppt->getValue<UT_Vector3>(ptN_ref, 0));
                              N[num][0] = ptAttr.x();
                              N[num][1] = ptAttr.y();
                              N[num][2] = ptAttr.z();
                              ++num;
                           }
                        }

                     if(myCVEXPointVars.cvex_v_pt) {
                           num = 0;
                           GA_FOR_ALL_GPOINTS(cvex_gdp, ppt) {
                              ptAttr = static_cast<UT_Vector3>(ppt->getValue<UT_Vector3>(ptV_ref, 0));
                              v[num][0] = ptAttr.x();
                              v[num][1] = ptAttr.y();
                              v[num][2] = ptAttr.z();
                              ++num;
                           }
                        }

                     if(myCVEXPointVars.cvex_pscale_pt) {
                           num = 0;
                           GA_FOR_ALL_GPOINTS(cvex_gdp, ppt)
                           pscale[num++] = static_cast<fpreal>(ppt->getValue<fpreal>(ptPscale_ref, 0));
                        }

//                    cout << "IN - P: " << P[num] << "\tN: " << N[num]<< "\tCd: " << Cd[num] << "\tpscale: " << pscale[num] << endl;

                  }
               else {
                     // If we're not processing the points of the prims, get the attr's offset for the attrs the user wants to use
                     if(method == CLUSTER_CVEX_POINT) {
                           num = 0;
                           GA_FOR_ALL_GPOINTS(cvex_gdp, ppt) {
                              pos = ppt->getPos();
                              //                    P[num] = ppt->getPos();
                              P[num][0] = pos.x();
                              P[num][1] = pos.y();
                              P[num][2] = pos.z();
                              // Set the id and instance id to match the points id
                              id[num] = static_cast<int>(ppt->getValue<int>(ptId_ref, 0));
                              inst_id[num] = num;
                              //                    cout << "IN - P: " << P[num] << " " << pos.x() << " " << pos.y() << " " << pos.z() << " " << num << endl;
                              ++num;
                           }
                        }
                     // Else we're processing the primitives attributes
                     else {
                           if(myCVEXPrimVars.cvex_Cd_prim) {
                                 num = 0;
                                 GA_FOR_ALL_PRIMITIVES(cvex_gdp, prim) {
                                    primAttr = static_cast<UT_Vector3>(ppt->getValue<UT_Vector3>(primCd_ref, 0));
                                    Cd[num][0] = primAttr.x();
                                    Cd[num][1] = primAttr.y();
                                    Cd[num][2] = primAttr.z();
                                    ++num;
                                 }
                              }

                           if(myCVEXPrimVars.cvex_Alpha_prim) {
                                 num = 0;
                                 GA_FOR_ALL_PRIMITIVES(cvex_gdp, prim)
                                 Alpha[num++] = static_cast<fpreal>(ppt->getValue<fpreal>(primAlpha_ref, 0));
                              }

                           if(myCVEXPrimVars.cvex_N_prim) {
                                 num = 0;
                                 GA_FOR_ALL_PRIMITIVES(cvex_gdp, prim) {
                                    primAttr = static_cast<UT_Vector3>(ppt->getValue<UT_Vector3>(primN_ref, 0));
                                    N[num][0] = primAttr.x();
                                    N[num][1] = primAttr.y();
                                    N[num][2] = primAttr.z();
                                    ++num;
                                 }
                              }

                           if(myCVEXPrimVars.cvex_v_prim) {
                                 num = 0;
                                 GA_FOR_ALL_PRIMITIVES(cvex_gdp, prim) {
                                    primAttr = static_cast<UT_Vector3>(ppt->getValue<UT_Vector3>(primV_ref, 0));
                                    v[num][0] = primAttr.x();
                                    v[num][1] = primAttr.y();
                                    v[num][2] = primAttr.z();
                                    ++num;
                                 }
                              }

                           if(myCVEXPrimVars.cvex_pscale_prim) {
                                 num = 0;
                                 GA_FOR_ALL_PRIMITIVES(cvex_gdp, prim)
                                 pscale[num++] = static_cast<fpreal>(ppt->getValue<fpreal>(primPscale_ref, 0));
                              }

                           if(myPrimType == CLUSTER_PRIM_METABALL && myCVEXPrimVars.cvex_weight_prim) {
                                 num = 0;
                                 GA_FOR_ALL_PRIMITIVES(cvex_gdp, prim)
                                 weight[num++] = static_cast<fpreal>(ppt->getValue<fpreal>(primWeight_ref, 0));
                              }

                           GA_FOR_ALL_PRIMITIVES(cvex_gdp, prim) {
                              num = 0;
                              id[num] = static_cast<int>(ppt->getValue<int>(primId_ref, 0));
                              inst_id[num] = num;
                              ++num;
                           }

                        }
                  }


#ifdef DEBUG
               std::cout << "VRAY_clusterThis::runCVEX() - Finished storing attributes" << std::endl;
#endif

               // Add the inputs to the VEX call
               if(myPrimType == CLUSTER_POINT) {
                     cvex.addInput("P", CVEX_TYPE_VECTOR3, P, num_points);
                     cvex.addInput("id", CVEX_TYPE_INTEGER, id, num_points);
                     cvex.addInput("inst_id", CVEX_TYPE_INTEGER, inst_id, num_points);

                     if(myCVEXPointVars.cvex_Cd_pt)
                        cvex.addInput("Cd", CVEX_TYPE_VECTOR3, Cd, num_points);
                     if(myCVEXPointVars.cvex_Alpha_pt)
                        cvex.addInput("Alpha", CVEX_TYPE_FLOAT, Alpha, num_points);
                     if(myCVEXPointVars.cvex_N_pt)
                        cvex.addInput("N", CVEX_TYPE_VECTOR3, N, num_points);
                     if(myCVEXPointVars.cvex_v_pt)
                        cvex.addInput("v", CVEX_TYPE_VECTOR3, v, num_points);
                     if(myCVEXPointVars.cvex_pscale_pt)
                        cvex.addInput("pscale", CVEX_TYPE_FLOAT, pscale, num_points);
                  }
               else {
                     if(method == CLUSTER_CVEX_POINT) {
                           cvex.addInput("P", CVEX_TYPE_VECTOR3, P, num_points);
                           cvex.addInput("id", CVEX_TYPE_INTEGER, id, num_points);
                           cvex.addInput("inst_id", CVEX_TYPE_INTEGER, inst_id, num_points);
                        }
                     else {
                           if(myCVEXPrimVars.cvex_Cd_prim)
                              cvex.addInput("Cd", CVEX_TYPE_VECTOR3, Cd, num_prim);
                           if(myCVEXPrimVars.cvex_Alpha_prim)
                              cvex.addInput("Alpha", CVEX_TYPE_FLOAT, Alpha, num_prim);
                           if(myCVEXPrimVars.cvex_N_prim)
                              cvex.addInput("N", CVEX_TYPE_VECTOR3, N, num_prim);
                           if(myCVEXPrimVars.cvex_v_prim)
                              cvex.addInput("v", CVEX_TYPE_VECTOR3, v, num_prim);
                           if(myCVEXPrimVars.cvex_pscale_prim)
                              cvex.addInput("pscale", CVEX_TYPE_FLOAT, pscale, num_prim);

                           if(myPrimType == CLUSTER_PRIM_METABALL && myCVEXPrimVars.cvex_weight_prim)
                              cvex.addInput("weight", CVEX_TYPE_FLOAT, weight, num_prim);

                           cvex.addInput("id", CVEX_TYPE_INTEGER, id, num_prim);
                           cvex.addInput("inst_id", CVEX_TYPE_INTEGER, inst_id, num_prim);
                        }

                  }

               // Add the motion blur pass flag
               cvex.addInput("mb_pass", CVEX_TYPE_INTEGER, &mb_pass, 1);
               // Add a time input parm
               cvex.addInput("time", CVEX_TYPE_FLOAT, &time, 1);

#ifdef DEBUG
               std::cout << "VRAY_clusterThis::runCVEX() - Added inputs" << std::endl;
#endif


               // Load the VEX function
               char * cvexArgs[2];
//            cvexArgs[0] = myCVEXFname;
               cvexArgs[0] = theCVEXFname;
               if(!cvex.load(1, cvexArgs)) {
                     std::cerr << "VRAY_clusterThis::runCVEX() - Unable to load cvex function:" << std::endl << cvexArgs[0] << std::endl;
                     throw VRAY_clusterThis_Exception("VRAY_clusterThis::runCVEX() - Unable to load cvex function, exiting VRAY_clusterThis::runCVEX() ...",1);
                  }


               if(myVerbose > CLUSTER_MSG_INFO)
                  std::cout << "VRAY_clusterThis::runCVEX() - Loaded VEX Function" << std::endl;

               CVEX_Value * P_val = NULL, *N_val = NULL, *v_val = NULL, *Cd_val = NULL, *Alpha_val = NULL,
                                                   *pscale_val = NULL, *id_val = NULL, *inst_id_val = NULL, *weight_val = NULL;


               // Find the inputs of the loaded VEX function
               if(myPrimType == CLUSTER_POINT) {

                     P_val = cvex.findInput("P", CVEX_TYPE_VECTOR3);

                     if(myCVEXPointVars.cvex_Cd_pt)
                        Cd_val = cvex.findInput("Cd", CVEX_TYPE_VECTOR3);
                     if(myCVEXPointVars.cvex_Alpha_pt)
                        Alpha_val = cvex.findInput("Alpha", CVEX_TYPE_FLOAT);
                     if(myCVEXPointVars.cvex_N_pt)
                        N_val = cvex.findInput("N", CVEX_TYPE_VECTOR3);
                     if(myCVEXPointVars.cvex_v_pt)
                        v_val = cvex.findInput("v", CVEX_TYPE_VECTOR3);
                     if(myCVEXPointVars.cvex_pscale_pt)
                        pscale_val = cvex.findInput("pscale", CVEX_TYPE_FLOAT);
                  }
               else {
                     if(method == CLUSTER_CVEX_POINT) {
                           P_val = cvex.findInput("P", CVEX_TYPE_VECTOR3);
                        }
                     else {
                           if(myCVEXPrimVars.cvex_Cd_prim)
                              Cd_val = cvex.findInput("Cd", CVEX_TYPE_VECTOR3);
                           if(myCVEXPrimVars.cvex_Alpha_prim)
                              Alpha_val = cvex.findInput("Alpha", CVEX_TYPE_FLOAT);
                           if(myCVEXPrimVars.cvex_N_prim)
                              N_val = cvex.findInput("N", CVEX_TYPE_VECTOR3);
                           if(myCVEXPrimVars.cvex_v_prim)
                              v_val = cvex.findInput("v", CVEX_TYPE_VECTOR3);
                           if(myCVEXPrimVars.cvex_pscale_prim)
                              pscale_val = cvex.findInput("pscale", CVEX_TYPE_FLOAT);

                           if(myPrimType == CLUSTER_PRIM_METABALL && myCVEXPrimVars.cvex_weight_prim)
                              weight_val = cvex.findInput("weight", CVEX_TYPE_FLOAT);
                        }

                  }

               id_val = cvex.findInput("id", CVEX_TYPE_INTEGER);
               inst_id_val = cvex.findInput("inst_id", CVEX_TYPE_INTEGER);


               if(myVerbose > CLUSTER_MSG_INFO) {
                     dumpValueList("VRAY_clusterThis::runCVEX() - Input Parameters", cvex.getInputList());
                  }

               //    dumpValue(P_val);
               //    dumpValue(N);
               //    dumpValue(v);
               //    dumpValue(Cd);
               //    dumpValue(Alpha);
               //    dumpValue(pscale);
               //    dumpValue(weight);


//            std::cout << "VRAY_clusterThis::runCVEX() - Prepared arrays" << std::endl;

               CVEX_Value * P_out = NULL, *N_out = NULL,  *Cd_out = NULL, *v_out = NULL, *Alpha_out = NULL, *pscale_out = NULL, *weight_out = NULL;


               // Now find the outputs of the loaded VEX function
               if(myPrimType == CLUSTER_POINT) {

                     P_out = cvex.findOutput("POut", CVEX_TYPE_VECTOR3);

                     if(myCVEXPointVars.cvex_Cd_pt)
                        Cd_out = cvex.findOutput("CdOut", CVEX_TYPE_VECTOR3);
                     if(myCVEXPointVars.cvex_Alpha_pt)
                        Alpha_out = cvex.findOutput("AlphaOut", CVEX_TYPE_FLOAT);
                     if(myCVEXPointVars.cvex_N_pt)
                        N_out = cvex.findOutput("NOut", CVEX_TYPE_VECTOR3);
                     if(myCVEXPointVars.cvex_v_pt)
                        v_out = cvex.findOutput("vOut", CVEX_TYPE_VECTOR3);
                     if(myCVEXPointVars.cvex_pscale_pt)
                        pscale_out = cvex.findOutput("pscaleOut", CVEX_TYPE_FLOAT);
                  }
               else {
                     if(method == CLUSTER_CVEX_POINT) {
                           P_out = cvex.findOutput("POut", CVEX_TYPE_VECTOR3);
                        }
                     else {

                           if(myCVEXPrimVars.cvex_Cd_prim)
                              Cd_out = cvex.findOutput("CdOut", CVEX_TYPE_VECTOR3);
                           if(myCVEXPrimVars.cvex_Alpha_prim)
                              Alpha_out = cvex.findOutput("AlphaOut", CVEX_TYPE_FLOAT);
                           if(myCVEXPrimVars.cvex_N_prim)
                              N_out = cvex.findOutput("NOut", CVEX_TYPE_VECTOR3);
                           if(myCVEXPrimVars.cvex_v_prim)
                              v_out = cvex.findOutput("vOut", CVEX_TYPE_VECTOR3);
                           if(myCVEXPrimVars.cvex_pscale_prim)
                              pscale_out = cvex.findOutput("pscaleOut", CVEX_TYPE_FLOAT);

                           if(myPrimType == CLUSTER_PRIM_METABALL && myCVEXPrimVars.cvex_weight_prim)
                              weight_out = cvex.findOutput("weightOut", CVEX_TYPE_FLOAT);

                        }
                  }


               //if (!P_out && !N_out && !v_out && !Cd_out && !Alpha_out && !pscale_out && !weight_out)
               //   std::cerr << "VRAY_clusterThis::runCVEX() - " << cvexArgs[0] << " doesn't seem to write to P, N, v, Cd, Alpha, pscale and weight ...???" << std::endl;

               // Setup the output arrays to prepare to recevve the processed VEX data
               if(myPrimType == CLUSTER_POINT) {
                     if(P_out) P_out->setData(POut, num_points);
                     if(Cd_out) Cd_out->setData(CdOut, num_points);
                     if(Alpha_out) Alpha_out->setData(AlphaOut, num_points);
                     if(N_out) N_out->setData(NOut, num_points);
                     if(v_out) v_out->setData(vOut, num_points);
                     if(pscale_out) pscale_out->setData(pscaleOut, num_points);
                  }
               else {

                     if(method == CLUSTER_CVEX_POINT) {

                           if(P_out) {
                                 P_out->setData(POut, num_points);
                                 //                   std::cout << "VRAY_clusterThis::runCVEX() - Set POut successfully  num_points: " << num_points << std::endl;
                              }
                        }
                     else {

                           if(Cd_out) Cd_out->setData(CdOut, num_prim);
                           if(Alpha_out) Alpha_out->setData(AlphaOut, num_prim);
                           if(N_out) N_out->setData(NOut, num_prim);
                           if(v_out) v_out->setData(vOut, num_prim);
                           if(pscale_out) pscale_out->setData(pscaleOut, num_prim);
                           if(weight_out && myPrimType == CLUSTER_PRIM_METABALL)
                              weight_out->setData(weightOut, num_prim);
                        }

                  }



               if(myVerbose > CLUSTER_MSG_INFO)
                  std::cout << "VRAY_clusterThis::runCVEX() - Calling VEX code in .vex file: " << theCVEXFname << std::endl;


               // Run the CVEX code
               if(myPrimType == CLUSTER_POINT) {
                     if(!cvex.run(num_points, false)) {
                           std::cerr << "VRAY_clusterThis::runCVEX() - VEX function call failed!" << std::endl;
                           return 1;
                        }
                  }

               else {
                     if(method == CLUSTER_CVEX_POINT) {
                           if(!cvex.run(num_points, false)) {
                                 std::cerr << "VRAY_clusterThis::runCVEX() - VEX function call failed!" << std::endl;
                                 return 1;
                              }
                        }
                     else {
                           if(!cvex.run(num_prim, false)) {
                                 std::cerr << "VRAY_clusterThis::runCVEX() - VEX function call failed!" << std::endl;
                                 return 1;
                              }
                        }

                  }


               if(myVerbose > CLUSTER_MSG_INFO)
                  std::cout << "VRAY_clusterThis::runCVEX() - Finished running VEX code" << std::endl;

               if(myVerbose > CLUSTER_MSG_INFO) {
                     dumpValueList("VRAY_clusterThis::runCVEX() - Output Parameters", cvex.getOutputList());
                  }

               /*
                               dumpValue(P_out);
                               dumpValue(N_out);
                               dumpValue(v_out);
                               dumpValue(Cd_out);
                               dumpValue(Alpha_out);
                               dumpValue(pscale_out);
                               dumpValue(weightOut);
               */

               num = 0;

               if(myPrimType == CLUSTER_POINT) {

                     GA_FOR_ALL_GPOINTS(cvex_gdp, ppt) {

                        pos.x() = POut[num][0];
                        pos.y() = POut[num][1];
                        pos.z() = POut[num][2];
//                    pos = POut[num];
                        pos.w() = 1.0;
                        ppt->setPos(pos);

                        if(myCVEXPointVars.cvex_Cd_pt) {
                              CdOut[num] = static_cast<UT_Vector3>(ppt->getValue<UT_Vector3>(ptCd_ref, 0));
//                              newAttr->assign( CdOut[num][0], CdOut[num][1], CdOut[num][2] );
                           }

                        if(myCVEXPointVars.cvex_Alpha_pt) {
                              AlphaOut[num] = static_cast<fpreal>(ppt->getValue<fpreal>(ptAlpha_ref, 0));
//                              *newAlpha = AlphaOut[num];
                           }

                        if(myCVEXPointVars.cvex_N_pt) {
                              NOut[num] = static_cast<UT_Vector3>(ppt->getValue<UT_Vector3>(ptN_ref, 0));
//                              newAttr->assign( NOut[num][0], NOut[num][1], NOut[num][2] );
                           }

                        if(myCVEXPointVars.cvex_v_pt) {
                              vOut[num] = static_cast<UT_Vector3>(ppt->getValue<UT_Vector3>(ptV_ref, 0));
//                              newAttr->assign( vOut[num][0], vOut[num][1], vOut[num][2] );
                           }

                        if(myCVEXPointVars.cvex_pscale_pt) {
                              pscaleOut[num] = static_cast<fpreal>(ppt->getValue<fpreal>(ptPscale_ref, 0));
//                              *newPscale = pscaleOut[num];
                           }

//                    cout << "OUT - POut: " << POut[num] << "\tNOut: " << NOut[num] << "\tCdOut: " << CdOut[num] << "\tnewPscale: " << *newPscale << endl;
                        num++;
                     }
                  }

               else {


                     if(method == CLUSTER_CVEX_POINT) {

                           num = 0;

                           GA_FOR_ALL_GPOINTS(cvex_gdp, ppt) {

                              pos.x() = POut[num][0];
                              pos.y() = POut[num][1];
                              pos.z() = POut[num][2];
                              //                    pos = POut[num];
                              pos.w() = 1.0;
                              ppt->setPos(pos);
//                       cout << "OUT - P: " << POut[num] << " " << pos.x() << " " << pos.y() << " " << pos.z() << " " << num << endl;
                              num++;
                           }

//                                cout << "OUT - POut: " << POut[num] << "\tNOut: " << NOut[num] << "\tCdOut: " << CdOut[num]
//                                       << "\tnewAlpha: " << *newAlpha << "\tnewPscale: " << *newPscale << endl;

                        }
                     else {

                           num = 0;

                           GA_FOR_ALL_PRIMITIVES(cvex_gdp, prim) {

                              if(myCVEXPrimVars.cvex_N_prim) {
                                    NOut[num] = static_cast<UT_Vector3>(prim->getValue<UT_Vector3>(primN_ref, 0));
                                 }

                              if(myCVEXPrimVars.cvex_v_prim) {
                                    vOut[num] = static_cast<UT_Vector3>(prim->getValue<UT_Vector3>(primV_ref, 0));
                                 }

                              if(myCVEXPrimVars.cvex_Cd_prim) {
                                    CdOut[num] = static_cast<UT_Vector3>(prim->getValue<UT_Vector3>(primCd_ref, 0));
                                 }

                              if(myCVEXPrimVars.cvex_Alpha_prim) {
                                    AlphaOut[num] = static_cast<float>(prim->getValue<float>(primAlpha_ref, 0));
                                 }

                              if(myCVEXPrimVars.cvex_pscale_prim) {
                                    pscaleOut[num] = static_cast<float>(prim->getValue<float>(primPscale_ref, 0));
                                 }

                              if(myPrimType == CLUSTER_PRIM_METABALL && myCVEXPrimVars.cvex_weight_prim) {
                                    weightOut[num] = static_cast<float>(prim->getValue<float>(primWeight_ref, 0));
                                 }

                              num++;
                           }
                        }

                  }

            } // for (pass < num_passes) ...

         // std::cout << "VRAY_clusterThis::runCVEX() - Successfully set P, N, v, Cd, Alpha, pscale and weight" << std::endl;

         memCleaner.cleanUp(P, POut, N, NOut, v, vOut, Cd, CdOut, weight, weightOut, Alpha, AlphaOut, pscale, pscaleOut, id, inst_id);

      }

   catch(VRAY_clusterThis_Exception e) {
         e.what();
         memCleaner.cleanUp(P, POut, N, NOut, v, vOut, Cd, CdOut, weight, weightOut, Alpha, AlphaOut, pscale, pscaleOut, id, inst_id);

         return 1;
      }

   catch(std::bad_alloc) {
         cout << "VRAY_clusterThis::runCVEX() - Failed memory allocation, freeing geometry and exiting" << endl << endl;
         memCleaner.cleanUp(P, POut, N, NOut, v, vOut, Cd, CdOut, weight, weightOut, Alpha, AlphaOut, pscale, pscaleOut, id,  inst_id);

         return 1;
      }

   catch(...) {
         cout << "VRAY_clusterThis::runCVEX() - Unknown exception encountered, freeing geometry and exiting" << endl << endl;
         memCleaner.cleanUp(P, POut, N, NOut, v, vOut, Cd, CdOut, weight, weightOut, Alpha, AlphaOut, pscale, pscaleOut, id, inst_id);

         return 1;
      }


   return 0;
}




#endif


/**********************************************************************************/
//  $Log: VRAY_clusterThisRunCVEX.C,v $
//  Revision 1.12  2012-09-09 05:00:55  mstory
//  More cleanup and testing.
//
//  Revision 1.11  2012-09-07 15:39:23  mstory
//   Removed all volume instancing (used in different project) and continu… …
//
//  …ed H12 modifications.
//
//  --mstory
//
//  Revision 1.10  2012-09-05 23:02:39  mstory
//  Modifications for H12.
//
//  Revision 1.9  2011-02-15 00:59:15  mstory
//  Refactored out rededundant attribute code in the child (deferred) instancicng mode.
//  Made remaining changes for H11 (and beyond) versions way of handiling attributes.
//
//
//  --mstory
//
//  Revision 1.8  2011-02-06 19:49:15  mstory
//  Modified for Houdini version 11.
//
//  Refactored a lot of the attribute code, cleaned up odds and ends.
//
//  Revision 1.7  2010-04-12 06:39:42  mstory
//  Finished CVEX modifications.
//
//  Revision 1.6  2010-04-10 10:11:42  mstory
//  Added additional CVEX processing.  Fixed a few annoying bugs.  Adding external disk geo source.
//
//  Revision 1.5  2010-02-26 08:06:32  mstory
//  Adding more CVEX options.
//
//  Revision 1.4  2010-02-23 08:36:22  mstory
//  Fixed most of the CVEX problems with primtive instancng.  Fixed seg faults from uninitilialized pointers in the CVEX variables,
//
//  Revision 1.3  2009-11-20 14:59:57  mstory
//  Release 1.4.7 ready.
//
//  Revision 1.2  2009-11-16 17:47:12  mstory
//  Fixed the curve instancing, still need to determine all attribites required for the curve (i.e. width)
//
//  Revision 1.1  2009-11-16 08:35:03  mstory
//  Created seperate source files for some of the functions in VRAY_clusterThis.C.
//

/**********************************************************************************/

