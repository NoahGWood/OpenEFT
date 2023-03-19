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

      FILE:     STACKMS.C

      AUTHORS:  Craig Watson
                cwatson@nist.gov
                G. T. Candela
      DATE:     08/01/1995
      UPDATED:  05/09/2005 by MDG
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.

#cat: stackms - Combines input matrices into one file.

Stacks "matrix" files: produces a file whose matrix has, as its
rows, the rows of all the input matrices. All input matrices must have
the same second dimension.  (If a user needs to run lintran on a
large set of input vectors, and has multiple processors available,
user may be able to save time by first running several simulataneous
instances of lintran.exe, each transforming a subset of the input
vectors, and then using the current program to combine the resulting
blocks of output vectors.)

*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <usagemcs.h>
#include <datafile.h>
#include <util.h>
#include <version.h>

int main(int argc, char *argv[])
{
  FILE *fp_in, *fp_out;
  char *matrixfile_out, *matrixfile_out_desc,
    *ascii_outfile, *messages, *the_matrixfile_out_desc, *cp,
    str[100];
  int ascii_out = 0, ascii_in, messages_int = 0, first_dim2, sum_dim1s, dim1,
    dim2, iarg, ijunk, i, old_message_len = 0;
  float *rowbuf;

  if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
     getVersion();
     exit(0);
  }

  if(argc < 6)
    usage("<matrixfile_in[matrixfile_in...]> <matrixfile_out>\n\
<matrixfile_out_desc> <ascii_outfile> <messages>");
  matrixfile_out = argv[argc - 4];
  matrixfile_out_desc = argv[argc - 3];
  ascii_outfile = argv[argc - 2];
  messages = argv[argc - 1];
  if(!strcmp(ascii_outfile, "y"))
    ascii_out = 1;
  else if(!strcmp(ascii_outfile, "n"))
    ascii_out = 0;
  else
    fatalerr("stackms", "ascii_outfile must be y or n", NULL);
  if(!strcmp(messages, "y"))
    messages_int = 1;
  else if(!strcmp(messages, "n"))
    messages_int = 0;
  else
    fatalerr("stackms", "messages must be y or n", NULL);
  matrix_read_dims(argv[1], &dim1, &first_dim2);
  sum_dim1s = dim1;
  for(iarg = 2; iarg < argc - 4; iarg++) {
    matrix_read_dims(argv[iarg], &dim1, &dim2);
    sum_dim1s += dim1;
    if(dim2 != first_dim2) {
      sprintf(str, "second dim, %d, of input matrix %s, does not \
equal second dim, %d, of first input matrix %s", dim2, argv[iarg],
        first_dim2, argv[1]);
      fatalerr("stackms", str, NULL);
    }
  }
  if(!(rowbuf = (float *)malloc(first_dim2 * sizeof(float))))
    fatalerr("stackms", "malloc", "rowbuf");
  if(!strcmp(matrixfile_out_desc, "-")) {
    if(!(the_matrixfile_out_desc = malloc(strlen("Made by stackms \
from") + (argc - 5) * 200)))
      fatalerr("stackms", "malloc", "the_matrixfile_out_desc");
    strcpy(the_matrixfile_out_desc, "Made by stackms from");
    for(iarg = 1; iarg < argc - 4; iarg++) {
      strcat(the_matrixfile_out_desc, " ");
      strcat(the_matrixfile_out_desc, argv[iarg]);
      if(iarg < argc - 5)
	strcat(the_matrixfile_out_desc, ",");
    }
  }
  else
    the_matrixfile_out_desc = matrixfile_out_desc;
  matrix_writerow_init(matrixfile_out, the_matrixfile_out_desc,
    ascii_out, sum_dim1s, first_dim2, &fp_out);
  for(iarg = 1; iarg < argc - 4; iarg++) {
    if(messages_int) {
      for(i = 0; i < old_message_len; i++)
	printf("\b");
      sprintf(str, "Starting to read input file %d of %d", iarg,
        argc - 5);
      fputs(str, stdout);
      fflush(stdout);
      old_message_len = strlen(str);
    }
    matrix_readrow_init(argv[iarg], &cp, &ascii_in, &dim1, &ijunk,
      &fp_in);
    free(cp);
    for(i = 0; i < dim1; i++) {
      matrix_readrow(fp_in, ascii_in, first_dim2, rowbuf);    
      matrix_writerow(fp_out, ascii_out, first_dim2, rowbuf);
    }
    fclose(fp_in);
  }
  if(messages_int)
    printf("\n");

  exit(0);
}
