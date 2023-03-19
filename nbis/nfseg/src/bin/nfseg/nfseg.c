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
      PACKAGE: NIST Fingerprint Segmentation

      FILE:    NFSEG.C

      ALGORITHM:
               Craig I. Watson

      DATE:    09/20/2004
      UPDATED: 05/09/2005 by MDG
      UPDATED: 04/10/2008 by Joseph C. Konczal - expanded ANSI/NIST support
      UPDATED: 07/10/2014 by Kenneth Ko

#cat: nfseg - Takes a single finger or 4 finger plain impression,
#cat:         grayscale fingerprint image and segments the fingerprint(s)
#cat:         into individual image files. This program will automatically
#cat:         process: ANSI/NIST, WSQ, JPEGB, JPEGL, and IHead image formats.

***********************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <imgtype.h>
#include <imgdecod.h>
#include <memalloc.h>
#include <wsq.h>
#include <jpegl.h>
#include <an2k.h>
#include <nfseg.h>
#include <version.h>

#ifdef TARGET_OS
#include <unistd.h>
#else
#include <unistd.h>
#include <getopt.h>
#endif

#ifndef __MSYS__
#include <libgen.h>
#endif

int debug = 0;
static int verbose = 0;

/* The following values are supplied by arguments that may be optional
   for ANSI/NIST files, but required for other file types.  A value of
   -1 indicates that no value has been specified, so the automatically
   determined value should be used. (Static variables are initialized
   with zero by default.) */

#define UNSET -1
static int fgp = UNSET, bthr_adj = UNSET, rot_search = UNSET,
   comp = UNSET, rot_seg = UNSET, output_images = 1;
static char *ifile, *ofile;	/* ifile is not optional */

static char *program;
static int ansi_nist_flag, old_style_args_flag;
static REC_SEL *opt_rec_sel;

/***********************************************************************/
static void usage(void) {
   fprintf(stderr, "\
   Usage: %s [options] <ANSI/NIST in> [<ANSI/NIST out>]\n\
         -v -- verbose\n\
         -b <BTHR_ADJ> -- override the automatically determined value\n\
         -r -- rotated search\n\
         -c <COMP_SEG> -- output segmented images with specified compression.\n\
         -R -- rotate output images\n\
      or  %s <FGP> <BTHR_ADJ> <ROT_SEARCH> <COMP_SEG> <ROT_SEG> <in file>\n\
         FGP\n\
            (0-14) (15=Two Thumbs)\n\
         BTHR_ADJ\n\
            1=Auto Adjust binarization threshold (inked paper)\n\
            0=Don't Adjust threshold (live scan)\n\
         ROT_SEARCH\n\
            1=Search for rotated fingers in the image.\n\
            0=Assume fingers are not rotated.\n\
         COMP_SEG\n\
            0=JPEGL | 1=WSQ5:1 | 2=WSQ15:1 | 3=NONE\n\
            Compression used on segmented image.\n\
         ROT_SEG\n\
            0=NO | 1=YES (Rotate Segmented Images?)\n", program, program);
   exit(EXIT_FAILURE);
}

/***********************************************************************
************************************************************************
#cat: parse_num_arg - Parse a positive integer command line argument value.

   Input:
      arg        - the string value to parse
      name       - a name to make code self-documenting and to use 
                   in error messages
   Return Code:
      integer    - indicate success by returning the integer value
      no return  - print usage message and exit on failure
************************************************************************/
static int parse_num_arg(const char *const arg, const char *const name)
{
   long int lval;		/* integer value of argument string */
   char *pend;	     /* end of parsed number string, should be '\0' */

   /* The result of strtol() is clamped to the range [LONG_MIN, LONG_MAX].
      If long and int are the same size, then errno needs to be checked
      to detect over/under-flow. */
#if (LONG_MAX == INT_MAX || LONG_MIN == INT_MIN)
   errno = 0;
#define PARSE_NUM_OVERFLOW(x) (ERANGE == errno)
#else
#define PARSE_NUM_OVERFLOW(x) ((x) < INT_MIN || (x) > INT_MAX)
#endif

   lval = strtol(arg, &pend, 10);
   if (*pend != '\0') {
      fprintf(stderr, "%s must be numeric: '%s'\n", name, arg);
      usage();
   }

   if (PARSE_NUM_OVERFLOW(lval)) {
      fprintf(stderr, "%s out of range [%d,%d]: %s\n", 
	      name, INT_MIN, INT_MAX, arg);
      usage();
   }
   return (int)lval;
}

