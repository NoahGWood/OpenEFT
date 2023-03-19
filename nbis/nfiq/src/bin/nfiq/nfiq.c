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

      FILE:    NFIQ.C

      ALGORITHM:
               Elham Tabassi
               Charles L. Wilson
               Criag I. Watson

      IMPLEMENTATION:
               Michael D. Garris

      DATE:    09/13/2004
      UPDATED: 11/21/2005 by KKO
      UPDATED: 02/28/2008 by Joseph C. Konczal - added ANSI/NIST record selector
      UPDATED: 09/30/2008 by Kenenth Ko - add version option.
      UPDATED: 12/30/2008 by JCK - remove non-existant -u option from usage msg.
      UPDATED: 01/08/2009 by JCK - add -q option to usage message.
      UPDATED: 12/22/2008 by Gregory Fiumara - added raw image support
      UPDATED: 02/11/2009 by Gregory Fiumara - made raw option POSIX compliant

#cat: nfiq - Takes a grayscale fingerprint image and computes an
#cat:        image quality value based on the NFIQ algorithm.
#cat:        This program will automatically process:
#cat:        ANSI/NIST, WSQ, JPEGB, JPEGL, IHead and raw image formats

***********************************************************************/
#include <stdio.h>
#include <nfiq.h>
#include <imgtype.h>
#include <imgdecod.h>
#include <unistd.h>
#include <strings.h>
#include <errno.h>
#include <version.h>
#include <parsargs.h>

/* The AR_SZ macro calculates the number of elements in an array. */
#define AR_SZ(x) (sizeof(x)/sizeof(*x))

typedef struct opt_flags_s {
   int verbose;
   int old_mode;
   int has_attribs;
} OPT_FLAGS;

void procargs(int, char **, OPT_FLAGS *, char **, char **, REC_SEL **, int *,
              int *, int *, int *);
void usage(void);

char *program;
int debug = 0;    /* required by wsq_decode_mem() in libwsq.a(decoder.o) */

