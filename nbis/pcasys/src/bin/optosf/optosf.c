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

      FILE:     OPTOSF.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.

#cat: optosf - Optimizes the overall smoothing factor (osf) which the
#cat:          Probabilistic Neural Net (PNN) classifier is to use.

Optimizes the overall smoothing factor (osf) which the
Probabilistic Neural Net (PNN) classifier is to use.  The idea here is
to let the prototypes set be the full set of prototypes to be used by
the classifier (already incorporating the optimized regional weights)
and to let the "tuning" set that is classified using these prototypes
to produce an activation error rate, be a smaller subset of (top)
feature vectors.  Each time a tuning vector is classified, it is left
out of the prototypes set.  (Otherwise, classication would presumably
do very well regardless of the osf, so the program would not learn a
good osf.)  It is important that the prototypes set be the full set to
be used in the finished classifier and not just a subset, since the
optimal osf depends on the number of prototypes; but the subset of it
used as the tuning set can be of any reasonable size.

The program makes a text file that has a line for each osf value
tried.  The line shows the osf, the resulting activation error rate,
and the resulting classification error rate.  The activation error
rate, which is the quantity optosf tries to minimize, is the average,
over the tuning set, of the squared difference between 1 and the
normalized PNN activation of the actual class.  The classification
error rate is the fraction of the tuning prints misclassified.  When
the program finishes, the last line of the output file will correspond
to the optimal osf that it finds.  The output file has a description
string before the osf lines; this is either outfile_desc (with newline
appended) or, if outfile_desc is - (hyphen), then the program makes a
reasonable description, which indicates the values of the important
parameters.  If verbose is y, the program writes each computed (osf,
activ. error, classif. error) to the standard output.

The optimal osf found by optosf should be specified in the parameter
file, when running the finished classifier.

*************************************************************************/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <little.h>
#include <usagemcs.h>
#include <table.h>
#include <datafile.h>
#include <memalloc.h>
#include <util.h>
#include <version.h>

static FILE *fp_out;
static int verbose_int;

static struct {
  char n_feats_use, osf_init, osf_initstep, osf_stepthr, tablesize,
    verbose, fvs_file, classes_file, n_fvs_use_as_protos_set,
    n_fvs_use_as_tuning_set, outfile, outfile_desc;
} setflags;

void optosf_pnn(int, int, int, float *, unsigned char *, int, float,
              float *, float *);
void optosf_read_parms(char [], int *, float *, float *, float *,
              int *, int *, char [], char [], int *, int *, char [], char []);
void optosf_check_parms_allset(void);
void out_prog(char []);


int main(int argc, char *argv[])
{
  char *prsfile, fvs_file[200], classes_file[200], outfile[200],
    outfile_desc[200], str[400], *datadir, *desc;
  unsigned char *classes;
  int n_feats_use, tablesize, n_fvs_use_as_protos_set,
    n_fvs_use_as_tuning_set;
  float osf_init, osf_initstep, osf_stepthr, osf, osf_step, osf_prev = 0,
    acerror, acerror_prev, classerror, classerror_prev = 0, *fvs;
  int n_cls;
  char **lcnptr;

  if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
     getVersion();
     exit(0);
  }

  Usage("<prsfile>"); /* required user parameters file */
  prsfile = *++argv;

  /* Reads default optosf parameters file, then user parameters file,
  which overrides defaults. Checks that no parameter is left unset. */
  memset(&setflags, 0, sizeof(setflags));

#ifdef __MSYS__
  sprintf(str, "./optosf.prs");
#else
  datadir = get_datadir();
  sprintf(str, "%s/parms/optosf.prs", datadir);
