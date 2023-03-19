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

      FILE:     MEANCOV.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.

#cat: meancov - Computes the mean vector and covariance matrix
#cat:           for a set of feature vectors.

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usagemcs.h>
#include <datafile.h>
#include <util.h>
#include <version.h>


int main(int argc, char *argv[])
{
  FILE *fp;
  char *meanfile_out, *meanfile_out_desc, *the_meanfile_out_desc,
    *covfile_out, *covfile_out_desc, *the_covfile_out_desc,
    str[500], *cjunk, *ascii_outfiles;
  int anint, ascii_out = 0, ascii_in, message_freq, dim1, dim2, a_dim2,
    iarg, old_message_len = 0, i, j, k, nvecs = 0;
  float *cov, *covp, *mean, *meanpe, *meanp, *meanp2, meanelt, *v,
    *vp, velt, *vp2, *vp2f;

  if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
     getVersion();
     exit(0);
  }

  if(argc < 8)
    usage("<vecsfile_in[vecsfile_in...]> <meanfile_out>\n\
<meanfile_out_desc> <covfile_out> <covfile_out_desc> <ascii_outfiles>\n\
<message_freq>");
  meanfile_out = argv[argc - 6];
  meanfile_out_desc = argv[argc - 5];
  covfile_out = argv[argc - 4];
  covfile_out_desc = argv[argc - 3];
  ascii_outfiles = argv[argc - 2];
  message_freq = atoi(argv[argc - 1]);
  if(!strcmp(ascii_outfiles, "y"))
    ascii_out = 1;
  else if(!strcmp(ascii_outfiles, "n"))
    ascii_out = 0;
  else
    fatalerr("meancov", "ascii_outfiles must be y or n", NULL);
  if(message_freq < 0)
    fatalerr("meancov", "message_freq must be >= 0", NULL);

  matrix_read_dims(argv[1], &anint, &dim2);
  if(argc > 8) { /* Several input files; check that all have same
    second dimension. */
    if(message_freq)
      printf("checking that all input matrices have same second \
dimension\n");
    for(iarg = 2; iarg < argc - 6; iarg++) {
      matrix_read_dims(argv[iarg], &anint, &a_dim2);
      if(a_dim2 != dim2) {
	sprintf(str, "second dim., %d, of input matrix %s, does \
not equal second dim., %d, of first input matrix %s", a_dim2,
          argv[iarg], dim2, argv[1]);
	fatalerr("meancov", str, NULL);
      }
    }
  }

  /* Accumulate stuff for mean and covariance.  Nonstrict lower
  triangle of covariance is sufficient, since it is symmetric. */
  if(!(cov = (float *)calloc((dim2 * (dim2 + 1)) / 2,
    sizeof(float))))
    fatalerr("meancov", "calloc", "cov");
  if(!(mean = (float *)calloc(dim2, sizeof(float))))
    fatalerr("meancov", "calloc", "mean");
  meanpe = mean + dim2;
  if(!(v = (float *)malloc(dim2 * sizeof(float))))
    fatalerr("meancov", "malloc", "v");
  for(iarg = 1; iarg < argc - 6; iarg++) {
    matrix_readrow_init(argv[iarg], &cjunk, &ascii_in, &dim1,
      &anint, &fp);
    free(cjunk);
    for(k = 0; k < dim1; k++) {
      if(message_freq && !(k % message_freq)) {
	for(i = 0; i < old_message_len; i++)
	  printf("\b");
	sprintf(str, "accumulating from vector %d (of %d) of file \
%d (of %d)", k + 1, dim1, iarg, argc - 7);
	fputs(str, stdout);
	fflush(stdout);
	old_message_len = strlen(str);
      }
      matrix_readrow(fp, ascii_in, dim2, v);
      for(meanp = mean, vp2f = vp = v, covp = cov; meanp < meanpe;
        vp2f++) {
	*meanp++ += (velt = *vp++);
	for(vp2 = v; vp2 <= vp2f;)
	  *covp++ += velt * *vp2++;
      }
    }
    fclose(fp);
    nvecs += dim1;
  }

  if(message_freq)
    printf("\nfinishing mean\n");
  for(meanp = mean; meanp < meanpe;)
    *meanp++ /= nvecs;

  printf("finishing covariance\n");
  for(i = 0, meanp = mean, covp = cov; i < dim2; i++, meanp++)
    for(j = 0, meanelt = *meanp, meanp2 = mean; j <= i; j++, meanp2++,
      covp++)
      *covp = *covp / nvecs - meanelt * *meanp2;

  if(!strcmp(meanfile_out_desc, "-")) {
    if(!(the_meanfile_out_desc = malloc(strlen("Mean vector, \
made by meancov from") + (argc - 7) * 200)))
      fatalerr("meancov", "malloc", "the_meanfile_out_desc");
    strcpy(the_meanfile_out_desc, "Mean vector, made by meancov from");
    for(iarg = 1; iarg <= argc - 7; iarg++) {
      strcat(the_meanfile_out_desc, " ");
      strcat(the_meanfile_out_desc, argv[iarg]);
      if(iarg < argc - 7)
	strcat(the_meanfile_out_desc, ",");
    }
  }
  else
    the_meanfile_out_desc = meanfile_out_desc;
  if(!strcmp(covfile_out_desc, "-")) {
    if(!(the_covfile_out_desc = malloc(strlen("Covariance, \
made by meancov from") + (argc - 7) * 200)))
      fatalerr("meancov", "malloc", "the_covfile_out_desc");
    strcpy(the_covfile_out_desc, "Covariance, made by meancov from");
    for(iarg = 1; iarg <= argc - 7; iarg++) {
      strcat(the_covfile_out_desc, " ");
      strcat(the_covfile_out_desc, argv[iarg]);
      if(iarg < argc - 7)
	strcat(the_covfile_out_desc, ",");
    }
  }
  else
    the_covfile_out_desc = covfile_out_desc;
  if(message_freq)
    printf("writing\n");
  matrix_write(meanfile_out, the_meanfile_out_desc, ascii_out, 1,
    dim2, mean);
  covariance_write(covfile_out, the_covfile_out_desc, ascii_out,
    dim2, nvecs, cov);

  exit(0);
}