/***********************************************************************
************************************************************************
#cat: check_tristate_value - Check a value that should be either 
#cat:              UNSET (-1), TRUE (1), or FALSE (0).

   Input:
      value      - the integer value to check
      name       - a name to make code self-documenting and to use 
                   in error messages
   Return Code:
      none       - return on success
      no return  - exit if value is not valid
************************************************************************/
static void check_tristate_value(int value, char *name)
{
   if (value != UNSET && value != TRUE && value != FALSE) {
      fprintf(stderr, "ERROR : invalid %s value %d\n", name, value);
      usage();
   }
   return;
}

/***********************************************************************
************************************************************************
#cat: procargs - Process command line arguments, setting various
#cat:              static variables with file scope as appropriate.

   Input:
     argc        - standard argument count
     argv        - standard argument array

   Return Code:
     none        - return on success
     no return   - exit if invalid arguments are detected
************************************************************************/
static void procargs(int argc, char **argv)
{
   int opt, output_images_flag = 0;
   const char *const option_spec = "vbc:rRf:i:n:q:";
   const char *rest;
   REC_SEL *fgp_sel = NULL, *imp_sel = NULL, *idc_sel = NULL,
      *lrt_sel = NULL, *nqm_sel = NULL;

   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   program = strrchr(argv[0], '/');
   if (NULL == program)
      program = argv[0];
   else
      ++program;

   if (argc < 2)
      usage();

   /* The following section handles the deprecated old-style arguments. --jck */
   old_style_args_flag = ((7 == argc) && ('-' != argv[1][0]));
   if (old_style_args_flag) {
      fgp =        parse_num_arg(argv[1], "FGP");
      bthr_adj =   parse_num_arg(argv[2], "BTHR_ADJ");
      rot_search = parse_num_arg(argv[3], "ROT_SEARCH");
      comp =       parse_num_arg(argv[4], "COMP_SEG");
      rot_seg =    parse_num_arg(argv[5], "ROT_SEG");
      ifile = argv[6];
      optind = 7;
      /* End of old-style deprecated argument handling. --jck */
   } else {
      /* Processing exclusive to new-style argument handling... -- jck */
      while (-1 != (opt = getopt(argc, argv, option_spec))) {
	 switch(opt) {
	    
	 case 'v':
	    verbose = 1;
	    break;

	 case 'b':
	    bthr_adj = 1;
	    break;
	    
	 case 'c':
	    output_images_flag = 1;
	    comp = parse_num_arg(optarg, "-c <COMP_SEG>");
	    break;
	    
	 case 'r':
	    rot_search = 1;
	    break;
	    
	 case 'R':
	    rot_seg = 1;
	    break;
	    
	 case 'f':
	    if (parse_rec_sel_option(rs_fgp, optarg, &rest, &fgp_sel, verbose))
	       usage();
	    if (parse_rec_sel_option(rs_imp, rest, NULL, &imp_sel, verbose))
	       usage();
	    break;
	    
	 case 'i':
	    if (parse_rec_sel_option(rs_imp, optarg, NULL, &imp_sel, verbose))
	       usage();
	    break;
	    
	 case 'n':
	    if (parse_rec_sel_option(rs_idc, optarg, NULL, &idc_sel, verbose))
	       usage();
	    break;
	    
	 case 't':
	    if (parse_rec_sel_option(rs_lrt, optarg, NULL, &lrt_sel, verbose))
	       usage();
	    break;
	    
	 case 'q':
	    if (parse_rec_sel_option(rs_nqm, optarg, NULL, &nqm_sel, verbose))
	       usage();
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

      /* ANSI/NIST input file, required */
      if (optind < argc) 
	 ifile = argv[optind++];
      else
	 usage();
      
      ansi_nist_flag = is_ANSI_NIST_file(ifile);
      if (ansi_nist_flag < 0)
	 exit(EXIT_FAILURE);
      else if ( 0 == ansi_nist_flag ) {
	 fprintf(stderr, "Input is not an ANSI/NIST file: '%s'.\n", ifile);
	 usage();
      }

      /* ANSI/NIST output file, optional */
      if (optind < argc)
	 ofile = argv[optind++];
      
      if (new_rec_sel(&opt_rec_sel, rs_and, 5,
		      fgp_sel, imp_sel, idc_sel, lrt_sel, nqm_sel) < 0)
	 exit(EXIT_FAILURE);

   }

   /* This processing applies to both old- and new-style arguments. */
   if (optind < argc) {
      fprintf(stderr, "WARNING : procargs : extra arguments ignored: %s",
	      argv[optind++]);
      for (/* empty */; optind < argc; optind++)
	 fprintf(stderr, ", %s", argv[optind]);
      fprintf(stderr, "\n");
   }

   output_images = (ofile == NULL || output_images_flag);
   
   if (comp != UNSET && (comp < 0 || comp > 3)) {
      fprintf(stderr, "Invalid COMP (%d)\n", comp);
      fprintf(stderr, "0=JPEGL | 1=WSQ5:1 | 2=WSQ15:1 | 3=NONE\n");
      exit(EXIT_FAILURE);
   }

   check_tristate_value(bthr_adj, "BTHR_ADJ");
   check_tristate_value(rot_search, "ROT_SEARCH");
   check_tristate_value(rot_seg, "ROT_SEG");

   return;
}

/***********************************************************************/
int main(int argc, char *argv[])
{
   char *filename;
   unsigned char *data, **pdata;
   int ret, i, w, h, d, img_type, dlen;
   seg_rec_coords *fing_boxes;
   int ppi, lossyflag = 0;
   int nf;


   procargs(argc, argv);

   if (old_style_args_flag) {
      /* This code supports the old-style invocation using 7 arguments
	 and allowing different kinds of image files. */

      /* FGP 1-12 is single finger : 13-14 four finger slaps : 15 for
	 two thumbs : Craig said it would be good to eliminate 23 and
	 use 15 instead.  -- jck */
      if(fgp < 1 && fgp > 14 && fgp != 15) {
	 fprintf(stderr, "ERROR: %s: Invalid FGP (%d). "
		 "Expecting 1-14 or 15 (Two thumbs)\n", argv[0], fgp);
	 exit(-2);
      }
      if(fgp < 15 && fgp > 12)
	 nf = 4;
      else if(fgp == 15)
	 nf = 2;
      else
	 nf = 1;

      /* READ THE INPUT FILE */
      if((ret = read_and_decode_grayscale_image(ifile, &img_type, &data, &dlen,
						&w, &h, &d, &ppi)))
	 exit(ret);
      
      /* TRY TO SEGMENT FINGER IMAGE */
      if((ret = segment_fingers(data, w, h, &fing_boxes, nf, fgp, bthr_adj,
				rot_search)))
	 exit(ret);
      
      
      /* PARSE FINGERS FROM ORIGINAL FINGER IMAGE */
      if((ret = parse_segfing(&pdata, data, w, h, fing_boxes, nf, rot_seg)))
	 exit(ret);
      
      free(data);
      
      /* OUTPUT RESULTS TO FILE */
      filename = basename(ifile);
      if((ret = write_parsefing(filename, -1, fgp, comp, ppi, 
				lossyflag, pdata, fing_boxes, nf, rot_seg)))
	 exit(ret);
      
      free(fing_boxes);
      for(i = 0; i < nf; i++)
	 free(pdata[i]);
      free(pdata);
      /* End of code supporting old-style interface. */

   } else {			/* ANSI/NIST file */
      /* This code, from here to the end, supports the new-style
	 interface for processing several images within ANSI/NIST files. */
      ANSI_NIST *ansi_nist, *new_ansi_nist;
      RECORD *imgrecord;
      FIELD *impfield, *fgpfield;
      char *fgp_str, *endp;
      int rec_i, imgrecord_i, img_id, impfield_i, imp, matches = 0;
      double ppmm;
      int img_fgp, img_bthr_adj;  /* per-image values */
      char *filename;

      if (read_ANSI_NIST_file(ifile, &ansi_nist) < 0)
	 exit(EXIT_FAILURE);

      if (ofile != NULL) {
	 if (copy_ANSI_NIST(&new_ansi_nist, ansi_nist) < 0) 
	    exit(EXIT_FAILURE);
      }

      if (NULL == (filename = malloc(strlen(ifile)+1))) {
         int len = strlen(ifile)+1;
         fprintf(stderr, "ERROR : cannot allocate %d bytes for filename %s\n",
                 len, ifile);
         exit(EXIT_FAILURE);
      }
      
      /* this loop's index jumps from one grayprint to the next */
      for ( rec_i = 1;
	    (ret = lookup_ANSI_NIST_grayprint(&imgrecord, &imgrecord_i,
					      rec_i, ansi_nist)) > 0; 
	    rec_i = imgrecord_i + 1 ) {

	 /* Skip latent images. */
	 if (!lookup_IMP_field(&impfield, &impfield_i, imgrecord))
	    continue;
	 imp = (int)strtol((char *)impfield->subfields[0]->items[0]->value,
			   &endp, 10);
	 if ('\0' != *endp) {
	    fprintf(stderr, "ERROR : main : corrupt IMP value: %s\n",
		    (char *)impfield->subfields[0]->items[0]->value);
	    exit(EXIT_FAILURE);
	 }
	 if (imp_is_latent(imp))
	    continue;

	 /* Skip records that don't match our command-line criteria. */
	 if (select_ANSI_NIST_record(imgrecord, opt_rec_sel) <= 0)
	    continue;
	 

	 /* If we get this far it's the right kind of image. */
	 ++matches;
	 
	 /* Figure out the finger position code. */
	 if (TYPE_4_ID == imgrecord->type) {
	    img_id = BIN_IMAGE_ID;
	    fgpfield = imgrecord->fields[FGP_ID-1];
	 } else if (TYPE_14_ID == imgrecord->type) {
	    img_id = DAT2_ID;
	    fgpfield = imgrecord->fields[FGP3_ID-1];
	 } else {
	    fprintf(stderr, "WARNING : main : skipped unexpected record type "
		    "index %d, Type-%u\n", imgrecord_i+1, imgrecord->type);
	    continue;
	 }
	 
	 if (UNSET == fgp) {
	    if ( fgpfield->subfields[0]->num_items > 1 && 
		 (TYPE_14_ID == imgrecord->type || 
		  strtol((char *)fgpfield->subfields[0]->items[1]->value,
			 &endp, 10) != 255) ) {
	       if ('\0' != *endp) {
		  fprintf(stderr, "ERROR : main : corrupt FGP value: %s\n",
			  (char *)fgpfield->subfields[0]->items[1]->value);
		  exit(EXIT_FAILURE);
	       }
	       fprintf(stderr, "WARNING : main : multiple items in subfield, "
		       " using only the first, [%d.%u.1] [Type-%u.03%u]\n", 
		       imgrecord_i+1, fgpfield->field_int+1,
		       imgrecord->type, fgpfield->field_int+1);
	    }
	    fgp_str = (char *)fgpfield->subfields[0]->items[0]->value;
	    img_fgp = strtol(fgp_str, &endp, 10);
	    if ('\0' != *endp) {
	       fprintf(stderr, "WARNING : main : currupt FGP value: %s\n",
		       fgp_str);
	       exit(EXIT_FAILURE);
	    }
	 } else {
	    img_fgp = fgp;
	 }

	 /* Determine expected number of fingers in image. */
	 if (19 == img_fgp) 	/* ignore TIP or EJI images */
	    continue;
	 else if (img_fgp == 13 || img_fgp == 14)
	    nf = 4;
	 else if (15 == img_fgp)
	    nf = 2;
	 else
	    nf = 1;
	 
	 /* Turn on binary threshold adjustment for non-livescan
	    prints, assuming they are from inked-paper. */
	 if (UNSET == bthr_adj) {
	    img_bthr_adj = !imp_is_live_scan(imp);
	 } else {
	    img_bthr_adj = bthr_adj;
	 }
	 
	 if (UNSET == comp)
	    comp = 0;		/* default compression */

	 if (UNSET == rot_search)
	    rot_search = 0;	/* default search rotation */

	 if (UNSET == rot_seg)
	    rot_seg = 0;	/* default output rotation */

	 if (verbose)
	    fprintf(stderr, "record index %d, Type-%u, fgp=%d, nf=%d, "
		    "imp=%d, bthr_adj=%d, rot_search=%d, comp=%d, rot_seg=%d\n",
		    imgrecord_i+1, imgrecord->type, img_fgp, nf,
		    imp, img_bthr_adj, rot_search, comp, rot_seg);

	 ret = decode_ANSI_NIST_image(&data, &w, &h, &d, &ppmm,
				      ansi_nist, imgrecord_i, 1);
	 if (ret < 0)
	    exit(EXIT_FAILURE);
	 else if (0 == ret)
	    continue;		/* unsuitable image ignored */
	 
	 ppi = sround(ppmm * MM_PER_INCH);
	 
	 if (segment_fingers(data, w, h, &fing_boxes, nf, img_fgp,
			     img_bthr_adj, rot_search))
	    exit(EXIT_FAILURE);

	 if (parse_segfing(&pdata, data, w, h, fing_boxes, nf, rot_seg))
	    exit(EXIT_FAILURE);

	 if (ofile != NULL) {
	    /* Convert Type-4 to 14 if necessary. */
	    if (TYPE_4_ID == imgrecord->type) {
	       RECORD *new_imgrecord;

	       ret = iafis2nist_fingerprint(&new_imgrecord, new_ansi_nist,
					    imgrecord_i);
	       if (ret < 0)
		  exit(EXIT_FAILURE);
	       else if (ret > 0) {
		  if (insert_ANSI_NIST_record_frmem(imgrecord_i, new_imgrecord,
						    new_ansi_nist))
		     exit(EXIT_FAILURE);
		  if (delete_ANSI_NIST_record(imgrecord_i+1, new_ansi_nist))
		     exit(EXIT_FAILURE);
	       }
	    }
	    
	    if (insert_parsefing(new_ansi_nist, imgrecord_i,
				 img_fgp, fing_boxes, nf, rot_search))
	       exit(EXIT_FAILURE);

	 }

	 if (output_images) {
	   /* write_parsefing calls fileroot, which is destructive */
	   strcpy(filename, ifile);

            if (write_parsefing(basename(filename), imgrecord_i, img_fgp, comp,
				ppi, lossyflag, pdata, fing_boxes, nf, rot_seg))
	       exit(EXIT_FAILURE);
	 }
	 
	 /* clean up before the next time around */
	 free(data);
	 free(fing_boxes);
	 for(i = 0; i < nf; i++)
	    free(pdata[i]);
	 free(pdata);
      }
      
      if (0 == matches)
	 fprintf(stderr, "WARNING : nfseg : no images match the criteria\n");

      if (ret < 0)
	 exit(EXIT_FAILURE);

      if (ofile != NULL && write_ANSI_NIST_file(ofile, new_ansi_nist) < 0)
	 exit(EXIT_FAILURE);
   }

   return(EXIT_SUCCESS);
}