#endif

  optosf_read_parms(str, &n_feats_use, &osf_init, &osf_initstep,
    &osf_stepthr, &tablesize, &verbose_int, fvs_file, classes_file,
    &n_fvs_use_as_protos_set, &n_fvs_use_as_tuning_set, outfile,
    outfile_desc);
  optosf_read_parms(prsfile, &n_feats_use, &osf_init, &osf_initstep,
    &osf_stepthr, &tablesize, &verbose_int, fvs_file, classes_file,
    &n_fvs_use_as_protos_set, &n_fvs_use_as_tuning_set, outfile,
    outfile_desc);
  optosf_check_parms_allset();

  if(n_fvs_use_as_tuning_set > n_fvs_use_as_protos_set) {
    sprintf(str, "n_fvs_use_as_tuning_set, %d, is > \
n_fvs_use_as_protos_set, %d", n_fvs_use_as_tuning_set,
      n_fvs_use_as_protos_set);
    fatalerr("optosf", str, NULL);
  }

  /* Read feature vectors and classes. */
  matrix_read_submatrix(tilde_filename(fvs_file, 0), 0,
    n_fvs_use_as_protos_set - 1, 0, n_feats_use - 1, &desc, &fvs);
  free(desc);
  classes_read_subvector_ind(tilde_filename(classes_file, 0), 0,
    n_fvs_use_as_protos_set - 1, &desc, &classes, &n_cls, &lcnptr);
  free(desc);
  free_dbl_char(lcnptr, n_cls);

  fp_out = fopen_ch(outfile, "wb");
  if(!strcmp(outfile_desc, "-"))
    fprintf(fp_out, "Optosf output file.  Parameters are: n_feats_use \
%d, osf_init %f, osf_initstep %f, osf_stepthr %f, fvs_file %s, \
classes_file %s, n_fvs_use_as_protos_set %d, \
n_fvs_use_as_tuning_set %d\n", n_feats_use, osf_init, osf_initstep,
      osf_stepthr, fvs_file, classes_file, n_fvs_use_as_protos_set,
      n_fvs_use_as_tuning_set);
  else
    fprintf(fp_out, "%s\n", outfile_desc);
  fflush(fp_out);

  /* Optimize osf by a very simple method.  Start off taking large
  steps, and if the error fails to decrease then reverse direction
  and halve the step size.  Stop when the step size becomes small.
  Store previously computed (osf,error) pairs for lookup, to prevent
  wasting cycles computing the error function more than once for the
  same input value. */
  optosf_pnn(n_feats_use, n_fvs_use_as_protos_set,
    n_fvs_use_as_tuning_set, fvs, classes, n_cls, osf_init, &acerror_prev,
    &classerror);
  sprintf(str, "osf: %f; activ. error: %f; classif. error: %f\n",
    osf_init, acerror_prev, classerror);
  out_prog(str);
  for(osf = osf_init + (osf_step = osf_initstep); ; osf += osf_step) {
    optosf_pnn(n_feats_use, n_fvs_use_as_protos_set,
      n_fvs_use_as_tuning_set, fvs, classes, n_cls, osf, &acerror,
      &classerror);
    sprintf(str, "osf: %f; activ. error: %f; classif. error: %f\n",
      osf, acerror, classerror);
    out_prog(str);
    if(acerror >= acerror_prev) {
      if(fabs((double)osf_step) <= osf_stepthr)
	break;
      osf_step /= -2;
    }
    osf_prev = osf;
    acerror_prev = acerror;
    classerror_prev = classerror;
  }

  /* Optimal osf. */
  sprintf(str, "Optimization finished; producing:\n  osf: %f; \
activ. error: %f; classif. error: %f\n", osf_prev, acerror_prev,
    classerror_prev);
  out_prog(str);

  exit(0);
}

/********************************************************************/

/* Computes PNN activation error rate and classification error rate
when a set of (KL) feature vectors is classified.

Inputs:

  n_feats_use: how many features

  n_fvs_use_as_protos_set: how many feature vectors in prototypes set.

  n_fvs_use_as_tuning_set: how many feature vectors in "tuning" set,
    i.e., set that is classified to produce activation error rate.
    The proto and tuning sets start at same vector.  Each time
    a tuning vector is classified, it is left out of the prototypes
    set.

  fvs: the feature vectors, of which the first n_fvs_use_as_protos_set
    are used as the protos, and the first n_fvs_use_as_tuning_set are
    used as the tuning set.

  classes: the classes of the feature vectors

  fac: smoothing factor, really osf (overall smoothing factor) here

Outputs:

  acerror: activation error rate, i.e., average, over tuning set,
    of squared difference between 1 and the normalized activation
    of the actual class.

  classerror: classification error rate, i.e., fraction of the
    tuning set that is misclassified.
*/

void optosf_pnn(int n_feats_use, int n_fvs_use_as_protos_set,
            int n_fvs_use_as_tuning_set, float *fvs, unsigned char *classes,
            int n_cls, float fac, float *acerror, float *classerror)
{
  unsigned char *tu_classes_p, *pr_classes_p, hypclass;
  int itu, ipr, i, nwrong;
  float accm, *tu_p, *pr_p, *p, *pe, *q, *ac, acsum, maxac,
    anac, a, sd;

  accm = 0.;
  nwrong = 0;
  malloc_flt(&ac, n_cls, "optosf_pnn ac");
  for(itu = 0, tu_p = fvs, tu_classes_p = classes;
    itu < n_fvs_use_as_tuning_set; itu++, tu_p += n_feats_use,
    tu_classes_p++) {

    if(!(itu % 10))
      printf("itu %d\n", itu);

    memset(ac, 0, n_cls * sizeof(float));
    for(ipr = 0, pr_p = fvs, pr_classes_p = classes;
      ipr < n_fvs_use_as_protos_set; ipr++, pr_p += n_feats_use,
      pr_classes_p++) {
      if(itu == ipr)
	continue;
      for(sd = 0., pe = (p = tu_p) + n_feats_use, q = pr_p;
        p < pe; p++, q++) {
	a = *p - *q;
	sd += a * a;
      }
      ac[*pr_classes_p] += exp((double)(-fac * sd));
    }
    for(acsum = maxac = ac[0], hypclass = 0, i = 1; i < n_cls;
      i++) {
      acsum += (anac = ac[i]);
      if(anac > maxac) {
	maxac = anac;
	hypclass = i;
      }
    }
    if(acsum > 0.) {
      a = 1. - ac[*tu_classes_p] / acsum;
      accm += a * a;
    }
    else
      accm += 1.;
    if(hypclass != *tu_classes_p)
      nwrong++;
  }
  *acerror = accm / n_fvs_use_as_tuning_set;
  *classerror = (float)nwrong / n_fvs_use_as_tuning_set;
  free(ac);
}

