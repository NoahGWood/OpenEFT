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

      FILE:     OPTRWS.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.

#cat: optrws - Optimizes the regional weights that are then applied
#cat:          to the eigen vectors.

Optimizes the regional weights.  These weights form a hxw array,
with each weight being associated with one 2x2-vector block of
orientation vectors from the (2*w)x(2*h)-vector orientation array.  The use
of the weights is that, in effect (up to an approximation), the
appropriate elements of orientation arrays get multiplied by the
weights before the computation of Euclidean distances.

Optimization is done in the sense of minimizing the activation error
rate that results when a set of fingerprints is classified, by a
Probabilistic Neural Net (PNN) that uses the same set of fingerprints
as prototypes, with leave-one-out, i.e. the particular fingerprint
being classified is omitted from the prototypes set each time.  The
activation error rate is a function of the regional weights.  Each
time a particular set of regional weights is to be tried, the K-L
feature vectors first transformed to make temporary new feature
vectors that incorporate these regional weights and then the PNN is
run using the temporary feature vectors.  The activation error rate
used is the average, over the tuning set, of the squared difference
between 1 and the normalized PNN activation of the actual class.  The
optimization method used is a very simple form of gradient descent.

At the "basepoints" along the optimization, the program records
the following, as files in outfiles_dir:
  The basepoints, as "matrix" file (dimensions h x w) bspt_0.bin,
    bspt_1.bin, etc. or bspt_0.asc, bspt_1.asc, etc. (suffix indicates
    binary or ascii, which is decided by the ascii_outfiles parm).
  The estimated gradients at the basepoints, as "matrix" files (also
    dimensions h x w) egrad_0.bin, etc. or egrad_0.asc, etc.
  The activation error rates at the basepoints, as text files
    acerr_0.txt, etc.

When this program needs to compute error values at a set of points
near a basepoint, in order to compute the estimated gradient, it can
start several processes, which run simultaneously and each do part of
the work.  If several processors are available, using this feature may
save a considerable amount of time.  To use this, cause your parms
file to set acerror_stepped_points_nprocs (number of processes to use
when estimating gradient) to a value > 1; the value probably should be
<= number of processors available.  If the operating system on your
computer does not have the fork() system call and the execl() function
(e.g., DOS), then optrws.c should be compiled with NO_FORK_AND_EXECL
defined; this can be done by modifying the optrws Makefile so that the
cc (C compiler) command uses -DNO_FORK_AND_EXECL as an additional
argument.  If optrws.c is compiled this way, the part of the code that
uses fork and execl, i.e.  the part that usually is run if
acerror_stepped_points_nprocs > 1, will not be compiled, and a bit of
code will be compiled that causes the program to print an error
message and exit if acerror_stepped_points_nprocs > 1.

*************************************************************************/

/* <fixup.h> must be near the top of the list of includes; otherwise,
   this source will not compile. */
#include <stdio.h>
#include <stdlib.h>
#include <fixup.h>
#include <sys/stat.h>
#include <sys/types.h>
#ifndef __MSYS__
#include <sys/wait.h>
#endif
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <little.h>
#include <usagemcs.h>
#include <table.h>
#include <optrws_r.h>
#include <pca.h>
#include <datafile.h>
#include <ioutil.h>
#include <version.h>


static FILE *fp_messages;
static int verbose_int;

static struct {
  char n_feats_use, n_klfvs_use, irw_init, irw_initstep, irw_stepthr,
    grad_est_stepsize, n_linesearches, linesearch_initstep,
    linesearch_stepthr, tablesize, acerror_stepped_points_nprocs,
    verbose, klfvs_file, classes_file, eigvecs_file,
    outfiles_dir, ascii_outfiles;
} setflags;

void optrws_read_parms(char [], int *, int *, float *, float *, float *,
            float *, int *, float *, float *, int *, int *, int *,
            char [], char [], char [], char [], int *);
void optrws_check_parms_allset(void);
void message_prog(char []);

