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

      FILE:     CJPEGL.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                Michael Garris
                mgarris@nist.gov
      DATE:     11/17/2000
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.
      UPDATED:  12/22/2008 by Gregory Fiumara - add read_raw()/read_ihead()

#cat: cjpegl - Takes an IHead or raw image pixmap and encodes it using
#cat:        Lossless JPEG (JPEGL), writing the compressed data to file.

*************************************************************************/

#include <stdio.h>
#include <sys/param.h>
#include <math.h>
#include <jpegl.h>
#include <intrlv.h>
#include <ihead.h>
#include <img_io.h>
#include <dataio.h>
#include <parsargs.h>
#include <version.h>

void procargs(int, char **, char **, char **, int *, int *, int *, int *,
              int *, int *, int *, int *, int *, char **);
void print_usage(char *);

int debug = 0;

/******************/
/*Start of Program*/
/******************/

int main(int argc, char *argv[])
{
   int ret, rawflag, intrlvflag;
   char *outext;                  /* ouput file extension */
   char *ifile, *cfile, ofile[MAXPATHLEN]; /* file names */
   int width, height, depth, ppi, ilen, olen;  /* image parameters */
   IHEAD *ihead;
   unsigned char *idata, *odata;          /* image pointers */
   IMG_DAT *img_dat;
   int hor_sampfctr[MAX_CMPNTS], vrt_sampfctr[MAX_CMPNTS];
   int n_cmpnts;
   char *comment_text;



   procargs(argc, argv, &outext, &ifile, &rawflag,
            &width, &height, &depth, &ppi, &intrlvflag,
            hor_sampfctr, vrt_sampfctr, &n_cmpnts, &cfile);

   /* If nonintrlv or H,V's are specified for YCbCr ... */
   if((!intrlvflag) || (argc > 6)){
      if((ret = read_raw_from_filesize(ifile, &idata, &ilen)))
         exit(ret);
      if((ret = test_image_size(ilen, width, height, hor_sampfctr,
                               vrt_sampfctr, n_cmpnts, intrlvflag)))
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

      /* If IHead file, then n_cmpnts must be set based on pixel depth. */
      if(depth == 8)
         n_cmpnts = 1;
      else if(depth == 24)
         n_cmpnts = 3;
      else{
         fprintf(stderr, "ERROR : main : depth = %d not equal to 8 or 24\n",
                 depth);
         exit(-2);
      }
   }

   if(cfile == (char *)NULL)
      comment_text = (char *)NULL;
   else{
      if((ret = read_ascii_file(cfile, &comment_text))){
         free(idata);
         exit(ret);
      }
   }

   /* If necessary, convert to non-interleaved. */
   if((n_cmpnts > 1) && (intrlvflag)){
      if((ret = intrlv2not_mem(&odata, &olen, idata, width, height, depth,
                              hor_sampfctr, vrt_sampfctr, n_cmpnts))){
         free(idata);
         if(comment_text != (char *)NULL)
            free(comment_text);
         exit(ret);
      }
      free(idata);
      idata = odata;
      ilen = olen;
   }

   if((ret = setup_IMG_DAT_nonintrlv_encode(&img_dat, idata,
                                    width, height, depth, ppi,
                                    hor_sampfctr, vrt_sampfctr, n_cmpnts,
                                    0, PRED4))){
      free(idata);
      if(comment_text != (char *)NULL)
         free(comment_text);
      return(ret);
   }
   free(idata);

   if(debug > 0){
      fprintf(stdout, "Image structure initialized\n");
      fflush(stderr);
   }

   if((ret = jpegl_encode_mem(&odata, &olen, img_dat, comment_text))){
      free_IMG_DAT(img_dat, FREE_IMAGE);
      if(comment_text != (char *)NULL)
         free(comment_text);
      exit(ret);
   }

   free_IMG_DAT(img_dat, FREE_IMAGE);
   if(comment_text != (char *)NULL)
      free(comment_text);

   if(debug > 0)
      fprintf(stdout, "Image data encoded, compressed byte length = %d\n",
              olen);

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
              int *width, int *height, int *depth, int *ppi, int *intrlvflag,
              int *hor_sampfctr, int *vrt_sampfctr, int *n_cmpnts,
              char **cfile)
{
   int argi;

   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   if((argc < 3) || (argc > 9)){
      print_usage(argv[0]);
      exit(-1);
   }

   *outext = argv[1];
   *ifile = argv[2];
   *rawflag = 0;
   *intrlvflag = 1;
   *width = -1;
   *height = -1;
   *depth = -1;
   *ppi = -1;
   *cfile = (char *)NULL;
   *n_cmpnts = 0;

   hor_sampfctr[0] = 1;
   vrt_sampfctr[0] = 1;
   hor_sampfctr[1] = 1;
   vrt_sampfctr[1] = 1;
   hor_sampfctr[2] = 1;
   vrt_sampfctr[2] = 1;

   /* If argc == 3, we are done here. */

   /* If IHead image file ... */
   if(argc == 4){
      if((strncmp(argv[3], "-r", 2) == 0) || (strncmp(argv[3], "-n", 2) == 0)
         || (strncmp(argv[3], "-Y", 2) == 0) ||
            (strncmp(argv[3], "-raw", 4) == 0 ||
            (strncmp(argv[3], "-raw_in", 7) == 0))){
            print_usage(argv[0]);
            fprintf(stderr, "       invalid arg 4 expecting comment file not\n");
            fprintf(stderr, "       -raw_in, -noninrlv, or -YCbCr\n");
            exit(-1);
      }
      *cfile = argv[3];
   }
   /* Otherwise Raw image file ... */
   else if(argc >= 5){
      /* If rawflag ... */
      if((strncmp(argv[3], "-r", 2) == 0) ||
         (strncmp(argv[3], "-raw", 4) == 0) ||
         (strncmp(argv[3], "-raw_in", 7) == 0)){
         *rawflag = 1;
         parse_w_h_d_ppi(argv[4], argv[0], width, height, depth, ppi);
         if(*depth == 8)
            *n_cmpnts = 1;
         else if(*depth == 24)
            *n_cmpnts = 3;
         else{
            print_usage(argv[0]);
            fprintf(stderr, "       depth = %d not equal to 8 or 24\n",
                    *depth);
            exit(-1);
         }
      }
      else{
         print_usage(argv[0]);
         fprintf(stderr, "       invalid arg 4, expected \"-raw\" option\n");
         exit(-1);
      }

      argi = 5;
      if(argi >= argc)
         return;
      /* If non-intrlvflag ... */
      if(strncmp(argv[argi], "-n", 2) == 0){
         if(*depth == 8){
            print_usage(argv[0]);
            fprintf(stderr, "       invalid -nonintrlv option, depth = 8\n");
            exit(-1);
         }
         *intrlvflag = 0;
         argi++;
         if(argi >= argc)
            return;
      }
      /* If YCbCr flag ... */
      if(strncmp(argv[argi], "-Y", 2) == 0){
         if(*depth == 8){
            print_usage(argv[0]);
            fprintf(stderr, "       invalid -YCbCr option, depth = 8\n");
            exit(-1);
         }
         argi++;
         if(argi >= argc){
            print_usage(argv[0]);
            fprintf(stderr, "       -YCbCr flag missing sample factors\n");
            exit(-1);
         }
         parse_h_v_sampfctrs(argv[argi], argv[0],
                             hor_sampfctr, vrt_sampfctr, n_cmpnts);
         argi++;
      }

      /* If Comment file ... */
      if(argi < argc){
         *cfile = argv[argi];
         argi++;
      }

      if(argi != argc){
         print_usage(argv[0]);
         fprintf(stderr, "       too many arguments specified\n");
         exit(-1);
      }
   }
}

/*****************************************************************/
void print_usage(char *arg0)
{
   fprintf(stderr, "Usage: %s <outext> <image file>\n", arg0);
   fprintf(stderr, "                  [-raw_in w,h,d,[ppi]\n");
   fprintf(stderr, "                     [-nonintrlv]\n");
   fprintf(stderr, "                     [-YCbCr H0,V0:H1,V1:H2,V2]]\n");
   fprintf(stderr, "                  [comment file]\n");
}
