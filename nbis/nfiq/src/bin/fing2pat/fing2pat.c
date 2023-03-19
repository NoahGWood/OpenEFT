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

/***********************************************************************
      PACKAGE: NIST Fingerprint Image Quality (NFIQ)

      FILE:    FING2PAT.C

      ALGORITHM:
               Elham Tabassi
               Charles L. Wilson
               Criag I. Watson

      IMPLEMENTATION:
               Michael D. Garris

      DATE:    09/13/2004
      UPDATED: 11/27/2006 by KKO
      UPDATED: 09/30/2008 by Kenenth Ko - add version option.

#cat: fing2pat - Takes a list of grayscale fingerprint images, and for
#cat:            each image, computes a feature vector for use by the
#cat:            NFIQ algorithm for MLP training.  If the optional file
#cat:            is passed using the "-z" flag, then the output feature
#cat:            vectors are ZNormalized.

***********************************************************************/
#include <stdio.h>
#include <nfiq.h>
#include <imgtype.h>
#include <imgdecod.h>
#include <lfs.h>
#include <version.h>

extern void procargs(int, char **, int *, char **, char **, char **);

int debug = 0;

/*************************************************************************
**************************************************************************/
int main(int argc, char *argv[])
{
   int ret, i, znormflag, optflag;
   char *imgclsfile, *opatsfile, *znormfile;
   char **imglist, **clslist;
   int num_imgs, img_type, total_imgs;
   unsigned char *idata, *bdata;
   int ilen, iw, ih, id, ippi;
   int bw, bh, bd;
   double ippmm;
   MINUTIAE *minutiae;
   int *direction_map, *low_contrast_map, *low_flow_map;
   int *high_curve_map, *quality_map;
   int map_w, map_h;
   float *feats, *featptr, *targs, *targptr;
   float znorm_means[NFIQ_VCTRLEN], znorm_stds[NFIQ_VCTRLEN];
   char *class_set[NFIQ_NUM_CLASSES] = {"1","2","3","4","5"};


   /* Process the command line arguments */
   procargs(argc, argv, &znormflag, &znormfile, &imgclsfile, &opatsfile);

   /* Read the input list of image files and their associated */
   /* quality levels (classes). */
   if((ret = read_imgcls_file(imgclsfile, &imglist, &clslist, &num_imgs))){
      exit(ret);
   }

   /* If znormalization set ... */
   if(znormflag){
      /* Read in the znorm statistics from file */
      if((ret = read_znorm_file(znormfile, znorm_means, znorm_stds,
                               NFIQ_VCTRLEN))){
         free_dbl_char(imglist, num_imgs);
         free_dbl_char(clslist, num_imgs);
         exit(ret);
      }
   }

   /* Allocate output buffer to hold feature vectors */
   feats = (float *)malloc(NFIQ_VCTRLEN * num_imgs * sizeof(float));
   if(feats == (float*)NULL){
      fprintf(stderr, "ERROR : main : malloc : feats\n");
      exit(-2);
   }
   featptr = feats;

   /* Allocate output buffer to hold target vectors                 */
   /* (a vector of all zeroes except for the position in the vector */
   /*  corresponding to the associated quality (class).             */
   targs = (float *)malloc(NFIQ_NUM_CLASSES * num_imgs * sizeof(float));
   if(targs == (float*)NULL){
      fprintf(stderr, "ERROR : main : malloc : targs\n");
      exit(-3);
   }
   targptr = targs;

   /* Empty images will be skipped, so keep track of actual number  */
   /* of feature vectors computed and written to the output buffers */
   total_imgs = 0;

   /* Foreach fingerprint image in the input list ... */
   for(i = 0; i < num_imgs; i++){

      /* Read and possibly decode the input image into memory ... */
      /* (This routine will automatically detect and load:        */
      /*   ANSI/NIST, WSQ, JPEGB, JPEGL, and IHead image formats  */
      if((ret = read_and_decode_grayscale_image(imglist[i], &img_type,
                    &idata, &ilen, &iw, &ih, &id, &ippi))){
         free_dbl_char(imglist, num_imgs);
         free_dbl_char(clslist, num_imgs);
         free(feats);
         free(targs);
         exit(ret);
      }

      /* If image scan density (pixels per inch) not defined, */
      /* then assume 500 */
      if(ippi == UNDEFINED)
         ippi = DEFAULT_PPI;
      /* Compute ppmm */
      ippmm = ippi / (double)MM_PER_INCH;

      /* Detect minutiae and compute quality map from input image */
      if((ret = get_minutiae(&minutiae, &quality_map, &direction_map,
                         &low_contrast_map, &low_flow_map, &high_curve_map,
                         &map_w, &map_h, &bdata, &bw, &bh, &bd,
                         idata, iw, ih, id, ippmm, &lfsparms_V2))){
         free_dbl_char(imglist, num_imgs);
         free_dbl_char(clslist, num_imgs);
         free(feats);
         free(targs);
         free(idata);
         exit(ret);
      }

      /* Done with input image and maps other than the quality map */
      free(idata);
      free(direction_map);
      free(low_contrast_map);
      free(low_flow_map);
      free(high_curve_map);
      free(bdata);

      optflag = 0;

      /* Compute an NFIQ feature vector from the minutiae detection */
      /* results. */
      ret = comp_nfiq_featvctr(featptr, NFIQ_VCTRLEN, minutiae,
                               quality_map, map_w, map_h, &optflag);
      /* If the image is determined "EMPTY" ... */
      if(ret == EMPTY_IMG){
         free_minutiae(minutiae);
         free(quality_map);
         /* Post a warning to stderr, and skip the image ... */
         fprintf(stderr, "WARNING : empty image being skipped : %s\n",
                 imglist[i]);
      }
      /* Otherwise, we have successfully computed a feature vector ... */
      else{
         /* Done with the rest of the minutiea results */
         free_minutiae(minutiae);
         free(quality_map);

         /* If the ZNormalize flag was set on the command line ... */
         if(znormflag)
            /* ZNormalize the feature vector */
            znorm_fniq_featvctr(featptr,
                                znorm_means, znorm_stds, NFIQ_VCTRLEN);

         /* Otherwise, leave the raw feature vector un-normalized */

         /* Bump the feature pointer to the start of the next */
         /* feature vector in the output buffer */
         featptr += NFIQ_VCTRLEN;

         /* Compute a target feature vector based on the associated */
         /* quality (class) passed in with the image file */
         comp_targvctr(targptr, clslist[i], class_set, NFIQ_NUM_CLASSES);

         /* Bump the target pointer to the start of the next */
         /* target vector in the output buffer */
         targptr += NFIQ_NUM_CLASSES;

         /* Bump number of feature vectors computed */
         total_imgs++;

         printf("%d (of %d): %s processed\n", i+1, num_imgs, imglist[i]);

      }
   }

   /* Write the resulting output buffers to file */
   if((ret = write_bin_nnpats(opatsfile, feats, targs, class_set,
                             total_imgs, NFIQ_VCTRLEN, NFIQ_NUM_CLASSES))){
      free_dbl_char(imglist, num_imgs);
      free_dbl_char(clslist, num_imgs);
      free(feats);
      free(targs);
      exit(ret);
   }

   /* Free all remaining allocated resources */
   free_dbl_char(imglist, num_imgs);
   free_dbl_char(clslist, num_imgs);
   free(feats);
   free(targs);

   /* Exit successfully */
   exit(0);
}

