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

      FILE:     DWSQ14.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                Michael Garris
                mgarris@nist.gov
      DATE:     11/24/1999
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.

#cat: dwsq14 - Takes an IHead formatted, WSQ compressed, image file,
#cat:        such as those used in the distribution of legacy data on
#cat:        NIST Special Database 14 and decodes it, reconstructing a
#cat:        "lossy" pixmap and saving it to an IHead or raw image file.
#cat:        This program should be used to convert legacy data only.
#cat:        The format of the files processed by this program should
#cat:        be considered obsolete.

*************************************************************************/

#include <stdio.h>
#include <sys/param.h>
#include <stdlib.h>
#include <wsq.h>
#include <ihead.h>
#include <nistcom.h>
#include <imgtype.h>
#include <img_io.h>
#include <version.h>

extern int wsq14_decode_file(unsigned char **, int *, int *, int *, int *,
                             FILE *);

void procargs(int, char **, char **, char **, int *);

int debug = 0;

/******************/
/*Start of Program*/
/******************/

int main(int argc, char *argv[])
{
   int ret, rawflag, img_type;
   unsigned char *idata, *odata;
   int ilen, width, height, depth, ppi, lossyflag;
   IHEAD *ihead;
   FILE *fp;
   char *outext, *ifile, ofile[MAXPATHLEN];
   NISTCOM *nistcom;


   procargs(argc, argv, &outext, &ifile, &rawflag);
   /* Set ppi to unknown */
   ppi = -1;
   nistcom = (NISTCOM *)NULL;

   /* Check image type */
   if((ret = read_raw_from_filesize(ifile, &idata, &ilen)))
      exit(ret);

   if((ret = image_type(&img_type, idata, ilen))) {
      free(idata);
      exit(ret);
   }
   free(idata);

   /* Open image file for reading based on image type */
   if((fp = fopen(ifile,"rb")) == NULL) {
      fprintf(stderr, "ERROR: main : fopen : %s\n",ifile);
      exit(-1);
   }
   /* If img_type is ihead ... */
   if(img_type == IHEAD_IMG){
      /* Read ihead header and check for WSQ_SD14 compression */
      ihead = readihdr(fp);
      if(atoi(ihead->compress) != WSQ_SD14){
         fprintf(stderr, "ERROR : main : input image not WSQ_SD14 compressed\n");
         fprintf(stderr, "        compression = %d, WSQ_SD14 = %d\n",
                 atoi(ihead->compress), WSQ_SD14);
         fclose(fp);
         free(ihead);
         exit(-1);
      }

      /* Get ppi from ihead header */
      sscanf(ihead->density,"%d", &ppi);

      /* Create a nistcom for the image attributes */
      if((ret = sd_ihead_to_nistcom(&nistcom, ihead, 14))) {
         fclose(fp);
         free(ihead);
         exit(ret);
      }
      free(ihead);
   }
   /* If image not WSQ_IMG or IHEAD_IMG, ERROR!!! */
   else if(img_type != WSQ_IMG) {
      fprintf(stderr, "ERROR : main : Invalid image\n");
      fprintf(stderr, "Expected a WSQ_SD14 compressed image in\n");
      fprintf(stderr, "either raw or ihead format.\n");
      fclose(fp);
      exit(-1);
   }

   /* Decode compressed image */
   if((ret = wsq14_decode_file(&odata, &width, &height, &depth, &lossyflag, 
                              fp))){
      fclose(fp);
      if(img_type == IHEAD_IMG)
         freefet(nistcom);
      exit(ret);
   }
   fclose(fp);

   if(debug > 1)
      fprintf(stderr, "Image pixmap constructed\n");

   /* Combine image attributes into current nistcom */
   if((ret = combine_wsq_nistcom(&nistcom, width, height, depth, ppi,
                                lossyflag, -1.0))){
     if(img_type == IHEAD_IMG)
        freefet(nistcom);
     free(odata);
     exit(ret);
   }
   if((ret = del_wsq_nistcom(nistcom))){
      free(odata);
      freefet(nistcom);
      exit(ret);
   }

   fileroot(ifile);
   sprintf(ofile, "%s.%s", ifile, NCM_EXT);
   /* Write NISTCOM */
   if((ret = writefetfile_ret(ofile, nistcom))){
     freefet(nistcom);
     free(odata);
     exit(ret);
   }
   freefet(nistcom);

   /* Write decompressed image */
   sprintf(ofile, "%s.%s", ifile, outext);
   if((ret = write_raw_or_ihead(!rawflag, ofile, odata, width, height,
                               depth, ppi))){
      free(odata);
      exit(ret);
   }
   free(odata);

   if(debug > 1)
      fprintf(stderr, "Image pixmap written to %s\n", ofile);

   exit(0);
}

/*****************************************************************/
void procargs(int argc, char *argv[], char **outext, char **ifile, int *rawflag)
{
   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   if(argc < 3) {
      fprintf(stderr,
      "Usage: %s <outext> <image file> [-raw_out]\n", argv[0]);
      exit(-1);
   }

   *outext = argv[1];
   *ifile = argv[2];
   *rawflag = 0;

   if(argc == 3)
      return;

   if((strncmp(argv[3], "-r", 2) == 0) ||
     (strncmp(argv[3], "-raw", 4) == 0) ||
     (strncmp(argv[3], "-raw_out", 8) == 0)){
      *rawflag = 1;
      return;
   }
   else {
      fprintf(stderr,
      "Usage: %s <output extension> <filename> [-raw_out]\n", argv[0]);
      exit(-1);
   }
}
