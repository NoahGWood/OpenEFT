/************************************************************************
                               NOTICE
 
This MITRE-modified NIST code was produced for the U. S. Government
under Contract No. W15P7T-07-C-F700. Pursuant to Title 17 Section 105 
of the United States Code, this software is not subject to copyright 
protection and is in the public domain. NIST and MITRE assume no 
responsibility whatsoever for use by other parties of its source code 
or open source server, and makes no guarantees, expressed or implied,
about its quality, reliability, or any other characteristic.

This software has been determined to be outside the scope of the EAR
(see Part 734.3 of the EAR for exact details) as it has been created solely
by employees of the U.S. Government; it is freely distributed with no
licensing requirements; and it is considered public domain.  Therefore,
it is permissible to distribute this software as a free download from the
internet.
   
The algorithm and its benefits are briefly described in the
MITRE Technical Report "Fingerprint Recompression after Segmentation"
(MTR080005), available at 
http://www.mitre.org/work/tech_papers/tech_papers_08/08_0060/index.html

************************************************************************/

/***********************************************************************
      PACKAGE: ANSI/NIST 2000 Standard Reference Implementation

      FILE:    MAIN.C

      AUTHORS: Michael D. Garris
               Stan Janet
      DATE:    03/07/2001
      MOD:     02/07/2008  Margaret Lepley
      UPDATED: 09/30/2008 by Kenenth Ko - add version option.

#cat: autocrop - Parses an ANSI/NIST 2000 file, cropping any
#cat:           Type-14 WSQ records at the included box using CropCoeff.
#cat:           Saves new WSQ to a derived filename.

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <version.h>

void procargs(int argc, char **argv);
void usage();
int autocrop(char *fiffile);

char *program;
int debug = 0;

/*********************************************************************/
int main(int argc, char *argv[])
{
   int ret;
   char *filename;
   extern int optind;

   setvbuf(stdout, (char *)NULL, _IOLBF, 0);
/*
   setlinebuf(stdout);
*/
   procargs(argc,argv);

   while(optind < argc){
      filename = argv[optind++];
      if((ret = autocrop(filename))) {
	if (ret < 0)
	  exit(ret);
	fprintf(stderr, "******ERROR******* :  Missed crop(s) on  %s\n\n", filename );
      }
   }

   exit(0);
}

/************************************************************************/
void procargs(int argc, char **argv)
{
   int c;
   char *option_spec = "c:v:";
   extern int atoi(), getopt(), optind;
   extern char *optarg;

   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   program = strrchr(*argv,'/');
   if (program == (char *) NULL)
      program = *argv;
   else
      program++;

   while ((c = getopt(argc,argv,option_spec)) != EOF){
      switch (c) {
         case 'v':
              debug = atoi(optarg);
              break;

         default:
              usage();
              break;
      } /* switch */
   }

   if (optind >= argc)
      usage();
}

/****************************************************************
Print usage message and exit.
****************************************************************/
void usage()
{
   static char usage_msg[] = "\
   Usage:\n\
   %s [options] <ANSI_NIST ...>\n\
      -v L         set verbose level\n";
   
   (void) fprintf(stderr, usage_msg, program);
   exit(1);
}