/*************************************************************************
**************************************************************************
   PROCARGS - Process command line arguments
   Input:
      argc  - system provided number of arguments on the command line
      argv  - system provided list of command line argument strings
   Output:
      znormflag  - optional flag "-z" signifying ZNormalization of output
                   feature vectors
      znormfile  - optional ZNormalization statistics file
                   (global means and stddevs for each NFIQ coef)
      imgclsfile - input list of fingerprint image files and associated
                   quality level (class)
      opatsfile  - output patterns file to hold feature and target vectors
                   for MLP training
**************************************************************************/
void procargs(int argc, char **argv, int *znormflag,
              char **znormfile, char **imgclsfile, char **opatsfile)
{
   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   if((argc != 3)&&(argc != 5)){
      fprintf(stderr,
         "Usage : %s [-z <znormfile>] <image&class list> <binpats out>\n",
              argv[0]);
      exit(1);
   }

   if(argc ==3){
      *znormflag = 0;
      *imgclsfile = argv[1];
      *opatsfile = argv[2];
      *znormfile = (char *)NULL;
   }
   else{
      if(strcmp("-z", argv[1]) != 0){
         fprintf(stderr,
            "Usage : %s [-z <znormfile>] <image&class list> <binpats out>\n",
                 argv[0]);
         exit(2);
      }
      *znormflag = 1;
      *imgclsfile = argv[3];
      *opatsfile = argv[4];
      *znormfile = argv[2];
   }
}
