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

      FILE:    DPYNORM.C

      AUTHORS: Stan Janet
               Michael D. Garris
      DATE:    12/03/1990
      UPDATED: 05/10/2005 by MDG

      ROUTINES:
               dpynorm()

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <dpyimage.h>

/*********************************************************************/
int dpynorm(int argc, char *argv[])
{
   int ret;
   int done=0, align, bpi, bytes;
   unsigned int iw, ih, depth, whitepix;
   unsigned char *data;
   extern int optind, verbose;

   while ( !done && (optind < argc)) {
      if((ret = readfile(argv[optind],&data,&bpi,&iw,&ih,&depth,
                        &whitepix,&align)))
	return(ret);

      if (depth == 1)
         bytes = howmany(iw,CHAR_BIT) * ih;
      else if(depth == 8)
         bytes = iw * ih;
      else /* if(depth == 24) */
         bytes = iw * ih * 3;

      if (verbose > 2) {
         unsigned int zero, one;
         (void) printf("%s:\n",argv[optind]);
         (void) printf("\timage size: %u x %u (%d bytes)\n",
                       iw,ih,bytes);
         (void) printf("\tdepth: %u\n",depth);
                       pixelcount(data,bytes,&zero,&one);
         (void) printf("\tpixel breakdown: %u zero, %u one\n\n",
                       zero,one);
      }

      if((ret = dpyimage(argv[optind],data,iw,ih,depth,whitepix,
                         align,&done))){
         free((char *)data);
         return(ret);
      }

      free((char *)data);
      optind++;
   }

   return(0);
}

