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
      PACKAGE: NIST Fingerprint Image Quality (NFIQ)

      FILE:    ZNORMDAT.C

      ALGORITHM:
               Elham Tabassi
               Charles L. Wilson
               Criag I. Watson

      IMPLEMENTATION:
               Michael D. Garris

      DATE:    09/13/2004
      UPDATED: 05/10/2005 by MDG
      UPDATED: 09/30/2008 by Kenenth Ko - add version option.

#cat: znormdat - Takes patterns file of (un-normalized) feature vectors,
#cat:            and foreach coefficient position across the vectors,
#cat:            computes the mean and stddev, writing these statistics
#cat:            out to support ZNormalization.

***********************************************************************/
#include <stdio.h>
#include <nfiq.h>
#include <mlp.h>
#include <version.h>

extern void procargs(int, char **, char **);

int debug = 0;

/*************************************************************************
**************************************************************************/
int main(int argc, char *argv[])
{
   int ret, i;
   char *ipatsfile;
   float *feats, *targs;
   char **class_set;
   int *classes;
   int nPats, nInps, nOuts;
   float *means, *stddevs;

   /* Process the command line arguments */
   procargs(argc, argv, &ipatsfile);

   /* Read in the input feature vector file */
   if((ret = read_bin_nnpats(ipatsfile, &feats, &targs, &classes,
                  &class_set, &nPats, &nInps, &nOuts))){
      exit(ret);
   }

   /* Do not need target vectors and class information to   */
   /* compute global statistics across feature vector coef, */
   /* so deallocate these other resources. */
   free(targs);
   free(classes);
   for(i = 0; i < nOuts; i++)
      free(class_set[i]);
   free(class_set);

   /* Compute the mean and stddev for each feature vector coef */
   if((ret = comp_znorm_stats(&means, &stddevs, feats, nPats, nInps))){
      free(feats);
      exit(ret);
   }

   /* Report the resulting statistics to stdout */
   for(i = 0; i < nInps; i++){
      printf("%f\t%e\n", means[i], stddevs[i]);
   }

   /* Deallocate remaining resources */
   free(feats);
   free(means);
   free(stddevs);

   /* Exit successful */
   exit(0);
}

/*************************************************************************
**************************************************************************
   PROCARGS - Process command line arguments
   Input:
      argc  - system provided number of arguments on the command line
      argv  - system provided list of command line argument strings
   Output:
      ipatsfile - input patterns file containing the list of feature
                  vectors across which ZNormalization statics are to
                  be computed
**************************************************************************/
void procargs(int argc, char **argv, char **ipatsfile)
{
   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   if(argc != 2){
      fprintf(stderr,
         "Usage : %s <binpats in>\n",
              argv[0]);
      exit(1);
   }

   *ipatsfile = argv[1];
}
