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

      FILE:     KLTRAN.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.

#cat: kltran - Runs a Karhunen-Loeve transform on a set of vectors.

*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <usagemcs.h>
#include <datafile.h>
#include <util.h>
#include <version.h>


int main(int argc, char *argv[])
{
  FILE *fp_in, *fp_out;
  char *tranmat_file, *vecsfile_out, *vecsfile_out_desc,
    *desc, *ascii_outfile, *adesc, str[400];
  int anint, nrows_use, message_freq, ascii_out = 0, tran_dim1, tran_dim2,
    nvecs, ascii_in, dim1, dim2, iarg, i, k, old_message_len = 0;
  float *tranmat, *invec, *invec_e, *outvec, *outvec_e, *p, *q, *r, a;
  char *mean_file;
  float *mnvec, *m;
  int mn_dim1, mn_dim2;

  if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
     getVersion();
     exit(0);
  }

  if(argc < 9)
    usage("<vecsfile_in[vecsfile_in...]> <mean file> <tranmat_file>\n\
<nrows_use> <vecsfile_out> <vecsfile_out_desc> <ascii_outfile> <message_freq>");
  mean_file = argv[argc - 7];
  tranmat_file = argv[argc - 6];
  nrows_use = atoi(argv[argc - 5]);
  vecsfile_out = argv[argc - 4];
  vecsfile_out_desc = argv[argc - 3];
  ascii_outfile = argv[argc - 2];
  message_freq = atoi(argv[argc - 1]);
  if(nrows_use < 1)
    fatalerr("kltran", "nrows_use must be >= 1", NULL);
  if(!strcmp(ascii_outfile, "y"))
    ascii_out = 1;
  else if(!strcmp(ascii_outfile, "n"))
    ascii_out = 0;
  else
    fatalerr("kltran", "ascii_outfile must be y or n", NULL);
  if(message_freq < 0)
    fatalerr("kltran", "message_freq must be >= 0", NULL);
  matrix_read_dims(tranmat_file, &tran_dim1, &tran_dim2);
  if(nrows_use > tran_dim1) {
    sprintf(str, "nrows_use must be <= first dim. (%d) of transform \
matrix", tran_dim1);
    fatalerr("kltran", str, NULL);
  }
  for(iarg = 1, nvecs = 0; iarg < argc - 7; iarg++) {
    matrix_read_dims(argv[iarg], &dim1, &dim2);
    nvecs += dim1;
    if(dim2 != tran_dim2) {
      sprintf(str, "second dim., %d, of vectors file %s, does not \
equal second dim., %d, of transform matrix %s", dim2, argv[iarg],
        tran_dim2, tranmat_file);
      fatalerr("kltran", str, NULL);
    }
  }
  if(!strcmp(vecsfile_out_desc, "-")) {
    if(!(desc = malloc(200 + (argc - 5) * 200)))
      fatalerr("kltran", "malloc", "desc");
    sprintf(desc, "Made by kltran, using \
transform matrix %s, from vectors file(s)", tranmat_file);
    for(iarg = 1; iarg < argc - 7; iarg++) {
      strcat(desc, " ");
      strcat(desc, argv[iarg]);
      if(iarg < argc - 8)
	strcat(desc, ",");
    }
  }
  else
    desc = vecsfile_out_desc;
  matrix_writerow_init(vecsfile_out, desc,
    ascii_out, nvecs, nrows_use, &fp_out);
  matrix_read_submatrix(tranmat_file, 0, nrows_use - 1, 0,
    tran_dim2 - 1, &adesc, &tranmat);
  free(adesc);

  matrix_read(mean_file, &adesc, &mn_dim1, &mn_dim2,
    &mnvec);
  free(adesc);
    if(mn_dim2 != tran_dim2) {
      sprintf(str, "second dim., %d, of mean vector %s, does not \
equal second dim., %d, of transform matrix %s", mn_dim2, mean_file,
        tran_dim2, tranmat_file);
      fatalerr("kltran", str, NULL);
    }

  if(!(invec = (float *)malloc(tran_dim2 * sizeof(float))))
    fatalerr("kltran", "malloc", "invec");
  invec_e = invec + tran_dim2;
  if(!(outvec = (float *)malloc(nrows_use * sizeof(float))))
    fatalerr("kltran", "malloc", "outvec");
  outvec_e = outvec + nrows_use;
  for(iarg = 1; iarg < argc - 7; iarg++) {
    matrix_readrow_init(argv[iarg], &adesc, &ascii_in, &dim1, &anint,
      &fp_in);
    free(adesc);
    for(k = 0; k < dim1; k++) {
      if(message_freq && !(k % message_freq)) {
	for(i = 0; i < old_message_len; i++)
	  printf("\b");
	sprintf(str, "read vector %d (of %d) of file %d (of %d)",
          k + 1, dim1, iarg, argc - 8);
	fputs(str, stdout);
	fflush(stdout);
	old_message_len = strlen(str);
      }
      matrix_readrow(fp_in, ascii_in, tran_dim2, invec);
      for(p = outvec, q = tranmat; p < outvec_e;) {
	for(a = 0., m = mnvec, r = invec; r < invec_e;) {
	  a += *q++ * (*r - *m);
          r++; m++;
        }
	*p++ = a;
      }
      matrix_writerow(fp_out, ascii_out, nrows_use, outvec);
    }
    fclose(fp_in);
  }
  if(message_freq)
    printf("\n");

  exit(0);
}
