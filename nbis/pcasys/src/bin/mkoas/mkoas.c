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

      FILE:     MKOAS.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.
      UPDATED:  03/25/2011 by Kenenth Ko

#cat: mkoas - Makes an orientation array for a set of fingerprint
#cat:         images, writing them as a "matrix" file, with each oa
#cat:         being one row of the matrix.

*************************************************************************/

#include <stdio.h>
#include <usagemcs.h>
#include <pca.h>
#include <little.h>
#include <imgdec.h>
#include <datafile.h>
#include <memalloc.h>
#include <version.h>

int debug = 0;

int main(int argc, char *argv[])
{
  FILE *fp_proc_images_list, *fp_oas;
  char oas_file[200], *prsfile, proc_rasterfile[200];
  static char **pixelrors;
  unsigned char *origras;
  static unsigned char **segras, **ehras, **segras_fg;
  int n_oas, ascii_oas, w, h, corepixel_x, corepixel_y, iproc,
    update_freq, clobber_oas_file;
  static float **avrors_x, **avrors_y, **reg_avrors_x, **reg_avrors_y,
    *oa, *poa;
  int sw, sh, aw, ah, raw, rah, sfgw, sfgh;
  int i, j, f;
  int ret, lossy_flag;
  char class;

  /* Parms-structures and single parms: */
  SGMNT_PRS sgmnt_prs;
  ENHNC_PRS enhnc_prs;
  int rors_slit_range_thresh;
  float r92a_discard_thresh;
  RGAR_PRS rgar_prs;

  if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
     getVersion();
     exit(0);
  }

  Usage("<prsfile>"); /* user is required to supply a parms file */
  prsfile = argv[1];
  mkoas_init(tilde_filename(prsfile, 0), &sgmnt_prs, &enhnc_prs,
    &rors_slit_range_thresh, &r92a_discard_thresh, &rgar_prs,
    &ascii_oas, &update_freq, &clobber_oas_file, &fp_proc_images_list,
    &n_oas, oas_file);

  if(!clobber_oas_file && exists(tilde_filename(oas_file, 0)))
     fatalerr("mkoas", "output file already exists", oas_file);

  sw = WIDTH;
  sh = HEIGHT;
  f = 1;

  for(iproc = 1; fscanf(fp_proc_images_list, "%s", proc_rasterfile)
    == 1; iproc++) {
  
    class = (char)0;
   
    if(update_freq && !((iproc - 1) % update_freq))
      printf("mkoas, %d/%d\n", iproc, n_oas);

    /* Read the fingerprint raster */
    if((ret = read_and_decode_pcasys(tilde_filename(proc_rasterfile, 0),
               &origras, &w, &h, &lossy_flag, &class))){
      fclose(fp_proc_images_list);
      if(f == 0)
        fclose(fp_oas);
      exit(ret);
    }

    /* Segment out a smaller, possibly rotated raster containing
    the foreground */
    sgmnt(origras, w, h, &sgmnt_prs, &segras, sw, sh, &segras_fg, &sfgw, &sfgh);
    free(origras);

    /* Enhance (filter) the segmented raster */
    enhnc(segras, &enhnc_prs, &ehras, sw, sh);
    free_dbl_char((char **)segras, sh);

    /* Enhance (filter) the segmented raster */
    /* Extract pixelwise ridge-orientations, and average them to
    produce a 28x30-vector grid of local orientations */
    rors(ehras, sw, sh, rors_slit_range_thresh, &pixelrors, &avrors_x,
         &avrors_y, &aw, &ah);
    free_dbl_char((char **)ehras, sh);

    /* Convert orientation grid to angles, and send them to r92
    registration program, which finds the "core" */
    r92a(avrors_x, avrors_y, aw, ah, r92a_discard_thresh, &corepixel_x,
      &corepixel_y);
    free_dbl_flt(avrors_x, ah);
    free_dbl_flt(avrors_y, ah);

    /* Compute, from the pixelwise-orientations made earlier, a new,
    "registered" 28x30-vector array of average orientations, by using
    averaging squares that are shifted according to the r92 result */
    rgar(pixelrors, sw, sh, corepixel_x, corepixel_y, &rgar_prs,
      &reg_avrors_x, &reg_avrors_y, &raw, &rah);

    /* Write the orientation array as the next row of the output
    "matrix" file. */
    if(f) {
      /* (make desc -- for now, leave empty:) */
      matrix_writerow_init(tilde_filename(oas_file, 0), "" /* description */,
        ascii_oas, n_oas, 2*raw*rah, &fp_oas);
      f = 0;
    }
    malloc_flt(&oa, 2*raw*rah, "mkoas oa");
    poa = oa;
    for(j = 0; j < rah; j++)
      for(i = 0; i < raw; i++)
        *poa++ = reg_avrors_x[j][i];
    for(j = 0; j < rah; j++)
      for(i = 0; i < raw; i++)
        *poa++ = reg_avrors_y[j][i];

    matrix_writerow(fp_oas, ascii_oas, 2*raw*rah, oa);
    free(oa);
    free_dbl_flt(reg_avrors_x, rah);
    free_dbl_flt(reg_avrors_y, rah);
  }
  fclose(fp_proc_images_list);
  fclose(fp_oas);

  exit(0);
}
