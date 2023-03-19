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

      FILE:     DIFFBYTS.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                Michael Garris
                mgarris@nist.gov
      DATE:     11/24/1999
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.

#cat: diff_bytes - Compares two input files byte by byte and returns
#cat:              a histogram of the gray level differences.

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <img_io.h>
#include <version.h>

int main(int argc, char *argv[])
{
  unsigned char *image1, *image2, *img1, *img2;
  int diffs[256];
  int df, i, t;
  int ret, ilen1, ilen2;

   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

  if(argc != 3) {
     fprintf(stderr,"Usage: %s <file1> <file2>\n", argv[0]);
     exit(1);
  }

  if((ret = read_raw_from_filesize(argv[1], &image1, &ilen1)))
    exit(ret);
  if((ret = read_raw_from_filesize(argv[2], &image2, &ilen2)))
    exit(ret);

  if(ilen1 != ilen2){
     fprintf(stderr, "ERROR : main : %s byte length %d != %s byte length %d\n",
             argv[1], ilen1, argv[2], ilen2);
     free(image1);
     free(image2);
     exit(-1);
  }

  img1 = image1;
  img2 = image2;

  for(i = 0; i < 256; i++)
     diffs[i] = 0;

  for(i = 0; i < ilen1; i++) {
     if(*img1 > *img2)
        df = *img1 - *img2;
     else
        df = *img2 - *img1;
     diffs[df]++;
     img1++;
     img2++;
  }

  t = 0;
  for(i = 0; i < 256; i++) {
     t += diffs[i];
     if(diffs[i] != 0) {
        printf("d[%d] = %d : %f\n", i, diffs[i],
               100.0 * ((float)t/(float)(ilen1)));
     }
   }

  free(image1);
  free(image2);

  exit(0);
}
