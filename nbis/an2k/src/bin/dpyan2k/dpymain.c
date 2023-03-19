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
      PACKAGE: ANSI/NIST 2007 Standard Reference Implementation

      FILE:    DPYMAIN.C

      AUTHORS: Michael D. Garris
               Stan Janet
      DATE:    03/07/2001
      UPDATED: 02/28/2007 by Kenneth Ko
      UPDATED: 09/03/2008 by Kenneth Ko
      UPDATED: 04/03/2008 by Joseph C. Konczal
      UPDATED: 09/30/2008 by Kenenth Ko - add version option.

#cat: dpyan2k - Parses an ANSI/NIST 2007 file, displaying all
#cat:           supported image record types to a separate X11 window.
#cat:           When a Type-9 record is encountered with IDC matching
#cat:           an image record's, this program plots the minutiae points
#cat:           recorded in the Type-9 record on top of the displayed
#cat:           image.

***********************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <dpyan2k.h>
#include <version.h>

extern char   *optarg;
extern int    optind, opterr, optopt;
static void procargs(int argc, char **argv, REC_SEL **rec_sel);
static void usage(void);

/*********************************************************************/
int main(int argc, char *argv[])
{
   int ret;
   REC_SEL *rec_selector;

   /* These setvbuf() calls are the C89 equivalent of BSD setlinebuf(). */
   setvbuf(stdout, (char *)NULL, _IOLBF, 0);
   setvbuf(stderr, (char *)NULL, _IOLBF, 0);

   procargs(argc, argv, &rec_selector);

   while(optind < argc){
      filename = argv[optind++];
      if((ret = dpyan2k(filename, rec_selector)))
         exit(ret);
   }

   exit(EXIT_SUCCESS);
}

/************************************************************************/
void procargs(int argc, char **argv, REC_SEL **rec_sel)
{
   int c;
   /* Other record selection options could be added, but the
      single-letter naming conflicts need to be resolved first. */
   char *option_spec = "a:b:d:f:t:q:r:w:H:i:In:Np:T:vW:xX:Y:";
   char *rec_sel_input_file = NULL, *rec_sel_output_file = NULL;
   REC_SEL *fgp_sel = NULL, *imp_sel = NULL, *idc_sel = NULL,
      *lrt_sel = NULL, *nqm_sel = NULL;
   const char *rest;
   char *nep;
   int optint;
   
   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   program = strrchr(*argv, '/');
   if (program == (char *) NULL)
      program = *argv;
   else
      program++;

   while ((c = getopt(argc,argv,option_spec)) != EOF){
      switch (c) {
         case 'a':
	      optint = (int)strtol(optarg, &nep, 10);
	      if (nep == optarg || *nep != '\0') {
		 fprintf(stderr, "ERROR : procargs : "
			 "invalid integer accellerator value : %s\n", optarg);
		 exit(EXIT_FAILURE);
	      }
              accelerator = MAX(1, optint);
              break;

         case 'b':
              border_width = (unsigned int)strtol(optarg, &nep, 10);
	      if (nep == optarg || *nep != '\0') {
		 fprintf(stderr, "ERROR : procargs : "
			 "invalid integer border-width value : %s\n", optarg);
		 exit(EXIT_FAILURE);
	      }
              break;

         case 'd':
              display_name = optarg;
              break;

         case 'f':
	      if (parse_rec_sel_option(rs_fgplp, optarg, &rest, &fgp_sel, verbose))
		 exit(EXIT_FAILURE);
	      if (parse_rec_sel_option(rs_imp, rest, NULL, &imp_sel, verbose))
		 exit(EXIT_FAILURE);
	      break;

         case 't':
	      if (parse_rec_sel_option(rs_lrt, optarg, NULL, &lrt_sel, verbose))
		 exit(EXIT_FAILURE);
	      break;

         case 'q':
	      if (parse_rec_sel_option(rs_nqm, optarg, NULL, &nqm_sel, verbose))
		 exit(EXIT_FAILURE);
	      break;

         case 'r':
	      rec_sel_input_file = optarg;
	      break;

         case 'w':
	      rec_sel_output_file = optarg;
	      break;

         case 'H':
	      init_wh = (unsigned int)strtol(optarg, &nep, 10);
	      if (nep == optarg || *nep != '\0') {
		 fprintf(stderr, "ERROR : procargs : "
			 "invalid integer window-height value : %s\n", optarg);
		 exit(EXIT_FAILURE);
	      }
              break;

         case 'i':
 	      if (parse_rec_sel_option(rs_imp, optarg, NULL, &imp_sel, verbose))
		 exit(EXIT_FAILURE);
	      break;

         case 'n':
	      if (parse_rec_sel_option(rs_idc, optarg, NULL, &idc_sel, verbose))
		 exit(EXIT_FAILURE);
	      break;

         case 'I':
              iafis_flag = True;
              nist_flag = False;
              break;

         case 'N':
              nist_flag = True;
              iafis_flag = False;
              break;

         case 'p':
	      pointwidth = (int)strtol(optarg, &nep, 10);
	      if (nep == optarg || *nep != '\0') {
		 fprintf(stderr, "ERROR : procargs : "
			 "invalid integer point-width value : %s\n", optarg);
		 exit(EXIT_FAILURE);
	      }
              break;

         case 'T':
              title = optarg;
              break;

         case 'v':
              verbose++;
              break;

         case 'W':
              init_ww = (unsigned int)strtol(optarg, &nep, 10);
	      if (nep == optarg || *nep != '\0') {
		 fprintf(stderr, "ERROR : procargs : "
			 "invalid integer window-width value : %s\n", optarg);
		 exit(EXIT_FAILURE);
	      }
              break;

         case 'x':
              debug++;
              break;

         case 'X':
              wx = (int)strtol(optarg, &nep, 10);
	      if (nep == optarg || *nep != '\0') {
		 fprintf(stderr, "ERROR : procargs : "
			 "invalid integer window-X value : %s\n", optarg);
		 exit(EXIT_FAILURE);
	      }
              break;

         case 'Y':
              wy = (int)strtol(optarg, &nep, 10);
	      if (nep == optarg || *nep != '\0') {
		 fprintf(stderr, "ERROR : procargs : "
			 "invalid integer window-Y value : %s\n", optarg);
		 exit(EXIT_FAILURE);
	      }
              break;

/* Not used in this application
         case 'N':
              nicevalue = atoi(optarg);
              break;

         case 'A':
              automatic = True;
              break;
         case 's':
              sleeptime = atoi(optarg);
              break;
         case 'O':
              no_window_mgr = True;
              break;
         case 'k':
              no_keyboard_input = True;
              break;
*/

         case '?':
              usage();
              break;
         default:
	    fprintf(stderr, "Programming error: "
		    "incompletely implemented option: '%c'\n",  c);
	    exit(EXIT_FAILURE);
      } /* switch */
   }

   if (optind >= argc)
      usage();

   if (new_rec_sel(rec_sel, rs_and, 5, 
		   fgp_sel, imp_sel, idc_sel, lrt_sel, nqm_sel) < 0)
      exit(EXIT_FAILURE);

   if (rec_sel_input_file) {
      if ((*rec_sel)->num_values > 0)
	 fprintf(stderr, "WARNING : procargs : reading selection options"
		 " from file %s, ignoring others on command line\n",
		 rec_sel_input_file);
      if (read_rec_sel_file(rec_sel_input_file, rec_sel) != 0)
	 exit(EXIT_FAILURE);
   }

   if (simplify_rec_sel(rec_sel))
	exit(EXIT_FAILURE);
   
   if (rec_sel_output_file
       && write_rec_sel_file(rec_sel_output_file, *rec_sel) != 0)
      exit(EXIT_FAILURE);
}

