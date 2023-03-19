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

      FILE:     DWSQ.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                Michael Garris
                mgarris@nist.gov
      DATE:     11/24/1999
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.

#cat: dwsq - Takes a WSQ compressed image file and decodes it,
#cat:        reconstructing a "lossy" pixmap and saving it to
#cat:        an IHead or raw image file.
#cat:        This is an implementation based on the Crinimal Justice
#cat:        Information Services (CJIS) document "WSQ Gray-scale
#cat:        Fingerprint Compression Specification", Dec. 1997.

*************************************************************************/

#include <stdio.h>
#include <sys/param.h>
#include <wsq.h>
#include <ihead.h>
#include <img_io.h>
#include <version.h>

void procargs(int , char **, char **, char **, int *);

int debug = 0; 

/******************/
/*Start of Program*/
/******************/

int main(int argc, char **argv)
{
   int ret;
   int rawflag;                   /* raw input data or Ihead image */
   char *outext;                  /* ouput file extension */
   char *ifile, ofile[MAXPATHLEN];  /* file names */
   int ilen;
   int width, height;             /* image parameters */
   int depth, ppi;
   unsigned char *idata, *odata;  /* image pointers */
   int lossyflag;                 /* data loss flag */
   NISTCOM *nistcom;              /* NIST Comment */
   char *ppi_str;


   procargs(argc, argv, &outext, &ifile, &rawflag);

   if((ret = read_raw_from_filesize(ifile, &idata, &ilen)))
      exit(ret);

   if((ret = wsq_decode_mem(&odata, &width, &height, &depth, &ppi,
                           &lossyflag, idata, ilen))){
      free(idata);
      exit(ret);
   }

   if(debug > 1)
      fprintf(stderr, "Image pixmap constructed\n");

   fileroot(ifile);
   sprintf(ofile, "%s.%s", ifile, NCM_EXT);

   /* Get NISTCOM from compressed data file */
   if((ret = getc_nistcom_wsq(&nistcom, idata, ilen))){
      free(idata);
      exit(ret);
   }
   free(idata);
   /* WSQ decoder always returns ppi=-1, so believe PPI in NISTCOM, */
   /* if it already exists. */
   ppi_str = (char *)NULL;
   if(nistcom != (NISTCOM *)NULL){
      if((ret = extractfet_ret(&ppi_str, NCM_PPI, nistcom))){
         free(odata);
         freefet(nistcom);
         exit(ret);
      }
   }
   if(ppi_str != (char *)NULL){
      ppi = atoi(ppi_str);
      free(ppi_str);
   }

   /* Combine NISTCOM with image features */
   if((ret = combine_wsq_nistcom(&nistcom, width, height, depth, ppi,
                                lossyflag, 0.0 /* will be deleted next */))){
     free(odata);
     if(nistcom != (NISTCOM *)NULL)
        freefet(nistcom);
     exit(ret);
   }
   if((ret = del_wsq_nistcom(nistcom))){
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
void procargs(int argc, char **argv, char **outext, char **ifile, int *rawflag)
{
   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   if((argc < 3) || (argc > 4)){
      fprintf(stderr, "Usage: %s <outext> <image file> [-raw_out]\n", argv[0]);
      exit(-1);
   }

   *rawflag = 0;
   *outext = argv[1];
   *ifile = argv[2];

   if(argc == 4){
      /* If rawflag ... */
      if((strncmp(argv[3], "-r", 2) == 0) ||
         (strncmp(argv[3], "-raw", 4) == 0) ||
         (strncmp(argv[3], "-raw_out", 8) == 0))
         *rawflag = 1;
      else{
         fprintf(stderr,
                 "Usage: %s <outext> <image file> [-raw_out]\n", argv[0]);
         fprintf(stderr,
                 "       invalid arg 3, expected \"-raw_out\" option\n");
         exit(-1);
      }
   }

   if(debug > 0)
      fprintf(stdout, "file-> %s\n", *ifile);
}
