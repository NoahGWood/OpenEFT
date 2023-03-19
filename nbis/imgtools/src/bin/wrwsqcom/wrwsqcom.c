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

      FILE:     WRWSQCOM.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
      DATE:     02/02/2001
      UPDATED:  05/10/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.

#cat: wrwsqcom - takes a WSQ compressed image file and inserts a
#cat:            comment in the file at the end of all other comments.

*************************************************************************/

#include <stdio.h>
#include <sys/param.h>
#include <wsq.h>
#include <img_io.h>
#include <imgtype.h>
#include <version.h>

void procargs(int, char **, char **, char **, char **);

int debug = 0; 

/******************/
/*Start of Program*/
/******************/

int main(int argc, char *argv[])
{
   int n, ret, img_type;
   char *ifile, *cfile, *comment_text;
   FILE *outfp;
   unsigned char *idata, *cdata;
   int ilen, clen;

   procargs(argc, argv, &ifile, &cfile, &comment_text);

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

   if(cfile != (char *)NULL)
      if((ret = read_ascii_file(cfile, &comment_text))){
         free(idata);
         exit(ret);
      }

   if((ret = add_comment_wsq(&cdata, &clen, idata, ilen,
                             (unsigned char *)comment_text))) {
      free(idata);
      if(cfile != (char *)NULL)
         free(comment_text);
      exit(ret);
   }
   free(idata);
   if(cfile != (char *)NULL)
      free(comment_text);

   if((outfp = fopen(ifile, "wb")) == NULL) {
      fprintf(stderr, "ERROR: main : fopen : %s\n", ifile);
      free(cdata);
      exit(-2);
   }

   if((n = fwrite(cdata, 1, clen, outfp)) != clen){
      fprintf(stderr, "ERROR: main : fwrite : ");
      /* MDG added '%' to 's' in format below on 05/10/2005 */
      fprintf(stderr, "only %d of %d bytes written from file %s\n",
              n, clen, ifile);
      free(cdata);
      exit(-3);
   }
   free(cdata);
   fclose(outfp);

   exit(0);
}

/*****************************************************************/
void procargs(int argc, char *argv[], char **ifile, char **cfile,
              char **ctext)
{
   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   if(argc != 4){
      fprintf(stderr, "Usage: %s <image file> ", argv[0]);
      fprintf(stderr, "<-f comment file | -t comment text>\n");
      exit(-1);
   }

   *ifile = argv[1];
   *cfile = (char *)NULL;
   *ctext = (char *)NULL;

   if(strncmp(argv[2], "-f", 2) == 0)
      *cfile = argv[3];
   else if(strncmp(argv[2], "-t", 2) == 0)
      *ctext = argv[3];
   else {
      fprintf(stderr, "Usage: %s <image file> ", argv[0]);
      fprintf(stderr, "<-f comment file | -t comment text>\n");
      exit(-1);
   }

   return;
}
