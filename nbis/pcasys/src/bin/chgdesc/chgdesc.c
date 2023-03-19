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

      FILE:     CHGDESC.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.
      UPDATED:  02/04/2009 by Joseph C. Konczal - include string.h

#cat: chgdesc - Given a PCASYS data file this command replaces its
#cat:           description.  The program makes a temporary copy of
#cat:           the file.

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <usagemcs.h>
#include <util.h>
#include <version.h>

int main(int argc, char *argv[])
{
  FILE *fp_in, *fp_out;
  char *datafile, *new_desc, tempfile[200], *p, achar, str[400];
  int i;

  if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
     getVersion();
     exit(0);
  }

  Usage("<datafile> <new_desc>");
  datafile = *++argv;
  new_desc = *++argv;
  for(p = new_desc; *p; p++)
    if(*p == '\n')
      fatalerr("chgdesc", "new description contains a newline", NULL);
  sprintf(tempfile, "%s_chgdesc%d", datafile, getpid());
  if(exists(tempfile)) {
    sprintf(str, "temporary file that must be produced, %s, already \
exists; exiting rather than clobbering that file", tempfile);
    fatalerr("chgdesc", str, NULL);
  }
  if(!(fp_out = fopen(tempfile, "wb")))
    fatalerr("chgdesc", "fopen for writing failed", tempfile);
  fprintf(fp_out, "%s\n", new_desc);
  if(!(fp_in = fopen(datafile, "rb"))) {
    fclose(fp_out);
    unlink(tempfile);
    fatalerr("chgdesc", "fopen for reading failed", datafile);
  }
  while((i = getc(fp_in)) != '\n')
    if(i == EOF) {
      fclose(fp_out);
      unlink(tempfile);
      fatalerr("chgdesc", "file ends partway through description \
field", datafile);
    }
  while(fread(&achar, sizeof(char), 1, fp_in) == 1)
    fwrite(&achar, sizeof(char), 1, fp_out);
  fclose(fp_in);
  fclose(fp_out);
  remove(datafile);  /*Win platform must remove before rename*/
  if(rename(tempfile, datafile)) {
    sprintf(str, "rename of %s to %s failed", tempfile, datafile);
    syserr("chgdesc", str, NULL);
  }

  exit(0);
}
