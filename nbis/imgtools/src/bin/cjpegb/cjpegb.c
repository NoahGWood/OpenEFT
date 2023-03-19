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

      FILE:     CJPEGB.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                Michael Garris
                mgarris@nist.gov
      DATE:     01/09/2001
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.
      UPDATED:  12/22/2008 by Gregory Fiumara - add read_raw()/read_ihead()

#cat: cjpegb - Takes an IHead or raw image pixmap and encodes it using
#cat:        Baseline (Lossy) JPEG, writing the compressed data to file.

*************************************************************************/

#include <stdio.h>
#include <sys/param.h>
#include <jpegb.h>
#include <intrlv.h>
#include <ihead.h>
#include <img_io.h>
#include <dataio.h>
#include <parsargs.h>
#include <version.h>

void procargs(int, char **, int *, char **, char **, int *,
              int *, int *, int *, int *, int *, char **);
void print_usage(char *);

int debug = 0;

/******************/
/*Start of Program*/
/******************/

int main(int argc, char *argv[])
{
   int ret, i;
   int quality, rawflag, intrlvflag;
   char *outext;                        /* ouput file extension */
   char *ifile, *cfile, ofile[MAXPATHLEN]; /* file names */
   int width, height, depth, ppi;       /* image parameters */
   int ilen, olen;
   IHEAD *ihead;
   unsigned char *idata, *odata;        /* image pointers */
   char *comment_text;
   int hor_sampfctr[MAX_CMPNTS], vrt_sampfctr[MAX_CMPNTS];

   procargs(argc, argv, &quality, &outext, &ifile, &rawflag,
            &width, &height, &depth, &ppi, &intrlvflag, &cfile);

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

   /* If necessary, convert to interleaved. */
   if((depth == 24) && (!intrlvflag)){
      for(i = 0; i < MAX_CMPNTS; i++){
         hor_sampfctr[i] = 1;
         vrt_sampfctr[i] = 1;
      }
      if((ret = not2intrlv_mem(&odata, &olen, idata, width, height, depth,
                              hor_sampfctr, vrt_sampfctr, depth>>3))){
         free(idata);
         if(comment_text != (char *)NULL)
            free(comment_text);
         exit(ret);
      }
      free(idata);
      idata = odata;
      ilen = olen;
   }

   if((ret = jpegb_encode_mem(&odata, &olen, quality,
                             idata, width, height, depth, ppi, comment_text))){
      free(idata);
      if(comment_text != (char *)NULL)
         free(comment_text);
      return(ret);
   }

   free(idata);
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
void procargs(int argc, char **argv, int *quality,
              char **outext, char **ifile, int *rawflag,
              int *width, int *height, int *depth, int *ppi,
              int *intrlvflag, char **cfile)
{
   int argi;

   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   if((argc < 4) || (argc > 8)){
      print_usage(argv[0]);
      exit(-1);
   }

   *quality = atoi(argv[1]);
   *outext = argv[2];
   *ifile = argv[3];
   *rawflag = 0;
   *intrlvflag = 1;
   *width = -1;
   *height = -1;
   *depth = -1;
   *ppi = -1;
   *cfile = (char *)NULL;

   /* If argc == 4, we are done here. */

   /* If IHead image file ... */
   argi = 4;
   if(argi >= argc)
      return;
   if(argc == 5){
      *cfile = argv[argi];
      return;
   }
   /* Otherwise (argc >= 6) expect raw flag ... */
   if((strncmp(argv[argi], "-r", 2) == 0) || 
      (strncmp(argv[argi], "-raw", 4) == 0) ||
      (strncmp(argv[argi], "-raw_in", 7) == 0))  {
      argi++;
      if(argi >= argc){
         print_usage(argv[0]);
         fprintf(stderr,
                 "       option \"-raw_in\" missing \"w,h,d,[ppi]\"\n");
         exit(-1);
      }
      *rawflag = 1;
      parse_w_h_d_ppi(argv[argi], argv[0], width, height, depth, ppi);
      argi++;
      if(argi >= argc)
         return;
   }
   else{
      print_usage(argv[0]);
      fprintf(stderr,
              "       expected \"-raw_in\" option\n");
      exit(-1);
   }

   /* If nonintrlv flag ... */
   if(strncmp(argv[argi], "-n", 2) == 0){
      *intrlvflag = 0;
      argi++;
      if(argi >= argc)
         return;
   }

   /* If we get here, set comment file ... */
   *cfile = argv[argi];
   return;
}

/*****************************************************************/
void print_usage(char *arg0)
{
   fprintf(stderr, "Usage: %s ", arg0);
   fprintf(stderr, "<q=20-95> <outext> <image file>\n");
   fprintf(stderr, "                  [-raw_in w,h,d,[ppi]\n");
   fprintf(stderr, "                     [-nonintrlv]]\n");
   fprintf(stderr, "                  [comment file]\n");
}
