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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <errno.h>
#include <ctype.h>
#include <sys/stat.h>

#ifdef TARGET_OS
#include <unistd.h>
#else
#include <getopt.h>
#endif

#include "an2k.h"
#include "chkan2k.h"

/* Certain data structures are allocated in chunks as more space is needed.
   This defines the size of those chunks. */
#define CAN_CHUNK 10

/* Declare any functions that might be used before they are defined. */
static int
parse_item_ext(CAN_CONTEXT *const, const CAN_TOKEN *const, CAN_CONFIG *const,
	       const FIELD_SPEC *const, ITEM_SPEC **);

/*************************************************************************
**************************************************************************
   checked_realloc - A single multi-purpose memory allocation function,
                used for almost all memory allocation needs of this program
                code, excluding independent allocation done by any
                libraries used.

   Input:
      ctx    - context information, e.g., input file, line number, etc.
      prev   - Pointer to storage being reallocated, or NULL for new space.
      name   - Used in error messages to indicate what the memory was for.
      size   - Size of a single instance of the kind of item to be allocated.
      count  - Number of items already allocated.
      incr   - Number of new items to allocate.
   Output:
      ctx    - updated statistics.
   Return:
      NULL      - logging a fatal error exits the program, otherwise NULL 
                  would indicate an error
      non-NULL  - success, location of new heap space
**************************************************************************/
static void *
checked_realloc(CAN_CONTEXT *const ctx, void *prev, const char *const name,
		const size_t size, const int count, const int incr)
{
   void *buf;
   
   buf = realloc(prev, size*(count+incr));
   if (NULL == buf) {
      if (NULL == prev) {
	 can_log(LOGL_FATAL, LOGTP_EXEC, (CAN_CONFIG *)NULL, ctx,
		 "Cannot allocate %d %s items (%d bytes): %s.\n",
		 incr, name, (unsigned long)(size*incr), strerror(errno));
      } else {
	 can_log(LOGL_FATAL, LOGTP_EXEC, (CAN_CONFIG *)NULL, ctx,
		 "Cannot increase %s allocation from %d items to %d "
		 "(%lu bytes): %s.\n", name, count, count+incr,
		 (unsigned long)(size * (count + incr)), strerror(errno));
      }
   } else {
      if (NULL == prev) {
	 can_log(LOGL_DEBUG, LOGTP_EXEC, (CAN_CONFIG *)NULL, ctx,
	      "Creating '%s' allocation for %d items (%lu bytes).\n",
	      name, count+incr, (unsigned long)(size * (count + incr)));

      } else {
	 can_log(LOGL_DEBUG, LOGTP_EXEC, (CAN_CONFIG *)NULL, ctx,
	      "Increasing '%s' allocation from %d items to %d (%lu bytes).\n",
	      name, count, count+incr, (unsigned long)(size * (count + incr)));
      }
   }
   
   return buf;
}

/*************************************************************************
**************************************************************************
   checked_alloc - A streamlined way to call checked_realloc() to obtain a
                newly allocated segment of memory instead of increasing the
                size of one previously obtained.

   Input:
      ctx    - context information, e.g., input file, line number, etc.
      name   - Used in error messages to indicate what the memory was for.
      size   - Size of a single item to be allocated.
      incr   - Number of new items to allocate.
   Output:
      ctx    - updated statistics.
   Return:
      NULL      - logging a fatal error exits the program, otherwise NULL 
                  would indicate an error
      non-NULL  - success, location of new heap space
**************************************************************************/
static void *
checked_alloc(CAN_CONTEXT *const ctx, const char *const name,
	      const size_t size, const int incr)
{
   return checked_realloc(ctx, (void *)NULL, name, size, 0, incr);
}

/*************************************************************************
**************************************************************************
   DEFINE_ALLOC_FUNCTION - a function-like macro that defines alloc_<NAME>
                functions to allocate memory for certain types of
                structures specified by <TYPE>, using the names specified
                by <NAME>.  There should be no semicolon after an
                invocation of this macro.

   Input:
      TYPE    - the data type to allocate, a pointer to that type is returned
      NAME    - name of the thing to allocate, i.e., the type name without
                the prefix and lower cased
   Output:
      a function named: alloc_<NAME>
**************************************************************************/

/*************************************************************************
**************************************************************************
   alloc_<NAME> - Allocate memory for a <TYPE> kind of structure, whose
                lower-case <NAME>, without any prefix, appears in place of
                the function name.  Initialize it all with zeros.

   Input:
      ctx    - context information, e.g., input file, line number, etc.
   Output:
      ctx    - updated statistics.
   Return:
      NULL      - logging a fatal error exits the program, otherwise NULL 
                  would indicate an error
      non-NULL  - success, location of new heap space for the structure
**************************************************************************/

