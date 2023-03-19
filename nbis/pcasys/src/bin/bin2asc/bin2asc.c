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

      FILE:     BIN2ASC.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.
      UPDATED:  02/04/2009 by Joseph C. Konczal - include string.h

#cat: bin2asc - Reads a PCASYS binary data file of any type and
#cat:           writes a corresponding ascii data file.

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <datafile.h>
#include <usagemcs.h>
#include <memalloc.h>
#include <util.h>
#include <version.h>


int main(int argc, char *argv[])
{
  FILE *fp_in;
  char *binary_data_in, *ascii_data_out, file_type, asc_or_bin,
    codes_line[5];
  static char desc[DESC_DIM], *desc2;
  int i, j, dim1, dim2;
  float *the_floats;
  char **long_classnames;
  unsigned char *the_classes;

  if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
     getVersion();
     exit(0);
  }

  Usage("<binary_data_in> <ascii_data_out>");
  binary_data_in = argv[1];
  ascii_data_out = argv[2];
  if(!(fp_in = fopen(binary_data_in, "rb")))
    fatalerr("bin2asc", "fopen for reading failed", binary_data_in);
  for(i = 0; i < DESC_DIM && (j = getc(fp_in)) != '\n'; i++) {
    if(j == EOF)
      fatalerr("bin2asc", "input ends partway through description \
field", binary_data_in);
    desc[i] = j;
  }
  if(i == DESC_DIM)
    fatalerr("bin2asc", "description too long", binary_data_in);
  desc[i] = 0;
  fgets(codes_line, 5, fp_in);
  sscanf(codes_line, "%c %c", &file_type, &asc_or_bin);
  if(asc_or_bin != PCASYS_BINARY_FILE)
    fatalerr("bin2asc", "not a PCASYS ascii file", binary_data_in);
  fclose(fp_in);

  if(file_type == PCASYS_MATRIX_FILE) {
    matrix_read(binary_data_in, &desc2, &dim1, &dim2, &the_floats);
    matrix_write(ascii_data_out, desc2, 1, dim1, dim2, the_floats);
    free(the_floats);
    free(desc2);
  }
  else if (file_type == PCASYS_COVARIANCE_FILE) {
    covariance_read(binary_data_in, &desc2, &dim1, &dim2, &the_floats);
    covariance_write(ascii_data_out, desc2, 1, dim1, dim2, the_floats);
    free(the_floats);
    free(desc2);
  }
  else if(file_type == PCASYS_CLASSES_FILE) {
    classes_read_ind(binary_data_in, &desc2, &dim1, &the_classes, &dim2,
                     &long_classnames);
    classes_write_ind(ascii_data_out, desc2, 1, dim1, the_classes,
                      dim2, long_classnames);
    free(the_classes);
    free(desc2);
    free_dbl_char(long_classnames, dim2);
  }
  else
    fatalerr("bin2asc", "illegal file type code", binary_data_in);

  exit(0);
}
