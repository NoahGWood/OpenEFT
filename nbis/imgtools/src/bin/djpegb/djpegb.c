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

      FILE:     DJPEGB.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                Michael Garris
                mgarris@nist.gov
      DATE:     01/09/2001
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.

#cat: djpegb - Takes a Baseline (Lossy) JPEG compressed image file and
#cat:          decodes it to an IHead or raw image file.

*************************************************************************/

#include <stdio.h>
#include <sys/param.h>
#include <jpegb.h>
#include <intrlv.h>
#include <ihead.h>
#include <img_io.h>
#include <version.h>

void procargs(int, char **, char **, char **, int *, int *);
void print_usage(char *);

int debug = 0;

/******************/
/*Start of Program*/
/******************/

int main(int argc, char *argv[])
{
   int ret, i;
   char *outext;                  /* ouput file extension */
   char *ifile, ofile[MAXPATHLEN];/* file names */
   int width, height;             /* image parameters */
   int depth, ppi;
   unsigned char *idata, *odata, *tdata;  /* image pointers */
   int ilen, olen, tlen;                  /* size of file in bytes */
   int rawflag, intrlvflag;
   int lossyflag;                 /* data loss flag */
   NISTCOM *nistcom;              /* NIST Comment */
   char *colorspace;
   int hor_sampfctr[MAX_CMPNTS], vrt_sampfctr[MAX_CMPNTS];


   procargs(argc, argv, &outext, &ifile, &rawflag, &intrlvflag);

   if((ret = read_raw_from_filesize(ifile, &idata, &ilen)))
      exit(ret);

   if((ret = jpegb_decode_mem(&odata, &width, &height, &depth, &ppi,
                             &lossyflag, idata, ilen))){
      free(idata);
      exit(ret);
   }

   if(debug > 1)
      fprintf(stdout, "Image pixmap constructed\n");

   if((depth == 24) && (!intrlvflag)){
      for(i = 0; i < MAX_CMPNTS; i++){
         hor_sampfctr[i] = 1;
         vrt_sampfctr[i] = 1;
      }
      if((ret = intrlv2not_mem(&tdata, &tlen, odata, width, height, depth,
                              hor_sampfctr, vrt_sampfctr, depth>>3))){
         free(idata);
         free(odata);
         exit(ret);
      }
      free(odata);
      odata = tdata;
      olen = tlen;
   }

   fileroot(ifile);
   sprintf(ofile, "%s.%s", ifile, NCM_EXT);

   /* Get NISTCOM from compressed data file */
   if((ret = getc_nistcom_jpegb(&nistcom, idata, ilen))){
      free(idata);
      exit(ret);
   }
   free(idata);

   if(depth == 8)
      colorspace = "GRAY";
   else
      colorspace = "RGB";

   /* Combine NISTCOM with image features */
   if((ret = combine_jpegb_nistcom(&nistcom, width, height, depth, ppi,
                          lossyflag, colorspace, depth>>3, 1 /* intrlv */,
                          -1.0 /* will be deleted next */))){
     free(odata);
     freefet(nistcom);
     exit(ret);
   }
   /* Delete compression attributes */
   if((ret = del_jpegb_nistcom(nistcom))){
      free(odata);
      freefet(nistcom);
      exit(ret);
   }

   /* Write NISTCOM */
   if((ret = writefetfile_ret(ofile, nistcom))){
     free(odata);
     freefet(nistcom);
     exit(ret);
   }
   freefet(nistcom);

   /* Write decoded image file. */
   sprintf(ofile, "%s.%s", ifile, outext);
   if((ret = write_raw_or_ihead(!rawflag, ofile,
                               odata, width, height, depth, ppi))){
      free(odata);
      exit(ret);
   }
   free(odata);
   if(debug > 1)
      fprintf(stdout, "Image pixmap written to %s\n", ofile);

   exit(0);
}

/*****************************************************************/
void print_usage(char *arg0)
{
   fprintf(stderr, "Usage: %s <outext> <image file>\n", arg0);
   fprintf(stderr, "               [-raw_out [-nonintrlv]]\n");
}

/*****************************************************************/
void procargs(int argc, char **argv, char **outext, char **ifile,
              int *rawflag, int *intrlvflag)
{
   int argi;

   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   if((argc < 3) || (argc > 5)){
      print_usage(argv[0]);
      exit(-1);
   }

   *outext = argv[1];
   *ifile = argv[2];
   *rawflag = 0;
   *intrlvflag = 1;

   if(argc == 3)
      return;

   argi = 3;
   /* If rawflag ... */
   if((strncmp(argv[argi], "-r", 2) == 0) ||
      (strncmp(argv[argi], "-raw", 4) == 0) ||
      (strncmp(argv[argi], "-raw_out", 8) == 0)){
      *rawflag = 1;
      argi++;
      if(argi >= argc)
         return;
   }
   else{
      print_usage(argv[0]);
      fprintf(stderr, "      expected option \"-raw_out\"\n");
      exit(-1);
   }
      
   /* If nonintrlv flag ... */
   if(strncmp(argv[argi], "-n", 2) == 0){
      *intrlvflag = 0;
      argi++;
   }

   if(argi < argc){
      print_usage(argv[0]);
      exit(-1);
   }
}
