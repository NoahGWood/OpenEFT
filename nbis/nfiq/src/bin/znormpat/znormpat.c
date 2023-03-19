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

      FILE:    ZNORMPAT.C

      ALGORITHM:
               Elham Tabassi
               Charles L. Wilson
               Criag I. Watson

      IMPLEMENTATION:
               Michael D. Garris

      DATE:    09/13/2004
      UPDATED: 05/10/2005 by MDG
      UPDATED: 09/30/2008 by Kenenth Ko - add version option.

#cat: znormpat - Takes A patterns file of (un-normalized) feature vectors,
#cat:            and ZNormalizes its feature vectors based on global statistics
#cat:            provided.

***********************************************************************/
#include <stdio.h>
#include <nfiq.h>
#include <mlp.h>
#include <version.h>

extern void procargs(int, char **, char **, char **, char **);

int debug = 0;

/*************************************************************************
**************************************************************************/
int main(int argc, char *argv[])
{
   int ret, i, j;
   char *ipatsfile, *opatsfile, *znormfile;
   float *feats, *featptr, *targs;
   char **class_set;
   int *classes;
   int nPats, nInps, nOuts;
   float znorm_means[NFIQ_VCTRLEN], znorm_stds[NFIQ_VCTRLEN];

   /* Process the command line arguments */
   procargs(argc, argv, &znormfile, &ipatsfile, &opatsfile);

   /* Read in the global statistics for ZNormalization */
   if((ret = read_znorm_file(znormfile, znorm_means, znorm_stds,
                               NFIQ_VCTRLEN))){
      exit(ret);
   }

   /* Read in the input feature vectors */
   if((ret = read_bin_nnpats(ipatsfile, &feats, &targs, &classes,
                  &class_set, &nPats, &nInps, &nOuts))){
      exit(ret);
   }

   /* Foreach input feature vector ... */
   featptr = feats;
   for(i = 0; i < nPats; i++){
      /* Normalize the current feature vector's coefs in place */
      znorm_fniq_featvctr(featptr, znorm_means, znorm_stds, nInps);
      /* Bump to the next input feature vector */
      featptr += nInps;
   }

   /* Write out the normalized feature vectors */
   if((ret = write_bin_nnpats(opatsfile, feats, targs, class_set,
                             nPats, NFIQ_VCTRLEN, NFIQ_NUM_CLASSES))){
      free(feats);
      free(targs);
      free(classes);
      for(j = 0; j < nOuts; j++)
         free(class_set[j]);
      free(class_set);
      exit(ret);
   }

   /* Free allocated resources */
   free(feats);
   free(targs);
   free(classes);
   for(j = 0; j < nOuts; j++)
      free(class_set[j]);
   free(class_set);

   /* Exit successfully */
   exit(0);
}

/*************************************************************************
**************************************************************************
   PROCARGS - Process command line arguments
   Input:
      argc  - system provided number of arguments on the command line
      argv  - system provided list of command line argument strings
   Output:
      znormfile - file containing global means and stddevs for computing
                  ZNormalization
      ipatsfile - input patterns file containing the list of feature
                  vectors to be ZNormalized
      opatfile  - output patters file containing the normalized feature
                  vectors
**************************************************************************/
void procargs(int argc, char **argv,
              char **znormfile, char **ipatsfile, char **opatsfile)
{
   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   if(argc != 4){
      fprintf(stderr,
         "Usage : %s <znormfile> <binpats in> <binpats out>\n",
              argv[0]);
      exit(1);
   }

   *znormfile = argv[1];
   *ipatsfile = argv[2];
   *opatsfile = argv[3];
}
