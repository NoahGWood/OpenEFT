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

      FILE:    AN2KTOOL.C

      AUTHOR:  Michael D. Garris
      DATE:    03/28/2000
      UPDATED: 05/09/2005 by MDG
      UPDATED: 01/31/2008 by Kenenth Ko
      UPDATED: 04/29/2008 by Joseph Konczal - Added more record
                          selection, and consolidated RFSI parsing and
                          error checking code into a function
                          parse_rfsi_component().
      UPDATED: 09/03/2008 by Kenneth Ko
      UPDATED: 09/30/2008 by Kenenth Ko - add version option.

#cat: an2ktool - Parses an ANSI/NIST 2007 file, manipulates
#cat:            its contents, and writes the results back out to file.
#cat:            Operations may be conducted at the logical record, field
#cat:            subfield, or information item.  Possible Operations include
#cat:            printing, deleting, substituting, or inserting data.

***********************************************************************/

#include <stdio.h>
#include <sys/param.h>
#include <an2k.h>
#include <version.h>

int parse_rfsi_component(const char *const, const char *const,
                         const char **, int *const);
void parse_rfsi_arg(int *const, int *const, int *const, int *const,
                    const char *const);
void print_usage(FILE *, const char *const);
void procargs(const int, char **, 
              int *const, int *const, int *const, int *const, int *const,
              const char **const, const char **const, const char **const);

/***********************************************************************/
int main(int argc, char *argv[])
{
   const char *ifile, *ofile;
   ANSI_NIST *ansi_nist;
   int ret;
   int op, record_i, field_i, subfield_i, item_i;
   const char *newvalue;

   /* Process the command line arguments. */
   procargs(argc, argv, &op, &record_i, &field_i,
            &subfield_i, &item_i, &newvalue, &ifile, &ofile);

   /* Read the ANSI_NIST file into memory. */
   if((ret = read_ANSI_NIST_file(ifile, &ansi_nist)))
      exit(ret);

   switch(op){
   case PRN_OP:
      /* Print the contents of the specified structure to the */
      /* specified file (or stdout).                          */
      ret = do_print(ofile, record_i, field_i, subfield_i, item_i, ansi_nist);
      break;
   case DEL_OP:
      /* Delete the specified structure and write the result out */
      /* to the specified file.                                  */
      ret = do_delete(ofile, record_i, field_i, subfield_i, item_i, ansi_nist);
      break;
   case SUB_OP:
      /* Replace the specified item value at the specified locaiton */
      /* and write the result out to the specified file.            */
      ret = do_substitute(ofile, record_i, field_i, subfield_i, item_i,
                          newvalue, ansi_nist);
      break;
   case INS_OP:
      /* Insert new item into specified location and write the */
      /* result out to the specified file.                     */
      ret = do_insert(ofile, record_i, field_i, subfield_i, item_i,
                      newvalue, ansi_nist);
      break;
   default:
      fprintf(stderr, "ERROR : main : "
              "operation -%c unsupported : ops = {d,i,p,s}\n", op);
      ret = -1;
      break;
   }

   free_ANSI_NIST(ansi_nist);
   exit(ret);
}

/***********************************************************************/
int parse_rfsi_component(const char *const sptr, const char *const name,
                         const char **eptr, int *const res)
{
   /* convert initial number */
   *res = strtol(sptr, (char **)eptr, 10);
   /* check what terminated the number */
   if (*eptr == sptr) {
      fprintf(stderr, "ERROR : parse_rfsi_component : "
              "%s may not be empty\n", name);
      exit(EXIT_FAILURE);
   }
   if (**eptr != '.' && **eptr != '\0') {
      fprintf(stderr, "ERROR : parse_rfsi_component : "
              "%s ending at '%s' is not numeric\n", name, *eptr);
      exit(EXIT_FAILURE);
   }
   if (*res <= 0) {
      fprintf(stderr, "ERROR : parse_rfsi_component : "
              "%s %d must be > 0\n", name, *res);
      exit(EXIT_FAILURE);
   }


   /* Structure indices are 1-oriented as specified by, and reported to, */
   /* to the user, but they are actually 0-oriented within the code.     */
   --*res;

   return (**eptr == '.');
}

/***********************************************************************/
void parse_rfsi_arg(int *const record_i, int *const field_i,
                    int *const subfield_i, int *const item_i,
                    const char *const arg)
{
   const char *sptr;
   int dot_found;

   *record_i = UNDEFINED_INT;
   *field_i = UNDEFINED_INT;
   *subfield_i = UNDEFINED_INT;
   *item_i = UNDEFINED_INT;

   /* If argument set to "all", then leave all indices UNDEFINED. */
   if(strcmp(arg, "all") == 0)
      return;

   sptr = arg;
   dot_found = parse_rfsi_component(sptr, "record index", &sptr , record_i);
   
   /* If '.' not found ... */
   if(!dot_found)
      /* Then at end of argument with only a record index specified. */
      return;
   
   /* Move start pointer to beginning character of next index. */
   sptr++;
   dot_found = parse_rfsi_component(sptr, "field ID", &sptr, field_i);

   if(!dot_found)
      /* Then at end of field index and end of the argument. */
      return;

   /* Now parse for subfield index. */         
   sptr++;
   dot_found = parse_rfsi_component(sptr, "subfield index", &sptr, subfield_i);

   if(!dot_found)
      /* Then at end of subfield index and end of the argument. */
      return;

   sptr++;
   dot_found = parse_rfsi_component(sptr, "item index", &sptr, item_i);

   if (dot_found)
      fprintf(stderr, "ERROR : unexpected data after item index: %s\n", sptr);
   
   return;
}

/***********************************************************************/
void print_usage(FILE *fp, const char *const pgrmname)
{
   fprintf(stderr, "\
Usage: %s \n\
       -print      <all|r[.f[.s[.i]]]>        <file in> [file out]\n\
       -delete     <r[.f[.s[.i]]]>            <file in> [file out]\n\
       -substitute <r.f.s.i>   <new value>    <file in> [file out]\n\
       -substitute <r[.f[.s]]> <fmttext file> <file in> [file out]\n\
       -insert     <r.f.s.i>   <new value>    <file in> [file out]\n\
       -insert     <r[.f[.s]]> <fmttext file> <file in> [file out]\n",
           pgrmname);
}

/***********************************************************************/
void procargs(const int argc, char **argv,
              int *const op, int *const record_i, int *const field_i,
              int *const subfield_i, int *const item_i,
              const char **const newvalue, const char **const ifile,
              const char **const ofile)
{
   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   if((argc < 4) || (*(argv[1]) != '-')){
      print_usage(stderr, argv[0]);
      exit(-1);
   }

   *op = *(argv[1]+1);
   parse_rfsi_arg(record_i, field_i, subfield_i, item_i, argv[2]);

   switch(*op){
   case PRN_OP:
   case DEL_OP:
      *newvalue = NULL;
      *ifile = argv[3];
      if (argc == 4)
         *ofile = NULL;
      else if (argc == 5)
         *ofile = argv[4];
      else {
         print_usage(stderr, argv[0]);
         exit(-1);
      }
      break;

   case SUB_OP:
   case INS_OP:
      *newvalue = argv[3];
      *ifile = argv[4];
      if (argc == 5)
         *ofile = NULL;
      else if (argc == 6) 
         *ofile = argv[5];
      else {
         print_usage(stderr, argv[0]);
         exit(-1);
      }
      break;

    default:
      fprintf(stderr, "Unsupported operation -%c\n", *op);
      print_usage(stderr, argv[0]);
      exit(-1);
   }
}
