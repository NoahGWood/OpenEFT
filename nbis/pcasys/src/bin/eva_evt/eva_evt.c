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

      FILE:     EVA_EVT.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.

#cat: eva_evt - Computes the eigen values and vectors of
#cat:           a covariance matrix.

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pca.h>
#include <usagemcs.h>
#include <memalloc.h>
#include <datafile.h>
#include <util.h>
#include <version.h>

int main(int argc, char *argv[])
{
   int i, j;
   int order;
   int nevtr; /* number evts required */
   int nevtf; /* number evts found */
   float *evas, *evts; /* eigen values and eigen vectors */
   float *tcov, *tcovp, *cov;
   char *desc;
   char *ascii_outfiles, *evtfile, *evtdesc, *evafile, *evadesc;
   char *covfile;
   int ascii_out = 0, nvecs;

   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   if (argc != 8)
      usage("<covfile> <num_eva_evt_wanted> <evafile> <eva_desc>\n\
<evtfile> <evt_desc> <ascii_outfiles>");

   covfile = argv[1];
   nevtr = atoi(argv[2]);
   evafile = argv[3];
   evadesc = argv[4];
   evtfile = argv[5];
   evtdesc = argv[6];
   ascii_outfiles = argv[7];
   if(!strcmp(ascii_outfiles, "y"))
      ascii_out = 1;
   else if(!strcmp(ascii_outfiles, "n"))
      ascii_out = 0;
   else
      fatalerr("eva_evt", "ascii_outfiles must be y or n", NULL);

   covariance_read(covfile, &desc, &order, &nvecs, &tcov);

   malloc_flt(&cov, order*order, "eva_evt cov");
   tcovp = tcov;
   /* only need to set nonstrict upper triangle for eigen */
   for(j = 0; j < order; j++)
      for(i = 0; i <= j; i++)
         cov[j+i*order] = *tcovp++;

   free(tcov);

   eigen(nevtr, &nevtf, &evts, &evas, cov, order);
   free(cov);
   free(desc);

   if(!strcmp(evadesc, "-")) {
      malloc_char(&evadesc, 200, "eva_evt evadesc");
      sprintf(evadesc, "%d Eigenvalues, made by eva_evt from covariance %s",
                        nevtf, covfile);
   }
   matrix_write(evafile, evadesc, ascii_out, 1, nevtf, evas);
   free(evadesc);

   if(!strcmp(evtdesc, "-")) {
      malloc_char(&evtdesc, 200, "eva_evt evadesc");
      sprintf(evtdesc, "%d Eigenvectors, made by eva_evt from covariance %s",
                        nevtf, covfile);
   }
   matrix_write(evtfile, evtdesc, ascii_out, nevtf, order, evts);
   free(evtdesc);

   free(evts);
   free(evas);
   exit(0);
}