int main(int argc, char *argv[])
{
  FILE *fp;
  char str[400], *prsfile, *datadir, *desc, klfvs_file[200],
    klfvs_file_tf[200], classes_file[200], classes_file_tf[200],
    eigvecs_file[200], eigvecs_file_tf[200], outfiles_dir[200],
    outfiles_dir_tf[200], optrwsgw_path[200], rws_bspt_file[200],
    rws_bspt_file_full_nbytes_str[6], n_feats_use_str[4],
    n_klfvs_use_str[7], seg_start_str[4], seg_end_str[4],
    grad_est_stepsize_str[40], temp_outfile[200];
  unsigned char *classes;
  int n_feats_use, n_klfvs_use, n_linesearches,
    acerror_stepped_points_nprocs, i, ibspt, ascii_outfiles_int,
    tablesize, optrws_pid = 0, *cproc_pids, pid, base_seg_size,
    n_larger_segs, iproc, seg_size, seg_start, seg_end, nleft, ret,
    fd, rws_bspt_file_full_nbytes, temp_outfile_full_nbytes;
  float *klfvs, *eigvecs, irw_init, irw_initstep, irw_stepthr,
    grad_est_stepsize, linesearch_initstep, linesearch_stepthr,
    egrad_slen, egrad_len, *acerrors_stepped, irw, irw_step,
    irw_prev = 0, acerror, acerror_prev, acerror_bspt, *rws,
    *rws_bspt, *egrad, *dh_uvec, dhdist, linesearch_step, dhdist_prev = 0;
  TABLE table;
  int j, n_feats, evt_sz, w, h, n_cls, rwsz;
  char **lcnptr;

  if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
     getVersion();
     exit(0);
  }

  Usage("<prsfile>");
  prsfile = *++argv; /* required user parms file */
  /* Read parameters, first from default optrws parms file and then
  from user parms file.  Then, check that no parm was left unset. */
  memset(&setflags, 0, sizeof(setflags));

#ifdef __MSYS__
  sprintf(str, "./optrws.prs");
#else
  datadir = get_datadir();
  sprintf(str, "%s/parms/optrws.prs", datadir);
#endif
 
  optrws_read_parms(str, &n_feats_use, &n_klfvs_use, &irw_init,
    &irw_initstep, &irw_stepthr, &grad_est_stepsize, &n_linesearches,
    &linesearch_initstep, &linesearch_stepthr, &tablesize,
    &acerror_stepped_points_nprocs, &verbose_int, klfvs_file,
    classes_file, eigvecs_file, outfiles_dir, &ascii_outfiles_int);
  optrws_read_parms(prsfile, &n_feats_use, &n_klfvs_use, &irw_init,
    &irw_initstep, &irw_stepthr, &grad_est_stepsize, &n_linesearches,
    &linesearch_initstep, &linesearch_stepthr, &tablesize,
    &acerror_stepped_points_nprocs, &verbose_int, klfvs_file,
    classes_file, eigvecs_file, outfiles_dir, &ascii_outfiles_int);
  optrws_check_parms_allset();

  w = ((WIDTH/WS)-2)/2;
  h = ((HEIGHT/WS)-2)/2;
  rwsz = w*h;
  if(!(1 <= acerror_stepped_points_nprocs &&
    acerror_stepped_points_nprocs <= rwsz)) {
    sprintf(str, "acerror_stepped_points_nprocs is %d; must have\n\
1 <= acerror_stepped_points_nprocs <= %d",
      acerror_stepped_points_nprocs, rwsz);
    fatalerr("optrws", str, NULL);
  }

  if(acerror_stepped_points_nprocs > 1) {
#ifdef NO_FORK_AND_EXECL
    fatalerr("optrws", "in this no-fork-and-execl version, \
acerror_stepped_points_nprocs must be 1", NULL);
#endif
    sprintf(optrwsgw_path, "%s/optrwsgw", "");
    optrws_pid = getpid();
  }

  strcpy(outfiles_dir_tf, tilde_filename(outfiles_dir, 0));
  strcpy(klfvs_file_tf, tilde_filename(klfvs_file, 0));
  strcpy(classes_file_tf, tilde_filename(classes_file, 0));
  strcpy(eigvecs_file_tf, tilde_filename(eigvecs_file, 0));

