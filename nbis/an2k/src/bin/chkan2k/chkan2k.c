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
#include <stdio.h>
#include <stdlib.h>

#ifdef TARGET_OS
#include <unistd.h>
#else
#include <getopt.h>
#endif

#include "an2k.h"
#include "version.h"
#include "chkan2k.h"

int debug = 0;			/* required by wsq decoder */

/*************************************************************************
**************************************************************************
   usage - Print a brief summary of command line options, then exit.

   Input:
      prog_name - program name, e.g., argv[0] 
**************************************************************************/
static void usage(const char *const prog_name)
{
   fprintf(stderr, "Usage: %s [options] an2k-input-files ...\n"
	   "\t-h\n"
	   "\t-b base-config-file\n"
	   "\t-c supplemental-config-file\n"
	   "\t-l log-level\n"
	   "\t-version\n",
	   prog_name);
   exit(EXIT_FAILURE);
}

/*************************************************************************
**************************************************************************
   procargs - Process command line arguments.  Exit on error by calling
              usage().

   Input:
      argc - number of arguments
      argv - array of argument strings
   Output:
      tbd
**************************************************************************/
static void procargs(int argc, char **argv, char** cfg_files,
		     const int cfg_files_max, int *cfg_count)
{
   int opt;
   const char *const option_spec = "b:c:hl:";
   int cfg_i = 1;

   if ((argc == 2) && strcmp(argv[1], "-version") == 0) {
      getVersion();
      exit(EXIT_SUCCESS);
   }

   while ((opt = getopt(argc, argv, option_spec)) != -1) {
      switch(opt) {
      case 'b':			/* override default config file */
	 if (1 != cfg_i) {
	    fprintf(stderr, "Override the default configuration first, "
		    "before adding to it.\n");
	 }
	 cfg_files[0] = optarg;
	 break;

     case 'c':			/* additional config file */
	 if (cfg_i == cfg_files_max) {
	    fprintf(stderr, "Too many configuration files, max=%d.\n", 
		    cfg_files_max);
	    exit(EXIT_FAILURE);
	 }
	 cfg_files[cfg_i++] = optarg;
	 break;

      case 'h':
	 usage(argv[0]);
	 break;

      case 'l':
	 if (set_log_level(optarg) < 0) {
	    exit(EXIT_FAILURE);
	 }
	 break;

       case '?':
	 usage(argv[0]);
	 break;

      default:
	 fprintf(stderr, "Programming error: "
		 "incompletely implemented option: '%c'.\n", opt);
	 exit(EXIT_FAILURE);
      }
   }
   if (optind == argc) {
      usage(argv[0]);
   }
   *cfg_count = cfg_i;
}

/*************************************************************************
**************************************************************************
   main - Entry point.

   Input:
      argc - number of command line arguments
      argv - array of string values of command line arguments
**************************************************************************/
int main(int argc, char **argv)
{
   int read_status, i;
   char *filename;
   const CAN_CONFIG *conf = NULL, *next_conf;
   CAN_CONTEXT file_stats, conf_stats, total_stats;
   ANSI_NIST *ansi_nist;
   char *config_files[MAX_CONFIG_FILES];
   int config_count;

   init_result_accumulator(&total_stats, "Grand Total");

   config_files[0] = DEFAULT_CONFIG_FILE;

   procargs(argc, argv, config_files, MAX_CONFIG_FILES, &config_count);

   for (i = 0; i < config_count; i++) {
      init_result_accumulator(&conf_stats, config_files[i]);
      next_conf = read_config(&conf_stats, conf);   
      if (NULL == next_conf) {
	 return EXIT_FAILURE;
      }
      aggregate_result_accumulator(&total_stats, &conf_stats);
      conf = next_conf;
   }

   for (i = optind; argv[i] != NULL; i++) {
      filename = argv[i];
      init_result_accumulator(&file_stats, filename);

      read_status = read_ANSI_NIST_file(filename, &ansi_nist);
      if(read_status != 0) {
	 ++file_stats.issue[LOGTP_EXEC-LOGTP_BASE][LOGL_ERROR-LOGL_BASE];

      } else {
	 check_ansi_nist(conf, ansi_nist, &file_stats);
	 free_ANSI_NIST(ansi_nist);
      }
      aggregate_result_accumulator(&total_stats, &file_stats);
   }

   if (optind+1 < argc) {
      report_result_accumulator(conf, &total_stats, TRUE);
   }

   return EXIT_SUCCESS;
}
