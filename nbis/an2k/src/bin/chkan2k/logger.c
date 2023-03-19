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
#include <stdarg.h>

#include "an2k.h"
#include "chkan2k.h"

/* The following macro and functions attempt to standardize the
   handling of different kinds of error messages, warnings, and
   informative messages. */

typedef struct chkan2k_error_level_descriptor_s {
   char *name;
   LOGL num;
   char *desc;
} LOGL_DESC;

const LOGL_DESC error_levels[] = {
   {"FATAL",   LOGL_FATAL,   "fatal error"},
   {"ERROR",   LOGL_ERROR,   "error"},
   {"WARNING", LOGL_WARNING, "minor or likely error"},
   {"INFO",    LOGL_INFO,    "informative message"},
   {"DEBUG",   LOGL_DEBUG,   "debugging information"}
};

typedef struct chkan2k_error_type_descriptor_s {
   char *name;
   LOGTP num;
   char *desc;
} LOGTP_DESC;

const LOGTP_DESC error_types[] = {
   {"EXEC",    LOGTP_EXEC,   "run-time"},
   {"CONFIG",  LOGTP_CONFIG, "configuration"},
   {"CHECK",   LOGTP_CHECK,  "conformance"}
};

static int reporting_level = LOGL_WARNING;

/*************************************************************************
**************************************************************************
   set_log_level - Set the severity level of log messages to report on
                   stderr.

   Input:
      arg - a character string uniquely identifying one of the log levels
   Return:
      The previous log level.
**************************************************************************/
int
set_log_level(const char *const arg)
{
   long level;
   int old_level;
   int i;
   
   level = UNSET;
   for (i = 0; i < sizeof error_levels/sizeof error_levels[0]; i++) {
      if (strncasecmp(arg, error_levels[i].name, strlen(arg)) == 0) {
         level = (long)error_levels[i].num;
         break;
      }
   }

   if (level == UNSET) {
      fprintf(stderr, "ERROR: %s is not a valid log level.\n", arg);
      exit(EXIT_FAILURE);
   }

   if (level < LOGL_BASE || level > (long)LOGL_DEBUG) {
      fprintf(stderr, "name\tdescription\n");
      for (i = 0; i < sizeof error_levels/sizeof error_levels[0]; i++) {
         printf(stderr, "\t%s\t%s\n", error_levels[i].name, 
            error_levels[i].desc);
      }
   }

   old_level = reporting_level;
   reporting_level = (int)level;
   return old_level;
}

/*************************************************************************
**************************************************************************
   log_va - Internal function used to print log messages to stderr with a
            common format followed by printf-style instance specific
            information, and to record statistics on the different levels
            of errors encountered.

   Input:
      level - severity level
      type  - type of operation that precipitated the trouble
      conf  - configuration structure used to check data
      field - pointer to the relevant ANSI/NIST FIELD, or NULL
      fd    - pointer the relevant FIELD_SPEC, or NULL
      rec_i - 0-based record index, or UNSET
      fld_i - 0-based field index, or UNSET
      sub_i - 0-based subfield index, or UNSET
      itm_i - 0-based item index, or UNSET
      stats - pointer to CAN_CONTEXT which accumulates statistics
      fmt   - printf style format
      ap    - standard argument list pointer
   Output:
      stats - pointer to updated statistics
**************************************************************************/
static void 
log_va(const LOGL level, const LOGTP type, const CAN_CONFIG *conf,
       const FIELD *const field, const FIELD_SPEC *const fd, const int rec_i,
       const int fld_i,const int sub_i, const int itm_i,
       CAN_CONTEXT *const stats, const char *const fmt, va_list ap)
{
   if ((level < LOGL_BASE) || (level > LOGL_BASE + LOGL_COUNT)) {
      (void)fprintf(stderr, "ERROR: severity %d out of range [%d, %d].\n",
		    level, LOGL_BASE, LOGL_BASE + LOGL_COUNT);
      exit(EXIT_FAILURE);
   } else if ((type < LOGTP_BASE) || (type > LOGTP_BASE + LOGTP_COUNT)) {
      (void)fprintf(stderr, "ERROR: error type %d out of range [%d, %d].\n",
		    type, LOGTP_BASE, LOGTP_BASE + LOGTP_COUNT);
   } else if (level <= reporting_level) {
      (void)fprintf(stderr, "%s %s: ", error_levels[level-LOGL_BASE].name,
		    error_types[type-LOGTP_BASE].name);
      if ((stats != NULL) && (stats->name != NULL)) {
	 if (stats->line_total > 0) {
	    (void)fprintf(stderr, "%s:%d: ", stats->name, stats->line_total);
	 } else {
	    (void)fprintf(stderr, "%s: ", stats->name);
	 }
      }
      if (rec_i != UNSET) {
	 (void)fprintf(stderr, "[%d", rec_i+1);
	 if (fld_i != UNSET) {
	    (void)fprintf(stderr, ".%d", fld_i+1);
	    if (sub_i != UNSET) {
	       (void)fprintf(stderr, ".%d", sub_i+1);
	       if (itm_i != UNSET) {
		  (void)fprintf(stderr, ".%d",  itm_i+1);
	       }
	    }
	 }
	 (void)fprintf(stderr, "] ");
      }
      if (field != NULL) {
	 fprintf(stderr, "[Type-%d.%03d] ",
		 field->record_type, field->field_int);
	 if (fd == NULL) {
	    *((const FIELD_SPEC **)(&fd)) = 
	       lookup_field_in_cfg_by_tf_ids_deep(conf, field->record_type,
						  field->field_int);
	 }
      } else if (fd != NULL) {
	 fprintf(stderr, "[Type-X.%03d] ", fd->idnum);
      }
      if (fd != NULL) {
	    fprintf(stderr, "%s ", fd->tag);
      }
      (void)vfprintf(stderr, fmt, ap);
   }
      
   if (stats != NULL) {
      ++stats->issue[type-LOGTP_BASE][level-LOGL_BASE];
   }
   if (LOGL_FATAL == level) {
      exit(EXIT_FAILURE);
   }
}

