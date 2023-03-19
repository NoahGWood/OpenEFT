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

      FILE:     DATAINFO.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.
      UPDATE:  12/02/2008 by Kenneth Ko - Fix to support 64-bit
      UPDATED:  02/04/2009 by Joseph C. Konczal - include string.h

#cat: datainfo - Dumps the "header" information for the PCASYS
#cat:            data file.

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <datafile.h>
#include <usagemcs.h>
#include <swap.h>
#include <util.h>
#include <version.h>

int main(int argc, char *argv[])
{
  FILE *fp;
  char *datafile, file_type, asc_or_bin, str[200], achar;
  int i, j, dim1, dim2, order, nvecs, n;

  if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

  Usage("<datafile>");
  datafile = *++argv;
  if(!(fp = fopen(datafile, "rb")))
    fatalerr("datainfo", "fopen for reading failed", datafile);
  printf("DESCRIPTION: ");
  for(i = 0; (j = getc(fp)) != '\n'; i++) {
    if(j == EOF) {
      printf("\n");
      fatalerr("datainfo", "file ends partway through description \
field", datafile);
    }
    putchar(achar = j);
  }
  putchar('\n');
  fgets(str, 200, fp);
  sscanf(str, "%c %c", &file_type, &asc_or_bin);
  printf("TYPE: ");
  if(file_type == PCASYS_MATRIX_FILE)
    printf("matrix\n");
  else if(file_type == PCASYS_COVARIANCE_FILE)
    printf("covariance\n");
  else if(file_type == PCASYS_CLASSES_FILE)
    printf("classes\n");
  else {
    printf("\n");
    fatalerr("datainfo", "illegal file-type code", datafile);
  }
  printf("ASCII OR BINARY: ");
  if(asc_or_bin == PCASYS_ASCII_FILE)
    printf("ascii\n");
  else if(asc_or_bin == PCASYS_BINARY_FILE)
    printf("binary\n");
  else {
    printf("\n");
    fatalerr("datainfo", "illegal ascii-or-binary code",
      datafile);
  }
  if(file_type == PCASYS_MATRIX_FILE) {
    if(asc_or_bin == PCASYS_ASCII_FILE)
      fscanf(fp, "%d %d", &dim1, &dim2);
    else { /* asc_or_bin == PCASYS_BINARY_FILE */
      fread(&dim1, sizeof(int), 1, fp);
      fread(&dim2, sizeof(int), 1, fp);
#ifdef __NBISLE__
      swap_int_bytes(dim1);
      swap_int_bytes(dim2);
#endif
    }
    printf("DIMENSIONS: %d by %d\n", dim1, dim2);
  }
  else if(file_type == PCASYS_COVARIANCE_FILE) {
    if(asc_or_bin == PCASYS_ASCII_FILE)
      fscanf(fp, "%d %d", &order, &nvecs);
    else { /* asc_or_bin == PCASYS_BINARY_FILE */
      fread(&order, sizeof(int), 1, fp);
      fread(&nvecs, sizeof(int), 1, fp);
#ifdef __NBISLE__
      swap_int_bytes(order);
      swap_int_bytes(nvecs);
#endif
    }
    printf("ORDER: %d\n", order);
    printf("NO. OF VECTORS USED: %d\n", nvecs);
  }
  else /* file_type == PCASYS_CLASSES_FILE) */ {
    if(asc_or_bin == PCASYS_ASCII_FILE)
      fscanf(fp, "%d", &n);
    else { /* asc_or_bin == PCASYS_BINARY_FILE */
      fread(&n, sizeof(int), 1, fp);
#ifdef __NBISLE__
      swap_int_bytes(n);
#endif
    }
    printf("NO. OF ENTRIES: %d\n", n);
  }

  exit(0);
}
