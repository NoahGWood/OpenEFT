/*******************************************************************************

License: 
This software and/or related materials was developed at the National Institute
of Standards and Technology (NIST) by employees of the Federal Government
in the course of their official duties. Pursuant to title 17 Section 105
of the United States Code, this software is not subject to copyright
protection and is in the public domain. 

This software and/or related materials have been determined to be not subject
to the EAR (see Part 734.3 of the EAR for exact details) because it is
a publicly available technology and software, and is freely distributed
to any interested party with no licensing requirements.  Therefore, it is 
permissible to distribute this software as a free download from the internet.

Disclaimer: 
This software and/or related materials was developed to promote biometric
standards and biometric technology testing for the Federal Government
in accordance with the USA PATRIOT Act and the Enhanced Border Security
and Visa Entry Reform Act. Specific hardware and software products identified
in this software were used in order to perform the software development.
In no case does such identification imply recommendation or endorsement
by the National Institute of Standards and Technology, nor does it imply that
the products and equipment identified are necessarily the best available
for the purpose.

This software and/or related materials are provided "AS-IS" without warranty
of any kind including NO WARRANTY OF PERFORMANCE, MERCHANTABILITY,
NO WARRANTY OF NON-INFRINGEMENT OF ANY 3RD PARTY INTELLECTUAL PROPERTY
or FITNESS FOR A PARTICULAR PURPOSE or for any purpose whatsoever, for the
licensed product, however used. In no event shall NIST be liable for any
damages and/or costs, including but not limited to incidental or consequential
damages of any kind, including economic damage or injury to property and lost
profits, regardless of whether NIST shall be advised, have reason to know,
or in fact shall know of the possibility.

By using this software, you agree to bear all risk relating to quality,
use and performance of the software and/or related materials.  You agree
to hold the Government harmless from any claim arising from your use
of the software.

*******************************************************************************/

/***********************************************************************
      PACKAGE: NIST Fingerprint Minutiae Detection

      FILE:    MINDTCT.C

      AUTHOR:  Michael D. Garris
      DATE:    04/18/2002
      UPDATED: 09/14/2004
      UPDATED: 05/09/2005 by MDG
      UPDATED: 01/31/2008 by Kenneth Ko
      UPDATED: 09/04/2008 by Kenneth Ko
      UPDATED: 09/30/2008 by Kenenth Ko - add version option.

#cat: mindtct - Uses Version 2 of the NIST Latent Fingerprint System (LFS)
#cat:           to detect minutiae and count ridges in a grayscale image.
#cat:           This version of the program will process:
#cat:           ANSI/NIST, WSQ, JPEGB, JPEGL, and IHead image formats.
#cat:           Results are written to various output files with
#cat:           predefined extensions appeneded to a specified output
#cat:           root path.

***********************************************************************/

#include <stdio.h>
#include <sys/param.h>
#include <an2k.h>
#include <lfs.h>
#include <imgdecod.h>
#include <imgboost.h>
#include <img_io.h>
#include <version.h>
void procargs(int, char **, int *, int *, char **, char **);

int debug = 0;

