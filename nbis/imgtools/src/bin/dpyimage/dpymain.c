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
      PACKAGE: NIST Image Display

      FILE:    DPYMAIN.C

      AUTHORS: Stan Janet
               Michael D. Garris
      DATE:    12/03/1990
      UPDATED: 05/10/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.
      UPDATED:  02/11/2009 by Greg Fiumara - make raw option POSIX compliant

#cat: dpyimage - Takes an IHead, WSQ, JPEGB, JPEGL, or raw image file,
#cat:            reconstructs the image pixmap (if needed), and renders
#cat:            the pixmap in an X11 Window.

***********************************************************************/

#include <usebsd.h>
#include <stdlib.h>
#include <string.h>
#include <dpyimage.h>
#include <version.h>
#include <unistd.h>

void procargs(int, char **);
void usage(void);
extern char *optarg;
extern int optind, opterr, optopt;

/*********************************************************************/

int main(int argc, char *argv[])
{
   int ret;

   setlinebuf(stdout);

   procargs(argc,argv);
   if (optind >= argc)
      usage();

   if((ret = xconnect()))
      exit(ret);

   screen = DefaultScreen(display);
   def_cmap = DefaultColormap(display,screen);
   dw = DisplayWidth(display,screen);
   dh = DisplayHeight(display,screen);
   rw = RootWindow(display,screen);
   visual = DefaultVisual(display,screen);
   bp = BlackPixel(display,screen);
   wp = WhitePixel(display,screen);

   if ((optind == argc - 1) || (dpy_mode == DPY_NORM)){
      if((ret = dpynorm(argc,argv)))
         exit(ret);
   }
   else if (dpy_mode == DPY_PIPE){
      if ((ret = pipecomm(argc,argv))){
         exit(ret);
      }
      /* Parent does not know if child exits on error, so        */
      /* parent thread should exit here because some X Resources */
      /* might not be allocated so cannot be "cleaned up" below  */
      exit(0);
   }
   else {                          /* dpy_mode == DPY_TMP */

      if (tmpdir == def_tmpdir) {  /* if not set on command line,  */
         char *p;		   /* check environment for TMPDIR */

         p = getenv("TMPDIR");
         if (p != (char *) NULL)
            tmpdir = p;
      }
      tmpcomm(argc,argv);
   }

   cleanup();

   if (verbose)
      (void) printf("Errors: %d\n",errors);

   exit(errors?1:0);
}

/************************************************************************/
void procargs(int argc, char **argv)
{
   int c;
   char *option_spec = "Aa:b:D:d:H:kN:Onr:s:T:tvW:xX:Y:";
   char *raw_attribs;

  if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   program = rindex(*argv,'/');
   if (program == (char *) NULL)
      program = *argv;
   else
      program++;

   while ((c = getopt(argc,argv,option_spec)) != EOF){
      switch (c) {
         case 'A':	automatic = True;
              break;

         case 'a':	accelerator = MAX(1,atoi(optarg));
              break;

         case 'b':	border_width = atoi(optarg);
              break;

         case 'D':	tmpdir = optarg;
              break;

         case 'd':	display_name = optarg;
              break;

         case 'H':	init_wh = atoi(optarg);
              break;

         case 'N':	nicevalue = atoi(optarg);
              break;

         case 'n':	dpy_mode = DPY_NORM;
              break;

         case 'r':	raw = True;
              /* Ignore "aw" in "-raw" for consistency among NBIS progams */
              if (strcmp(optarg, "aw") == 0)
                 raw_attribs = argv[optind++];
              else
                 raw_attribs = optarg;

              c = sscanf(raw_attribs,"%u,%u,%u,%u",
                         &raw_w,&raw_h,
                         &raw_depth,&raw_whitepix);
              if (c != 4) {
                 (void) fprintf(stderr,
                                "%s: cannot parse raw parameters\n", program);
                 usage();
              }
              break;

         case 's':	sleeptime = atoi(optarg);
              break;

         case 'T':	title = optarg;
              break;

         case 't':	dpy_mode = DPY_TMP;
              break;

         case 'v':	verbose++;
              break;

         case 'X':	wx = atoi(optarg);
              break;

         case 'x':	debug++;
              break;

         case 'Y':	wy = atoi(optarg);
              break;

         case 'W':	init_ww = atoi(optarg);
              break;

         case 'O':	no_window_mgr = True;
              break;

         case 'k':	no_keyboard_input = True;
              break;

         default:	usage();
              break;
      }/* switch */
   }
}

/****************************************************************
Print usage message and exit.
****************************************************************/
void usage(void)
{
   static char usage_msg[] = "\
   Usage:\n\
   %s [options] image-file ...\n\
	-r, raw  w,h,d,wp	files are raw data with given width, height\n\
				depth and white pixel\n\
	-A		        auto advance through images\n\
	-s n		        sleep n seconds before advancing [%d]\n\
	-a n		        set drag accelerator [1]\n\
	-v			verbose\n\
	-x			debug mode (create core dump on X11 error)\n\
	-b n			set border width to n [%d]\n\
	-N n			nice I/O process with increment n\n\
	-O			override redirect on window (no window\n\
				manager)\n\
	-k			no keyboard input\n\
	-W n			set window width to n\n\
	-H n			set window height to n\n\
	-X n			set window x pixels from display border [0]\n\
	-Y n			set window y pixels from display border [0]\n\
	-n			do not fork; one process reads and displays\n\
				images\n\
	-T title		set title for images [filename]\n\
	-t			transfer images by temporary files\n\
	-D dir			create temporary files in directory [%s]\n\
	-d display		connect to alternate X11 server\n";

   (void) fprintf(stderr,
	          usage_msg,
                  program,DEF_SLEEPTIME,DEF_BORDER_WIDTH,DEF_TMPDIR);
   exit(1);
}
