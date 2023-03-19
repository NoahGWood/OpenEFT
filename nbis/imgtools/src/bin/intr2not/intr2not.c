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


/************************************************************************

      PACKAGE:  IMAGE ENCODER/DECODER TOOLS

      FILE:     INTR2NOT.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                Michael Garris
                mgarris@nist.gov
      DATE:     11/17/2000
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.
      UPDATED:  12/22/2008 by Gregory Fiumara - add read_raw()/read_ihead()

#cat: intr2not - Takes an IHead or raw pixmap with interleaved color
#cat:            components.  It de-interleaves the pixmap creating
#cat:            one 8-bit image plane for each color component.  This
#cat:            can handle specific downsamplings of component planes
#cat:            that are commonly used with YCbCr colorspace images.

*************************************************************************/

#include <stdio.h>
#include <math.h>
#include <sys/param.h>
#include <intrlv.h>
#include <ihead.h>
#include <img_io.h>
#include <parsargs.h>
#include <version.h>

void procargs(int, char **, char **, char **, int *, int *, int *,
              int *, int *, int *, int *, int *);
void print_usage(char *);

int debug = 0;

/******************/
/*Start of Program*/
/******************/

int main(int argc, char *argv[])
{
   int ret, rawflag;
   char *outext;                   /* ouput file extension */
   char *ifile, ofile[MAXPATHLEN]; /* file names */
   int width, height, depth, ppi, ilen, olen;  /* image parameters */
   IHEAD *ihead;
   unsigned char *idata, *odata;           /* image pointers */
   int hor_sampfctr[MAX_CMPNTS], vrt_sampfctr[MAX_CMPNTS];
   int n_cmpnts;


   procargs(argc, argv, &outext, &ifile, &rawflag,
            &width, &height, &depth, &ppi,
            hor_sampfctr, vrt_sampfctr, &n_cmpnts);

   /* If H,V's are specified on command line, then YCbCr ... */
   /* so read a raw input file. */
   if(argc == 7){
      if((ret = read_raw_from_filesize(ifile, &idata, &ilen)))
         exit(ret);

      if((ret = test_image_size(ilen, width, height, hor_sampfctr,
                               vrt_sampfctr, n_cmpnts, 1)))
         exit(ret);
   }
   else{
      /* If raw image flagged... */
      if(rawflag) {
         if((ret = read_raw(ifile, &idata, &width, &height, &depth)))
            exit(ret);
      }
      /* Otherwise, input image is an IHead image */
      else {
         if((ret = read_ihead(ifile, &ihead, &idata, &width, &height, &depth)))
            exit(ret);
      }
   }

   if(debug > 0)
      fprintf(stdout, "File %s read\n", ifile);


   /* If IHead image file ... */
   if(!rawflag){
      /* Get PPI from IHead. */
      ppi = get_density(ihead);
      free(ihead);
   }

   if((ret = intrlv2not_mem(&odata, &olen, idata, width, height, depth,
                           hor_sampfctr, vrt_sampfctr, n_cmpnts))){
      free(idata);
      exit(ret);
   }
   free(idata);

   if(debug > 0)
      fprintf(stdout, "Image data converted to non-interleaved\n");

   fileroot(ifile);
   sprintf(ofile, "%s.%s", ifile, outext);

   if((ret = write_raw_from_memsize(ofile, odata, olen))){
      free(odata);
      exit(ret);
   }

   if(debug > 0)
      fprintf(stdout, "Image data written to file %s\n", ofile);

   free(odata);

   exit(0);
}

/*****************************************************************/
void procargs(int argc, char **argv,
              char **outext, char **ifile, int *rawflag,
              int *width, int *height, int *depth, int *ppi,
              int *hor_sampfctr, int *vrt_sampfctr, int *n_cmpnts)
{
   int argi, n_cmpnts_tst;

   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   if((argc < 3) || (argc > 7)){
      print_usage(argv[0]);
      exit(-1);
   }

   *outext = argv[1];
   *ifile = argv[2];
   *rawflag = 0;
   *width = -1;
   *height = -1;
   *depth = -1;
   *ppi = -1;
   hor_sampfctr[0] = 1;
   vrt_sampfctr[0] = 1;
   hor_sampfctr[1] = 1;
   vrt_sampfctr[1] = 1;
   hor_sampfctr[2] = 1;
   vrt_sampfctr[2] = 1;
   *n_cmpnts = 3;

   if(argc == 3)
      return;
   argi = 3;
   /* If rawflag ... */
   if((strncmp(argv[argi], "-r", 2) == 0) ||
      (strncmp(argv[argi], "-raw", 4) == 0) ||
      (strncmp(argv[argi], "-raw_in", 7) == 0)){
      *rawflag = 1;
      argi++;
      if(argi == argc){
         print_usage(argv[0]);
         fprintf(stderr,
            "       expected \"w,h,d,[ppi]\" following \"-raw\" option\n");
         exit(-1);
      }
      parse_w_h_d_ppi(argv[argi], argv[0], width, height, depth, ppi);
      /* We are requiring 3 components with each component having 8 bits. */
      /* Thus pixel depth must be 24.  The number of permitted components */
      /* could be changed, for example extended to 4, but this            */
      /* implementation requires 8 bits per component regardless.         */
      if(*depth != 24){
         print_usage(argv[0]);
         fprintf(stderr, "       pixel depth = %d must be 24\n", *depth);
         exit(-1);
      }
      argi++;
      if(argi == argc)
         return;

      /* If YCbCr flag ... */
      if(strncmp(argv[argi], "-Y", 2) == 0){
         argi++;
         if(argi == argc){
            print_usage(argv[0]);
            fprintf(stderr,
                    "       H,V sample factors required with -YCbCr\n");
            exit(-1);
         }
         parse_h_v_sampfctrs(argv[argi], argv[0],
                             hor_sampfctr, vrt_sampfctr, &n_cmpnts_tst);
         if(n_cmpnts_tst != 3) {
            print_usage(argv[0]);
            fprintf(stderr, "       expecting 3 components\n");
            exit(-1);
         }
         argi++;
      }
   }

   if(argi != argc){
      print_usage(argv[0]);
      fprintf(stderr, "       Invalid argument list\n");
      exit(-1);
   }
}

/*****************************************************************/
void print_usage(char *arg0)
{
   fprintf(stderr, "Usage: %s <outext> <image file>\n", arg0);
   fprintf(stderr, "                  [-raw_in w,h,d,[ppi]\n");
   fprintf(stderr, "                     [-YCbCr H0,V0:H1,V1:H2,V2]]\n");
}
