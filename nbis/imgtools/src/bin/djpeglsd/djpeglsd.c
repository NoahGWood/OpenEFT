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

      FILE:     DJPEGL.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
      DATE:     12/15/2000
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.

#cat: djpeglsd - Takes an IHead formatted, JPEGL compressed, image file,
#cat:            such as those distributed with NIST Special Databases
#cat:            {4,9,10,18} and decodes it, reconstructing a "lossless"
#cat:            pixmap and saving it to an IHead or raw image file.
#cat:            If the input file is from a NIST Special Database, then
#cat:            image attributes are stored as a separate NISTCOM text file.
#cat:            This program should be used to convert legacy data only.
#cat:            The format of the files processed by this program should
#cat:            be considered obsolete.

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <jpeglsd4.h>
#include <ihead.h>
#include <nistcom.h>
#include <memalloc.h>
#include <img_io.h>
#include <ioutil.h>
#include <version.h>

#define SD_NUM 4
int sd_list[] = {4,9,10,18};


/**************/
/*routine list*/
/**************/

void procargs(int, char **, char **, char **, int *, int *);

int debug = 0;
/**************/
/*main routine*/
/**************/

int main(int argc, char *argv[])
{
   FILE *fp;
   int ret, rawflag, sd_id;
   char *outext, *ifile, ofile[MAXPATHLEN];
   IHEAD *ihead;
   int width, height, depth, ppi;
   unsigned char *idata, *odata;
   int i, complen, compcode;
   NISTCOM *nistcom;


   procargs(argc, argv, &outext, &ifile, &sd_id, &rawflag);
   nistcom = (NISTCOM *)NULL;

   /* Read ihead header to get image compression information */
   fp = fopen(ifile, "rb");
   if(fp == (FILE *)NULL) {
      fprintf(stderr, "ERROR : main : fopen : %s\n", ifile);
      exit(-1);
   }

   ihead = readihdr(fp);
   if(ihead == (IHEAD *)NULL) {
      fprintf(stderr, "ERROR : main : readihdr :  ihead\n");
      fclose(fp);
      exit(-1);
   }

   /* Get image attributes from header */
   sscanf(ihead->height,"%d", &height);
   sscanf(ihead->width,"%d", &width);
   sscanf(ihead->depth,"%d", &depth);
   sscanf(ihead->density,"%d", &ppi);
   sscanf(ihead->complen,"%d", &complen);
   sscanf(ihead->compress,"%d", &compcode);

   /* Convert image attributes to a nistcom comment */
   if(sd_id){
      if((ret = sd_ihead_to_nistcom(&nistcom, ihead, sd_id))) {
         fclose(fp);
         free(ihead);
         exit(ret);
      }
   }
   free(ihead);

   if((ret = combine_jpegl_nistcom(&nistcom, width, height, depth, ppi, 0, 1,
                                  (int *)NULL, (int *)NULL, 0, -1))) {
      fclose(fp);
      exit(ret);
   }
   if((ret = del_jpegl_nistcom(nistcom))){
      freefet(nistcom);
      fclose(fp);
      exit(ret);
   }

   /* If old JPEGL compressed file ... */
   if(compcode == JPEG_SD) {
      /* Allocate space and read compressed data */
      idata = (unsigned char *)malloc(complen * sizeof(unsigned char));
      if(idata == (unsigned char *)NULL) {
         fprintf(stderr, "ERROR : main : malloc : idata\n");
         fclose(fp);
         freefet(nistcom);
         exit(-1);
      }
      i = fread(idata, sizeof(unsigned char), complen, fp);
      if(i != complen) {
         fprintf(stderr, "ERROR : main : fread : %s\n", ifile);
         fclose(fp);
         freefet(nistcom);
         free(idata);
         exit(-1);
      }
      fclose(fp);

      /* Allocate space for decompressed data */
      malloc_uchar(&odata, width*height, "main");
      odata = (unsigned char *)malloc(width*height * sizeof(unsigned char));
      if(odata == (unsigned char *)NULL) {
         fprintf(stderr, "ERROR : main : malloc : odata\n");
         freefet(nistcom);
         free(idata);
         exit(-1);
      }
   
      /* Decompress data block */
      if((ret = jpegl_sd4_decode_mem(idata, complen,
                                    width, height, depth, odata))){
         freefet(nistcom);
         free(idata);
         free(odata);
         exit(ret);
      }
      free(idata);

      fileroot(ifile);
      sprintf(ofile, "%s.%s", ifile, NCM_EXT);

      /* Write a nistcom file */
      if((ret = writefetfile_ret(ofile, nistcom))){
         freefet(nistcom);
         free(odata);
         exit(ret);
      }
      freefet(nistcom);

      sprintf(ofile, "%s.%s", ifile, outext);

      /* Write decompressed data */
      if((ret = write_raw_or_ihead(!rawflag, ofile, odata, width, height, depth, ppi))) {
         free(odata);
         exit(ret);
      }
      free(odata);
   }
   /* else, Not a old JPEGL compressed file ... */
   else
      fprintf(stderr, "WARNING : main : Image not JPEGL SD[4,9,10,18] compressed, DO NOTHING\n");

   exit(0);
}


/*******************************************/
/*routine to process command line arguments*/
/*******************************************/
void procargs(int argc, char *argv[], char **outext, char **ifile,
              int *sd_id, int *rawflag)
{
   int i, argi, found;

   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   if(argc < 3) {
      fprintf(stderr, "Usage: %s <outext> <image file> [-sd #] [-raw_out]\n", argv[0]);
      exit(-1);
   }

   *outext = argv[1];
   *ifile = argv[2];
   *rawflag = 0;
   *sd_id = 0;

   if(argc == 3)
      return;

   argi = 3;
   if(strncmp(argv[argi], "-s", 2) == 0) {
      argi++;
      if(argc >= 5) {
         *sd_id = atoi(argv[argi]);

         found = 0;
         for(i = 0; i < SD_NUM; i++){
            if(*sd_id == sd_list[i]){
               found = 1;
               break;
            }
         }

         if(!found){
            fprintf(stderr, "Usage: %s <outext> <image file> [-sd #] [-raw_out]\n", argv[0]);
            fprintf(stderr, "              SD list = {4,9,10,18}\n");
            fprintf(stderr, "              SD %d is not a recognized database\n", *sd_id);
            exit(-1);
         }
      }
      else {
         fprintf(stderr, "Usage: %s <outext> <image file> [-sd #] [-raw_out]\n", argv[0]);
         exit(-1);
      }
      argi++;
      if(argi >= argc)
         return;
   }

   if((strncmp(argv[argi], "-r", 2) == 0) ||
      (strncmp(argv[argi], "-raw", 4) == 0) ||
      (strncmp(argv[argi], "-raw_out", 28) == 0)){
      *rawflag = 1;
      return;
   }
   else {
      fprintf(stderr, "Usage: %s <outext> <image file> [-sd #] [-raw_out]\n", argv[0]);
      exit(-1);
   }

}