#ifdef __MSYS__
  mkdir(outfiles_dir_tf);
#else
  mkdir(outfiles_dir_tf, 0700);
#endif

  sprintf(str, "%s/messages.txt", outfiles_dir_tf);
  fp_messages = fopen_ch(str, "w");

  /* Read data: K-L feature vectors and their classes, and
  eigenvectors. */
  message_prog("read K-L feature vectors, classes, and \
eigenvectors\n");
  matrix_read_submatrix(klfvs_file_tf, 0, n_klfvs_use - 1, 0,
    n_feats_use - 1, &desc, &klfvs);
  free(desc);
  classes_read_subvector_ind(classes_file_tf, 0, n_klfvs_use - 1, &desc,
    &classes, &n_cls, &lcnptr);
  free(desc);
  free_dbl_char(lcnptr, n_cls);
  matrix_read_dims(eigvecs_file_tf, &n_feats, &evt_sz);
  if(8*rwsz != evt_sz)
    fatalerr("optrws","8*rwsz != evt_sz","sizes are incompatible");
  matrix_read_submatrix(eigvecs_file_tf, 0, n_feats_use - 1, 0, evt_sz-1,
    &desc, &eigvecs);
  free(desc);

  /* A simple linearly searched table, which will be used, when
  optimizing irw and during line searches in the main optimization,
  to look up previous results and thereby avoid some redoing of
  error computations. */
  table_init(&table, tablesize);

  /* Optimize irw (initial regional weight), an initial value to
  which to set all the regional weights at the start (later) of their
  optimization as separate weights.  This is done by using a single
  factor (squared) for the pnn and optimizing this factor: that is
  approximately equivalent to using the factor for all weights. */
  message_prog("optimize irw (initial value for all \
regional weights)\n");
  acerror_prev = optrws_pnn_acerror(n_feats_use, n_klfvs_use, klfvs,
    classes, irw_init * irw_init, n_cls);
  sprintf(str, "irw %f, acerror %f\n", irw_init, acerror_prev);
  message_prog(str);
  table_store(&table, irw_init, acerror_prev);
  for(irw = irw_init + (irw_step = irw_initstep); ; irw += irw_step) {
    if(!table_lookup(&table, irw, &acerror)) {
      acerror = optrws_pnn_acerror(n_feats_use, n_klfvs_use, klfvs,
        classes, irw * irw, n_cls);
      table_store(&table, irw, acerror);
    }
    sprintf(str, "irw %f, acerror %f\n", irw, acerror);
    message_prog(str);
    if(acerror >= acerror_prev) {
      if(fabs((double)irw_step) <= irw_stepthr)
	break;
      irw_step /= -2;
    }
    irw_prev = irw;
    acerror_prev = acerror;
  }
  table_clear(&table);

  /* The main part of the optimization of the regional weights.  Uses
  a simple form of gradient descent, which appears to be sufficient
  for this task, although it may be compute-intensive. */
  /* Duplicate the best irw, which was just found, into all weights,
  forming the 0'th "basepoint" for the subsequent optimization of
  the weights.  Write 0'th basepoint. */
  malloc_flt(&rws_bspt, rwsz, "optrws rws_bspt");
  for(i = 0; i < rwsz; i++)
    rws_bspt[i] = irw_prev;
  sprintf(rws_bspt_file, "%s/bspt_0.%s", outfiles_dir_tf,
    ascii_outfiles_int ? "asc" : "bin");
  rws_bspt_file_full_nbytes = matrix_write(rws_bspt_file, "",
    ascii_outfiles_int, h, w, rws_bspt);
  sprintf(rws_bspt_file_full_nbytes_str, "%d",
    rws_bspt_file_full_nbytes);

  /* Compute and write activation error rate at 0'th basepoint. */
  acerror_bspt = rws_to_acerror(rws_bspt, w, h, eigvecs, evt_sz, n_feats_use,
    n_klfvs_use, klfvs, classes, n_cls);
  sprintf(str, "acerror_bspt %f\n", acerror_bspt);
  message_prog(str);
  sprintf(str, "%s/acerr_0.txt", outfiles_dir_tf);
  fp = fopen_ch(str, "w");
  fprintf(fp, "%f\n", acerror_bspt);
  fclose(fp);

  /* Do n_linesearches iterations of {estimate gradient; line search
  along resulting downhill-pointing line}. */

  malloc_flt(&acerrors_stepped, rwsz, "optrws acerrors_stepped");
  malloc_flt(&rws, rwsz, "optrws rws");
  malloc_flt(&egrad, rwsz, "optrws egrad");
  malloc_flt(&dh_uvec, rwsz, "optrws dh_uvec");
  for(ibspt = 0; ibspt < n_linesearches; ibspt++) {
    sprintf(str, "ibspt %d\n", ibspt);
    message_prog(str);

    /* Compute error values at points stepped to from the basepoint
    along the coordinate axes.  These will be used to estimate the
    gradient. */
    message_prog("compute error at stepped-to points\n");
    if(acerror_stepped_points_nprocs == 1)
      /* Use one process, namely this process. */
      for(i = 0; i < rwsz; i++) {
        for(j = 0; j < rwsz; j++)
           rws[j] = rws_bspt[j];
	rws[i] += grad_est_stepsize;
	acerrors_stepped[i] = rws_to_acerror(rws, w, h, eigvecs, evt_sz,
          n_feats_use, n_klfvs_use, klfvs, classes, n_cls);
	sprintf(str, "i = %d; acerrors_stepped[i] = %f\n", i,
          acerrors_stepped[i]);
	message_prog(str);
      }
    else {
#ifndef NO_FORK_AND_EXECL
      /* Use several processes (instances of the optrwsgw program) to
      compute the error values at the stepped-to points.  Divide
      the rwsz points into acerror_stepped_points_nprocs approximately
      equal segments and assign one segment to each process. */

      /* Start processes (child processes). */
      message_prog("start child processes\n");
      base_seg_size = rwsz / acerror_stepped_points_nprocs;
      n_larger_segs = rwsz % acerror_stepped_points_nprocs;
      malloc_int(&cproc_pids, rwsz, "optrws cprod_pids");
      for(iproc = seg_start = 0; iproc <
        acerror_stepped_points_nprocs; iproc++, seg_start = seg_end) {
	seg_size = (iproc < n_larger_segs ? base_seg_size + 1 :
          base_seg_size);
	seg_end = seg_start + seg_size;
	ret = fork();
	if(ret) /* Still this process; ret is process id of
                child process */
	  cproc_pids[iproc] = ret;
	else {    /* Child process.  Run an instance of optrwsgw. */
	  sprintf(n_feats_use_str, "%d", n_feats_use);
	  sprintf(n_klfvs_use_str, "%d", n_klfvs_use);
	  sprintf(seg_start_str, "%d", seg_start);
	  sprintf(seg_end_str, "%d", seg_end);
	  sprintf(grad_est_stepsize_str, "%f", grad_est_stepsize);
	  sprintf(temp_outfile, "/tmp/optrwsgw_optrws-pid-%d_%d",
            optrws_pid, iproc);
	  execl(optrwsgw_path, "optrwsgw", n_feats_use_str,
            n_klfvs_use_str, klfvs_file_tf, classes_file_tf,
            eigvecs_file_tf, rws_bspt_file,
            rws_bspt_file_full_nbytes_str, seg_start_str, seg_end_str,
            grad_est_stepsize_str, temp_outfile, (char *)0);
	  /* If control gets here, execl has failed: */
	  perror("execl");
	  exit(1);
	}
      }

      /* Wait for all child processes to finish.  Check pids returned
      by wait against list of child process pids, since wait returns
      can be caused by events other than child process exits. */
      message_prog("wait for all child processes to exit\n");
      for(nleft = acerror_stepped_points_nprocs; nleft;) {
	pid = wait(NULL);
	for(iproc = 0; iproc < acerror_stepped_points_nprocs; iproc++)
	  if(pid == cproc_pids[iproc]) {
	    nleft--;
	    break;
	  }
      }
      free(cproc_pids);

      /* Read (and remove) the temporary output files of the child
      processes, thereby building the complete vector of error values
      at all rwsz stepped-to points.  Before reading any file here,
      make sure it has the expected number of bytes. */
      message_prog("read output files of child processes\n");
      for(iproc = seg_start = 0; iproc <
        acerror_stepped_points_nprocs; iproc++, seg_start = seg_end) {
	seg_size = (iproc < n_larger_segs ? base_seg_size + 1 :
          base_seg_size);
	seg_end = seg_start + seg_size;
	sprintf(temp_outfile, "/tmp/optrwsgw_optrws-pid-%d_%d",
          optrws_pid, iproc);
	temp_outfile_full_nbytes = seg_size * sizeof(float);
	while(filesize(temp_outfile) != temp_outfile_full_nbytes)
	  sleep(1);
	fd = open_read_ch(temp_outfile);
	read(fd, (float *)acerrors_stepped + seg_start,
          temp_outfile_full_nbytes);
	close(fd);
	unlink(temp_outfile);
      }
#endif
    }

    /* From error values at stepped-to points, compute estimated
    gradient (secant method) and its length, whence unit-length
    (estimated-)downhill-pointing vector. */
    message_prog("compute estimated gradient, its length, \
whence dh_uvec\n");
    for(i = 0, egrad_slen = 0.; i < rwsz; i++) {
      egrad[i] = (acerrors_stepped[i] - acerror_bspt) /
        grad_est_stepsize;
      egrad_slen += egrad[i] * egrad[i];
    }
    egrad_len = sqrt((double)egrad_slen);
    /* Estimated-downhill-pointing unit-length vector: */
    for(i = 0; i < rwsz; i++)
      dh_uvec[i] = -egrad[i] / egrad_len;

    /* Write estimated gradient. */
    sprintf(str, "%s/egrad_%d.%s", outfiles_dir_tf, ibspt,
      ascii_outfiles_int ? "asc" : "bin");
    matrix_write(str, "", ascii_outfiles_int, h, w, egrad);

    /* Use a very simple line search method, which appears to be
    sufficient for this particular task, to approximately find the
    minimum, or at least, a local minimum, along the downhill-pointing
    line.  Resulting point will be the next basepoint. */
    message_prog("line search:\n");
    acerror_prev = acerror_bspt;
    sprintf(str, "  acerror_prev %f\n", acerror_prev);
    message_prog(str);
    for(dhdist = linesearch_step = linesearch_initstep; ; dhdist +=
      linesearch_step) {
      if(!table_lookup(&table, dhdist, &acerror)) {
	for(i = 0; i < rwsz; i++)
	  rws[i] = rws_bspt[i] + dhdist * dh_uvec[i];
	acerror = rws_to_acerror(rws, w, h, eigvecs, evt_sz, n_feats_use,
          n_klfvs_use, klfvs, classes, n_cls);
	table_store(&table, dhdist, acerror);
      }
      sprintf(str, "  dhdist %f, acerror %f\n", dhdist, acerror);
      message_prog(str);
      if(acerror >= acerror_prev) {
	if(fabs((double)linesearch_step) <= linesearch_stepthr)
	  break;
	linesearch_step /= -2;
      }
      dhdist_prev = dhdist;
      acerror_prev = acerror;
    }
    table_clear(&table);
    for(i = 0; i < rwsz; i++)
      rws_bspt[i] += dhdist_prev * dh_uvec[i];

    /* Write next basepoint. */
    sprintf(rws_bspt_file, "%s/bspt_%d.%s",
      outfiles_dir_tf, ibspt + 1, ascii_outfiles_int ?
      "asc" : "bin");
    rws_bspt_file_full_nbytes = matrix_write(rws_bspt_file, "",
      ascii_outfiles_int, h, w, rws_bspt);
    sprintf(rws_bspt_file_full_nbytes_str, "%d",
      rws_bspt_file_full_nbytes);

    /* Compute and write activation error rate at next basepoint. */
    acerror_bspt = rws_to_acerror(rws_bspt, w, h, eigvecs, evt_sz, n_feats_use,
      n_klfvs_use, klfvs, classes, n_cls);
    sprintf(str, "acerror_bspt for basepoint %d: %f\n",
      ibspt + 1, acerror_bspt);
    message_prog(str);
    sprintf(str, "%s/acerr_%d.txt", outfiles_dir_tf, ibspt + 1);
    fp = fopen_ch(str, "w");
    fprintf(fp, "%f\n", acerror_bspt);
    fclose(fp);
  }
  free(rws_bspt);
  free(rws);
  free(egrad);
  free(dh_uvec);
  free(acerrors_stepped);

  exit(0);
}

