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

      PACKAGE:  PCASYS TOOLS

      FILE:     FIXWTS.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                Charles Wilson
                cwilson@nist.gov
      DATE:     10/01/2000
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.
      UPDATED:  02/04/2009 by Joseph C. Konczal - include string.h

#cat: fixwts - M-weighted robust weight filter from network
#cat:          activations.

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include <usagemcs.h>
#include <memalloc.h>
#include <util.h>
#include <version.h>


float wtfunc(float *);
float medk(int, float [], int);


int main(int argc, char *argv[])
{	
   int npats, ninps, nhids, nouts;
   char line[1000], junk[100];
   float w, *vout, *r, *yr;
   int i, j, k, k1;
   float a, b, s;
   double sum;
   FILE *fp;

   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   if(argc != 3)
      usage("<long_error_file> <output_pat_wts>");

   if((fp = fopen(argv[1], "rb")) == NULL)
      syserr("fixwts", "fopen", argv[1]);

   fscanf(fp, "%d %d %d %d", &npats, &ninps, &nhids, &nouts);
   fgets(line,999,fp);
   fgets(line,999,fp);

   malloc_flt(&vout, nouts, "fixwts vout");
   malloc_flt(&r, npats+1, "fixwts r");
   malloc_flt(&yr, npats+1, "fixwts yr");

   k1 = npats/2;
   for (i = 0; i <= npats; i++) {
      fscanf(fp, "%s %s %s %s %d", junk, junk, junk, junk, &j);
      sum = 0.0;
      for (k = 0; k < nouts; k++) {
         fscanf(fp, "%e", &vout[k]);
         if (k == (j-1))
            sum = sum + 1.0 - 2.0 * vout[k] + vout[k] * vout[k];
         else
            sum = sum + vout[k] * vout[k];
      }
      r[i] = sqrt(sum);
      yr[i] = fabs( (double) r[i]);
   }
   fclose(fp);
   free(vout);

   if((fp = fopen(argv[2], "wb")) == NULL)
      syserr("fixwts", "fopen", argv[2]);

   s = medk(npats,yr,k1);
   for (i = 0; i <= npats; i++) {
      yr[i] = r[i];
      if (r[i] != 0.0) {
         a = r[i]/s;
         b = wtfunc(&a);
         w = sqrt( (double) (b/r[i]));
      }
      else
         w = 1.0;

      fprintf(fp, "%10.7f\n",w);
   }
   fclose(fp);
   free(r);
   free(yr);
   exit(0);
}

/************************************************************************/
float wtfunc(float *z)
/* weight function for robust M-weights after Andrews (1973)   */
#define	CLIM	9.0	/* robust limit value */

{
   float a, b;

   if ((*z < -CLIM)|(*z > CLIM))
      return 0.0;
   else {
      a = *z/CLIM;
      b = 1.0-a*a;
      return (a*b*b);
   }
}

/************************************************************************/
/* medk.c 25-Dec-86 20:50     from JLB
   finds k-th smallest number in array x. rearranges x.
   algorithm from Wirth
*/

float medk(int n, float x[], int k)
{
   int i, j, left, right;
   double middle;
    
   if (n < k || k < 1)
      return x[0];
    
   left = 0;
   right = n - 1;
    
   while (left < right) {
      middle = x[k];
      for (i = left, j = right ; i <= j ; ) {
         while (x[i] < middle)
            i++;
         while (middle < x[j])
            j--;
         if (i <= j) {
            float temp;
            temp = x[i];
            x[i] = x[j];
            x[j] = temp;
            i++;
            j--;
         }
      }
      if (j < k) 
         left = i;
      if (k < i) 
         right = j;
    }
    return x[k];
}
/* end of medk.c */