/*************************************************************************
**************************************************************************
   log_chk - Write information to the log about the results of checking the
                specified ANSI/NIST ITEM, SUBFIELD, SUBFIELD, or RECORD,
                depending on whether the corresponding indices and pointers
                are defined.

   Input:
      sev   - severity level
      field - pointer the ANSI/NIST FIELD containing the item
      fd    - pointer to the relevant FIELD_SPEC
      rec_i - 0-based record index, or UNSET
      fld_i - 0-based field index, or UNSET
      sub_i - 0-based subfield index, or UNSET
      itm_i - 0-based item index, or UNSET
      stats - pointer to CAN_CONTEXT which accumulates statistics
      fmt   - printf-style format string
      ...   - arguments required by the printf-style format string in fmt
   Output:
      stats - pointer to updated statistics
**************************************************************************/
void
log_chk(const LOGL sev, const CAN_CONFIG *conf, 
	const FIELD *const field, const FIELD_SPEC *const fd,
	const int rec_i, const int fld_i, const int sub_i, const int itm_i,
	CAN_CONTEXT *const stats, const char *const fmt, ...)
{
   va_list ap;

   va_start(ap, fmt);
   log_va(sev, LOGTP_CHECK, conf, field, fd, rec_i, fld_i, sub_i, itm_i,
	  stats, fmt, ap);
   va_end(ap);
}

/*************************************************************************
**************************************************************************
   can_log - Write information to the log not specifically pertaining to a
             particular field.

   Input:
      level - severity level of the event
      type  - type of thing that generated the event
      conf  - pointer to CAN_CONFIG
      stats - pointer to CAN_CONTEXT which accumulates statistics
      fmt   - printf-style format string
      ...   - arguments required by the printf-style format string in fmt
   Output:
      stats - pointer to updated statistics
**************************************************************************/
void 
can_log(const LOGL level, const LOGTP type, const CAN_CONFIG *const conf,
	CAN_CONTEXT *const stats, const char *const fmt, ...) {
   va_list ap;

   va_start(ap, fmt);
   log_va(level, type, conf, (FIELD*)NULL, (FIELD_SPEC *)NULL,
	  UNSET, UNSET, UNSET, UNSET, stats, fmt, ap);
   va_end(ap);
}

/*************************************************************************
**************************************************************************
   init_result_accumulator - Clear the parsing and error statistics
              accumulator, in preparation to start parsing an ANSI/NIST
              input file.

   Input:
      acc  - pointer to the CAN_CONTEXT to clear
      name - name of the file to be processed next
   Output:
      acc  - pointer to the initialized structure
**************************************************************************/
void
init_result_accumulator(CAN_CONTEXT *const acc, const char *const name)
{   
   memset(acc, 0, sizeof(CAN_CONTEXT));
   acc->name = name;
}

/*************************************************************************
**************************************************************************
   aggregate_result_accumulator - Add new results to a running total of
              previous results.

   Input:
      total - pointer to CAN_CONTEXT containing previous total results
      more  - pointer to CAN_CONTEXT containing results to add to the total
   Output:
      total - pointer to CAN_CONTEXT containing total combined results
**************************************************************************/
void 
aggregate_result_accumulator(CAN_CONTEXT *const total,
			     const CAN_CONTEXT *const more)
{
   int i, j;
   
   for (i = 0; i < LOGTP_COUNT; i++) {
      for (j = 0; j < LOGL_COUNT; j++) {
	 total->issue[i][j] += more->issue[i][j];
      }
   }
   total->rec_total += more->rec_total;
   total->rec_skip  += more->rec_skip;
   total->rec_check += more->rec_check;

   for (i = 0; i < NUM_RECORD_TYPE_SLOTS; i++) {
      total->record[i] += more->record[i];
   }
}

