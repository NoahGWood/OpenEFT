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

      FILE:     CMBMCS.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.

#cat: cmbmcs - Combines mean/covariance pairs.

*************************************************************************/

/* Combines mean/covariance pairs. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usagemcs.h>
#include <datafile.h>
#include <util.h>
#include <version.h>

int main(int argc, char *argv[])
{
  char *meanfile_out, *meanfile_out_desc, *desc, *covfile_out, *cp,
    *covfile_out_desc, *ascii_outfiles, str[400];
  int npairs, ipair, iarg_mean, iarg_cov, iarg, dim1, first_dim2,
    dim2, order, *nvecs, nvecs_tot, ijunk1, ijunk2, i, j, k,
    ascii_out = 0, tri_nelts, n_infiles;
  float *cmb_mean, *cmb_cov, *w, the_w, *cov, *mean, *p;
  char *mda = "Combined mean, made by cmbmcs from mean files",
    *cda = "Combined covariance, made by cmbmcs from these files (all mean files listed, then all covariance files):";

  if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
    getVersion();
    exit(0);
  }

  if((argc < 8) || (argc & 1))
    usage("<meanfile_in[meanfile_in...]> <covfile_in[covfile_in...]>\n\
<meanfile_out> <meanfile_out_desc> <covfile_out> <covfile_out_desc>\n\
<ascii_outfiles>");
  meanfile_out = argv[argc - 5];
  meanfile_out_desc = argv[argc - 4];
  covfile_out = argv[argc - 3];
  covfile_out_desc = argv[argc - 2];
  ascii_outfiles = argv[argc - 1];
  npairs = (argc - 6) / 2;
  if(!strcmp(ascii_outfiles, "y"))
    ascii_out = 1;
  else if(!strcmp(ascii_outfiles, "n"))
    ascii_out = 0;
  else
    fatalerr("cmbmcs", "ascii_outfiles must be y or n", NULL);
  if(!(nvecs = (int *)malloc(npairs * sizeof(int))))
    fatalerr("cmbmcs", "malloc", "nvecs");
  if(!(w = (float *)malloc(npairs * sizeof(float))))
  fatalerr("cmbmcs", "malloc", "w");
  matrix_read_dims(argv[1], &dim1, &first_dim2);
  if(dim1 != 1) {
    sprintf(str, "first dimension (%d) of mean file %s is not 1",
      dim1, argv[1]);
    fatalerr("cmbmcs", str, NULL);
  }
  covariance_read_order_nvecs(argv[1 + npairs], &order, nvecs);
  if(order != first_dim2) {
    sprintf(str, "order (%d) of covariance file %s does not \
equal second dimension (%d) of first mean file %s", order,
      argv[1 + npairs], first_dim2, argv[1]);
    fatalerr("cmbmcs", str, NULL);
  }
  nvecs_tot = *nvecs;
  for(iarg_cov = (iarg_mean = 2) + npairs; iarg_mean <= npairs;
    iarg_mean++, iarg_cov++) {
    matrix_read_dims(argv[iarg_mean], &dim1, &dim2);
    if(dim1 != 1) {
      sprintf(str, "first dimension (%d) of mean file %s is not 1",
        dim1, argv[iarg_mean]);
      fatalerr("cmbmcs", str, NULL);
    }
    if(dim2 != first_dim2) {
      sprintf(str, "second dimension (%d) of mean file %s does not \
equal second dimension (%d) of first mean file %s", dim2,
        argv[iarg_mean], first_dim2, argv[1]);
      fatalerr("cmbmcs", str, NULL);
    }
    covariance_read_order_nvecs(argv[iarg_cov], &order, nvecs +
      iarg_mean - 1);
    if(order != first_dim2) {
      sprintf(str, "order (%d) of covariance file %s does not \
equal second dimension (%d) of first mean file %s", order,
        argv[iarg_cov], first_dim2, argv[1]);
      fatalerr("cmbmcs", str, NULL);
    }
    nvecs_tot += *(nvecs + iarg_mean - 1);
  }
  if(!(cmb_mean = (float *)malloc(order * sizeof(float))))
    fatalerr("cmbmcs", "malloc", "cmb_mean");
  for(i = 0, iarg_mean = 1; i < npairs; i++, iarg_mean++) {
    the_w = w[i] = (float)nvecs[i] / nvecs_tot;
    matrix_read(argv[iarg_mean], &cp, &ijunk1, &ijunk2, &mean);
    free(cp);
    if(!i)
      for(j = 0; j < order; j++)
	cmb_mean[j] = the_w * mean[j];
    else
      for(j = 0; j < order; j++)
	cmb_mean[j] += the_w * mean[j];
    free(mean);
  }
  if(!strcmp(meanfile_out_desc, "-")) {
    if(!(desc = malloc(strlen(mda) + npairs * 200)))
      fatalerr("cmbmcs", "malloc", "desc");
    strcpy(desc, mda);
    for(iarg_mean = 1; iarg_mean <= npairs; iarg_mean++) {
      strcat(desc, " ");
      strcat(desc, argv[iarg_mean]);
      if(iarg_mean < npairs)
	strcat(desc, ",");
    }
  }
  else
    desc = meanfile_out_desc;
  matrix_write(meanfile_out, desc, ascii_out, 1, order, cmb_mean);
  tri_nelts = (order * (order + 1)) / 2;
  if(!(cmb_cov = (float *)malloc(tri_nelts * sizeof(float))))
    fatalerr("cmbmcs", "malloc", "cmb_cov");
  for(i = 0, p = cmb_cov; i < order; i++)
    for(j = 0; j <= i; j++)
      *p++ = -cmb_mean[i] * cmb_mean[j];
  for(ipair = 0, iarg_cov = (iarg_mean = 1) + npairs;
    iarg_mean <= npairs; ipair++, iarg_mean++, iarg_cov++) {
    the_w = w[ipair];
    matrix_read(argv[iarg_mean], &cp, &ijunk1, &ijunk2, &mean);
    free(cp);
    for(i = 0, p = cmb_cov; i < order; i++)
      for(j = 0; j <= i; j++)
	*p++ += the_w * mean[i] * mean[j];
    free(mean);

    covariance_read(argv[iarg_cov], &cp, &ijunk1, &ijunk2, &cov);
    for(k = 0; k < tri_nelts; k++)
      cmb_cov[k] += the_w * cov[k];
  }
  if(!strcmp(covfile_out_desc, "-")) {
    if(!(desc = malloc(strlen(cda) + (n_infiles = 2 * npairs) * 200)))
      fatalerr("cmbmcs", "malloc", "desc");
    strcpy(desc, cda);
    for(iarg = 1; iarg <= n_infiles; iarg++) {
      strcat(desc, " ");
      strcat(desc, argv[iarg]);
      if(iarg < n_infiles)
	strcat(desc, ",");
    }
  }
  else
    desc = covfile_out_desc;
  covariance_write(covfile_out, desc, ascii_out, order, nvecs_tot,
    cmb_cov);

  exit(0);
}
