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

      FILE:     PCASYS.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.
      UPDATED:  03/31/2011 by Kenenth Ko

#cat: pcasys - Main program of fingerprint classification system demo.
#cat:          pcasysgy (graphic demo) and pcasysgn (non-graphic demo).

*************************************************************************/

#include <stdio.h>
#include <pca.h>
#include <usagemcs.h>
#include <little.h>
#include <jpegl.h>
#include <jpegb.h>
#include <wsq.h>
#include <nistcom.h>
#include <memalloc.h>
#include <version.h>
#include <string.h>
int debug = 0;

int main(int argc, char *argv[])
{
  FILE *fp_demo_images_list, *fp_out;
  char *prsfile, demo_rasterfile[200];
  static char **pixelrors;
  unsigned char *origras, actual_class, nn_hyp_class, hyp_class;
  static unsigned char **segras, **ehras, **segras_fg;
  int w, h, corepixel_x, corepixel_y, ndemo, nwrong, *confuse,
    found_conup, idemo, nout = 0, i;
  float *featvec, nn_confidence, confidence, *normacs;
  static float **avrors_x, **avrors_y, **reg_avrors_x, **reg_avrors_y,
               **avrors2_x, **avrors2_y;
  int sw, sh, aw, ah, aw2, ah2, raw, rah;
  int sfgw, sfgh;
  int tfw = 0, tfh = 0;
  char *cls_str = (char *)NULL;
  int ret, lossy_flag;
  char class, class_in;

  /***** Parameters-structures and single parameters: *****/
  SGMNT_PRS sgmnt_prs;
  ENHNC_PRS enhnc_prs;
  int rors_slit_range_thresh;
  float r92a_discard_thresh;
  RGAR_PRS rgar_prs;
  int trnsfrm_nrows_use;
  int pnn_mlp;
  PNN_PRS pnn_prs;
  MLP_PARAM mlp_prs;
  PSEUDO_PRS pseudo_prs;
  float combine_clash_confidence;
  int n_skip;

  /***** Data: *****/
  float *protos_fvs, *tranmat;
  unsigned char *protos_classes;

  if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
    getVersion();
    exit(0);
  }


  /* Usage is: pcasysgy [prsfile] or pcasysgn [prsfile], i.e.
  takes an optional user parameters file. */
  
  if(!(argc == 1 || argc == 2))
    usage("[prsfile]");
  if(argc == 1)
    prsfile = (char *)NULL;
  else
    prsfile = tilde_filename(argv[1], 0);

  /* Call initialization routine.  It reads parameters and data,
  and fopens demo images list for reading and outfile for writing. */
  pcasys_init(prsfile, &sgmnt_prs, &enhnc_prs, &rors_slit_range_thresh,
    &r92a_discard_thresh, &rgar_prs, &trnsfrm_nrows_use, &pnn_mlp, &pnn_prs,
    &mlp_prs, &pseudo_prs, &combine_clash_confidence, &protos_fvs,
    &protos_classes, &tranmat, &fp_demo_images_list, &fp_out);
  featvec = (float *)malloc_ch(trnsfrm_nrows_use * sizeof(float));
  nwrong = 0;                       /* none classified wrongly yet */
  if(pnn_mlp == PNN_CLSFR) {
    nout = pnn_prs.nclasses;
    calloc_flt(&normacs, nout, "pcasys normacs");
    tfw = pnn_prs.trnsfrm_cls;
    tfh = pnn_prs.trnsfrm_rws;
    cls_str = pnn_prs.cls_str;
  }
  else if(pnn_mlp == MLP_CLSFR) {
    nout = mlp_prs.nouts;
    tfw = mlp_prs.trnsfrm_cls;
    tfh = mlp_prs.trnsfrm_rws;
    cls_str = mlp_prs.cls_str;
  }
  calloc_int(&confuse, nout * nout, "pcasys confuse");

  sw = WIDTH;
  sh = HEIGHT;

  /* Run the classifier on each of the listed demo fingerprints */
  n_skip = 0;
  idemo = 1;

  while(fgets(demo_rasterfile, sizeof demo_rasterfile, fp_demo_images_list) != NULL)
  {
    if (demo_rasterfile[0] != '\n')
    {
      idemo++;   
      class = (char)0;

      /* Remove the tailing new line character and replace
      it to end of string character. Also, acquire the finger 
      class if appropriate. */
      for(i = 0; i < sizeof(demo_rasterfile); i++)
      {
        if (demo_rasterfile[i] == ' ')
        {
          demo_rasterfile[i] = '\0';
          class = demo_rasterfile[++i];
          break;
        }
        if (demo_rasterfile[i] == '\n')
        { 
          demo_rasterfile[i] = '\0';
          break;
        }
      }

      if(isverbose())
        printf("%s:\n", lastcomp(demo_rasterfile));

      /* Read the fingerprint raster to be classified, and find out
      from its header what its actual class is (to be used later for
      scoring) */

      char *msys_fname = tilde_filename(demo_rasterfile, 2);
      ret = readfing(msys_fname, &origras, &w, &h,
        &lossy_flag, &class, cls_str, &actual_class, fp_out);

      if(ret != 0) {
         printf("SKIPPED\n\n");
         fprintf(fp_out, "SKIPPED\n");
         n_skip++;
         continue;
      }

      /* Segment out a smaller, possibly rotated raster containing
      the foreground */
      sgmnt(origras, w, h, &sgmnt_prs, &segras, sw, sh, &segras_fg, &sfgw, &sfgh);
      free(origras);

      /* Enhance (filter) the segmented raster */
      enhnc(segras, &enhnc_prs, &ehras, sw, sh);
      free_dbl_uchar(segras, sh);

      /* Extract pixelwise ridge-orientations, and average them to
      produce a 28x30-vector grid of local orientations */
      rors(ehras, sw, sh, rors_slit_range_thresh, &pixelrors, &avrors_x, &avrors_y, &aw, &ah);
      free_dbl_uchar(ehras, sh);

      /* Convert orientation grid to angles, and send them to r92
      registration program, which finds the "core" */
      r92a(avrors_x, avrors_y, aw, ah, r92a_discard_thresh, &corepixel_x,
        &corepixel_y);
      free_dbl_flt(avrors_x, ah);
      free_dbl_flt(avrors_y, ah);

      /* Compute, from the pixelwise-orientations made earlier, a new,
      "registered" 28x30-vector grid of average orientations, by using
      averaging squares that are shifted according to the r92 result */
      rgar(pixelrors, sw, sh, corepixel_x, corepixel_y, &rgar_prs,
        &reg_avrors_x, &reg_avrors_y, &raw, &rah);

      /* Apply a linear transform that both (1) applies unequal weights
      to the regions of the registered orientation array and (2) reduces
      the dimensionality. */
      trnsfrm(reg_avrors_x, reg_avrors_y, raw, rah, trnsfrm_nrows_use, tranmat,
        tfw, tfh, featvec);
      free_dbl_flt(reg_avrors_x, rah);
      free_dbl_flt(reg_avrors_y, rah);

      /* Run a Probabilistic Neural Net (PNN) classifier on the
      low-dimensional feature vector, producing a hypothetical class
      for the demo print and a classification confidence. */
      if(pnn_mlp == PNN_CLSFR)
         pnn(featvec, &pnn_prs, protos_fvs, protos_classes, normacs,
             &nn_hyp_class, &nn_confidence);
      /* Use a Multi-Layer Perceptron Neural Network (MLP) */
      else if(pnn_mlp == MLP_CLSFR)
         mlp_single(mlp_prs, featvec, (char *)&nn_hyp_class, &nn_confidence,
                    acsmaps_code_to_fn2(mlp_prs.acfunc_hids),
                    acsmaps_code_to_fn2(mlp_prs.acfunc_outs));

      /* Take the pixelwise orientations made earlier and average them
      (without registration) to make a 58x62-orientation grid, for
      use by the pseudoridge analyzer. */
      ar2(pixelrors, sw, sh, &avrors2_x, &avrors2_y, &aw2, &ah2);
      free_dbl_char(pixelrors, sh);

      /* Follow pseudoridges through the print, looking for a
      concave-upward shape. */
      found_conup = pseudo(segras_fg, sfgw, sfgh, avrors2_x, avrors2_y,
                           aw2, ah2, &pseudo_prs);
      free_dbl_uchar(segras_fg, sfgh);
      free_dbl_flt(avrors2_x, ah2);
      free_dbl_flt(avrors2_y, ah2);

      /* Use a rule to combine the outputs of the NN and of the
      pseudoridge tracer. */
      combine(nn_hyp_class, nn_confidence, found_conup,
        combine_clash_confidence, &hyp_class, &confidence, cls_str);

      /* Update the scoring: number wrong and confusion matrix */
      if(actual_class != hyp_class)
        nwrong++;
      confuse[actual_class*nout+hyp_class]++;

      /* Write results for this demo print */
      results(actual_class, nn_hyp_class, nn_confidence,
        found_conup, hyp_class, confidence, fp_out, demo_rasterfile, cls_str,
        nout);
    }
  }
  
  if(pnn_mlp == PNN_CLSFR)
    free(normacs);

  /* (Finished going through demo prints.) */
  fclose(fp_demo_images_list);
  ndemo = idemo - 1 - n_skip;
  
  /* Compute and write summary information: error rate and
  confusion matrices. */
  summary(nwrong, ndemo, confuse, fp_out, nout, cls_str);
  free(confuse);

  exit(0);
}
