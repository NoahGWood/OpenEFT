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

      FILE:    AN2K2IAF.C

      AUTHOR:  Michael D. Garris
      DATE:    09/12/2000
      UPDATED: 05/09/2005 by MDG
      UPDATED: 01/31/2008 by Kenneth Ko
      UPDATED: 09/03/2008 by Kenneth Ko
      UPDATED: 09/30/2008 by Kenenth Ko - add version option.
	  
#cat: an2k2iaf - Takes an ANSI/NIST 2007 file, and converts
#cat:            any Type-9 records with NIST fields populated to Type-9's
#cat:            with FBI/IAFIS fields populated.  Where possible, it
#cat:            also converts tagged field image records into binary
#cat:            field records.

***********************************************************************/

#include <stdio.h>
#include <an2k.h>
#include <version.h>

void procargs(int, char **, char **, char **);

/*************************************************************************
**************************************************************************/
int main(int argc, char *argv[])
{
   char *ifile, *ofile;
   int ret;
   ANSI_NIST *ansi_nist;

   /* Process command line arguments. */
   procargs(argc, argv, &ifile, &ofile);

   /* Read in an ANSI/NIST file into memory. */
   if((ret = read_ANSI_NIST_file(ifile, &ansi_nist)))
      exit(ret);

   /* Convert Type-9 records. */
   if((ret = nist2iafis_type_9s(ansi_nist))){
      free_ANSI_NIST(ansi_nist);
      exit(ret);
   }

   /* Convert any Type-13&14 records to Type-4&6. */
   if((ret = nist2iafis_fingerprints(ansi_nist))){
      free_ANSI_NIST(ansi_nist);
      exit(ret);
   }

   /* Write out new ANSI/NIST file. */
   if((ret = write_ANSI_NIST_file(ofile, ansi_nist))){
      free_ANSI_NIST(ansi_nist);
      exit(ret);
   }

   /* Deallocate working memory. */
   free_ANSI_NIST(ansi_nist);

   /* Exit normally. */
   exit(0);
}

/*************************************************************************
**************************************************************************
   PROCARGS - Process command line arguments
   Input:
      argc  - system provided number of arguments on the command line
      argv  - system provided list of command line argument strings
   Output:
      ifile - input image file name to be processed by this program
      ofile - output image file name to be created by this program
**************************************************************************/
void procargs(int argc, char **argv, char **ifile, char **ofile)
{
   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   if(argc != 3){
      fprintf(stderr, "Usage : %s <file in><file out>\n", argv[0]);
      exit(-1);
   }

   *ifile = argv[1];
   *ofile = argv[2];
}