/*************************************************************************
**************************************************************************/
int main(int argc, char *argv[])
{
   int ret;
   char *imgfile = NULL, *outfile = NULL;
   unsigned char *idata;
   int img_type, ilen, iw, ih, id, ippi;
   int nfiq;
   float conf;
   OPT_FLAGS flags = { 0, 0, 0 };
   REC_SEL *opt_rec_sel = NULL;

   /* Process command line arguments */
   procargs(argc, argv, &flags, &imgfile, &outfile, &opt_rec_sel, &iw, &ih,
            &id, &ippi);

   /* new code added to select and operate on multiple images in
      ANSI/NIST files, previous code remains after "else" for other
      file types */
   ret = is_ANSI_NIST_file(imgfile);
   if (ret < 0)
      return ret;
   if (ret == TRUE && !flags.old_mode) { /* it is an ANSI/NIST file */
      ANSI_NIST *ansi_nist, *new_ansi_nist;
      RECORD *imgrecord;
      FIELD *fgpfield;
      int imgrecord_i, rec_i;
      double ippmm;
      int img_id, fgp_int, matches = 0;
      char *fgp_str, *nep;

      if (read_ANSI_NIST_file(imgfile, &ansi_nist))
         exit(EXIT_FAILURE);

      if (outfile != NULL) {
         if (copy_ANSI_NIST(&new_ansi_nist, ansi_nist) < 0)
            exit(EXIT_FAILURE);
         if (flags.verbose)
            printf("writing output with NQM fields to %s\n", outfile);
      }

      /* this loop's index jumps from one grayprint to the next */
      for ( rec_i = 1;
            (ret = lookup_ANSI_NIST_grayprint(&imgrecord, &imgrecord_i,
                                              rec_i, ansi_nist)) > 0;
            rec_i = imgrecord_i + 1 ) {
         
         /* Thanks to the authors of read_and_decode_grayscale_image(),
            from which I borrowed some of the following code
            with heavy editing. */

         /* Now we look for any reason not to calculate the nfiq... */

         /* Skip records that don't match our command-line criteria. */
         if (select_ANSI_NIST_record(imgrecord, opt_rec_sel) <= 0)
            continue;

         if (TYPE_4_ID == imgrecord->type) { 
            const char *strval = (char *)imgrecord->fields[IMP_ID-1]
               ->subfields[0]->items[0]->value;
            const int imp = (int)strtol(strval, &nep, 10);

            if (nep == strval || *nep != '\0') {
               fprintf(stderr, "ERROR : main : expected an integer IMP value "
                       "at index [%d.%d.1.1] [Type-4.%03d], found '%s'\n",
                       imgrecord_i+1, IMP_ID, IMP_ID, strval);
               continue;
            }

            /* Type-4 can hold latent images, which we want to skip */
            if (imp_is_latent(imp)) {
               fprintf(stderr, "WARNING : main : skipped latent image "
                       "record index [%d] [Type-4]\n", imgrecord_i+1);
               continue;
            }

            if (outfile == NULL) { /* Use type-4 record as-is. */
               img_id = BIN_IMAGE_ID;
               fgpfield = imgrecord->fields[FGP_ID-1];
            } else { /* Convert type-4 to type-14. */
               RECORD *new_imgrecord;

               ret = iafis2nist_fingerprint(&new_imgrecord, new_ansi_nist,
                                            imgrecord_i);
               if (ret < 0) {   /* error */
                  exit(EXIT_FAILURE);
               } else if (ret == 0) { /* record ignored */
                  fprintf(stderr, "WARNING : main : record not converted "
			  "by iafis2nist_fingerprint, index [%d] [Type-4]\n",
                          imgrecord_i+1);
                  continue;
               } 
               
               if (insert_ANSI_NIST_record_frmem(imgrecord_i, new_imgrecord,
                                                 new_ansi_nist))
                  exit(EXIT_FAILURE);
               if (delete_ANSI_NIST_record(imgrecord_i+1, new_ansi_nist))
                  exit(EXIT_FAILURE);
               /* free(imgrecord); */
               imgrecord = new_imgrecord;
               img_id = DAT2_ID;
               fgpfield = imgrecord->fields[FGP3_ID-1];
            }
         } else if (TYPE_14_ID == imgrecord->type) {
            img_id = DAT2_ID;
            fgpfield = imgrecord->fields[FGP3_ID-1];
         } else {
            fprintf(stderr, "WARNING : main : skipped unexpected record type "
                    "index [%d] [Type-%d]\n", imgrecord_i+1, imgrecord->type);
            continue;
         }

         /* Find the finger position and check whether it is a single finger. */
         fgp_str = (char *)fgpfield->subfields[0]->items[0]->value;
         fgp_int = (int)strtol(fgp_str, &nep, 10);
         if (nep == fgp_str || *nep != '\0') {
            fprintf(stderr, "ERROR : main : expected an integer FGP value "
                    "at index [%d.%d.1.1] [Type-%d.03%d], found '%s'.\n",
                    imgrecord_i+1, fgpfield->field_int+1, imgrecord->type,
                    fgpfield->field_int+1, fgp_str);
            continue;
         }

         if ( fgpfield->subfields[0]->num_items > 1 && 
              (TYPE_14_ID == imgrecord->type || 
               strcmp((char *)fgpfield->subfields[0]->items[1]->value, "255")) != 0) {
            int item_i;

            fprintf(stderr, "WARNING : main : using first FGP value only: %d, "
                    "ignoring %d others in subfield [%d.%d.1] [Type-%d.03%d]: ",
                    fgp_int, fgpfield->subfields[0]->num_items-1,
                    imgrecord_i+1, fgpfield->field_int+1, imgrecord->type,
                    fgpfield->field_int+1);
            for (item_i = 1;
                 item_i < fgpfield->subfields[0]->num_items;
                 item_i++)
               fprintf(stderr, " %s", 
                       fgpfield->subfields[0]->items[item_i]->value);
            fprintf(stderr, "\n");
         }

         if (13 == fgp_int || 14 == fgp_int) {
            fprintf(stderr, "WARNING : main : ignoring multi-finger slap, "
                    "record index [%d] [Type-%d] fgp %d\n",
                    imgrecord_i+1, imgrecord->type, fgp_int);
            continue;
         } else if (15 == fgp_int) {
            fprintf(stderr, "WARNING : main : ignoring two-thumb slap, "
                    "record index [%d] [Type-%d] fgp %d\n",
                    imgrecord_i+1, imgrecord->type, fgp_int);
            continue;
         } else if (19 == fgp_int) {
            fprintf(stderr, "WARNING : main : ignoring EJI or tip, "
                    "record index [%d] [Type-%d] fgp %d\n",
                    imgrecord_i+1, imgrecord->type, fgp_int);
            continue;
         }

         /* Finally, if we get this far it's the right kind of image. */
         ++matches;

         if (decode_ANSI_NIST_image(&idata, &iw, &ih, &id, &ippmm ,
                                    ansi_nist, imgrecord_i, 1) < 0)
            exit(EXIT_FAILURE);
         
         ippi = sround(ippmm * MM_PER_INCH);
         
         if (flags.verbose == 0) {
            printf("%d %d", imgrecord_i+1, fgp_int);
         } else {
            printf("[%d.%d] [Type-%d.%03d] fgp %d\n", imgrecord_i+1, 
                   imgrecord->num_fields, /* image data is always last field */
                   imgrecord->type, img_id, fgp_int);
         }
         if (comp_nfiq(&nfiq, &conf, idata, iw, ih, id, ippi,
                       &flags.verbose) < 0)
            exit(EXIT_FAILURE);

         if (flags.verbose == 0) {
            printf("%2d\n", nfiq);
         } else if(flags.verbose == 1) {
            printf("%d\t%4.2f\n", nfiq, conf);
         }
         if (outfile != NULL) {
            char nfiq_str[4];
            FIELD *field;
            SUBFIELD *subfield;
            int field_i;

            if (imgrecord->type != TYPE_14_ID) {
               fprintf(stderr, "ERROR : main : Cannot add NQM "
                       "to a Type-%d record\n", imgrecord->type);
               continue;
            }

	    if (ansi_nist->version < VERSION_0400) {
	       ansi_nist->version = VERSION_0400;
	       if (substitute_ANSI_NIST_item(0, VER_ID-1, 0, 0, "0400",
                                             new_ansi_nist) < 0) {
		  fprintf(stderr, "ERROR : main : "
			  "Cannot change VER to 0400 to support NQM field.\n");
		  exit(EXIT_FAILURE);
	       }
	    }

            /* Find the right place to insert the field.  The first
               fields through FGP=13 are required.  The others between
               FGP=13 and NMQ=22 are optional. */
            for (field_i = 0; field_i < imgrecord->num_fields; field_i++){
               if (imgrecord->fields[field_i]->field_int > NQM_ID) {
                  break;
               }
            }
            
            /* convert the number to text */
            sprintf(nfiq_str, "%d", nfiq);
            
            if (lookup_ANSI_NIST_field(&field, &field_i, NQM_ID, imgrecord)) {
               if (substitute_ANSI_NIST_item(imgrecord_i, field_i, 0, 1,
                                             nfiq_str, new_ansi_nist) < 0) 
                  exit(EXIT_FAILURE);
            } else {
               if (new_ANSI_NIST_field(&field, imgrecord->type, NQM_ID) < 0)
                  exit(EXIT_FAILURE);
               if(insert_ANSI_NIST_field_frmem(imgrecord_i, field_i,
                                               field, new_ansi_nist) < 0)
                  exit(EXIT_FAILURE);
            
               if (alloc_ANSI_NIST_subfield(&subfield) < 0)
                  exit(EXIT_FAILURE);
               if (insert_ANSI_NIST_subfield_frmem(imgrecord_i, field_i, 0,
                                                   subfield, new_ansi_nist) < 0)
                  exit(EXIT_FAILURE);
               
               if (insert_ANSI_NIST_item(imgrecord_i, field_i, 0, 0,
                                         fgp_str, new_ansi_nist) < 0)
                  exit(EXIT_FAILURE);
               
               if (insert_ANSI_NIST_item(imgrecord_i, field_i, 0, 1,
                                         nfiq_str, new_ansi_nist) < 0)
                  exit(EXIT_FAILURE);
            }
         }
      }

      if (0 == matches)
         fprintf(stderr, "WARNING : nfiq : no images match the criteria\n");

      if (ret < 0)
         exit(EXIT_FAILURE);
      if (outfile != NULL && write_ANSI_NIST_file(outfile, new_ansi_nist) < 0)
         exit(EXIT_FAILURE);

   } else {                     /* old code, not an ANSI/NIST file */
      /* previous code, handles types other than ANSI/NIST */

      /* Read and if needed decode the input image into memory ... */
      /* User says this is a raw image file */
      if(flags.has_attribs) {
         if((ret = read_raw(imgfile, &idata, &iw, &ih, &id)))
            exit(ret);
      } else {
         /* This routine will automatically detect and load:         */
         /* ANSI/NIST, WSQ, JPEGB, JPEGL, and IHead image formats   */
         if((ret = read_and_decode_grayscale_image(imgfile, &img_type,
                    &idata, &ilen, &iw, &ih, &id, &ippi))) {
            if(ret == -3) /* UNKNOWN_IMG */
               fprintf(stderr, "Hint: Use -raw for raw images\n");
            exit(ret);
         }
      }

      /* Compute the NFIQ value */
      ret = comp_nfiq(&nfiq, &conf, idata, iw, ih, id, ippi, &flags.verbose);
      /* If system error ... */
      if(ret < 0) {
         free(idata);
         exit(ret);
      }
      
      /* Report results to stdout */
      if (flags.verbose == 0) {
            printf("%d\n", nfiq);
      } else if(flags.verbose == 1) {
            printf("%d\t%4.2f\n", nfiq, conf);
      }

      /* Deallocate image data */
      free(idata);
   }

   /* Exit successfully */
   exit(EXIT_SUCCESS);
}

