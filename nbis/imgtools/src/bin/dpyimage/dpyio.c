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


/***********************************************************************
      PACKAGE: NIST Image Display

      FILE:    DPYIO.C

      AUTHORS: Stan Janet
               Michael D. Garris
      DATE:    12/03/1990
      UPDATED: 05/10/2005 by MDG

      ROUTINES:
               readfile()
               createfile()
               unlinkfile()
               buildheader()
               writeheader()
               readheader()
               writedata()
               readdata()
               fdclose()

***********************************************************************/

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/param.h>
#include <imgtype.h>
#include <imgdecod.h>
#include <dpyimage.h>

/**********************************************************************/
int readfile(char *filename, unsigned char **data, int *bpi, unsigned int *iw, unsigned int *ih,
             unsigned int *depth, unsigned int *whitepix, int *align)
{
   extern int raw;
   extern unsigned int raw_w, raw_h, raw_depth, raw_whitepix;
   int ret, ilen, img_type;

   ret = read_and_decode_dpyimage(filename, &img_type, data, &ilen,
                                  (int *)iw, (int *)ih, (int *)depth, bpi);

   /* If ERROR on reading file ... */
   if(ret < 0){
      return(ret);
   }

   if(ret == IMG_IGNORE)
      return(ret);

   if(img_type == UNKNOWN_IMG){
      if(raw){
         *bpi = 0;
         *align = CHAR_BIT;
         *whitepix = raw_whitepix;
         *iw = raw_w;
         *ih = raw_h;
         *depth = raw_depth;
      }
      else{
         fprintf(stderr, "ERROR : readfile : input file %s is assumed RAW\n",
                 filename);
         fprintf(stderr, "                   -r option must be specified\n");
         return(-2);
      }
   }
   else{
      *align = CHAR_BIT;
      if(*depth == 1)
         *whitepix = 0;
      else
         *whitepix = 255;
   }

   return(0);
}

/***************************************************************/
int createfile(char *filename)
{
   int ret;
   FILE *fp;

   fp = fopen(filename,"wb");
   if (fp == (FILE *) NULL) {
      (void) fprintf(stderr,"dpyimage: cannot open file %s for writing\n",
                     filename);
      return(-2);
   }

   if((ret = fclose(fp))){
      (void) fprintf(stderr,"dpyimage: cannot close file %s\n",
                     filename);
      return(ret);
   }

   return(0);
}

/**************************************************************/
void unlinkfile(char *filename)
{
   if (unlink(filename) < 0) {
      char buffer[2*MAXPATHLEN];
      int e;

      e = errno;
      (void) sprintf(buffer,"dpyimage: cannot unlink %s",filename);
      errno = e;
      perror(buffer);
   }
}

/****************************************************************/
void buildheader(struct header_t *header, char *filename,
                 unsigned int w, unsigned int h, unsigned int depth, unsigned int whitepix, int align)
{
   (void) strcpy(header->filename,filename);
   header->iw = w;
   header->ih = h;
   header->depth = depth;
   header->whitepix = whitepix;
   header->align = align;
}

/**************************************************************/
int writeheader(FILE *fp, struct header_t *header)
{
   int n;

   n = fwrite((char *)header,1,HEADERSIZE,fp);
   if (n != HEADERSIZE) {
      (void) fprintf(stderr,
                     "%s: header fwrite returned %d, expected %d\n",
                     program,n,HEADERSIZE);
      return(-2);
   }

   return(0);
}

/************************************************************/
int readheader(FILE *fp, struct header_t *header)
{
   int n;

   n = fread((char *)header,1,HEADERSIZE,fp);
   if (n != HEADERSIZE) {
      (void) fprintf(stderr,
                     "%s: header fread returned %d, expected %d\n",
                     program,n,HEADERSIZE);
      return(-2);
   }

   return(0);
}

/*************************************************************/
int writedata(FILE *fp, unsigned char *data, int len)
{
   int n;

   n = fwrite((char *)data,1,len,fp);
   if (n != len) {
      (void) fprintf(stderr,
                     "%s: data fwrite returned %d, expected %d\n",
                     program,n,len);
      return(-2);
   }

   return(0);
}

/************************************************************/
int readdata(FILE *fp, unsigned char *data, int bytes)
{
   int n;

   n = fread((char *)data,1,bytes,fp);
   if (n != bytes) {
      (void) fprintf(stderr,
                     "%s: data fread returned %d, expected %d\n",
                     program,n,bytes);
      return(-2);
   }

   return(0);
}

/********************************************************************/
int fdclose(int fd, char *s)
{
   if (close(fd) < 0) {
      int e;
      char buffer[MAXPATHLEN];

      e = errno;
      (void) sprintf(buffer, "%s: cannot close fd %d (%s)", program,fd,s);
      errno = e;
      perror(buffer);
      return(-2);
   }

   return(0);
}