/********************************************************************/

/* Reads an optosf parms file. */

void optosf_read_parms(char parmsfile[], int *n_feats_use, float *osf_init,
            float *osf_initstep, float *osf_stepthr, int *tablesize,
            int *verbose_int, char fvs_file[], char classes_file[],
            int *n_fvs_use_as_protos_set, int *n_fvs_use_as_tuning_set,
            char outfile[], char outfile_desc[])
{
  FILE *fp;
  char str[1000], *p, name_str[50], val_str[1000];

  fp = fopen_ch(parmsfile, "rb");
  while(fgets(str, 1000, fp)) {
    if((p = strchr(str, '#')))
      *p = 0;
    if(sscanf(str, "%s %s", name_str, val_str) < 2)
      continue;

    if(!strcmp(name_str, "n_feats_use")) {
      *n_feats_use = atoi(val_str);
      setflags.n_feats_use = 1;
    }
    else if(!strcmp(name_str, "osf_init")) {
      *osf_init = atof(val_str);
      setflags.osf_init = 1;
    }
    else if(!strcmp(name_str, "osf_initstep")) {
      *osf_initstep = atof(val_str);
      setflags.osf_initstep = 1;
    }
    else if(!strcmp(name_str, "osf_stepthr")) {
      *osf_stepthr = atof(val_str);
      setflags.osf_stepthr = 1;
    }
    else if(!strcmp(name_str, "tablesize")) {
      *tablesize = atoi(val_str);
      setflags.tablesize = 1;
    }
    else if(!strcmp(name_str, "verbose")) {
      if(!strcmp(val_str, "y"))
	*verbose_int = 1;
      else if(!strcmp(val_str, "n"))
	*verbose_int = 0;
      else
	fatalerr("optosf_read_parms (file optosf.c)", "verbose is \
neither y nor n", NULL);
      setflags.verbose = 1;
    }
    else if(!strcmp(name_str, "fvs_file")) {
      strcpy(fvs_file, val_str);
      setflags.fvs_file = 1;
    }
    else if(!strcmp(name_str, "classes_file")) {
      strcpy(classes_file, val_str);
      setflags.classes_file = 1;
    }
    else if(!strcmp(name_str, "n_fvs_use_as_protos_set")) {
      *n_fvs_use_as_protos_set = atoi(val_str);
      setflags.n_fvs_use_as_protos_set = 1;
    }
    else if(!strcmp(name_str, "n_fvs_use_as_tuning_set")) {
      *n_fvs_use_as_tuning_set = atoi(val_str);
      setflags.n_fvs_use_as_tuning_set = 1;
    }
    else if(!strcmp(name_str, "outfile")) {
      strcpy(outfile, val_str);
      setflags.outfile = 1;
    }
    else if(!strcmp(name_str, "outfile_desc")) {
      strcpy(outfile_desc, val_str);
      setflags.outfile_desc = 1;
    }

    else
      fatalerr("optosf_read_parms (file optosf.c)",
        "illegal parm name", name_str);
  }
}

/********************************************************************/

/* Checks that every parm has been set. */

void optosf_check_parms_allset()
{
  if(!setflags.n_feats_use)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
n_feats_use was never set", NULL);
  if(!setflags.osf_init)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
osf_init was never set", NULL);
  if(!setflags.osf_initstep)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
osf_initstep was never set", NULL);
  if(!setflags.osf_stepthr)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
osf_stepthr was never set", NULL);
  if(!setflags.tablesize)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
tablesize was never set", NULL);
  if(!setflags.verbose)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
verbose was never set", NULL);
  if(!setflags.fvs_file)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
fvs_file was never set", NULL);
  if(!setflags.classes_file)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
classes_file was never set", NULL);
  if(!setflags.n_fvs_use_as_protos_set)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
n_fvs_use_as_protos_set was never set", NULL);
  if(!setflags.n_fvs_use_as_tuning_set)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
n_fvs_use_as_tuning_set was never set", NULL);
  if(!setflags.outfile)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
outfile was never set", NULL);
  if(!setflags.outfile_desc)
    fatalerr("optosf_check_parms_allset (file optosf.c)", "parm \
outfile_desc was never set", NULL);
}

/********************************************************************/

void out_prog(char str[])
{
  fprintf(fp_out, "%s", str);
  fflush(fp_out);
  if(verbose_int)
    printf("%s", str);
}

/********************************************************************/