/*************************************************************************
**************************************************************************
   PROCARGS - Process command line arguments
   Input:
      argc  - system provided number of arguments on the command line
      argv  - system provided list of command line argument strings
   Output:
      imgfile - input fingerprint image file of format type:
                ANSI/NIST, WSQ, JPEGB, JPEGL, or IHead
                
**************************************************************************/
void procargs(int argc, char **argv, OPT_FLAGS *flags, char **imgfile, 
              char **outfile, REC_SEL **rec_sel, int *iw, int *ih, int *id, 
              int *ippi)
{
   extern char *optarg;
   extern int optind, opterr, optopt;
   const char *const option_spec = "dvof:i:n:t:q:r:w";
   int opt;
   REC_SEL *fgp_sel = NULL, *imp_sel = NULL, *idc_sel = NULL,
      *lrt_sel = NULL, *nqm_sel = NULL;
   const char *rest;
   char *rec_sel_input_file = NULL, *rec_sel_output_file = NULL;
   char *raw_attribs;

   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   program = strrchr(*argv,'/');
   if (program)
      program++;
   else
      program = *argv;

   while (-1 != (opt = getopt(argc, argv, option_spec))) {
      switch (opt) {

      case 'd':                 /* default, now this is the default - jck */
         flags->verbose = 0;
         break;
         
      case 'v':                 /* verbose */
         flags->verbose = 1;
         break;

      case 'o':                 /* the old behavior, first image only - jck */
         flags->old_mode = 1;
         break;

      case 'f':
         if (parse_rec_sel_option(rs_fgp, optarg, &rest, &fgp_sel, flags->verbose))
            usage();
         if (parse_rec_sel_option(rs_imp, rest, NULL, &imp_sel, flags->verbose))
            usage();
         break;
            
      case 'i':                 /* impression type */
         if (parse_rec_sel_option(rs_imp, optarg, NULL,
                                  &imp_sel, flags->verbose) < 0)
            exit(EXIT_FAILURE);
         break;

      case 'n':                 /* image designation character (number) */
         if (parse_rec_sel_option(rs_idc, optarg, NULL,
                                  &idc_sel, flags->verbose) < 0)
            exit(EXIT_FAILURE);
         break;

      case 't':                 /* logical record type */
         if (parse_rec_sel_option(rs_lrt, optarg, NULL,
                                  &lrt_sel, flags->verbose) < 0)
            exit(EXIT_FAILURE);
         break;
         
      case 'q':
         if (parse_rec_sel_option(rs_nqm, optarg, NULL,
                                  &nqm_sel, flags->verbose) < 0)
            exit(EXIT_FAILURE);
         break;

      case 'r':                 /* read selector file OR raw option */
         /* read selector file */
         if (!strcmp(optarg, "aw") == 0)
            rec_sel_input_file = optarg;
         /* raw option */
         else {
            raw_attribs = argv[optind++];
            if (raw_attribs == NULL) {
               fprintf(stderr, "WARNING : procargs : Raw option passed with "
                       "no attributes\n");
               exit(EXIT_FAILURE);
            }

            parse_w_h_d_ppi(raw_attribs, program, iw, ih, id, ippi);
            /* Sanity check on attributes -- based on WSQ attributes */
            if (*id != 8)
            {
               fprintf(stderr, "ERROR : procargs : Raw image depth "
                       "is not 8\n");
               exit(EXIT_FAILURE);
            }
            if (*iw < MIN_IMG_DIM || *ih < MIN_IMG_DIM)
            { 
               fprintf(stderr, "ERROR : procargs : Raw image dimensions "
                       "must be at least %dx%d\n", MIN_IMG_DIM, MIN_IMG_DIM);
               exit(EXIT_FAILURE);
            }
            /* If PPI declared, it must be 500. nfiq assumes UNDEFINED = 500 */
            if (*ippi != 500 && *ippi != UNDEFINED)
            {
               fprintf(stderr, "ERROR : procargs : Raw image PPI "
                       "must be 500\n");
               exit(EXIT_FAILURE);
            }

            flags->has_attribs = 1;
         }

         break;

      case 'w':                 /* write selector file */
         rec_sel_output_file = optarg;
         break;

      case '?':
         usage();
         break;

      default:
         fprintf(stderr, "Programming error: "
                 "incompletely implemented option: '%c'\n", opt);
         exit(EXIT_FAILURE);
      }
   }
   
   /* Create a record selector including all the possible selection items */
   if (new_rec_sel(rec_sel, rs_and, 5,
                   fgp_sel, imp_sel, idc_sel, lrt_sel, nqm_sel) < 0)
      exit(EXIT_FAILURE);

   /* It would serve no purpose to be able to read and write selector
      files in the same invocation, unless mixing of command line
      selectors and a selector file were implemented. */
   if (rec_sel_input_file && rec_sel_output_file) {
      fprintf(stderr, "ERROR : procargs : one may read or write files of"
              " record selection criteria, but not both at once.\n");
      exit(EXIT_FAILURE);
   }

   /* Selectors are read either from the command line or from a file,
      but the two cannot be mixed.  (However, a function could be
      written to combine two sets of selectors, if we want to be able
      to do that.)  */
   if (rec_sel_input_file) {
      if ((*rec_sel)->num_values > 0)
         fprintf(stderr, "WARNING : procargs : reading selection options"
                 " from file %s, ignoring others on command line\n",
                 rec_sel_input_file);
      if (read_rec_sel_file(rec_sel_input_file, rec_sel) != 0)
         exit(EXIT_FAILURE);
   }

   /* The easiest way of creating a set of selectors might produce
      some unnessary components that are either empty or that perform
      boolean combinations with only one input, so we try to eliminate
      those things. */
   simplify_rec_sel(rec_sel);
   if (rec_sel_output_file
       && write_rec_sel_file(rec_sel_output_file, *rec_sel) != 0)
      exit(EXIT_FAILURE);

   if (optind < argc)
      *imgfile = argv[optind++];
   else
      usage();                  /* no input file specified */

   if (optind < argc)
      *outfile = argv[optind++];

   if (optind < argc) {
      fprintf(stderr, "WARNING : procargs : extra arguments ignored: %s",
              argv[optind++]);
      for (/* empty */; optind < argc; optind++) 
         fprintf(stderr, ", %s", argv[optind]);
      fprintf(stderr, "\n");
   }   
}