/********************************************************************/

/* Reads an optrws parms file. */

void optrws_read_parms(char parmsfile[], int *n_feats_use, int *n_klfvs_use,
            float *irw_init, float *irw_initstep, float *irw_stepthr,
            float *grad_est_stepsize, int *n_linesearches,
            float *linesearch_initstep, float *linesearch_stepthr,
            int *tablesize, int *acerror_stepped_points_nprocs,
            int *verbose_int, char klfvs_file[], char classes_file[],
            char eigvecs_file[], char outfiles_dir[], int *ascii_outfiles_int)
{
  FILE *fp;
  char str[1000], *p, name_str[50], val_str[1000];

  fp = fopen_ch(parmsfile, "r");
  while(fgets(str, 1000, fp)) {
    if((p = strchr(str, '#')))
      *p = 0;
    if(sscanf(str, "%s %s", name_str, val_str) < 2)
      continue;

    if(!strcmp(name_str, "n_feats_use")) {
      *n_feats_use = atoi(val_str);
      setflags.n_feats_use = 1;
    }
    else if(!strcmp(name_str, "n_klfvs_use")) {
      *n_klfvs_use = atoi(val_str);
      setflags.n_klfvs_use = 1;
    }
    else if(!strcmp(name_str, "irw_init")) {
      *irw_init = atof(val_str);
      setflags.irw_init = 1;
    }
    else if(!strcmp(name_str, "irw_initstep")) {
      *irw_initstep = atof(val_str);
      setflags.irw_initstep = 1;
    }
    else if(!strcmp(name_str, "irw_stepthr")) {
      *irw_stepthr = atof(val_str);
      setflags.irw_stepthr = 1;
    }
    else if(!strcmp(name_str, "grad_est_stepsize")) {
      *grad_est_stepsize = atof(val_str);
      setflags.grad_est_stepsize = 1;
    }
    else if(!strcmp(name_str, "n_linesearches")) {
      *n_linesearches = atoi(val_str);
      setflags.n_linesearches = 1;
    }
    else if(!strcmp(name_str, "linesearch_initstep")) {
      *linesearch_initstep = atof(val_str);
      setflags.linesearch_initstep = 1;
    }
    else if(!strcmp(name_str, "linesearch_stepthr")) {
      *linesearch_stepthr = atof(val_str);
      setflags.linesearch_stepthr = 1;
    }
    else if(!strcmp(name_str, "tablesize")) {
      *tablesize = atoi(val_str);
      setflags.tablesize = 1;
    }
    else if(!strcmp(name_str, "acerror_stepped_points_nprocs")) {
      *acerror_stepped_points_nprocs = atoi(val_str);
      setflags.acerror_stepped_points_nprocs = 1;
    }
    else if(!strcmp(name_str, "verbose")) {
      if(!strcmp(val_str, "y"))
	*verbose_int = 1;
      else if(!strcmp(val_str, "n"))
	*verbose_int = 0;
      else
	fatalerr("optrws_read_parms (file optrws.c)", "parm verbose \
must be y or n", NULL);
      setflags.verbose = 1;
    }
    else if(!strcmp(name_str, "klfvs_file")) {
      strcpy(klfvs_file, val_str);
      setflags.klfvs_file = 1;
    }
    else if(!strcmp(name_str, "classes_file")) {
      strcpy(classes_file, val_str);
      setflags.classes_file = 1;
    }
    else if(!strcmp(name_str, "eigvecs_file")) {
      strcpy(eigvecs_file, val_str);
      setflags.eigvecs_file = 1;
    }
    else if(!strcmp(name_str, "outfiles_dir")) {
      strcpy(outfiles_dir, val_str);
      setflags.outfiles_dir = 1;
    }
    else if(!strcmp(name_str, "ascii_outfiles")) {
      if(!strcmp(val_str, "y"))
	*ascii_outfiles_int = 1;
      else if(!strcmp(val_str, "n"))
	*ascii_outfiles_int = 0;
      else
	fatalerr("optrws_read_parms (file optrws.c)", "parm \
ascii_outfiles must be y or n", NULL);
      setflags.ascii_outfiles = 1;
    }

    else
      fatalerr("optrws_read_parms (file optrws.c)",
        "illegal parm name", name_str);
  }
}

