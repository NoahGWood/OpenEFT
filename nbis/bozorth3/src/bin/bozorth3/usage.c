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
      PACKAGE:        Bozorth Fingerprint Matcher

      FILE:           USAGE.C

      ALGORITHM:      Allan S. Bozorth (FBI)
      MODIFICATIONS:  Michael D. Garris (NIST)
                      Stan Janet (NIST)
      DATE:           09/21/2004


#proc: usage - Prints usage for Bozorth3 matcher program

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <bozorth.h>

void usage( FILE * fp )
{
fprintf( fp, "Usage:\n" );
fprintf( fp, "   To compute match scores for fingerprint pairs:\n" );
fprintf( fp, "        %s [options] probefile.xyt galleryfile.xyt [probefile galleryfile ...]\n",  PROGRAM );
fprintf( fp, "        %s [options] -M mates.lis\n",                       PROGRAM );
fprintf( fp, "        %s [options] -P probes.lis  gallery*.xyt\n",        PROGRAM );
fprintf( fp, "        %s [options] -G gallery.lis probe*.xyt\n",          PROGRAM );
fprintf( fp, "   To compute match scores for one fingerprint against many:\n" );
fprintf( fp, "        %s [options] -p probefile.xyt      gallery*.xyt\n", PROGRAM );
fprintf( fp, "        %s [options] -p probefile.xyt   -G gallery.lis\n",  PROGRAM );
fprintf( fp, "        %s [options] -g galleryfile.xyt    probe*.xyt\n",   PROGRAM );
fprintf( fp, "        %s [options] -g galleryfile.xyt -P probes.lis\n",   PROGRAM );
fprintf( fp, "\n" );
fprintf( fp, "General options:\n" );
fprintf( fp, "   -h                      print this help message and exit\n" );
fprintf( fp, "   -v                      enable verbose mode\n" );

#ifdef TESTING
fprintf( fp, "   -i                      initialize globals to 0x00 before every match score computation\n" );
fprintf( fp, "   -ii                     initialize globals to 0xFF before every match score computation\n" );
#endif

fprintf( fp, "\n" );
fprintf( fp, "Input options:\n" );
fprintf( fp, "   -m1                     all xyt files use representation according to ANSI INCITS 378-2004\n");
fprintf( fp, "   -n <max-minutiae>       set maximum number of munitiae to use from any file [%d]; legal range is [%d,%d]\n",
								DEFAULT_BOZORTH_MINUTIAE,
								MIN_BOZORTH_MINUTIAE,
								MAX_BOZORTH_MINUTIAE );
fprintf( fp, "   -A parameter=<value>\n" );
fprintf( fp, "          minminutiae=#    set minimum number of munitiae for match score to be more than 0 [%d]\n",
								MIN_COMPUTABLE_BOZORTH_MINUTIAE );
fprintf( fp, "          maxfiles=#       set maximum number of files in any gallery, probe, or mates list file [%d]\n",
								MAX_FILELIST_LENGTH );
fprintf( fp, "          plines=#-#       process a subset of files in the probe file\n" );
fprintf( fp, "          glines=#-#       process a subset of files in the gallery file\n" );
fprintf( fp, "          dryrun           only print the filenames between which match scores would be computed\n" );
fprintf( fp, "\n" );
fprintf( fp, "Thresholding options:\n" );
fprintf( fp, "   -T <threshold>          set match score threshold\n" );
fprintf( fp, "   -q                      quit processing the probe file when a gallery file is found that meets the match score threshold\n" );

#ifdef PARALLEL_SEARCH
fprintf( fp, "   -A wfd=<fd>             write to this file descriptor when the probe file has been found in the gallery with match score threshold\n" );
fprintf( fp, "   -A rfd=<fd>             monitor this file descriptor for readability indicating that the probe file has been found in the gallery\n" );
#endif

fprintf( fp, "\n" );
fprintf( fp, "Output options:\n" );
fprintf( fp, "   -A nooutput             compute match scores, but don't print them\n" );
fprintf( fp, "   -A outfmt=[spg]*        output lines will contain (s)core, (p)robe and/or (g)allery filename [%s]\n",
								DEFAULT_SCORE_LINE_FORMAT );
fprintf( fp, "   -D <score-dir>          set the directory to write score files in\n" );
fprintf( fp, "   -o <score-file>         set the filename to store scores in\n" );
fprintf( fp, "   -e <stderr-file>        set the filename to store other output\n" );
fprintf( fp, "   -b                      use Standard I/O default buffering to print the match scores\n" );
fprintf( fp, "   -l                      use line-buffering to print the match scores\n" );
}