/* silence the linker error from procargs.h */
void print_usage(char *c) {}

void usage(void)
{
   (void)fprintf(stderr, "   Usage:\n\
   %s [options] <fingimage in> [<fingimage out>]\n\
      -d               default - print only the image quality value,\n\
                       preceeded by the ANSI/NIST record number if applicable\n\
      -v               verbose - print all the feature vectors, image quality\n\
                       value, and network activation values\n\
      -o               old behavior - print values for only the first\n\
                       grayscale fingerprint record in an ANSI/NIST file.\n",
	   program);
   (void)fprintf(stderr, "\
      -raw w,h,d[,ppi]   calculate the NFIQ score from a raw image file\n\
      -f n[:i]         Select records by finger position n [and impression\n\
                       type i].  Use -f help for more details.\n\
      -i n             Select records by impression type n.  Use -i help for\n\
                       more details.\n\
      -n n             Select a record by the image descriptor n.  Use -n\n\
                       help for more details.\n");
   (void)fprintf(stderr, "\
      -q n             Select records using the stored NQM value, if it\n\
                       exists.\n\
      -t n             Select record of type n.  Use -t help for more \n\
                       details.\n\
      -r filename      Read selectors from a text file.\n\
      -w filename      Write selectors to a text file.\n");

   exit(EXIT_FAILURE);
}
