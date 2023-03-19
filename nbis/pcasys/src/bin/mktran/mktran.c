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

      FILE:     MKTRAN.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.

#cat: mktran - Makes a transform matrix Psi^t * W, where Psi's columns
#cat:          are the first n_eigvecs_use eigenvectors from eigvecs_file
#cat:          and W is the diagonal matrix of the weights from regwts_file.

File formats:
  regwts_file: matrix, dims. rw x rh
  eigvecs_file: matrix, dims. n by nfeats where n is the number
    of eigenvectors contained in the file
  tranmat_file: matrix, n_eigvecs_use by nfeats.

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usagemcs.h>
#include <datafile.h>
#include <memalloc.h>
#include <util.h>
#include <version.h>


void mktran_regwts_to_w(float *, int, int, float *, int, int);

int main(int argc, char *argv[])
{
  char *regwts_file, *eigvecs_file, *tranmat_file,
    *tranmat_file_desc, the_tranmat_file_desc[500],
    *adesc, str[400], *ascii_outfile;
  int n_eigvecs_use, n_eigvecs_have, ascii_out = 0, i, j;
  float *regwts, *eigvecs, *tranmat, *p, *q;
  static float *w;
  int rw, rh, nfeats;

  if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
     getVersion();
     exit(0);
  }

  Usage("<regwts_file> <eigvecs_file> <n_eigvecs_use>\n\
<tranmat_file> <tranmat_file_desc> <ascii_outfile>");
  regwts_file = *++argv;
  eigvecs_file = *++argv;
  n_eigvecs_use = atoi(*++argv);
  tranmat_file = *++argv;
  tranmat_file_desc = *++argv;
  ascii_outfile = *++argv;
  if(!strcmp(ascii_outfile, "y"))
    ascii_out = 1;
  else if(!strcmp(ascii_outfile, "n"))
    ascii_out = 0;
  else
    fatalerr("mktran", "ascii_outfile must be y or n", NULL);

  matrix_read(regwts_file, &adesc, &rh, &rw, &regwts);
  free(adesc);

  matrix_read_dims(eigvecs_file, &n_eigvecs_have, &nfeats);
  if(rw*rh != nfeats/8) {
    sprintf(str, "#regwts (rw*rh)[%d] != #nfeats/8 [%d]", rw*rh, nfeats/8);
    fatalerr("mktran", str, NULL);
  }

  if(n_eigvecs_use > n_eigvecs_have) {
    sprintf(str, "no. of eigenvectors to use, %d, is larger than \
no. of eigenvectors, %d, contained in file %s", n_eigvecs_use,
n_eigvecs_have, eigvecs_file);
    fatalerr("mktran", str, NULL);
  }    

  matrix_read_submatrix(eigvecs_file, 0, n_eigvecs_use - 1, 0,
    nfeats-1, &adesc, &eigvecs);
  free(adesc);

  if(!(tranmat = (float *)malloc(n_eigvecs_use * nfeats *
    sizeof(float))))
    fatalerr("mktran", "malloc", "tranmat");

  malloc_flt(&w, 8*(rw*rh), "mktran w");
  mktran_regwts_to_w(regwts, rw, rh, w, 2*rw, 4*rh);

  for(i = 0, p = tranmat, q = eigvecs; i < n_eigvecs_use; i++)
    for(j = 0; j < nfeats; j++)
      *p++ = *q++ * w[j];

  free(w);
  if(!strcmp(tranmat_file_desc, "-"))
    sprintf(the_tranmat_file_desc, "Transform matrix, made by \
mktran from regional weights %s and eigenvectors %s, using first %d \
eigenvectors", regwts_file, eigvecs_file, n_eigvecs_use);
  else
    strcpy(the_tranmat_file_desc, tranmat_file_desc);
  matrix_write(tranmat_file, the_tranmat_file_desc, ascii_out,
    n_eigvecs_use, nfeats, tranmat);

  exit(0);
}

/*******************************************************************/

/* Makes the diagonal matrix corresponding to the regional weights,
making only the elements on the diagonal. */

void mktran_regwts_to_w(float *regwts, int rw, int rh, float *w,
                        int iw, int ih)
{
  int i, ii, iis, iie, j, jj, jjs, jje, k;
  float a, *w_ptr;
  int p;

  w_ptr = w + iw*(ih/2);
  for(i = iis = k = 0, iie = 2; i < rh; i++, iis += 2, iie += 2)
    for(j = jjs = 0, jje = 2; j < rw; j++, jjs += 2, jje += 2, k++)
      for(ii = iis, a = regwts[k]; ii < iie; ii++)
	for(jj = jjs; jj < jje; jj++) {
          p = ii*iw+jj;
          w[p] = w_ptr[p] = a;
        }
}

/*******************************************************************/