/*************************************************************************
**************************************************************************
   function_name - description...

   Input:
      arg   - description...
   Output:
      arg   - description...
   Return:
      description...
**************************************************************************/
static void 
report_record_type_counts(const CAN_CONFIG *const conf,
			  CAN_CONTEXT *const stats, const int show_all)
{
   int i;
   const RECORD_SPEC *rd;
   
   fprintf(stdout, "rec-type  count\t description\n");
   for (i = 0; i < NUM_RECORD_TYPE_SLOTS; i++) {
      if ((stats->record[i] > 0) || (show_all != 0)) {
	 rd = lookup_record_in_cfg_by_type_num_deep(conf, i);
	 if ((rd != NULL) || (stats->record[i] > 0)) {
	    fprintf(stdout, "%3d\t%4d\t%s\n", i, stats->record[i],
		    rd != NULL ? rd->name : "Invalid record type");
	 }
      }
   }
}

/*************************************************************************
**************************************************************************
   function_name - description...

   Input:
      arg   - description...
   Output:
      arg   - description...
   Return:
      description...
**************************************************************************/
static void
report_issue_type_counts(const CAN_CONTEXT *const stats, const int show_all)
{
   int i, j, header_printed = 0;
   const LOGL_DESC *level;
   const LOGTP_DESC *type;
   
   /* skip fatal errors, i.e., i == 0, since any of those would have
      prevented the program from reaching this point */
   for (i = 1; i < LOGL_COUNT; i++) {
      level = &error_levels[i];
      for (j = 0; j < LOGTP_COUNT; j++) {
	 if (stats->issue[j][i] > 0 || show_all != 0) {
	    if (header_printed == 0) {
	       fprintf(stdout, "err-type  level\t count\tdescription\n");
	       header_printed = 1;
	    }
	    type = &error_types[j];
	    fprintf(stdout, "%s\t%s\t%4d\t%s %s\n",
		    type->name, level->name, stats->issue[j][i],
		    type->desc, level->desc);
	 }
      }
   }
}

/*************************************************************************
**************************************************************************
   function_name - description...

   Input:
      arg   - description...
   Output:
      arg   - description...
   Return:
      description...
**************************************************************************/
void 
report_result_accumulator(const CAN_CONFIG *const conf,
			  CAN_CONTEXT *const stats, const int show_all)
{
   fprintf(stdout, "Summary of %s:\n", stats->name);
   report_record_type_counts(conf, stats, show_all);
   report_issue_type_counts(stats, show_all);
}

#define REC_LVL_CHUNK 10

/*************************************************************************
**************************************************************************
   function_name - description...

   Input:
      arg   - description...
   Output:
      arg   - description...
   Return:
      description...
**************************************************************************/
static void 
expand_record_field_accumulator(CAN_CONTEXT *const acc)
{
   int new_count;
   size_t new_size;  
   DUP_FLD_IDX *new_rfts;

   new_count = acc->rec_fld_alloc + REC_LVL_CHUNK;
   new_size = new_count * sizeof(DUP_FLD_IDX);
   can_log(LOGL_DEBUG, LOGTP_EXEC, NULL, acc, 
	   "Increasing allocation from %lu bytes to %lu, "
	   "to count %d kinds of fields in a record.\n", 
	   (unsigned long)acc->rec_fld_alloc * sizeof(DUP_FLD_IDX),
	   (unsigned long)new_size, new_count);

   new_rfts = (DUP_FLD_IDX *)realloc((void *)acc->rec_fld_types, new_size);
   if (new_rfts == NULL) {
      can_log(LOGL_FATAL, LOGTP_EXEC, NULL, acc,
	      "Cannot expand %lu bytes to %lu to count fields in a record.\n", 
	      (unsigned long)(acc->rec_fld_alloc * sizeof(DUP_FLD_IDX)),
	      (unsigned long)new_size);
   }
   acc->rec_fld_alloc = new_count;
   acc->rec_fld_types = new_rfts;
}

/*************************************************************************
**************************************************************************
   function_name - description...

   Input:
      arg   - description...
   Output:
      arg   - description...
   Return:
      description...
**************************************************************************/
void
reset_record_field_accumulator(CAN_CONTEXT *const acc)
{
   acc->rec_fld_used = 0;
}

/*************************************************************************
**************************************************************************
   function_name - description...

   Input:
      arg   - description...
   Output:
      arg   - description...
   Return:
      description...
**************************************************************************/
int
check_for_duplicate_fields(CAN_CONTEXT *const acc, const FIELD *const fld,
			   const int fld_i)
{
   DUP_FLD_IDX *rft;

   /* allocate more space if necessary */
   if (acc->rec_fld_used == acc->rec_fld_alloc) {
      expand_record_field_accumulator(acc);
   }

   /* find duplicates */
   for (rft = acc->rec_fld_types; 
	rft - acc->rec_fld_types < acc->rec_fld_used;
	rft++) {
      if (rft->field_num == fld->field_int) {
	 return rft->index;
      }
   }

   /* all records were checked with no duplicates found, add this one */
   if (rft - acc->rec_fld_types == acc->rec_fld_used) {
      ++acc->rec_fld_used;
      rft->field_num = fld->field_int;
      rft->index = fld_i;
   }
   return UNSET;
}				   