/********************************************************************/

/* Checks that all parms are set. */

void optrws_check_parms_allset()
{
  if(!setflags.n_feats_use)
    fatalerr("optrws_check_parms_allset (file optrws.c)", "parm \
n_feats_use was never set", NULL);
  if(!setflags.n_klfvs_use)
    fatalerr("optrws_check_parms_allset (file optrws.c)", "parm \
n_klfvs_use was never set", NULL);
  if(!setflags.irw_init)
    fatalerr("optrws_check_parms_allset (file optrws.c)", "parm \
irw_init was never set", NULL);
  if(!setflags.irw_initstep)
    fatalerr("optrws_check_parms_allset (file optrws.c)", "parm \
irw_initstep was never set", NULL);
  if(!setflags.irw_stepthr)
    fatalerr("optrws_check_parms_allset (file optrws.c)", "parm \
irw_stepthr was never set", NULL);
  if(!setflags.grad_est_stepsize)
    fatalerr("optrws_check_parms_allset (file optrws.c)", "parm \
grad_est_stepsize was never set", NULL);
  if(!setflags.n_linesearches)
    fatalerr("optrws_check_parms_allset (file optrws.c)", "parm \
n_linesearches was never set", NULL);
  if(!setflags.linesearch_initstep)
    fatalerr("optrws_check_parms_allset (file optrws.c)", "parm \
linesearch_initstep was never set", NULL);
  if(!setflags.linesearch_stepthr)
    fatalerr("optrws_check_parms_allset (file optrws.c)", "parm \
linesearch_stepthr was never set", NULL);
  if(!setflags.tablesize)
    fatalerr("optrws_check_parms_allset (file optrws.c)", "parm \
tablesize was never set", NULL);
  if(!setflags.acerror_stepped_points_nprocs)
    fatalerr("optrws_check_parms_allset (file optrws.c)", "parm \
acerror_stepped_points_nprocs was never set", NULL);
  if(!setflags.verbose)
    fatalerr("optrws_check_parms_allset (file optrws.c)", "parm \
verbose was never set", NULL);
  if(!setflags.klfvs_file)
    fatalerr("optrws_check_parms_allset (file optrws.c)", "parm \
klfvs_file was never set", NULL);
  if(!setflags.classes_file)
    fatalerr("optrws_check_parms_allset (file optrws.c)", "parm \
classes_file was never set", NULL);
  if(!setflags.eigvecs_file)
    fatalerr("optrws_check_parms_allset (file optrws.c)", "parm \
eigvecs_file was never set", NULL);
  if(!setflags.outfiles_dir)
    fatalerr("optrws_check_parms_allset (file optrws.c)", "parm \
outfiles_dir was never set", NULL);
  if(!setflags.ascii_outfiles)
    fatalerr("optrws_check_parms_allset (file optrws.c)", "parm \
ascii_outfiles was never set", NULL);
}

/********************************************************************/

/* Writes message to the messages file, and if verbose also writes it
to the standard output. */

void message_prog(char message[])
{
  fprintf(fp_messages, "%s", message);
  fflush(fp_messages);
  if(verbose_int)
    printf("%s", message);
}

/********************************************************************/
