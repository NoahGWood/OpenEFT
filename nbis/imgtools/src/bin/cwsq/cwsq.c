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

      FILE:     CWSQ.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                Michael Garris
                mgarris@nist.gov
      DATE:     11/24/1999
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.

#cat: cwsq - Takes an IHead or raw image pixmap and encodes it using WSQ.
#cat:        This is an implementation based on the Crinimal Justice
#cat:        Information Services (CJIS) document "WSQ Gray-scale
#cat:        Fingerprint Compression Specification", Dec. 1997.

*************************************************************************/

/*****************/
/* Include files */
/*****************/

#include <stdio.h>
#include <sys/param.h>
#include <wsq.h>
#include <ihead.h>
#include <img_io.h>
#include <dataio.h>
#include <parsargs.h>
#include <version.h>

void procargs(int, char **, float *, char **, char **, int *,
              int *, int *, int *, int *, char **);
void print_usage(char *);

/* Contols globally, the level of debug reporting */
/* in this application. */
int debug = 0;

/******************/
/*Start of Program*/
/******************/

int main(int argc, char *argv[])
{
   int ret;
   char *outext;
   int rawflag;             /* input image flag: 0 == Raw, 1 == IHead */
   float r_bitrate;         /* target bit compression rate */
   char *ifile, *cfile, ofile[MAXPATHLEN];     /* Input/Output filenames */
   IHEAD *ihead;            /* Ihead pointer */
   unsigned char *idata;    /* Input data */
   int width, height;       /* image characteristic parameters */
   int depth, ppi;
   unsigned char *odata;    /* Output data */
   int olen;                /* Number of bytes in output data. */
   char *comment_text;


   /* Process the command-line argument list. */
   procargs(argc, argv, &r_bitrate, &outext, &ifile, &rawflag,
            &width, &height, &depth, &ppi, &cfile);

   /* Read the image into memory (IHead or raw pixmap). */
   if((ret = read_raw_or_ihead_wsq(!rawflag, ifile,
                              &ihead, &idata, &width, &height, &depth)))
      exit(ret);

   if(debug > 0)
      fprintf(stdout, "File %s read\n", ifile);

   /* If IHead image file ... */
   if(!rawflag){
      /* Get PPI from IHead. */
      ppi = get_density(ihead);
      free(ihead);
   }

   if(cfile == (char *)NULL)
      comment_text = (char *)NULL;
   else{
      if((ret = read_ascii_file(cfile, &comment_text))){
         free(idata);
         exit(ret);
      }
   }

   /* Encode/compress the image pixmap. */
   if((ret = wsq_encode_mem(&odata, &olen, r_bitrate,
                           idata, width, height, depth, ppi, comment_text))){
      free(idata);
      if(comment_text != (char *)NULL)
         free(comment_text);
      exit(ret);
   }

   free(idata);
   if(comment_text != (char *)NULL)
      free(comment_text);

   if(debug > 0)
      fprintf(stdout, "Image data encoded, compressed byte length = %d\n",
              olen);

   /* Generate the output filename. */
   fileroot(ifile);
   sprintf(ofile, "%s.%s", ifile, outext);

   if((ret = write_raw_from_memsize(ofile, odata, olen))){
      free(odata);
      exit(ret);
   }

   if(debug > 0)
      fprintf(stdout, "Image data written to file %s\n", ofile);

   free(odata);

   /* Exit normally. */
   exit(0);
}

/*****************************************************************/
void procargs(int argc, char **argv, float *r_bitrate,
              char **outext, char **ifile, int *rawflag,
              int *width, int *height, int *depth, int *ppi, char **cfile)
{
   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   if((argc < 4) || (argc > 7)){
      print_usage(argv[0]);
      exit(-1);
   }

   sscanf(argv[1], "%f", r_bitrate);
   *outext = argv[2];
   *ifile = argv[3];
   *rawflag = 0;
   *width = -1;
   *height = -1;
   *depth = -1;
   *ppi = -1;
   *cfile = (char *)NULL;

   /* If argc == 4, we are done here. */

   /* If IHead image file ... */
   if(argc == 5){
      *cfile = argv[4];
   }
   /* Otherwise Raw image file ... */
   else if(argc == 6 || argc == 7){
      /* If rawflag ... */
      if((strncmp(argv[4], "-r", 2) == 0) ||
         (strncmp(argv[4], "-raw", 4) == 0) ||
         (strncmp(argv[4], "-raw_in", 7) == 0)){
         *rawflag = 1;
         parse_w_h_d_ppi(argv[5], argv[0], width, height, depth, ppi);
         if(*depth != 8){
            print_usage(argv[0]);
            fprintf(stderr, "       image depth = %d not 8\n", *depth);
            exit(-1);
         }
      }
      else{
         print_usage(argv[0]);
         fprintf(stderr,
                 "       invalid arg 4, expected \"-raw_in\" option\n");
         exit(-1);
      }
      /* If Comment file ... */
      if(argc == 7){
         *cfile = argv[6];
      }
   }
}

/*****************************************************************/
void print_usage(char *arg0)
{
   fprintf(stderr, "Usage: %s ", arg0);
   fprintf(stderr, "<r bitrate> <outext> <image file>\n");
   fprintf(stderr,
           "                 [-raw_in w,h,d,[ppi]] [comment file]\n\n");
   fprintf(stderr,
           "   r bitrate = compression bit rate (2.25==>5:1, .75==>15:1)\n\n");
}