/*************************************************************************
**************************************************************************/
int main(int argc, char *argv[])
{
   int boostflag, m1flag;
   char *ifile, *oroot, ofile[MAXPATHLEN];
   unsigned char *idata, *bdata;
   int img_type;
   int ilen, iw, ih, id, ippi, bw, bh, bd;
   double ippmm;
   int img_idc, img_imp;
   int *direction_map, *low_contrast_map, *low_flow_map;
   int *high_curve_map, *quality_map;
   int map_w, map_h;
   int ret;
   MINUTIAE *minutiae;
   ANSI_NIST *ansi_nist;
   RECORD *imgrecord;
   int imgrecord_i;

   /* Process command line arguments. */
   procargs(argc, argv, &boostflag, &m1flag, &ifile, &oroot);

   /* 1. READ FINGERPRINT IMAGE FROM FILE INTO MEMORY. */

   /* Is input file in ANSI/NIST format? */
   if((ret = is_ANSI_NIST_file(ifile)) < 0) {
      /* If error ... */
      exit(ret);
   }

   /* If file is ANSI/NIST format ... */
   if(ret){
      img_type = ANSI_NIST_IMG;
      /* Parse ANSI/NIST file into memory structure */
      if((ret = read_ANSI_NIST_file(ifile, &ansi_nist)))
         exit(ret);
      /* Get first grayscale fingerprint record in ANSI/NIST file. */
      if((ret = get_first_grayprint(&idata, &iw, &ih, &id,
                                &ippmm, &img_idc, &img_imp,
                                &imgrecord, &imgrecord_i, ansi_nist)) < 0){
         /* If error ... */
         free_ANSI_NIST(ansi_nist);
         exit(ret);
      }
      /* If grayscale fingerprint not found ... */
      if(!ret){
         free_ANSI_NIST(ansi_nist);
         fprintf(stderr, "ERROR : main : ");
         fprintf(stderr, "grayscale image record not found in %s\n", ifile);
         exit(-2);
      }
   }
   /* Otherwise, not an ANSI/NIST file */
   else{
      /* Read the image data from file into memory */
      if((ret = read_and_decode_grayscale_image(ifile, &img_type,
                    &idata, &ilen, &iw, &ih, &id, &ippi))){
         exit(ret);
      }
      /* If image ppi not defined, then assume 500 */
      if(ippi == UNDEFINED)
         ippmm = DEFAULT_PPI / (double)MM_PER_INCH;
      else 
         ippmm = ippi / (double)MM_PER_INCH;
   }

   /* 2. ENHANCE IMAGE CONTRAST IF REQUESTED */
   if(boostflag)
      trim_histtails_contrast_boost(idata, iw, ih); 

   /* 3. GET MINUTIAE & BINARIZED IMAGE. */
   if((ret = get_minutiae(&minutiae, &quality_map, &direction_map,
                         &low_contrast_map, &low_flow_map, &high_curve_map,
                         &map_w, &map_h, &bdata, &bw, &bh, &bd,
                         idata, iw, ih, id, ippmm, &lfsparms_V2))){
      if(img_type == ANSI_NIST_IMG)
         free_ANSI_NIST(ansi_nist);
      free(idata);
      exit(ret);
   }

   /* Done with input image data */
   free(idata);

   /* 4. WRITE MINUTIAE & MAP RESULTS TO TEXT FILES */
   if((ret = write_text_results(oroot, m1flag, bw, bh,
                               minutiae, quality_map,
                               direction_map, low_contrast_map,
                               low_flow_map, high_curve_map, map_w, map_h))){
      if(img_type == ANSI_NIST_IMG)
         free_ANSI_NIST(ansi_nist);
      free_minutiae(minutiae);
      free(quality_map);
      free(direction_map);
      free(low_contrast_map);
      free(low_flow_map);
      free(high_curve_map);
      free(bdata);
      exit(ret);
   }

   /* Done with minutiae detection maps. */
   free(quality_map);
   free(direction_map);
   free(low_contrast_map);
   free(low_flow_map);
   free(high_curve_map);

   /* 5. WRITE ADDITIONAL RESULTS */

   /* If input is ANSI/NIST ... */
   if(img_type == ANSI_NIST_IMG){

      /* Update ansi/nist structure with results. */
      if((ret = update_ANSI_NIST_lfs_results(ansi_nist, minutiae,
                                            bdata, bw, bh, bd,
                                            ippmm, img_idc, img_imp))){
         if(img_type == ANSI_NIST_IMG)
            free_ANSI_NIST(ansi_nist);
         free_minutiae(minutiae);
         free(bdata);
         exit(ret);
      }

      /* Write updated ANSI/NIST structure to output file */
      sprintf(ofile, "%s.%s", oroot, AN2K_OUT_EXT);
      if((ret = write_ANSI_NIST_file(ofile, ansi_nist))){
         if(img_type == ANSI_NIST_IMG)
            free_ANSI_NIST(ansi_nist);
         free_minutiae(minutiae);
         free(bdata);
         exit(ret);
      }

      /* Done with ANSI/NIST structure */
      free_ANSI_NIST(ansi_nist);
   }
   /* Otherwise, input image is not ANSI/NIST ... */
   else{
      sprintf(ofile, "%s.%s", oroot, BINARY_IMG_EXT);
      if((ret = write_raw_from_memsize(ofile, bdata, bw*bh))){
         free_minutiae(minutiae);
         free(bdata);
         exit(ret);
      }
   }

   /* Done with minutiae and binary image results */
   free_minutiae(minutiae);
   free(bdata);

   /* Exit normally. */
   exit(0);
}

/*************************************************************************
**************************************************************************
   PROCARGS - Process command line arguments
   Input:
      argc  - system provided number of arguments on the command line
      argv  - system provided list of command line argument strings
   Output:
      boostflag - contrast boost flag "-b"
      ifile     - input image file name to be processed by this program
      ifile     - output image file name to be created by this program
**************************************************************************/
void procargs(int argc, char **argv, int *boostflag, int *m1flag,
              char **ifile, char **oroot)
{
   int a;

   *boostflag = FALSE;
   *m1flag = FALSE;

   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   if(argc == 3){
      *ifile = argv[1];
      *oroot = argv[2];
      return;
   }

   if((argc == 4) || (argc == 5)){
      a = 1;
      while(a < argc-2){
         if(strcmp(argv[a], "-b") == 0){
            *boostflag = TRUE;
         }
         else if(strcmp(argv[a], "-m1") == 0){
               *m1flag = TRUE;
         }
         else{
            fprintf(stderr, "Unrecognized flag \"%s\"\n", argv[a]);
            fprintf(stderr,
                    "Usage : %s [-b] [-m1] <finger_img_in> <oroot>\n",
                    argv[0]);
            fprintf(stderr,
   "        -b  = contrast boost image\n");
            fprintf(stderr,
   "        -m1 = output \"*.xyt\" according to ANSI INCITS 378-2004\n");
            exit(1);
         }
         a++;
      }
   }
   else{
      fprintf(stderr, "Invalid number of arguments on command line\n");
      fprintf(stderr,
              "Usage : %s [-b] [-m1] <finger_img_in> <oroot>\n",
              argv[0]);
      fprintf(stderr,
   "        -b  = contrast boost image\n");
      fprintf(stderr,
   "        -m1 = output \"*.xyt\" according to ANSI INCITS 378-2004\n");
            exit(2);
   }
         
   *ifile = argv[a++];
   *oroot = argv[a];
}
