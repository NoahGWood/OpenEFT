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

      FILE:    TALLY.C

      AUTHOR:  Stan Janet
      DATE:    12/03/1990
      UPDATED: 05/10/2005 by MDG

      ROUTINES:
               bitcount()
               bytecount()
               pixelcount()

***********************************************************************/

#include <stdio.h>
#include <string.h>
#include <limits.h>

#define BYTE_RANGE	(1<<CHAR_BIT)

/**********************************************************************/
int bitcount(register unsigned int c)
{
   register int b;

   for (b=0; c; c >>= 1){
      if (c & 01)
         b++;
   }

   return(b);
}

/**********************************************************************/
void bytecount(register unsigned char *data, register unsigned int n, register unsigned int *v)
{
   memset(v, 0, BYTE_RANGE*sizeof(unsigned int));
   while (n-- > 0)
      v[*data++]++;
}

/**********************************************************************/
void pixelcount(register unsigned char *data, register unsigned int bytes,
                register unsigned int *zero, register unsigned int *one)
{
   unsigned int i;
   static unsigned char counts[BYTE_RANGE];
   static int counts_initialized = 0;

   if (! counts_initialized) {
      counts_initialized = 1;
      memset(counts, 0, BYTE_RANGE);
      for (i=0; i < BYTE_RANGE; i++)
         counts[i] = bitcount(i);
   }

   *zero = 0;
   *one = 0;
   while (bytes-- > 0) {
      i = counts[*data++];
      *one += i;
      *zero += (CHAR_BIT - i);
   }
}
