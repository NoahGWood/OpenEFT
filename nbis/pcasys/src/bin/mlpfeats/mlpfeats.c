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

      FILE:     MLPFEATS.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
      DATE:     04/06/2001
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.

#cat: mlpfeats - Converts a feature/class file set from PCASYS to the MLP
#cat:            feature file format.

*************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <datafile.h>
#include <swap.h>
#include <swapbyte.h>
#include <memalloc.h>
#include <util.h>
#include <version.h>

int main(int argc, char *argv[])
{
   FILE *pout;
   char *feats_file, *cls_file, *mlp_file;
   float *feats, *cls_targs;
   char **classes, *feats_desc, *cl_desc;
   int nfeats, npats, ncls, nouts;
   unsigned char *cls_ids;
   int i;
   int itmp, itmp2;

   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   if (argc != 4) {
      fprintf(stderr,
         "Usage: %s <feats_file> <class_file> <mlp_feats_file>\n\n", argv[0]);
      exit(-1);
   }

   feats_file = argv[1];
   cls_file = argv[2];
   mlp_file = argv[3];

   matrix_read(feats_file, &feats_desc, &npats, &nfeats, &feats);
#ifdef __NBISLE__
   swap_float_bytes_vec(feats, npats*nfeats);
#endif
   fprintf(stdout, "npats = %d\nnfeats = %d\n", npats, nfeats);

   classes_read_ind(cls_file, &cl_desc, &ncls, &cls_ids, &nouts, &classes);

   fprintf(stdout, "nouts = %d\nncls = %d\n", nouts, ncls);
   for(i = 0; i < nouts; i++)
      fprintf(stdout, "%s ", classes[i]);
   fprintf(stdout, "\n");

   if(ncls != npats) {
      fprintf(stderr,
         "# classes (%d) in class file != # patterns (%d) in feats file\n",
          ncls, npats);
      free(feats);
      free(cls_ids);
      free_dbl_char(classes, nouts);
      exit(-1);
   }

   cls_targs = (float *)calloc(ncls * nouts, sizeof(float));
   if(cls_targs == (float *)NULL) {
      free(feats);
      free(cls_ids);
      free_dbl_char(classes, nouts);
      syserr(argv[0], "calloc", "cls_targs");
   }

   for(i = 0; i < ncls; i++)
      cls_targs[i*nouts+cls_ids[i]] = 1.0;
   free(cls_ids);
#ifdef __NBISLE__
   swap_float_bytes_vec(cls_targs, ncls*nouts);
#endif

   if((pout = fopen(mlp_file, "wb")) == NULL) {
      free(feats);
      free_dbl_char(classes, nouts);
      free(cls_targs);
      syserr(argv[0],"fopen","out_infile");
   }

/* write npats, nfeats, nouts and three unused zeros */
   itmp = 24;
#ifdef __NBISLE__
   swap_int_bytes(itmp);
   swap_int_bytes(npats);
   swap_int_bytes(nfeats);
   swap_int_bytes(nouts);
#endif
   fwrite(&itmp, sizeof(int), 1, pout);
   fwrite(&npats, sizeof(int), 1, pout);
   fwrite(&nfeats, sizeof(int), 1, pout);
   fwrite(&nouts, sizeof(int), 1, pout);
   itmp2 = 0;
   fwrite(&itmp2, sizeof(int), 1, pout);
   fwrite(&itmp2, sizeof(int), 1, pout);
   fwrite(&itmp2, sizeof(int), 1, pout);
   fwrite(&itmp, sizeof(int), 1, pout);
#ifdef __NBISLE__
   swap_int_bytes(npats);
   swap_int_bytes(nfeats);
   swap_int_bytes(nouts);
#endif

   itmp = 32 * (nouts);
#ifdef __NBISLE__
   swap_int_bytes(itmp);
#endif
   fwrite(&itmp, sizeof(int), 1, pout);
   for(i = 0; i < nouts; i++)
         fwrite(classes[i], sizeof(char), 32, pout);
   fwrite(&itmp, sizeof(int), 1, pout);
   free_dbl_char(classes, nouts);


   itmp = sizeof(float) * nfeats;
#ifdef __NBISLE__
   swap_int_bytes(itmp);
#endif
    itmp2 = sizeof(float) * (nouts);
#ifdef __NBISLE__
    swap_int_bytes(itmp2);
#endif
   for(i = 0; i < npats; i++) {
         fwrite(&itmp, sizeof(int), 1, pout);
         fwrite(&(feats[i*nfeats]), sizeof(float), nfeats, pout);
         fwrite(&itmp, sizeof(int), 1, pout);

         fwrite(&itmp2, sizeof(int), 1, pout);
         fwrite(&(cls_targs[i*nouts]), sizeof(float), nouts, pout);
         fwrite(&itmp2, sizeof(int), 1, pout);
   }
   free(cls_targs);
   free(feats);

   fclose(pout);

   exit(0);
}