#define DEFINE_ALLOC_FUNCTION(TYPE, NAME)				\
   static TYPE *							\
   alloc_##NAME(CAN_CONTEXT *ctx)					\
   {									\
      TYPE *new = (TYPE *)checked_alloc(ctx, #NAME, sizeof(TYPE), 1);	\
      if (NULL != new) {						\
	 memset((void *)new, 0, sizeof(TYPE));				\
      }									\
      return new;							\
   }									\

DEFINE_ALLOC_FUNCTION(CAN_CONFIG,    config)
DEFINE_ALLOC_FUNCTION(CAN_STANDARD,  standard)
DEFINE_ALLOC_FUNCTION(RECORD_SPEC,   record)
DEFINE_ALLOC_FUNCTION(FIELD_SPEC,    field)
DEFINE_ALLOC_FUNCTION(ITEM_SPEC,     item)
DEFINE_ALLOC_FUNCTION(CAN_LIST,      list)
DEFINE_ALLOC_FUNCTION(ITEM_SPEC_VAL, val)
DEFINE_ALLOC_FUNCTION(CAN_OPTION,    option)
#undef DEFINE_ALLOC_FUNCTION

/*************************************************************************
**************************************************************************
   free_standard - Free the memory used by a CAN_STANDARD structure, and
                all members that point to other memory used only by this
                stucture.  Assumes that unused pointers are NULL, which can
                be freed without trouble.

   Input: std - pointer to the structure to be freed
**************************************************************************/
static void
free_standard(CAN_STANDARD *std)
{
   if (NULL != std) {
      /* fields that are not set should be NULL, which can be freed */
      free(std->tag);
      free(std->name);
      free(std->ref);
      free(std->date);
      /* the parent standard exists independently and should not be freed */
      free(std);
   }
}

/*************************************************************************
**************************************************************************
   free_record - Free the memory used by a RECORD_SPEC structure, and all
                members that point to other memory used only by this
                stucture.  Assumes that unused pointers are NULL, which can
                be freed without trouble.

   Input: rec - pointer to the structure to be freed
**************************************************************************/
static void
free_record(RECORD_SPEC *rec)
{
   if (NULL != rec) {
      /* fields that are not set should be NULL, which can be freed */
      free(rec->fields);      /* free the pointers, but not the pointees */
      free(rec);
   }
}

/*************************************************************************
**************************************************************************
   free_field - Free the memory used by a FIELD_SPEC structure, and all
                members that point to other memory used only by this
                stucture.  Assumes that unused pointers are NULL, which can
                be freed without trouble.

   Input: fld - pointer to the structure to be freed
**************************************************************************/
static void
free_field(FIELD_SPEC *fld)
{
   if (NULL != fld) {
      free(fld->tag);
      free(fld->records);     /* free the pointers, but not the pointees */
      free(fld->items);	      /* free the pointers, but not the pointees */
      free(fld);
   }
}

/*************************************************************************
**************************************************************************
   free_list - Free the memory used by a CAN_LIST structure, and all
                members that point to other memory used only by this
                stucture.  Assumes that unused pointers are NULL, which can
                be freed without trouble.

   Input: list - pointer to the structure to be freed
**************************************************************************/
static void
free_list(CAN_LIST *list)
{
   if (NULL != list) {
      free(list->vals);
      free(list);
   }
}

/*************************************************************************
**************************************************************************
   free_item - Free the memory used by a ITEM_SPEC structure, and all
                members that point to other memory used only by this
                stucture.  Assumes that unused pointers are NULL, which can
                be freed without trouble.

   Input: itm - pointer to the structure to be freed
**************************************************************************/
static void
free_item(ITEM_SPEC *itm)
{
   free_list(itm->enum_vals);
   free(itm);
}

/*************************************************************************
**************************************************************************
   free_val - Free the memory used by a ITEM_SPEC_VAL structure, and all
                members that point to other memory used only by this
                stucture.  Assumes that unused pointers are NULL, which can
                be freed without trouble.

   Input: val - pointer to the structure to be freed
**************************************************************************/
static void
free_val(ITEM_SPEC_VAL *val)
{
   if (NULL != val) {
      free(val->str);
      free(val);
   }
}

/*************************************************************************
**************************************************************************
   free_option - Free the memory used by a CAN_OPTION structure, and all
                members that point to other memory used only by this
                stucture.  Assumes that unused pointers are NULL, which can
                be freed without trouble.

   Input: cfg - pointer to the structure to be freed
**************************************************************************/
static void
free_option(CAN_OPTION *opt)
{
   if (NULL != opt) {
      free(opt->name);

      free(opt);
   }
}

/*************************************************************************
**************************************************************************
   free_config - Free the memory used by a CAN_CONFIG structure, and all
                members that point to other memory used only by this
                stucture.  Assumes that unused pointers are NULL, which can
                be freed without trouble.

   Input: cfg - pointer to the structure to be freed
**************************************************************************/
static void
free_config(CAN_CONFIG *cfg)
{
   int i;

   if (NULL != cfg) {
#define FREE_EVERY(WHAT)			\
      for (i = 0; i < cfg->num_##WHAT##s; i++) {	\
	 free_##WHAT(cfg->WHAT##s[i]);		\
      }						\
      free(cfg->WHAT##s)

      FREE_EVERY(standard);
      FREE_EVERY(record);
      FREE_EVERY(field);
      FREE_EVERY(item);
      FREE_EVERY(list);
#undef FREE_EVERY      
      free(cfg);
   }
}


/*************************************************************************
**************************************************************************
   DEFINE_MORE_PTRS_FUNCTION - a function-like macro that defines functions
                to allocate or reallocate different kinds of pointers to
                type <TYPE> within the base structure type <BTYP>.  These
                are used by the store_<BNAM>_<NAME> function and some are
                also called elsewhere in this configuration file parsing
                code.

                The base type <BTYP> should include integer fields named
                num_<NAME>s and alloc_<NAME>s, and an array of <TYPE>**
                named <NAME>s, which are use to hold, respectively, the
                number of pointers stored, the number of pointers
                allocated, and the pointers themselves.

                There should be no semicolon after an invocation of this
                macro.
   
  Input:
     BTYP - base type of structure, i.e., the one that contains the table
            of pointers in which to store the data
     BNAM - short, lower case version of the name of the base type, used in
            generating the function name and error messages
     TYPE - type of structure of interest, whose pointers we want to store
     NAME - non-prefixed, lower case version of the type name of the
            structure of interest, which appears in field names in the base
            type.
   Output:
     a function named: more_<BNAM>_<NAME>_ptrs
**************************************************************************/

/*************************************************************************
**************************************************************************
   more_<BNAM>_<NAME>_ptrs - Make more space in the specified <BTYP>
                structure for pointers of the type <TYPE>, whose
                lower-case, non-prefixed name appears in <NAME>.

   Input:
      bp      - pointer to a <BTYP> structure in which to increase
                the space allocated to store <TYPE> pointers
   Return:
      zero     - success
      negative - failure
**************************************************************************/

#define DEFINE_MORE_PTRS_FUNCTION(BTYP, BNAM, TYPE, NAME)		\
   static int								\
   more_##BNAM##_##NAME##_ptrs(BTYP *const bp, const int incr)     \
   {									\
      TYPE **new_space;							\
									\
      new_space = (TYPE **)checked_realloc				\
	 ((CAN_CONTEXT *)NULL, (void *)bp->NAME##s, #BNAM" "#NAME" pointers",\
 	  sizeof(TYPE *), bp->alloc_##NAME##s, incr);			\
      if (NULL != new_space) {						\
	 bp->NAME##s = new_space;					\
	 bp->alloc_##NAME##s += incr;					\
	 return 0;							\
      } else {								\
	 return -__LINE__;						\
      }									\
   }

DEFINE_MORE_PTRS_FUNCTION(CAN_CONFIG,  cfg, CAN_STANDARD,  standard)
DEFINE_MORE_PTRS_FUNCTION(CAN_CONFIG,  cfg, RECORD_SPEC,   record)
DEFINE_MORE_PTRS_FUNCTION(CAN_CONFIG,  cfg, FIELD_SPEC,    field)
DEFINE_MORE_PTRS_FUNCTION(CAN_CONFIG,  cfg, ITEM_SPEC,     item)
DEFINE_MORE_PTRS_FUNCTION(CAN_CONFIG,  cfg, CAN_LIST,      list)
DEFINE_MORE_PTRS_FUNCTION(RECORD_SPEC, rec, FIELD_SPEC,    field)
DEFINE_MORE_PTRS_FUNCTION(FIELD_SPEC,  fld, RECORD_SPEC,   record)
DEFINE_MORE_PTRS_FUNCTION(FIELD_SPEC,  fld, ITEM_SPEC,     item)
DEFINE_MORE_PTRS_FUNCTION(CAN_LIST,    lst, ITEM_SPEC_VAL, val)
DEFINE_MORE_PTRS_FUNCTION(CAN_CONFIG,  cfg, CAN_OPTION,    option)
#undef DEFINE_MORE_PTRS_FUNCTION

/*************************************************************************
**************************************************************************
   tok_str_cmp - Analogous to strcmp, but compares the value of a CAN_TOKEN
                object with a string.  defined in the configuration file.
                Unline strcmp, however, it does not return positive or
                negative values to indicate ordering, only zero to indicate
                a match and nonzero to indicate a non-match.

   Input:
      tok   - a pointer to the token to compare with the string
      str   - a pointer to the character string to compare with the token
   Return:
      zero     - indicates a match
      non-zero - indicates a non-match
**************************************************************************/
static int
tok_str_cmp(const CAN_TOKEN *const tok, const char *const str)
{
   return (strlen(str) != tok->end-tok->val)
      || (strncmp(tok->val, str, tok->end-tok->val) != 0);
}


extern ITEM_SPEC_TYPE_DESC item_types[];
extern int num_item_types;

/*************************************************************************
**************************************************************************
   item_type_from_tok - Compare the token value string with the defined
                item types and return a pointer to the descriptor of the
                one that matches.

   Input:
      tok  - a pointer to the token containing the item type name to look up
   Return:
      NULL     - failure
      non-NULL - success, and address of the item type descriptor
**************************************************************************/
static const ITEM_SPEC_TYPE
item_type_from_tok(const CAN_TOKEN *const tok)
{
   int i;

   for(i = 0; i < num_item_types; i++) {
      if (tok_str_cmp(tok, item_types[i].name) == 0) {
	 return item_types[i].type;
      }
   }
   return 0;
}

/*************************************************************************
**************************************************************************
   token_type_name - Return a printable string corresponding to the token
                type of the given token, or a printable error string.
                Designed for easy use in error messages, with no need to
                check the return value.

   Input:
      tok - a token whose type is to be expressed as a string
   Return: 
      a printable string corresponding to the given token type identifier,
      or a printable indication that it is not defined
**************************************************************************/
static const char *
token_type_name(const CAN_TOKEN *const tok)
{
   switch(tok->type) {
   case TOK_SIMPLE:  return "simple";
   case TOK_STRING:  return "string";
   case TOK_LIST:    return "list";
   case TOK_ITEM:    return "item";
   case TOK_EMPTY:   return "empty";
   case TOK_COMMENT: return "comment";
   default:          return "<undefined>";
   }
}

/*************************************************************************
**************************************************************************
   str_from_tok - Allocate a new string large enough to hold the data
                pointed to by the given token, and copy the data into it.

   Input:
      tok  - a pointer to the token containing the string to copy
   Return:
      NULL     - failure
      non-NULL - success, and address of the new string
**************************************************************************/
static char *
str_from_tok(const CAN_TOKEN *const tok)
{
   char *str;
   str = (char *)checked_alloc((CAN_CONTEXT *)NULL, "string", sizeof(char),
			       tok->end-tok->val+1);
   if (NULL != str) {
      strncpy(str, tok->val, tok->end-tok->val);
      str[tok->end-tok->val] = '\0';
   }
   return str;
}

/*************************************************************************
**************************************************************************
   num_from_tok - Convert the value of a token to a positive integer or
                zero, or return a negative number to indicate an error.

   Input:
      ctx       - context information, e.g., input file, line number, etc.      
      tok       - a pointer to the token containing the integer to convert
   Output:
      ctx       - updated statistics
   Return:
      zero - success
      negative - failure
**************************************************************************/
static int
num_from_tok(CAN_CONTEXT *const ctx, const CAN_TOKEN *const tok,
	     int *const result)
{
   char *end;
   long lnum;

   errno = 0;
   lnum = strtol(tok->val, &end, 10);
   if ((0 != errno) || (tok->end != end)) {
      return -__LINE__;
   }
   if ((lnum < INT_MIN) || (lnum > INT_MAX)) {
      can_log(LOGL_ERROR, LOGTP_CONFIG, (CAN_CONFIG *)NULL, ctx,
	      "Number %ld out of integer range [%d, %d]\n",
	      lnum, INT_MIN, INT_MAX);
      return -__LINE__;
   }
   *result = (int)lnum;
   return 0;
}

/*************************************************************************
**************************************************************************
   log_parse_error - Log an indication that the most recently scanned token
                could not be parsed as expected.

   Input:
      cfg      - pointer to a configuration structure in use
      ctx      - context information, e.g., input file, line number, etc.
      tok      - the object being parsed
      what     - name of the thing parsed here, e.g., "lower limit".
      more     - more information to append
   Output:
      ctx      - updated statistics
**************************************************************************/
static void
log_parse_error(const CAN_CONFIG *const cfg, CAN_CONTEXT *const ctx,
		const CAN_TOKEN *const tok, const char *const what,
		const char *const more)
{
      can_log(LOGL_WARNING, LOGTP_CONFIG, cfg, ctx,
	      "Cannot parse %s: "TOK_FMT"%s%s\n", what, TOK_ARGS(tok),
	      NULL != more ? ": " : "", NULL != more ? more : "");
}

/*************************************************************************
**************************************************************************
   log_bad_value - Log an indication that the value of the most recently
                scanned token was not acceptable.

   Input:
      cfg      - pointer to a configuration structure in use
      ctx      - context information, e.g., input file, line number, etc.
      tok      - the object being parsed
      what     - name of the thing parsed here, e.g., "lower limit".
      more     - more information to append
   Output:
      ctx      - updated statistics
**************************************************************************/
static void
log_value_error(const CAN_CONFIG *const cfg, CAN_CONTEXT *const ctx,
		const CAN_TOKEN *const tok, const char *const what,
		const char *const more)
{
      can_log(LOGL_WARNING, LOGTP_CONFIG, cfg, ctx,
	      "Invalid %s: "TOK_FMT"%s%s\n", what, TOK_ARGS(tok),
	      NULL != more ? ": " : "", NULL != more ? more : "");
}

/*************************************************************************
**************************************************************************
   set_limits_from_tok - Convert the value of a token to a numerical range.
                The string value may contain a single number of asterisk,
                which is used to set both limits to the specified value, or
                to UNSET (-1), or a pair of numbers or asterisks separated
                by a dash, which are used to set the minimum and maximum
                values respectively.

   Input:
      ctx       - context information, e.g., input file, line number, etc.
      tok       - a pointer to the token containing the integer to convert
      quiet     - report debug messages instead of errors when input is
                  not numerical
   Output:
      ctx       - updated statistics
      lim       - min and max limit values
   Return:
      zero      - success
      negative  - failure
**************************************************************************/
static int
set_limits_from_tok(CAN_CONTEXT *const ctx, const CAN_TOKEN *const tok, 
		    CAN_LIMITS *const lim, const int quiet)
{
   char *dash;
   CAN_TOKEN subtok[2];

   dash = strchr(tok->val, '-');
   if (dash < tok->val+1 || dash > tok->end) { /* single value */
      if (tok_str_cmp(tok, "*") == 0) {
	 lim->min = lim->max = UNSET;
      } else {
	 if (num_from_tok(ctx, tok, &lim->min) != 0) {
	    if (quiet == 0) {
	       log_parse_error((CAN_CONFIG *)NULL, ctx, tok, "numerical limit",
			       errno > 0 ? strerror(errno) : "Not numeric");
	    }
	    return -__LINE__;
	 }
	 lim->max = lim->min;
      }
      
   } else {			/* range of values */
      subtok[0].val = tok->val;
      subtok[0].end = dash;
      subtok[1].val = dash+1;
      subtok[1].end = tok->end;
      if (tok_str_cmp(&subtok[0], "*") == 0) {
	 lim->min = UNSET;
      } else {
	 if (num_from_tok(ctx, &subtok[0], &lim->min) != 0) {
	    if (quiet == 0) {
	       log_parse_error((CAN_CONFIG *)NULL, ctx, &subtok[0], 
			       "lower limit", strerror(errno));
	    }
	    return -__LINE__;
	 }
      }
      if (tok_str_cmp(&subtok[1], "*") == 0) {
	 lim->max = UNSET;
      } else {
	 if (num_from_tok(ctx, &subtok[1], &lim->max) != 0) {
	    if (quiet == 0) {
	       log_parse_error((CAN_CONFIG *)NULL, ctx, &subtok[1],
		       "upper limit", strerror(errno));
	    }
	    return -__LINE__;
	 }
      }
   }
   return 0;
}

/*************************************************************************
**************************************************************************
   fp_from_tok - Convert the value of a token to a floating point number.

   Input:
      ctx    - context information, e.g., input file, line number, etc.
      tok    - a pointer to the token containing the floating point number
   Output:
      ctx    - updated statistics
      result - a pointer to a double to hold the result if successful
   Return:
      zero     - success
      non-zero - failure
**************************************************************************/
static int
fp_from_tok(CAN_CONTEXT *const ctx, const CAN_TOKEN *const tok,
	    double *const result)
{
   char *end;

   errno = 0;
   *result = strtod(tok->val, &end);
   if ((errno != 0) || (tok->end != end)) {
      return -__LINE__;
   }
   return 0;
}

/*************************************************************************
**************************************************************************
   rdt_enum_from_tok - Examine a token that should indicate the data type
                of a record, and return this code module's corresponding
                integer code, or a negative number indicating an error.

   Input:
      ctx    - context information, e.g., input file, line number, etc.
      tok    - a pointer to the token containing the data type string
   Output:
      ctx    - updated statistics
   Return:
      positive - success, local data type code
      negative - failure
**************************************************************************/
static CAN_REC_DATA_TYPE
rdt_enum_from_tok(CAN_CONTEXT *const ctx, const CAN_TOKEN *tok)
{
   if (tok_str_cmp(tok, "ASCII") == 0)
      return RDT_ASCII;
   else if (tok_str_cmp(tok, "Binary") == 0)
      return RDT_BINARY;
   else if (tok_str_cmp(tok, "ASCII/Binary") == 0)
      return RDT_ASCBIN;

   can_log(LOGL_ERROR, LOGTP_CONFIG, (CAN_CONFIG *)NULL, ctx,
	   "Unknown record data format type "TOK_FMT".\n", TOK_ARGS(tok));
   return -__LINE__;
}


/*************************************************************************
**************************************************************************
   rdt_str_from_enum - Take an enumeration value and return a corresponding
                string.  Suitable for use in error messages without error
                checking the result.

   Input:
      rdt    - an enumerated record data type
   Return:
      Pointer to a character string indicating the type of data in a
      record, or "Unknown" if the input does not correspond to any known
      record data type.
**************************************************************************/
static const char *
rdt_str_from_enum(const CAN_REC_DATA_TYPE rdt)
{
   if (rdt == RDT_ASCII)
      return "ASCII";
   else if (rdt == RDT_BINARY)
      return "Binary";
   else if (rdt == RDT_ASCBIN)
      return "ASCII/Binary";

   return "Unknown";
}

/*************************************************************************
**************************************************************************
    get_first_token - Take an input buffer and a pointer to a token
                structure, fill in the addresses of the beginning and end
                of the token, and identify the type of token.

   Input:
      ctx      - context information, e.g., input file, line number, etc.
      buf      - line buffer
      tok      - token structure to hold results
   Output:
      tok      - beginning, end, and type of a single token
   Return:
      0        - success
      negative - error, something went wrong, or end of line
**************************************************************************/
static int
get_first_token(CAN_CONTEXT *const ctx, const char *const buf,
		CAN_TOKEN *const tok)
{
   const char *p, *q;
   int list_level;
   
   tok->type = TOK_EMPTY;
   if (NULL == buf) {
      return -__LINE__;
   }

   p = buf;
   while(isspace(*p)) { ++p; }
   q = p;

   if (*p == '#') {
      tok->type = TOK_COMMENT;	/* skip comment */
      return -__LINE__;
   } else if (*p == '"') {
      ++p;			/* recognize and dequote a quoted string */
      ++q;
      while (*q != '"') {
	 if ('\0' == *q) {
	    can_log(LOGL_ERROR, LOGTP_CONFIG, (CAN_CONFIG *)NULL, ctx,
		    "Unterminated string, missing quote \":"
		    TOK_FMT".\n", TOK_ARGS(tok));
	    return -__LINE__;
	 }
	 ++q;
      }
      tok->type = TOK_STRING;
   } else if (*p == '[') {
      list_level = 1;
      ++p;		 /* recognize a square-bracketed list, may be nested */
      ++q;
      while (list_level > 0) {
	 switch (*q) {
	 case '[':
	    ++list_level;
	    break;
	 case ']':
	    --list_level;
	    break;
	 case '\0':
	    can_log(LOGL_ERROR, LOGTP_CONFIG, (CAN_CONFIG *)NULL, ctx,
		    "Unterminated list, missing square bracket ]: "
		    TOK_FMT".\n", TOK_ARGS(tok));
	    return -__LINE__;
	 default:
	    break;
	 }
     	 ++q;
      }
      q--;			/* remove the last ']' from the token value */
      tok->type = TOK_LIST;
   } else if (*p == '{') {
      ++p;			/* recognize a curly-bracketed item */
      ++q;
      while (*q != '}') {
	 if ('\0' == *q) {
	    can_log(LOGL_ERROR, LOGTP_CONFIG, (CAN_CONFIG *)NULL, ctx,
		    "Unterminated item, missing curly bracket }: "
		    TOK_FMT".\n", TOK_ARGS(tok));
	    return -__LINE__;
	 }
	 ++q;
      }
      tok->type = TOK_ITEM;
   } else {			/* must be an ordinary simple token */
      while (isalnum(*q) || ispunct(*q)) {
	 ++q;
	 tok->type = TOK_SIMPLE;
      }
   }

   if (q == p) {
      tok->val = tok->end = NULL;
      return -__LINE__;
   }
   tok->val = p;
   tok->end = q;
   return 0;
}

/*************************************************************************
**************************************************************************
    get_next_token - Take pointers to the current token and another token
                structure, fill in the addresses of the beginning and end
                of the next token, and identify the type of token.  Will
                work correctly if the input and output tokens are the same.

   Input:
      ctx      - context information, e.g., input file, line number, etc.
      prev     - token indicating current parsing position
   Output:
      next     - token structure to hold results
   Return:
      0        - success
      negative - error
**************************************************************************/
static int
get_next_token(CAN_CONTEXT *const ctx, const CAN_TOKEN *const prev,
	       CAN_TOKEN *const next)
{
   if (*prev->end == '\0') {
      next->type = TOK_EMPTY;
      return -__LINE__;
   }
   return get_first_token(ctx, prev->end+1, next);
}

/*************************************************************************
**************************************************************************
   DEFINE_LOOKUP_FUNCTION - function-like macro that defines functions to
                look up different kinds of configuration entries in their
                respective tables and return a pointer to the relevent
                entry or NULL if it is not found.  The lookup function
                takes two parameters, a pointer to a valid configuration
                structure (CAN_CONFIG *), and a query parameter, and
                returns a pointer to the a structure containing the
                matching configuration parameters, or NULL if there is no
                match.
   
                There should be no semicolon after an invocation of this
                macro.

   Input:
      BTP - base type in which the list and counts of TTP structures are
            found
      BNM - name of the base type, in lower case
      TTP - target type, a pointer to that type is returned
      TNM - name of the thing sought, in lower case, coordinated with names
            defined in the BTP data structures, e.g., "item", which is used
            to automatically create "bp->items", "bp->num_items", etc
      CNM - criteria name, added to the function name to distinguish
            between two functions that look for the same kind of object
            using different kinds of criteria
      QTP - the type of object used as a query, not automatically converted
            to a pointer

      CMP - code to compare the query object with a candidate object, to be
            pasted into an 'if' statment so that, if true, a pointer to the
            candidate object will be returned as the lookup result
   Output:
      a fuction: lookup_<TNM>_in_<BNM>_by_<CNM>(const BTP *const, const QTP)
**************************************************************************/

/*************************************************************************
**************************************************************************
   lookup_<TNM>_in_<BNM>_by_<CNM> - Find a <TTP> object in the <TNM> slot
                 of the <BTP structure specified as a function parameter,
                 using the comparison code in <CMP> to compare it with the
                 <QTP> data passed as a function parameter.

   Input:
      tbl      - pointer to a <BTP> structure containing the list in which
                 to search
      qry      - a <QTP> parameter providing data to be compared with
   Return:
      non-NULL - address of the relevant <TTP> structure
      NULL     - nothing matched the given criteria
**************************************************************************/

#define DEFINE_FLAT_LOOKUP_FUNCTION(BTP, BNM, TTP, TNM, QTP, CNM, CMP)	\
   TTP *lookup_##TNM##_in_##BNM##_by_##CNM##_flat			\
   (const BTP *const btp, const QTP qry)				\
   {									\
      TTP **p;								\
      									\
      if (btp->num_##TNM##s > 0) {					\
	 for (p = btp->TNM##s;						\
	      p - btp->TNM##s < btp->num_##TNM##s;			\
	      p++) {							\
	    if (CMP) {							\
	       return *p;						\
	    }								\
	 }								\
      }									\
      return NULL;							\
   }

#define DEFINE_DEEP_LOOKUP_FUNCTION(BTP, BNM, TTP, TNM, QTP, CNM)	\
   TTP *lookup_##TNM##_in_##BNM##_by_##CNM##_deep			\
   (const BTP *const btp, const QTP qry)				\
   {									\
      const BTP *bcur;							\
      TTP *result;							\
      									\
      for (bcur = btp; bcur != NULL; bcur = bcur->parent) {		\
	 result = lookup_##TNM##_in_##BNM##_by_##CNM##_flat(bcur, qry);	\
	 if (NULL != result) {						\
	    return result;						\
	 }								\
      }									\
      return NULL;							\
   }			

#define DEFINE_BOTH_LOOKUP_FUNCTIONS(BTP, BNM, TTP, TNM, QTP, CNM, CMP) \
   DEFINE_FLAT_LOOKUP_FUNCTION(BTP, BNM, TTP, TNM, QTP, CNM, CMP) \
   DEFINE_DEEP_LOOKUP_FUNCTION(BTP, BNM, TTP, TNM, QTP, CNM) \

DEFINE_BOTH_LOOKUP_FUNCTIONS(CAN_CONFIG, cfg, CAN_STANDARD, standard,
			     CAN_TOKEN *, tag_tok,
			     tok_str_cmp(qry, (*p)->tag) == 0)

DEFINE_BOTH_LOOKUP_FUNCTIONS(CAN_CONFIG, cfg, RECORD_SPEC, record,
			     int, type_num, qry == (*p)->idnum)

/*************************************************************************
**************************************************************************
    field_match - Auxiliary function used in the lookup_field_by_id_nums
                function defined below.  Returns 1 (true) for match, 0
                (false) for non-match.

   Input:
      tgt - pointer to an array of two integers containing the target match
            criteria, the first of which is the type id, and the second of
            which is the field id
      fld - a pointer to a FIELD_SPEC structure to compare with the target
            values
   Return:
      true (1)  - the given field matches the criteria.
      false (0) - the given field does not match the criteria
**************************************************************************/
static int
field_match(const int *const tgt, const FIELD_SPEC *const fld)
{
   RECORD_SPEC **rtp;

   /* check whether the field ids match--necessary but not sufficient */
   if (fld->idnum != tgt[1]) {
      return 0;
   }

   /* check whether the field definition is valid for the given record type */
   for (rtp = fld->records; rtp - fld->records < fld->num_records; ++rtp) {
      if (tgt[0] == (*rtp)->idnum) {
	 return 1;
      }
   }
   return 0;
}

DEFINE_BOTH_LOOKUP_FUNCTIONS(CAN_CONFIG, cfg, FIELD_SPEC, field, 
			     int *const, id_nums, field_match(qry, (*p)) > 0)


DEFINE_BOTH_LOOKUP_FUNCTIONS(CAN_CONFIG, cfg, ITEM_SPEC, item,
			     CAN_TOKEN *, tag_tok,
			     tok_str_cmp(qry, (*p)->tag) == 0)

DEFINE_BOTH_LOOKUP_FUNCTIONS(CAN_CONFIG, cfg, CAN_LIST, list,
			     CAN_TOKEN *, tag_tok,
			     tok_str_cmp(qry, (*p)->tag) == 0)

DEFINE_BOTH_LOOKUP_FUNCTIONS(CAN_CONFIG, cfg, CAN_STANDARD, standard,
			     int, anver,
			     ((strcmp((*p)->name, "ANSI/NIST") == 0) 
			      && (*p)->ver.u.num == qry))

DEFINE_BOTH_LOOKUP_FUNCTIONS(CAN_CONFIG, cfg, CAN_OPTION, option, 
			     char *const, name, (strcmp(qry, (*p)->name ) == 0))

DEFINE_FLAT_LOOKUP_FUNCTION(RECORD_SPEC, rec, FIELD_SPEC, field,
			    char *const, tag, strcmp((*p)->tag, qry) == 0)

DEFINE_FLAT_LOOKUP_FUNCTION(CAN_LIST, lst, ITEM_SPEC_VAL, val, char *const, str,
			    strcmp((*p)->str, qry) == 0)

DEFINE_FLAT_LOOKUP_FUNCTION(CAN_LIST, lst, ITEM_SPEC_VAL, val, int, num,
			    qry == (*p)->u.num)

#undef DEFINE_LOOKUP_FUNCTION
#undef DEFINE_DEEP_LOOKUP_FUNCTION

/*************************************************************************
**************************************************************************
    lookup_field_in_cfg_by_tf_ids_flat - Lookup configured specfications of
                a field, given the type id and field number.  This provides
                a convenient way to use lookup_field_by_id_nums_flat() when
                the numbers are not already stored in a two-member array.

   Input:
      cfg      - pointer to a configuration structure in which to perform
                 the lookup
      type_id  - type identifier, to distinguish similarly numbered fields
      fld_num  - field number of the field of interest
   Return:
      non-NULL - address of the relevant field specification structure
      NULL     - no field matched the given criteria
**************************************************************************/
FIELD_SPEC *
lookup_field_in_cfg_by_tf_ids_flat
   (const CAN_CONFIG *cfg, const int type_id, const int fld_num)
{
   int qry[2];

   qry[0] = type_id;
   qry[1] = fld_num;
   return lookup_field_in_cfg_by_id_nums_flat(cfg, qry);
}

/*************************************************************************
**************************************************************************
    lookup_field_in_cfg_by_tf_ids_deep - Lookup configured specfications of
                a field, given the type id and field number.  This provides
                a convenient way to use lookup_field_by_id_nums_deep() when
                the numbers are not already stored in a two-member array,
                and it is exported for use in other modules of this
                program.

   Input:
      cfg      - pointer to a configuration structure in which to perform
                 the lookup
      type_id  - type identifier, to distinguish similarly numbered fields
      fld_num  - field number of the field of interest
   Return:
      non-NULL - address of the relevant field specification structure
      NULL     - no field matched the given criteria
**************************************************************************/
FIELD_SPEC *
lookup_field_in_cfg_by_tf_ids_deep
   (const CAN_CONFIG *cfg, const int type_id, const int fld_num)
{
   int qry[2];

   qry[0] = type_id;
   qry[1] = fld_num;
   return lookup_field_in_cfg_by_id_nums_deep(cfg, qry);
}

/*************************************************************************
**************************************************************************
   DEFINE_STORE_FUNCTION - a function-like macro that defines functions to
                store up different kinds of configuration entries in their
                respective tables.  It checks and allocates more space for
                pointers if necesary.

                There should be no semicolon after an invocation of this
                macro.
   Input:
      BTYP - base type of structure, i.e., the one that contains the table
             of pointers in which to store the data
      BNAM - short, lower case version of the name of the base type, used
             in generating the function name and error messages
      TYPE - type of structure of interest, whose pointers we want to store
      NAME - non-prefixed, lower case version of the type name of the
             structure of interest, which appears in field names in the
             base type.
   Output:
      a function named: store_<bnam>_<name>
**************************************************************************/

/*************************************************************************
**************************************************************************
   store_<bnam>_<name> - Insert a <type> object into the appropriate slot
                 of the <btyp> structure.  Allocate more slots if needed.
   Input:
      ctx      - context information, e.g., input file, line number, etc.
      deposit  - pointer to the structure to add
      cfg      - pointer to a configuration structure in which to increase
                 the space allocated to the particular type of pointer
   Output:
      ctx      - updated statistics
   Return:
      zero     - success
      negative - failure
**************************************************************************/

#define DEFINE_STORE_FUNCTION(BTYP, BNAM, TYPE, NAME)			\
   static int								\
   store_##BNAM##_##NAME(CAN_CONTEXT *const ctx, TYPE *const deposit,	\
			 BTYP *const bp)				\
   {									\
      typedef struct name_s { const char *const name; } CAN_NAME;	\
      									\
      can_log(LOGL_DEBUG, LOGTP_CONFIG, (CAN_CONFIG *)NULL, ctx,	\
	      "Store " #NAME " '%s' in " #BNAM " '%s'.\n",		\
	      ((CAN_NAME *)deposit)->name, ((CAN_NAME *)bp)->name);	\
      if (bp->num_##NAME##s == bp->alloc_##NAME##s) {			\
         if (more_##BNAM##_##NAME##_ptrs(bp, CAN_CHUNK) != 0) {	\
	    return -__LINE__;						\
	 }								\
      }									\
      bp->NAME##s[bp->num_##NAME##s++] = deposit;			\
      return 0;								\
   }

DEFINE_STORE_FUNCTION(CAN_CONFIG,  cfg, CAN_STANDARD,  standard)
DEFINE_STORE_FUNCTION(CAN_CONFIG,  cfg, RECORD_SPEC,   record)
DEFINE_STORE_FUNCTION(CAN_CONFIG,  cfg, FIELD_SPEC,    field)
DEFINE_STORE_FUNCTION(CAN_CONFIG,  cfg, ITEM_SPEC,     item)
DEFINE_STORE_FUNCTION(CAN_CONFIG,  cfg, CAN_LIST,      list)
DEFINE_STORE_FUNCTION(RECORD_SPEC, rec, FIELD_SPEC,    field)
DEFINE_STORE_FUNCTION(FIELD_SPEC,  fld, ITEM_SPEC,     item)
DEFINE_STORE_FUNCTION(CAN_LIST,    lst, ITEM_SPEC_VAL, val)
DEFINE_STORE_FUNCTION(CAN_CONFIG,  cfg, CAN_OPTION,    option)
#undef DEFINE_STORE_FUNCTION

/*************************************************************************
**************************************************************************
   log_parse - Log an indication of what the parsing code is seeking and
                the current input token the point of invocation of this
                function.

   Input:
      cfg  - pointer to a configuration structure in use
      ctx  - context information, e.g., input file, line number, etc.
      what - what is expected
      tok  - the current token
   Output:
      ctx  - updated statistics
**************************************************************************/
static void
log_parse(const CAN_CONFIG *const cfg, CAN_CONTEXT *const ctx,
		const char *const what, const CAN_TOKEN *const tok)
{
   can_log(LOGL_DEBUG, LOGTP_CONFIG, cfg, ctx,
	   "Parsing '%s': %s token: "TOK_FMT", rest: \"%s\".\n",
	   what, token_type_name(tok), TOK_ARGS(tok), tok->end);
}

/*************************************************************************
**************************************************************************
   log_token_type_error - Log an indication that the wrong kind of token
                was scanned where something else was expected.

   Input:
      cfg  - pointer to a configuration structure in use
      ctx  - context information, e.g., input file, line number, etc.
      tok  - the token that is of the wrong type
      what - name of the thing sought here, e.g., "tag", "occ", "items",
             (not the token type expected)
   Output:
      ctx  - updated statistics
**************************************************************************/
static void
log_token_type_error(const CAN_CONFIG *const cfg, CAN_CONTEXT *const ctx,
		     const CAN_TOKEN *const tok, const char *const what)
{
   can_log(LOGL_ERROR, LOGTP_CONFIG, cfg, ctx,
	   "Wrong kind of value for '%s': type: %s, content: "TOK_FMT".\n",
	   what, token_type_name(tok), TOK_ARGS(tok));
}

/*************************************************************************
**************************************************************************
   log_truncation_error - Log an indication that the record being scanned
                ended sooner than expected, i.e., without all the required
                fields

   Input:
      cfg      - pointer to a configuration structure in use
      ctx      - context information, e.g., input file, line number, etc.
      head     - the first token of the line, or object being parsed
      what     - name of the thing parsed here, e.g., "Standard", "Field", etc.
      preceeding - name of the last field found
      expected -  name of the expected field
   Output:
      ctx      - updated statistics
**************************************************************************/
static void
log_truncation_error(const CAN_CONFIG *const cfg, CAN_CONTEXT *const ctx,
		     const CAN_TOKEN *const head, const char *const what,
		     const char *const preceeding, const char *const expected)
{
   can_log(LOGL_ERROR, LOGTP_CONFIG, cfg, ctx,
	   "%s entry truncated after %s: \"%s\"<%s expected>.\n",
	   what, preceeding, head->val, expected);
}

/*************************************************************************
**************************************************************************
   log_extra_data_error - Log an indication that the record being scanned
               contained more data than expected, i.e., extra fields

   Input:
      cfg  - pointer to a configuration structure in use
      ctx  - context information, e.g., input file, line number, etc.
      head - the first token of the line, or object being parsed
      what - name of the thing parsed here, e.g., "Standard", "Field", etc.
   Output:
      ctx  - updated statistics
**************************************************************************/
static void
log_extra_data_error(const CAN_CONFIG *const cfg, CAN_CONTEXT *const ctx,
		     const CAN_TOKEN *const tok, const char *const what)
{
      can_log(LOGL_WARNING, LOGTP_CONFIG, cfg, ctx,
	      "Extra data after %s spec ignored: \"%s\".\n", what, tok->val);
}

/*************************************************************************
**************************************************************************
   parse_list_ext - parse a list of items, either from a separate list
                specification of from an anonymous list appearing within,
                for example, a field specification.

   Input:
      ctx      - context information, e.g., input file, line number, etc.
      list     - a pointer to a list structure, if it is a named list, the
                 tag will already be assigned, otherwise one will be generated
      head     - the first token of the list being parsed
      res_type - if not zero, specifies a certain type of element to expect.
                 An error is reported if the input cannot be interpreted as 
                 the expected type of element.
   Output:
      list     - the results are inserted into this list
      ctx      - updated statistics
   Return:
      zero     - success
      negative - error
**************************************************************************/
static int
parse_list_ext(CAN_CONTEXT *const ctx, CAN_LIST *const list,
	       const CAN_TOKEN *const head, const ITEM_SPEC_TYPE res_type)
{
   char *str, buf[25];
   CAN_TOKEN tok[1];
   static int anon_list_no = 1;
   CAN_LIMITS lim[1];
   ITEM_SPEC_VAL *val;
   int i;
   
   if (NULL == list->tag) {	/* anonymous list */
      list->tag = checked_alloc(ctx, "list tag", sizeof(char), 16);
      if (NULL == list->tag) {
	 return -__LINE__;
      }
      sprintf(list->tag, "anon-list-%04d", anon_list_no++);
   }

   /* prepare to parse the list */
   str = str_from_tok(head);
   if (NULL == str) {
      return -__LINE__;
   }
   if (get_first_token(ctx, str, tok) != 0) {
      log_truncation_error((CAN_CONFIG *)NULL, ctx, head,
			   "list", "tag", "values");
      free(str);
      return -__LINE__;
   }

   do {
      val = NULL;
      if (((res_type == 0) || (res_type == ITM_NUM) || (res_type == ITM_SNUM))
	  && (TOK_SIMPLE == tok->type)) {

	 /* look for a single number or a bounded range */
	 if (set_limits_from_tok(ctx, tok, lim, (res_type == 0)) != 0) {
	    if (res_type == 0) {
	       goto after_num;
	    }
	    log_parse_error((CAN_CONFIG *)NULL, ctx, tok, 
			    "numeric list element", (char *)NULL);
	    free(str);
	    return -__LINE__;
	 }
	 if (lim->max == UNSET) {
	    if (res_type == 0) {
	       goto after_num;
	    }
	    can_log(LOGL_ERROR, LOGTP_CONFIG, (CAN_CONFIG *)NULL, ctx,
		    "Cannot convert unbounded range to a finite list: %d-*.\n",
		    lim->min);
	    free(str);
	    return -__LINE__;
	 }
	 if (lim->min == UNSET) {	/* zero is the lower bound here */
	    lim->min = 0;
	 }
 	
	 for (i = lim->min; i <= lim->max; i++) {
	    val = alloc_val(ctx);
	    if (NULL == val) {
	       free(str);
	       free_val(val);
	       return -__LINE__;
	    }
	    sprintf(buf, "%d", i);
	    val->str = checked_alloc(ctx, "value string", 
				     sizeof(char), strlen(buf)+1);
	    if (NULL == val->str) {
	       return -__LINE__;
	    }
	    strcpy(val->str, buf);
	    val->u.num = i;
	    val->type = ITM_NUM;
	    if (store_lst_val(ctx, val, list) != 0) {
	       free(str);
	       free_val(val);
	       return -__LINE__;
	    }
	 }
      } 
   after_num:

      if ((NULL == val) && ((res_type == 0) || (res_type == ITM_STR)) && 
	  ((TOK_SIMPLE == tok->type) || (TOK_STRING == tok->type))) {
	 val = alloc_val(ctx);
	 if (NULL == val) {
	    free(str);
	    free_val(val);
	    return -__LINE__;
	 }
	 val->str = str_from_tok(tok);
	 if (NULL == val->str) {
	    return -__LINE__;
	 }
	 val->type = ITM_STR;
	 if (store_lst_val(ctx, val, list) != 0) {
	    free(str);
	    free_val(val);
	    return -__LINE__;
	 }
      }

      if (NULL == val) {
	 char buf[25];
	 
	 sprintf(buf, "list element %d", list->num_vals+1);
	 log_token_type_error((CAN_CONFIG *)NULL, ctx, tok, buf);
	 free(str);
	 return -__LINE__;
      }
      
   } while (get_next_token(ctx, tok, tok) == 0);

   free(str);
   return 0;
}

/*************************************************************************
**************************************************************************
   parse_standard - parse a specification of a standard, ANSI/NIST, EBTS,
                or other, which defines subsequently specified records,
                fields, items, or parameters

   Input:
      ctx      - context information, e.g., input file, line number, etc.
      head     - the first token of the standard being parsed
      cfg      - configuration structure in which to store results
   Output:
      ctx      - updated statistics
      cfg      - updated configuration structure
   Return:
      zero     - success
      negative - error
**************************************************************************/
static int
parse_standard(CAN_CONTEXT *const ctx, const CAN_TOKEN *const head,
	       CAN_CONFIG *const cfg)
{
   const CAN_STANDARD *dup;
   CAN_STANDARD *std;
   CAN_TOKEN tok[1];
   int num;
   double fp;

   log_parse(cfg, ctx, "standard", head);

   /* verify the standard is not already defined */
   if (TOK_SIMPLE != head->type) {
      log_token_type_error(cfg, ctx, head, "standard tag");
      return -__LINE__;
   } 
   dup = lookup_standard_in_cfg_by_tag_tok_flat(cfg, head);
   if (NULL != dup) {
      can_log(LOGL_WARNING, LOGTP_CONFIG, cfg, ctx,
	      "Ignoring duplicate standard specification for "TOK_FMT
	      ": %s.\n", TOK_ARGS(head), head->val);
      return -__LINE__;
   }

   std = alloc_standard(ctx);
   if (NULL == std) {
      return -__LINE__;
   }

   /* assign fields, starting with .tag */
   std->tag = str_from_tok(head);
   if (NULL == std->tag) {
      free_standard(std);
      return -__LINE__;
   }

   /* parent standard, predecssor and supplier of default values */
   if (get_next_token(ctx, head, tok) != 0) {
      log_truncation_error(cfg, ctx, head, "Standard", "tag", "prior standard");
      free_standard(std);
      return -__LINE__;
   } 
   if (TOK_SIMPLE != tok->type) {
      log_token_type_error(cfg, ctx, tok, "prior standard");
      free_standard(std);
      return -__LINE__;
   } 
   if (tok_str_cmp(tok, "*") == 0) {
      std->parent = NULL;
   } else if ((std->parent = lookup_standard_in_cfg_by_tag_tok_deep(cfg, tok))
	      == NULL) {
      can_log(LOGL_ERROR, LOGTP_CONFIG, cfg, ctx,
	      "Specified prior standard "TOK_FMT" not found.\n",
	      TOK_ARGS(tok));
      free_standard(std);
      return -__LINE__;
   }
   
   /* standard name, version-independent short form */
   if (get_next_token(ctx, tok, tok) != 0) {
      log_truncation_error(cfg, ctx, head,
			   "Standard", "prior standard", "name");
      free_standard(std);
      return -__LINE__;
   } 
   if ((TOK_SIMPLE != tok->type) && (TOK_STRING != tok->type)) {
      log_token_type_error(cfg, ctx, tok, "standard name");
      free_standard(std);
      return -__LINE__;
   } 
   if (tok_str_cmp(tok, "*") == 0) {
      if (NULL == std->parent) {
	 can_log(LOGL_ERROR, LOGTP_CONFIG, cfg, ctx,
		 "No standard name, and no predecessor to get it from.\n");
	 free_standard(std);
	 return -__LINE__;
      }
      std->name = std->parent->name;
   } else if ((std->name = str_from_tok(tok)) == NULL) {
      return -__LINE__;
   }

   /* standard version */
   if (get_next_token(ctx, tok, tok) != 0) {
      log_truncation_error(cfg, ctx, head, "Standard", "name", "version");
      free_standard(std);
      return -__LINE__;
   } else if (TOK_SIMPLE == tok->type) {
      std->ver.str = str_from_tok(tok);
      if (NULL == std->ver.str) {
	 return -__LINE__;
      }
      if (num_from_tok(ctx, tok, &num) == 0) {
	 std->ver.type = ITM_NUM;
	 std->ver.u.num = num;
      } else if (fp_from_tok(ctx, tok, &fp) == 0) {
	 std->ver.type = ITM_FP;
	 std->ver.u.fp = fp;
      } else {
	 std->ver.type = ITM_STR;
      }
   } else {
      log_token_type_error(cfg, ctx, tok, "standard version");
      free_standard(std);
      return -__LINE__;
   }
   
   /* standard reference, formal specification of name and version */
   if (get_next_token(ctx, tok, tok) != 0) {
      log_truncation_error(cfg, ctx, head, "Standard", "version", "reference");
      free_standard(std);
      return -__LINE__;
   } 
   if ((TOK_SIMPLE != tok->type) && (TOK_STRING != tok->type)) {
      log_token_type_error(cfg, ctx, tok, "standard reference");
      free_standard(std);
      return -__LINE__;
   }
   std->ref = str_from_tok(tok);
   if (NULL == std->ref) {
      return -__LINE__;
   }

   /* date of issue or ratification */
   if (get_next_token(ctx, tok, tok) != 0) {
      log_truncation_error(cfg, ctx, head, "Standard", "reference", "date");
      free_standard(std);
      return -__LINE__;
   } 
   if (TOK_SIMPLE != tok->type) {
      log_token_type_error(cfg, ctx, tok, "standard date");
      free_standard(std);
      return -__LINE__;
   }  
   if ((std->date = str_from_tok(tok)) == NULL) {
      return -__LINE__;
   }

   /* check end */
   if (get_next_token(ctx, tok, tok) == 0) {
      log_extra_data_error(cfg, ctx, tok, "standard");
   }

   return store_cfg_standard(ctx, std, cfg);
}

/*************************************************************************
**************************************************************************
   parse_record - parse a record-type specification

   Input:
      ctx      - context information, e.g., input file, line number, etc.
      head     - the first token of the record being parsed
      cfg      - configuration structure in which to store results
   Output:
      ctx      - updated statistics
      cfg      - updated configuration structure
   Return:
      zero     - success
      negative - error
**************************************************************************/
static int
parse_record(CAN_CONTEXT *const ctx, const CAN_TOKEN *const head, 
	     CAN_CONFIG *const cfg)
{
   CAN_TOKEN tok[1];
   RECORD_SPEC *dup, *rec;

   log_parse(cfg, ctx, "record type", head);
   
   rec = alloc_record(ctx);
   if (NULL == rec) {
      return -__LINE__;
   }

   /* ID number of the record type */
   if (TOK_SIMPLE != head->type) {
      log_token_type_error(cfg, ctx, head, "record id");
      free_record(rec);
      return -__LINE__;
   } 
   if (num_from_tok(ctx, head, &rec->idnum) != 0) {
      can_log(LOGL_ERROR, LOGTP_CONFIG, cfg, ctx,
	      "Could not parse record type number: "TOK_FMT".\n",
	      TOK_ARGS(head));
      free_record(rec);
      return -__LINE__;
   } 
   dup = lookup_record_in_cfg_by_type_num_flat(cfg, rec->idnum);
   if (NULL != dup) {
      can_log(LOGL_WARNING, LOGTP_CONFIG, cfg, ctx,
	      "Ignoring duplicate record specification for "TOK_FMT
	      ": %s.\n", TOK_ARGS(head), head->val);
      free_record(rec);
      return -__LINE__;
   }

   /* standard where the record is defined */
   if (get_next_token(ctx, head, tok) != 0) {
      log_truncation_error(cfg, ctx, head, "Record", "id", "standard");
      free_record(rec);
      return -__LINE__;
   }
   if (TOK_SIMPLE != tok->type) {
      log_token_type_error(cfg, ctx, tok, "record standard");
      free_record(rec);
      return -__LINE__;
   } 
   rec->std = lookup_standard_in_cfg_by_tag_tok_deep(cfg, tok);
   if (NULL == rec->std) {
      can_log(LOGL_ERROR, LOGTP_CONFIG, cfg, ctx,
	      "Cannot find standard: "TOK_FMT".\n", TOK_ARGS(tok));
      free_record(rec);
      return -__LINE__;
   }

   /* data type of the record */
   if (get_next_token(ctx, tok, tok) != 0) {
      log_truncation_error(cfg, ctx, head, "Record", "standard", "data type");
      free_record(rec);
      return -__LINE__;
   } 
   if (TOK_SIMPLE != tok->type) {
      log_token_type_error(cfg, ctx, tok, "record data type");
      free_record(rec);
      return -__LINE__;
   } 
   rec->data_type = rdt_enum_from_tok(ctx, tok);
   if (rec->data_type < 0) {
      can_log(LOGL_ERROR, LOGTP_CONFIG, cfg, ctx, 
	      "Cannot find data type: "TOK_FMT", expected '%s', '%s', "
	      "or '%s'.\n", TOK_ARGS(tok), rdt_str_from_enum(RDT_ASCII),
	      rdt_str_from_enum(RDT_BINARY), rdt_str_from_enum(RDT_ASCBIN));
      free_record(rec);
      return -__LINE__;
   }
   
   /* name of the record type */
   if (get_next_token(ctx, tok, tok) != 0) {
      log_truncation_error(cfg, ctx, head, "Record", "data type", "name");
      free_record(rec);
      return -__LINE__;
   }
   if ((TOK_SIMPLE != tok->type) && (TOK_STRING !=tok->type)) {
      log_token_type_error(cfg, ctx, tok, "record name");
      free_record(rec);
      return -__LINE__;
   } 
   rec->name = str_from_tok(tok);
   if (NULL == rec->name) {
      return -__LINE__;
   }

   /* check end */
   if (get_next_token(ctx, tok, tok) == 0) {
      log_extra_data_error(cfg, ctx, tok, "record");
   }

   return store_cfg_record(ctx, rec, cfg);
}

/*************************************************************************
**************************************************************************
   parse_field - parse a field specification

   Input:
      ctx      - context information, e.g., input file, line number, etc.
      head     - the first token of the record being parsed
      cfg      - configuration structure in which to store results
   Output:
      ctx      - updated statistics
      cfg      - updated configuration structure
   Return:
      zero     - success
      negative - error
**************************************************************************/
static int
parse_field(CAN_CONTEXT *const ctx, const CAN_TOKEN *const head,
	    CAN_CONFIG *const cfg)
{
   CAN_TOKEN tok[1], item_tok[1];
   FIELD_SPEC *fld;
   CAN_LIST single_record_type, *record_types = NULL;
   ITEM_SPEC_VAL single_record_value, *srvp = &single_record_value;
   ITEM_SPEC *itm;
   char *item_list_str;
   int i;

   log_parse(cfg, ctx, "field", head);

   if (TOK_SIMPLE != head->type) {
      log_token_type_error(cfg, ctx, head, "field tag");
      return -__LINE__;
   }

   fld = alloc_field(ctx);
   if (NULL == fld) {
      return -__LINE__;
   }

   /* tag of field, this and the type number uniquely identify a field */
   fld->tag = str_from_tok(head);
   if (NULL == fld->tag) {
      free_field(fld);
      return -__LINE__;
   }

   /* types where this field is valid, field id numbers are not unique */
   if (get_next_token(ctx, head, tok) != 0) {
      log_truncation_error(cfg, ctx, head, "Field", "tag", "record types");
      free_field(fld);
      return -__LINE__;
   }
   if (TOK_SIMPLE == tok->type) {
      if (num_from_tok(ctx, tok, &srvp->u.num) == 0) {
      	/* a single record type id -- convert it to a list containing one
	   item and then handle like the other lists below */
	 srvp->type = ITM_NUM;
	 single_record_type.tag = "anon";
	 single_record_type.num_vals = 1;
	 single_record_type.alloc_vals = 0;
	 single_record_type.vals = &srvp;
	 record_types = &single_record_type;
      } else {	/* a named list of record type ids */
	 record_types = lookup_list_in_cfg_by_tag_tok_deep(cfg, tok);
	 if (NULL == record_types) {
	    can_log(LOGL_ERROR, LOGTP_CONFIG, cfg, ctx,
		    "Unknown record type specifier, neither number nor list: "
		    TOK_FMT".\n", TOK_ARGS(tok));
	    free_field(fld);
	    return -__LINE__;
	 }
      }
   } else if (TOK_LIST == tok->type) {
      record_types = alloc_list(ctx);
      if (parse_list_ext(ctx, record_types, tok, ITM_NUM) != 0) {
	    free_field(fld);
	    return -__LINE__;
      }
   } else {
      log_token_type_error(cfg, ctx, tok, "field record types");
      return -__LINE__;
   }
   if (more_fld_record_ptrs(fld, record_types->num_vals) != 0) {
      return -__LINE__;
   }

   /* id number, this and the type uniquely identify a field */
   if (get_next_token(ctx, tok, tok) != 0) {
      log_truncation_error(cfg, ctx, head, "Field", "types", "id");
      free_field(fld);
      return -__LINE__;
   }
   if (TOK_SIMPLE != tok->type) {
      log_token_type_error(cfg, ctx, tok, "field id num");
      free_field(fld);
      return -__LINE__;
   }
   if (num_from_tok(ctx, tok, &fld->idnum) != 0) {
      can_log(LOGL_ERROR, LOGTP_CONFIG, cfg, ctx,
	      "Could not parse field id number: "TOK_FMT".\n",
	      TOK_ARGS(tok));
      free_field(fld);
      return -__LINE__;
   }

   /* verify uniqueness of all type number, field id combinations */
   for (i = 0; i < record_types->num_vals; i++) {
      const FIELD_SPEC *dup;
      int rtp_num = record_types->vals[i]->u.num;

      fld->records[i] = lookup_record_in_cfg_by_type_num_deep(cfg, rtp_num);
      if (NULL == fld->records[i]) {
	 can_log(LOGL_ERROR, LOGTP_CONFIG, cfg, ctx,
		 "Unknown record type number %d.\n", rtp_num);
	 return -__LINE__;
      }
      
      dup = lookup_field_in_cfg_by_tf_ids_flat(cfg, rtp_num, fld->idnum);
      if (NULL != dup) {
	 can_log(LOGL_FATAL, LOGTP_CONFIG, cfg, ctx,
		 "Field %d (%s) in type-%d is already defined.\n",
		 fld->idnum, fld->tag, fld->records[i]->idnum);
      } else if (store_rec_field(ctx, fld, fld->records[i]) != 0) {
	 can_log(LOGL_ERROR, LOGTP_CONFIG, cfg, ctx,
		 "Error adding field %s to record type-%d.\n",
		 fld->tag, fld->records[i]->idnum);
	 return -__LINE__;
      }
   }
   fld->num_records = record_types->num_vals;

   /* standard */
   if (get_next_token(ctx, tok, tok) != 0) {
      log_truncation_error(cfg, ctx, tok, "Field", "id num", "standard");
      free_field(fld);
      return -__LINE__;
   }
   if (TOK_SIMPLE != tok->type) {
      log_token_type_error(cfg, ctx, tok, "field standard");
      free_field(fld);
      return -__LINE__;
   } 
   fld->std = lookup_standard_in_cfg_by_tag_tok_deep(cfg, tok);
   if (NULL == fld->std) {
      can_log(LOGL_ERROR, LOGTP_CONFIG, cfg, ctx,
	      "Cannot find standard: "TOK_FMT".\n", TOK_ARGS(tok));
      free_field(fld);
      return -__LINE__;
   }

   /* the number of allowed occurrences */
   if (get_next_token(ctx, tok, tok) != 0) {
      log_truncation_error(cfg, ctx, head, "Field", "standards", "occurrences");
      free_field(fld);
      return -__LINE__;
   }
   if (TOK_SIMPLE != tok->type) {
      log_token_type_error(cfg, ctx, tok, "field occurrences");
      free_field(fld);
      return -__LINE__;
   } 
   if (set_limits_from_tok(ctx, tok, &fld->occ, 0) != 0) {
      log_parse_error(cfg, ctx, tok, "field occurrences", (char *)NULL);
      free_field(fld);
      return -__LINE__;
   }

   /* the allowed sizes, in bytes including terminators */
   if (get_next_token(ctx, tok, tok) != 0) {
      log_truncation_error(cfg, ctx, head, "Field", "occurrences", "size");
      free_field(fld);
      return -__LINE__;
   }
   if (TOK_SIMPLE != tok->type) {
      log_token_type_error(cfg, ctx, tok, "field size");
      free_field(fld);
      return -__LINE__;
   } 
   if (set_limits_from_tok(ctx, tok, &fld->size, 0) != 0) {
      log_parse_error(cfg, ctx, tok, "field size", (char *)NULL);
      free_field(fld);
      return -__LINE__;
   }

   /* items */
   if (get_next_token(ctx, tok, tok) != 0) {
      log_truncation_error(cfg, ctx, head, "Field", "size", "items");
      free_field(fld);
      return -__LINE__;
   }
   if (tok_str_cmp(tok, "*") != 0) { /* item details provided */
      if ((TOK_SIMPLE == tok->type) || (TOK_ITEM == tok->type)) {
	 if (TOK_SIMPLE == tok->type) { /* named item */
	    
	    itm = lookup_item_in_cfg_by_tag_tok_deep(cfg, tok);
	    if (NULL == itm) {
	       can_log(LOGL_ERROR, LOGTP_CONFIG, cfg, ctx,
		       "Named item "TOK_FMT" not found.\n", TOK_ARGS(tok));
	       free_field(fld);
	       return -__LINE__;
	    }
	 } else if (TOK_ITEM == tok->type) { /* embedded item */
	    if (parse_item_ext(ctx, tok, cfg, fld, &itm) != 0) {
	       free_field(fld);
	       return -__LINE__;
	    }
	 }
	 if (store_fld_item(ctx, itm, fld) != 0) {
	    free_field(fld);
	    return -__LINE__;
	 }
      } else if (TOK_LIST == tok->type) { /* list of items */
	 log_parse(cfg, ctx, "item list", tok);
	 item_list_str = str_from_tok(tok);
	 if (get_first_token(ctx, item_list_str, item_tok) != 0) {
	    log_truncation_error(cfg, ctx, item_tok,
				 "Item list", "beginning", "first item");
	    free_field(fld);
	    free(item_list_str);
	    return -__LINE__;
	 }
	 do {
	    if (TOK_SIMPLE == item_tok->type) {
	       itm = lookup_item_in_cfg_by_tag_tok_deep(cfg, item_tok);
	       if (NULL == itm) {
		  can_log(LOGL_ERROR, LOGTP_CONFIG, cfg, ctx,
			  "Named item "TOK_FMT" in item list "TOK_FMT
			  " not found.\n", TOK_ARGS(item_tok), TOK_ARGS(tok));
		  free_field(fld);
		  free(item_list_str);
		  return -__LINE__;
	       }
	    } else if (TOK_ITEM == item_tok->type) { /* embedded item */
	       if (parse_item_ext(ctx, item_tok, cfg, fld, &itm) != 0) {
		  free_field(fld);
		  free(item_list_str);
		  return -__LINE__;
	       }
	    } else {
	       log_token_type_error(cfg, ctx, item_tok, "field item-list item");
	       free_field(fld);
	       free(item_list_str);	    
	       return -__LINE__;
	    }
	    if (store_fld_item(ctx, itm, fld) != 0) {
	       free_field(fld);
	       free(item_list_str);
	       return -__LINE__;
	    }
	 } while (get_next_token(ctx, item_tok, item_tok) == 0);
	 free(item_list_str);
	 if (TOK_EMPTY != item_tok->type) {
	    printf("item_tok->type: %s, expected empty\n", 
		   token_type_name(item_tok));
	    free_field(fld);
	    return -__LINE__;
	 }
      } else {
	 log_token_type_error(cfg, ctx, tok, "items");
	 free_field(fld);
	 return -__LINE__;
      }
   }

   /* check end */
   if (get_next_token(ctx, tok, tok) == 0) {
      log_extra_data_error(cfg, ctx, tok, "field");
   }

   return store_cfg_field(ctx, fld, cfg);
}

/*************************************************************************
**************************************************************************
   parse_item_ext - parse an item specification, either from a separately
                named and defined item specification, or one embedded
                within a field specification.

   Input:
      ctx      - context information, e.g., input file, line number, etc.
      head     - the first token of the record being parsed
      cfg      - configuration structure in which to store results
      fld      - field in which the item is defined, or NULL
      itmp     - location in which to store a pointer to the resulting
                 item specification structure
   Output:
      ctx      - updated statistics
      cfg      - updated configuration structure
      itmp     - a pointer to the new item specification structure
   Return:
      zero     - success
      negative - error
**************************************************************************/
static int
parse_item_ext(CAN_CONTEXT *const ctx, const CAN_TOKEN *const head,
	       CAN_CONFIG *const cfg, const FIELD_SPEC *const fld,
	       ITEM_SPEC **itmp)
{
   char *str = NULL;
   ITEM_SPEC *itm;
   CAN_TOKEN tok[1];

   log_parse(cfg,ctx, "item spec", head);

   itm = alloc_item(ctx);
   if (NULL == itm) {
      return -__LINE__;
   }
   if (NULL != itmp) {
      *itmp = itm;
   }

   if (NULL == fld) {		/* independent item specification */
      /* tag */
      if (TOK_SIMPLE != head->type) {
	 log_token_type_error(cfg, ctx, head, "item tag");
	 return -__LINE__;
      }
      itm->tag = str_from_tok(head);
      if (NULL == itm->tag) {
	 free_item(itm);
	 return -__LINE__;
      }

      /* standard */
      if (get_next_token(ctx, head, tok) != 0) {
	 log_truncation_error(cfg, ctx, head, "item", "tag", "standard"); 
	 free_item(itm);
	 return -__LINE__;
      }
      if (TOK_SIMPLE != tok->type) {
	 log_token_type_error(cfg, ctx, tok, "item standard");
	 free_item(itm);
	 return -__LINE__;
      } 
      itm->std = lookup_standard_in_cfg_by_tag_tok_deep(cfg, tok);
      if (NULL == itm->std) {
	 can_log(LOGL_ERROR, LOGTP_CONFIG, cfg, ctx,
		 "Cannot find standard: "TOK_FMT".\n", TOK_ARGS(tok));
	 free_item(itm);
	 return -__LINE__;
      }
      
      /* get next token */
      if (get_next_token(ctx, tok, tok) != 0) {
	 log_truncation_error(cfg, ctx, head, 
			      "item", "standard", "occurrences"); 
	 free_item(itm);
	 return -__LINE__;
      }
   } else {			/* anonymous item within a field spec */
      itm->tag = checked_alloc(ctx, "anonymous item tag", sizeof(char),
			       strlen(fld->tag)+6);
      if (NULL == itm->tag) {
	 free_item(itm);
	 return -__LINE__;
      }
      sprintf(itm->tag, "t%d-%s%c",
	      fld->records[0]->idnum, fld->tag, 'a'+fld->num_items);

      itm->std = fld->std;

      /* set next token appropriately */
      str = str_from_tok(head);
      if ((NULL == str) || (get_first_token(ctx, str, tok) != 0)) {
	 free(str);
	 free_item(itm);
	 return -__LINE__;
      }
   }
   
   /* occurences - already loaded into tok */
   if (set_limits_from_tok(ctx, tok, &itm->occ, 0) != 0) {
      log_parse_error(cfg, ctx, tok, "item occurrences", (char *)NULL);
      free(str);
      free_item(itm);
      return -__LINE__;
   }
   
   /* size of item */
   if (get_next_token(ctx, tok, tok) != 0) {
      log_truncation_error(cfg, ctx, head, "item", "occurrences", "size");
      free(str);
      free_item(itm);
      return -__LINE__;
   }
   if (set_limits_from_tok(ctx, tok, &itm->size, 0) != 0) {
      log_parse_error(cfg, ctx, tok, "item size", (char *)NULL);
      free(str);
      free_item(itm);
      return -__LINE__;
   }

   /* type of item */
   if (get_next_token(ctx, tok, tok) != 0) { 
      log_truncation_error(cfg, ctx, head, "item", "size", "type");
      free(str);
      free_item(itm);
      return -__LINE__;
   }
   
   itm->type = item_type_from_tok(tok);
   if (0 == itm->type) {
      log_parse_error(cfg, ctx, tok, "item type", "type not defined");
      free(str);
      free_item(itm);
      return -__LINE__;
   }

   if (get_next_token(ctx, tok, tok) == 0) {
      if (TOK_LIST == tok->type) { /* enumerated values */
	 CAN_LIST *list;

	 list = alloc_list(ctx);
	 if (NULL == list) {
	    free_list(list);
	    return -__LINE__;
	 }
	 if (parse_list_ext(ctx, list, tok, itm->type) != 0) {
	    log_parse_error(cfg, ctx, tok,
			    "enumerated item values", (char *)NULL);
	    free_list(list);
	    return -__LINE__;
	 }
	 itm->enum_vals = list;
      } else if (TOK_SIMPLE == tok->type) { /* possible limits */
	 ; /* tba */
      }
   }

   /* tbd */
   free(str);
   return store_cfg_item(ctx, itm, cfg);
}

/*************************************************************************
**************************************************************************
   parse_item - Parse a stand-alone item specification.  This function
                calls parse_item_ext with appropriate additional parameters

   Input:
      ctx      - context information, e.g., input file, line number, etc.
      head     - the first token of the item being parsed
      cfg      - configuration structure in which to store results
   Output:
      ctx      - updated statistics
      cfg      - updated configuration structure
   Return:
      zero     - success
      negative - error
**************************************************************************/
static int
parse_item(CAN_CONTEXT *const ctx, const CAN_TOKEN *const head,
	   CAN_CONFIG *const cfg)
{
   return parse_item_ext(ctx, head, cfg, (FIELD_SPEC *)NULL,(ITEM_SPEC **)NULL);
}

/*************************************************************************
**************************************************************************
   parse_list - Parse a stand-alone list specification.  This function does
                some preparation, then calls parse_list_ext with
                appropriate additional parameters

   Input:
      ctx      - context information, e.g., input file, line number, etc.
      head     - the first token of the item being parsed
      cfg      - configuration structure in which to store results
   Output:
      ctx      - updated statistics
      cfg      - updated configuration structure
   Return:
      zero     - success
      negative - error
**************************************************************************/
static int
parse_list(CAN_CONTEXT *const ctx, const CAN_TOKEN *const head,
	   CAN_CONFIG *const cfg)
{
   CAN_LIST *list;
   CAN_TOKEN tok[1];

   log_parse(cfg, ctx, "list spec", head);
   
   if (TOK_SIMPLE != head->type) {
      log_token_type_error(cfg, ctx, tok, "list tag");
      return -__LINE__;
   }
   
   list = alloc_list(ctx);
   
   list->tag = str_from_tok(head);
   if (NULL == list->tag) {
      free_list(list);
      return -__LINE__;
   }

   if (get_next_token(ctx, head, tok) != 0) {
      log_truncation_error(cfg, ctx, head, "list", "tag", "value");
      free_list(list);
      return -__LINE__;
   } else if (TOK_LIST == tok->type) {
      if (parse_list_ext(ctx, list, tok, 0) != 0) {
	 log_parse_error(cfg, ctx, tok, "list", (char *)NULL);
	 free_list(list);
	 return -__LINE__;
      }
   } else {
      log_token_type_error(cfg, ctx, tok, "list tag");
      free_list(list);
      return -__LINE__;
   }   
   
   return store_cfg_list(ctx, list, cfg);
}

/* The following structures define what option names and values are
   accepted by the option parsing function.  */

typedef struct chkan2k_option_desc_s {
   const char *name;
   const size_t num_values;
   const char **values;
} CAN_OPTION_DESC;

const char* image_sets_values[] = {
   "twoindex", "tenprint", "segmented", "auto", "none"
};

const CAN_OPTION_DESC options[] = {
   { "image-sets", 
     sizeof image_sets_values/sizeof *image_sets_values,
     image_sets_values }
};

/*************************************************************************
**************************************************************************
   parse_option - Parse an option specification and store the results in the
                given configuration structure.  Currently only one type of
                option is defined.

   Input:
      ctx      - context information, e.g., input file, line number, etc.
      head     - the first token of the item being parsed
      cfg      - configuration structure in which to store results
   Output:
      ctx      - updated statistics
      cfg      - updated configuration structure
   Return:
      zero     - success
      negative - error
**************************************************************************/
static int
parse_option(CAN_CONTEXT *const ctx, const CAN_TOKEN *const head,
	     CAN_CONFIG *const cfg)
{
   int i, j;
   CAN_TOKEN tok[1];
   CAN_OPTION *opt;

   log_parse(cfg, ctx, "option", head);

   opt = alloc_option(ctx);

   /* get option name */
   opt->name = str_from_tok(head);
   if (NULL == opt->name) {
      free_option(opt);
      return -__LINE__;
   }

   /* verify option name */
   for (i = 0; i < sizeof options/sizeof *options; i++) {
      if (strcmp(options[i].name, opt->name) == 0) {
	 break;
      }
   }
   if (sizeof options/sizeof *options == i) {
      log_value_error(cfg, ctx, head, "option name", opt->name);
      free_option(opt);
      return -__LINE__;
   }

   /* get option value */
   if (get_next_token(ctx, head, tok) != 0) {
      log_truncation_error(cfg, ctx, head, "option", "name", "value");
      free_option(opt);
      return -__LINE__;
   }

   opt->value = str_from_tok(tok);
   if (NULL == opt->value) {
      free_option(opt);
      return -__LINE__;
   }

   /* verify option value */
   for (j = 0; j < options[i].num_values; j++) {
      if (strcmp(options[i].values[j], opt->value) == 0) {
	 break;
      }
   }
   if (options[i].num_values == j) {
      log_value_error(cfg, ctx, tok, "option value", (char *)NULL);
      free_option(opt);
      return -__LINE__;      
   }

   /* check end */
   if (get_next_token(ctx, tok, tok) == 0) {
      log_extra_data_error(cfg, ctx, tok, "option");
   }

   return store_cfg_option(ctx, opt, cfg);
}

/* The following typedefs, structure, and array and are used in determining
   which kind of entry is being parsed, and how to parse it. */
typedef int
(PARSE_FUNC)(CAN_CONTEXT *const, const CAN_TOKEN *const, CAN_CONFIG *const);

typedef struct chkan2k_category_s {
   char * name;
   PARSE_FUNC *func;
} CONF_CAT;

const static CONF_CAT config_categories[] = {
   { "standards", &parse_standard },
   { "records",   &parse_record },
   { "fields",    &parse_field },
   { "items",     &parse_item },
   { "lists",     &parse_list },
   { "options",   &parse_option }
};

/*************************************************************************
**************************************************************************
   lookup_config_category - Look up whether the value of a token
                corresponds to an identifier of a configuration category,
                and return a pointer to the corresponding CONF_CAT
                structure, or NULL.

   Input:
      tok      - the token that might contain an category name
   Return:
      non-NULL - success, address of appropriate CONF_CAT structure
      NULL     - not found, or error
**************************************************************************/
const CONF_CAT *
lookup_config_category(const CAN_TOKEN *tok)
{
   const CONF_CAT *p;

   for (p = config_categories; p - config_categories <
	   sizeof config_categories/sizeof config_categories[0]; p++) {
      if ((strlen(p->name) == (tok->end-tok->val)) &&
	  (strncmp(p->name, tok->val, tok->end-tok->val) == 0)) {
	 return p;
      }
   }
   return NULL;
}

/*************************************************************************
**************************************************************************
    change_category - Check to see if the given token specifies a new
                 category of things to parse, otherwise assume it is the
                 first token in an instance of the previously specified
                 category.

   Input:
      ctx      - context information, e.g., input file, line number, etc.
      tok      - next token
      cat      - previous category
   Output:
      ctx      - updated statistics
      cat      - updated category
   Return:
      1 - change to a new category
      0 - no change
**************************************************************************/
static int
change_category(CAN_CONTEXT *const ctx, const CAN_TOKEN *const tok,
		const CONF_CAT **cat)
{
   const CONF_CAT *result;
   
   result = lookup_config_category(tok);
   if (NULL == result) {
      if (NULL == *cat) {
	 can_log(LOGL_ERROR, LOGTP_CONFIG, (CAN_CONFIG *)NULL, ctx,
		 "Undefined configuration category " TOK_FMT 
		 " in %s at line %d.\n", TOK_ARGS(tok), ctx->name,
		 ctx->line_total);
      }
      return 0;
   } else {
      *cat = result;
      return 1;
   }
}

/*************************************************************************
**************************************************************************
    get_logical_line - Based roughly on fgets, this function reads a
                 "logical line" which may be extended, by use of
                 backslash-newline, to more than one physical line.  Since
                 a logical line may be arbitrarily long, the input buffer,
                 allocated from the heap, may be reallocated to provide as
                 much space as needed.

   Input:
      ctx      - context information, e.g., input file, line number, etc.
      s        - pointer to the location of the input buffer 
      size     - pointer to the size of the input buffer
  Output:
      ctx      - updated statistics
      s        - updated buffer location, in case it is reallocated
      size     - updated buffer size, in case it is reallocated
   Return:
      zero     - success
      one      - no more data
      negative - error
**************************************************************************/
static int
get_logical_line(CAN_CONTEXT *ctx, char **s, int *size) {
   int c, incr;
   char *p, *new;

   p = *s;
   while((c = getc(ctx->fp)) != EOF) {
      if ((p - *s) == *size) {
	 incr = *size > 0 ? *size : 100;
	 new = checked_realloc(ctx, *s, "input buffer", sizeof(char),
			       *size, incr);
	 if (NULL == new) {
	    return -__LINE__;
	 }
	 *s = new;
	 p = new + *size;
	 *size += incr;
      }
      *p = c;

      /* newline might be escaped, check and take appropriate action */
      if ('\n' == c) {
	 ++ctx->line_total;
	 if ((p > *s) && (*(p-1) == '\\')) {
	    --p;
	    continue;
	 }
	 *p = '\0';
	 return 0;
      }
      p++;
   }

   /* last line might include no newline */
   if ((p - *s) > 0) {
      *p = '\0';
      return 0;
   }

   return 1;
}

/*************************************************************************
**************************************************************************
   parse_config - Parse a chkan2k configuration file and produce a
                 structure containing the digested contents in a convenient
                 format for use by the ANSI/NIST file checking routines.

   Input:
      ctx      - context information, e.g., input file, line number, etc.
      cfg      - configuration structure in which to store results
   Output:
      ctx      - updated statistics
      cfg      - updated configuration structure
   Return:
      zero     - success
      negative - error
**************************************************************************/
static int
parse_config(CAN_CONTEXT *ctx, CAN_CONFIG *cfg)
{
   int buf_size, read_status;
   char *line_buf;   
   CAN_TOKEN head[1];
   const CONF_CAT *current_cat = NULL;

   buf_size = 0;
   line_buf = NULL;

   /* read each line and parse according the current state and contents */
   for (read_status = get_logical_line(ctx, &line_buf, &buf_size);
	0 == read_status;
	read_status = get_logical_line(ctx, &line_buf, &buf_size)) {

      if (ferror(ctx->fp) != 0) {
	 can_log(LOGL_ERROR, LOGTP_EXEC, cfg, ctx,
		 "Trouble reading configuration file '%s' at line %d: %s\n",
		 ctx->name, ctx->line_total, strerror(errno));
	 return -__LINE__;
      }

      if (get_first_token(ctx, line_buf, head) != 0) {
	 ++ctx->line_skip;      /* skip empty lines and comments */
	 can_log(LOGL_DEBUG, LOGTP_CONFIG, cfg, ctx,
		 "Skip %s line: \"%s\".\n",
		 token_type_name(head), line_buf);
	 continue;
      }

      if (change_category(ctx, head, &current_cat) == 1) {
	 can_log(LOGL_DEBUG, LOGTP_CONFIG, cfg, ctx,
		 "Change category to %s.\n", current_cat->name);
	 continue;
      }
      ++ctx->line_check;
      
      if (NULL == current_cat) {
	 can_log(LOGL_FATAL, LOGTP_CONFIG, cfg, ctx,
		 "Cannot parse line without a valid specification category.\n");
	 return -__LINE__;
      }

      /* parse the rest of the line here */
      if ((*current_cat->func)(ctx, head, cfg) != 0) {
	 can_log(LOGL_ERROR, LOGTP_CONFIG, cfg, ctx,
		 "Cannot parse %s: %s.\n", current_cat->name, line_buf);
	 return -__LINE__;
      }
   }

   if (ctx->line_check < 1) {	/* absolute minimum lines */
      can_log(LOGL_ERROR, LOGTP_CONFIG, cfg, ctx, "No configuration.\n");
      return -__LINE__;
   } else if ((NULL == cfg->parent) && (ctx->line_check < 50)) {
      can_log(LOGL_INFO, LOGTP_CONFIG, cfg, ctx,
	      "Unusually small configuration file, only %d specification%s.\n",
	      ctx->line_check, ctx->line_check != 1 ? "s" : "");
   }

   return 0;
}

/*************************************************************************
**************************************************************************
   read_config - Read configuration file and call parse_config to digest it
                 and produce a structure containing the digested contents
                 in a convenient format for use by the ANSI/NIST file
                 checking routines.

   Input:
      acc      - structure specifying the name of the configuration file
      prev     - structure containing contents of previously read config file
   Output:
      acc      - structure containing file statistics and information
                 about any problems or anomalies detected
   Return:
      NULL     - error
      non-NULL - address of a structure representing the configuration
                 parameters specifying what to check in an ANSI/NIST file
**************************************************************************/
const CAN_CONFIG *
read_config(CAN_CONTEXT *ctx, const CAN_CONFIG *const prev)
{
   struct stat statbuf;
   static CAN_CONFIG *cfg;
   char *filename;

   if (NULL == ctx->name) {
      can_log(LOGL_FATAL, LOGTP_CONFIG, (const CAN_CONFIG *)NULL, ctx,
	      "No configuration file specified.\n");
      return NULL;
   }
   ctx->line_total = ctx->line_skip = ctx->line_check = 0;

   if (stat(ctx->name, &statbuf) != 0) {
      filename = (char *)checked_alloc(ctx, "config filename", 1, 
				       strlen(DEFAULT_CONFIG_DIR) + strlen(ctx->name) + 2);
      sprintf(filename, "%s/%s", DEFAULT_CONFIG_DIR, ctx->name);
   } else {
      filename = (char *)ctx->name;
   }
   
   ctx->fp = fopen(filename, "r");
   if (NULL == ctx->fp) {
      can_log(LOGL_FATAL, LOGTP_CONFIG, cfg, ctx,
	      "Cannot open configuration file %s: %s.\n", filename, strerror(errno));
      return NULL;
   }

   if (filename != ctx->name) {
      free(filename);
   }
   
   cfg = alloc_config(ctx);
   cfg->name = ctx->name;
   /* This is the only place where we want to write the parent field. */
   ((CAN_CONFIG *)cfg)->parent = (CAN_CONFIG *)prev;
   if (parse_config(ctx, cfg) != 0) {
      free_config(cfg);
      cfg = NULL;
   }

   if (fclose(ctx->fp) != 0) {
      can_log(LOGL_WARNING, LOGTP_CONFIG, cfg, ctx,
	      "Trouble closing configuration file: %s.\n", strerror(errno));
   }
   return cfg;
}

#ifdef UNIT_TEST
/* The following code is used only for generating a program to test the
   correctness of the parsing of a configuration file. It parses a
   specified configuration file, generates errors and warnings, and prints
   out the contents of the configuration structure if it is successfully
   parsed. */

static void
print_standard(FILE *fpout, const CAN_STANDARD *const std)
{
   fprintf(fpout, "std: tag=%s, name=%s, ver ", std->tag, std->name);
   if (std->ver.type == ITM_STR) {
      fprintf(fpout, "str=%s", std->ver.str);
   } else if (std->ver.type == ITM_NUM) {
      fprintf(fpout, "num=%d", std->ver.u.num);
   } else if (std->ver.type == ITM_FP) {
      fprintf(fpout, "fp=%f", std->ver.u.fp);
   } else {
      fprintf(fpout, "unk type=%d", std->ver.type);
   }
   fprintf(fpout, ", ref=%s, date=%s\n", std->ref, std->date);
}

static void
print_record(FILE *fpout, const RECORD_SPEC *const rtp)
{
   fprintf(fpout, "rec: id=%d, std=%s, dtype=%s, name=%s.\n",
	   rtp->idnum, rtp->std->ref, rdt_str_from_enum(rtp->data_type),
	   rtp->name);
}

static void
print_field(FILE *fpout, const FIELD_SPEC *const fld)
{
   int i;

   fprintf(fpout, "fld: tag=%s, id=%d, std=%s, occ=%d to %d, "
	   "size=%d to %d, rec types=[", fld->tag, fld->idnum, fld->std->ref, 
	   fld->occ.min, fld->occ.max, fld->size.min, fld->size.max);
   for (i = 0;  i < fld->num_records; i++) {
      fprintf(fpout, "%d ", fld->records[i]->idnum);
   }
   fprintf(fpout, "\b], items=[");
   for (i = 0; i < fld->num_items; i++) {
      fprintf(fpout, "%s ", fld->items[i]->tag);
   }
   fprintf(fpout, "\b]\n");
}

static void
print_value(FILE *fpout, const ITEM_SPEC_VAL *const val)
{
   const char *name = lookup_item_type_desc_by_num(val->type)->name;

   switch (val->type) {
   case ITM_NUM:
   case ITM_SNUM:
      fprintf(fpout, "%s %d", name, val->u.num);
      break;
   case ITM_HEX:
      fprintf(fpout, "%s %x", name, (unsigned)val->u.num);
      break;
   case ITM_FP:
      fprintf(fpout, "%s %f", name, val->u.fp);
      break;
   case ITM_STR:
   case ITM_DATE:
      fprintf(fpout, "%s %s", name, val->str);
      break;
   case ITM_BIN:
   case ITM_IMAGE:
   default:
      fprintf(fpout, "%s %02x %02x %02x %02x...", name,
	      (unsigned)val->str[0], (unsigned)val->str[1] ,
	      (unsigned)val->str[2], (unsigned)val->str[3]);
      break;
   }
}

static void
print_list_ext(FILE *fpout, const CAN_LIST *const list)
{
   int i;

   fprintf(fpout, "list: tag=%s, vals=[", list->tag);
   for (i = 0; i < list->num_vals; i++) {
      if (ITM_NUM == list->vals[i]->type) {
	 fprintf(fpout, "%d ", list->vals[i]->u.num);
      } else if (ITM_FP == list->vals[i]->type) {
	 fprintf(fpout, "%f ", list->vals[i]->u.fp);
      } else if (ITM_STR == list->vals[i]->type) {
	 fprintf(fpout, "\"%s\" ", list->vals[i]->str);
      } else {
	 fprintf(stderr, "Unknown item value type %d.\n", list->vals[i]->type);
      }
   }
   fprintf(fpout, "\b]");
}

static void
print_list(FILE *fpout, const CAN_LIST *const list)
{
   print_list_ext(fpout, list);
   fprintf(fpout, "\n");
}

static void
print_item(FILE *fpout, const ITEM_SPEC *const itm)
{
   fprintf(fpout, "item: tag=%s, std=%s, occ=%d to %d, size=%d to %d, "
	   "type=%s, ", itm->tag, itm->std->ref, itm->occ.min, itm->occ.max,
	   itm->size.min, itm->size.max, 
	   lookup_item_type_desc_by_num(itm->type)->name);
   if (NULL != itm->min) {
      fprintf(fpout, "min=");
      print_value(fpout, itm->min);
      fprintf(fpout, ", ");
   }
   if (NULL != itm->min) {
      fprintf(fpout, "max=");
      print_value(fpout, itm->min);
      fprintf(fpout, ", ");
   }
   if (NULL != itm->enum_vals) {
      print_list(fpout, itm->enum_vals);
      fprintf(fpout, ", ");
   }
   fprintf(fpout, "\b\b \n");
}

static void
print_option(FILE *fpout, const CAN_OPTION *const opt)
{
   fprintf(fpout, "opt: name=%s, value=%s\n", opt->name, opt->value);
}

static void
print_config(FILE *fpout, /*@null@*/ const CAN_CONFIG *const cfg)
{
   int i;

   if (NULL == cfg) {
      fprintf(stderr, "No configuration data.\n");
      return;
   }
#define CAN_PRINT(WHAT)				\
   for (i = 0; i < cfg->num_##WHAT##s; i++) {	\
      print_##WHAT(fpout, cfg->WHAT##s[i]);	\
   }

   CAN_PRINT(standard)
   CAN_PRINT(record)
   CAN_PRINT(field)
   CAN_PRINT(item)
   CAN_PRINT(list)
   CAN_PRINT(option)
#undef CAN_PRINT
}

int debug = 0;			/* required by wsq library */

int
main(int argc, char **argv)
{
   CAN_CONTEXT config_ctx;
   const CAN_CONFIG *cfg;
   int opt;
   
   while ((opt = getopt(argc, argv, "d:")) != -1) {
      switch(opt) {
      case 'd':
	 set_log_level(optarg);
	 break;

      default:
	 fprintf(stderr, "Usage: %s [-d DEBUG | INFO | WARNING | "
		 "ERROR | FATAL ] config-file\n", argv[0]);
      }
   }
   init_result_accumulator(&config_ctx, argv[optind]);
   cfg = read_config(&config_ctx, (CAN_CONFIG *)NULL);
   if (NULL == cfg) {
      fprintf(stderr, "UNIT_TEST: Failed to read configuration file.\n");
      return EXIT_FAILURE;
   }
   report_result_accumulator(cfg, &config_ctx, 0);
   print_config(stderr, cfg);
   return EXIT_SUCCESS;
}
#endif
