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

      FILE:     SD_RFMT.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                Michael Garris
                mgarris@nist.gov
      DATE:     01/31/2001
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.

#cat: sd_rfmt - Takes an IHead encapsulated image file from the NIST
#cat:           archive of fingerprint and mugshot NIST Special Databases
#cat:           {4,9,10,14,&18}.  The file is converted into a
#cat:           "standardized" JPEGL or WSQ format.  Attribute data
#cat:           stored in the input IHead header is reformatted and
#cat:           stored in a comment block in the output file.
#cat:           This program should be used to convert legacy data only.
#cat:           The format of the files processed by this program should
#cat:           be considered obsolete.

*************************************************************************/

#include <stdio.h>
#include <sys/param.h>
#include <stdlib.h>
#include <nistcom.h>
#include <wsq.h>
#include <jpegl.h>
#include <jpeglsd4.h>
#include <ihead.h>
#include <img_io.h>
#include <ioutil.h>
#include <version.h>

#define SD_NUM 5
int sd_list[] = {4,9,10,14,18};


/**************/
/*routine list*/
/**************/

extern int wsq14_2_wsq(unsigned char **, int *, FILE *);

void procargs(int, char **, int *sd_id, char **, char **);

int debug = 0;

/**************/
/*main routine*/
/**************/

int main(int argc, char **argv)
{
   int ret, sd_id;
   char *outext, *ifile, ofile[MAXPATHLEN];
   FILE *infp;
   IHEAD *ihead;
   int width, height, depth, ppi;
   unsigned char *tidata, *idata, *odata, *ocdata;
   int i, complen, olen, oclen;
   IMG_DAT *img_dat;
   NISTCOM *nistcom;
   char *comment_text;
   int sampfctr;

   procargs(argc, argv, &sd_id, &outext, &ifile);

   infp = fopen(ifile, "rb");
   if(infp == (FILE *)NULL) {
      fprintf(stderr, "Error opening file %s for reading\n", ifile);
      exit(-2);
   }

   ihead = readihdr(infp);
   if(ihead == (IHEAD *)NULL) {
      fprintf(stderr, "Error reading IHEAD header\n");
      exit(-3);
   }

   sscanf(ihead->height,"%d", &height);
   sscanf(ihead->width,"%d", &width);
   sscanf(ihead->depth,"%d", &depth);
   sscanf(ihead->density,"%d", &ppi);
   sscanf(ihead->complen,"%d", &complen);

   /* Construct NISTCOM */
   if((ret = sd_ihead_to_nistcom(&nistcom, ihead, sd_id))){
      free(ihead);
      exit(ret);
   }
   free(ihead);
   if(sd_id == 14){
      if((ret = combine_wsq_nistcom(&nistcom, width, height, depth, ppi,
                                   1, -1.0 /* unknown bitrate */))){
         freefet(nistcom);
         exit(ret);
      }
   }
   else{
      if((ret = combine_jpegl_nistcom(&nistcom, width, height, depth, ppi,
                                   0, 1, (int *)NULL, (int *)NULL, 0, PRED4))){
         freefet(nistcom);
         exit(ret);
      }
   }

   /* Convert NISTCOM to string. */
   if((ret = fet2string(&comment_text, nistcom))){
      freefet(nistcom);
      exit(ret);
   }
   freefet(nistcom);

   /* Switch on converter, supplying a NISTCOM ... */
   /* If SD14 ... WSQ convert. */
   if(sd_id == 14){
      /* Convert image data to new format in memory. */
      if((ret = wsq14_2_wsq(&odata, &olen, infp))) {
         fclose(infp);
         free(comment_text);
         exit(ret);
      }
      fclose(infp);
      /* Add comment text into new data format stream. */
      if((ret = add_comment_wsq(&ocdata, &oclen, odata, olen,
                                (unsigned char *)comment_text))){
         free(odata);
         free(comment_text);
         exit(ret);
      }
      free(odata);
      odata = ocdata;
      olen = oclen;
   }
   /* Otherwise, SD 4,9,10,18 ... JPEGL convert. */
   else{
      if((tidata = (unsigned char *)malloc(complen)) == (unsigned char *)NULL){
         fprintf(stderr, "ERROR : %s : malloc : tidata\n", argv[0]);
         exit(-4);
      }

      i = fread(tidata, sizeof(unsigned char), complen, infp);
      if(i != complen) {
         fprintf(stderr, "Error reading compressed data from %s\n", ifile);
         free(tidata);
         exit(-5);
      }
      fclose(infp);

      if(debug > 0)
         fprintf(stdout, "File %s read\n", ifile);

      if((idata = (unsigned char *)malloc(width*height))==(unsigned char *)NULL){
         fprintf(stderr, "ERROR : %s : malloc : idata\n", argv[0]);
         free(tidata);
         free(comment_text);
         exit(-6);
      }

      if((ret = jpegl_sd4_decode_mem(tidata, complen,
                                    width, height, depth, idata))){
         free(tidata);
         free(comment_text);
         free(idata);
         exit(ret);
      }
      free(tidata);
      if(debug > 0){
         fprintf(stdout, "Done decode JPEGL SD4 image\n");
         fprintf(stdout, "Starting JPEGL compression\n");
      }

      /* Used to setup integer array of length 1 initialized to 1. */
      sampfctr = 1;
      if((ret = setup_IMG_DAT_nonintrlv_encode(&img_dat, idata,
                                       width, height, depth, ppi,
                                       &sampfctr, &sampfctr, 1,
                                       0, PRED4))) {
         free(comment_text);
         free(idata);
         exit(ret);
      }

      if(debug > 0)
         fprintf(stdout, "Image structure initialized\n");

      if((ret = jpegl_encode_mem(&odata, &olen, img_dat, comment_text))){
         free(comment_text);
         free_IMG_DAT(img_dat, FREE_IMAGE);
         exit(ret);
      }
      free(comment_text);
      free_IMG_DAT(img_dat, FREE_IMAGE);
   }

   if(debug > 0)
      fprintf(stdout, "Image data converted, reformatted byte length = %d\n",
              olen);

   /* Write reformatted file. */

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

/*******************************************/
/*routine to process command line arguments*/
/*******************************************/

void procargs(int argc, char **argv, int *sd_id, char **outext, char **ifile)
{
   int i, found;

   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   if(argc != 4) {
      fprintf(stderr, "Usage: %s <SD #> <outext> <image file>\n", argv[0]);
      fprintf(stderr, "               SD list = {4,9,10,14,18}\n");
      exit(-1);
   }

   *sd_id = atoi(argv[1]);
   found = 0;
   for(i = 0; i < SD_NUM; i++){
      if(*sd_id == sd_list[i]){
         found = 1;
         break;
      }
   }

   if(!found){
      fprintf(stderr, "Usage: %s <SD #> <outext> <image file>\n", argv[0]);
      fprintf(stderr, "              SD list = {4,9,10,14,18}\n");
      fprintf(stderr,
              "              SD %d is not a recognized database\n", *sd_id);
      exit(-1);
   }

   *outext = argv[2];
   *ifile = argv[3];
}