/****************************************************************
Print usage message and exit.
****************************************************************/
void usage(void)
{
   (void)fprintf(stderr, "\
   Usage:\n\
   %s [options] <ANSI_NIST ...>\n\
      -f n[:i]     finger position n [and impression type i] to display.\n\
                   These can be lists, ranges, and named values, e.g.: '1,6',\n\
                   '1-5', or 'thumb'.  Use -fhelp for more details.\n\
      -t n         logical record type to display\n\
      -q n         select fingers using pre-computed NIST quality metric\n\
      -r filename  read record selection parameters from a file\n", program);
   (void)fprintf(stderr, "\
      -w filename  write record selection parameters to a file\n\
      -a n         set drag accelerator [1]\n\
      -v           set verbose\n\
      -x           set debug mode (core dump on X11 error)\n\
      -b n         set border width to n pixels [%d]\n\
      -i n         impression type to display\n\
      -I           use IAFIS Type-9 fields 13-23\n\
      -n n         image number to display\n", DEF_BORDER_WIDTH);
   (void)fprintf(stderr, "\
      -N           use NIST Type-9 fields 5-12 (default)\n\
      -p n         set point width to n pixels [%d]\n\
      -W n         set window width to n pixels\n\
      -H n         set window height to n pixel \n\
      -X n         set window x pixels from display border [0]\n\
      -Y n         set window y pixels from display border [0]\n\
      -T title     set title for image windows\n\
      -d display   connect to alternate X11 server\n", pointwidth);

/* Not used in this application
      -N n         nice I/O process with increment n\n\
      -A           auto advance through images\n\
      -s n         sleep n seconds before advancing [%d]\n\
      -O           override redirect on window (no window manager)\n\
      -k           no keyboard input\n\
*/
   exit(EXIT_FAILURE);
}
