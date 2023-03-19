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

      FILE:     DLWSQCOM.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
      DATE:     02/02/2001
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.

#cat: dlwsqcom - takes a WSQ compressed image file and deletes all
#cat:            comments in the file.

*************************************************************************/

#include <stdio.h>
#include <sys/param.h>
#include <wsq.h>
#include <img_io.h>
#include <imgtype.h>
#include <dataio.h>
#include <swap.h>
#include <defs.h>
#include <version.h>

void procargs(int, char **, char **);

int debug = 0; 

/******************/
/*Start of Program*/
/******************/

int main(int argc, char *argv[])
{
   int n, ret, img_type;
   char *ifile;
   FILE *outfp;
   unsigned char *idata, *cdata;
   int ilen, clen;

   procargs(argc, argv, &ifile);

   if((ret = read_raw_from_filesize(ifile, &idata, &ilen)))
      exit(ret);

   if((ret = image_type(&img_type, idata, ilen))) {
      free(idata);
      exit(ret);
   }

   if(img_type != WSQ_IMG) {
      fprintf(stderr, "ERROR : main : image is not WSQ compressed\n");
      free(idata);
      exit(-1);
   }

   if((ret = delete_comments_wsq(&cdata, &clen, idata, ilen))) {
      free(idata);
      exit(ret);
   }
   free(idata);

   if((outfp = fopen(ifile, "wb")) == NULL) {
      fprintf(stderr, "ERROR: main : fopen : %s\n", ifile);
      free(cdata);
      exit(-2);
   }

   if((n = fwrite(cdata, 1, clen, outfp)) != clen){
      fprintf(stderr, "ERROR: main : fwrite : "
	      "only %d of %d bytes written from file %s\n",
              n, clen, ifile);
      free(cdata);
      exit(-3);
   }
   free(cdata);
   fclose(outfp);

   exit(0);
}

/*****************************************************************/
void procargs(int argc, char *argv[], char **ifile)
{
   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   if(argc != 2){
      fprintf(stderr, "Usage: %s <image file>\n", argv[0]);
      exit(-1);
   }
   *ifile = argv[1];

   return;
}
