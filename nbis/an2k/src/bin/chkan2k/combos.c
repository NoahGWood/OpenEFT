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
      PACKAGE: ANSI/NIST 2007 Reference Implementation

      FILE:    ANSI_NIST.C
      AUTHOR:  Joseph C. Konczal
      DATE:    02/11/2009


***********************************************************************
               ROUTINES:
                        check_record_combination_sense()
                        
***********************************************************************/

#include <stdio.h>
#include <errno.h>
#include <float.h>

#include "an2k.h"
#include "chkan2k.h"

/*************************************************************************
**************************************************************************
   check_record_combinations - Check the fingerprint images of an ANSI/NIST
               file for certain hard coded combinations of finger number
               and impression type.  If a specific combination was
               specified in the options section of the configuration file,
               check for it and report any deviations as errors. If no
               combination or "auto" was specified, use a heuristic to
               determine which, if any, is closest to the file contents and
               warn about any differences.

   Input:
      cfg       - configuration data structure
      ansi_nist - entire ANSI/NIST file structure
      stats     - current counts of things checked, types of issues found, etc.
   Output:
      stats     - updated statistics
**************************************************************************/
void
check_record_combinations(const CAN_CONFIG *const cfg,
			  const ANSI_NIST *const ansi_nist, 
			  CAN_CONTEXT *const stats)
{
   typedef enum { TWOINDEX = 500, TENPRINT, SEGMENTED } IMAGE_COUNT_TYPE;
   IMAGE_COUNT_TYPE image_count_type;
   LOGL log_level;
   CAN_OPTION *cnt_opt;
   RECORD *rec;
   FIELD *fgpf, *impf;
   int fgp_i, rec_i, fgpf_i, impf_i, fgp, imp;
   int rolled[11], flat[20];
   int total_count, twoindex_count, tenprint_count, segmented_count;
   const int twoindex_exp = 2, tenprint_exp = 14, segmented_exp = 22;
   float top_score, twoindex_score, tenprint_score, segmented_score;
   
   cnt_opt = lookup_option_in_cfg_by_name_deep(cfg, "image-sets");
   if ((NULL == cnt_opt) || (strcmp(cnt_opt->value, "none") == 0)) {
      return;		/* no need to check anything */
   }

   memset((void *)rolled, 0, sizeof rolled);
   memset((void *)flat, 0, sizeof flat);
   
   /* first check and warn if a combination of type-4 and type-14 records
      is found */
   if ((stats->record[TYPE_4_ID] > 0) 
       && (stats->record[TYPE_14_ID] > 0)) {
      can_log(LOGL_WARNING, LOGTP_CHECK, cfg, stats, 
	      "combination of %d type-4 and %d type-14 records\n",
	      stats->record[TYPE_4_ID], stats->record[TYPE_14_ID]);
   }

   /* separately count the rolled and flat impressions for each fingerprint
      of types 4 and 14 */
   total_count = 0;
   for(rec_i = 0; rec_i < ansi_nist->num_records; rec_i++) {
      rec = ansi_nist->records[rec_i];

      if ((rec->type == TYPE_4_ID) || (rec->type == TYPE_14_ID)) {
	 ++total_count;

	 if (lookup_FGP_field(&fgpf, &fgpf_i, rec) == 0) {
	    can_log(LOGL_ERROR, LOGTP_CHECK, cfg, stats, "FGP not found.\n");
	    continue;
	 }
	 if (parse_item_numeric(cfg, fgpf, (FIELD_SPEC *)NULL,
				rec_i, fgpf_i, 0, 0, &fgp, stats) != 0) {
	    can_log(LOGL_ERROR, LOGTP_CHECK, cfg, stats, 
		    "Cannot parse FGP: \"%s\".\n",
		    (char *)fgpf->subfields[0]->items[0]->value);
	    continue;
	 }

	 if (lookup_IMP_field(&impf, &impf_i, rec) == 0) {
	    can_log(LOGL_ERROR, LOGTP_CHECK, cfg, stats, "IMP not found.\n");
	    continue;
	 } 
	 if (parse_item_numeric(cfg, impf, (FIELD_SPEC *)NULL,
				rec_i, fgpf_i, 0, 0, &imp, stats) != 0) {
	    
	    can_log(LOGL_ERROR, LOGTP_CHECK, cfg, stats, 
		    "Cannot parse FGP: \"%s\".\n",
		    (char *)fgpf->subfields[0]->items[0]->value);
	    continue;
	 }

	 if (imp_is_rolled(imp) != 0) {
	    if (fgp > 10) {
	       can_log(LOGL_ERROR, LOGTP_CHECK, cfg, stats,
		       "rolled impression type %d of finger position %d "
		       "not possible, only 0 through 10 can be rolled.\n",
		       imp, fgp);
	    } else {
	       rolled[fgp]++;
	    }
	 } else if (imp_is_flat(imp) != 0) {
	    flat[fgp]++;
	 } else {
	    can_log(LOGL_DEBUG, LOGTP_CHECK, cfg, stats,
		    "impression type %d is neither rolled nor flat\n", imp);
	 }
      }
   }
   
   /* check for unidentified fingers */
   if (rolled[0] > 0) {
      can_log(LOGL_WARNING, LOGTP_CHECK, cfg, stats,
	      "file contains %d rolled image%s of unidentified finger%s\n",
	      rolled[0], rolled[0] > 1 ? "s" : "", rolled[0] > 1 ? "s" : "");
   }
   if (flat[0] > 0) {
      can_log(LOGL_WARNING, LOGTP_CHECK, cfg, stats,
	      "file contains %d flat image%s of unidentified finger%s\n",
	      flat[0], flat[0] > 1 ? "s" : "", flat[0] > 1 ? "s" : "");
   }


   twoindex_count = tenprint_count = segmented_count = 0;
   for (fgp_i = 1; fgp_i < 11; fgp_i++) {
      if (rolled[fgp_i] > 0) {
	 ++tenprint_count;
	 if (fgp_i == 2 || fgp_i == 7) {
	    ++twoindex_count;
	 }
      }
   }
   segmented_count = tenprint_count;

   for (fgp_i = 1; fgp_i < 15; fgp_i++) {
      if (flat[fgp_i] > 0) {
	 ++segmented_count;
	 if (fgp_i > 10) {
	    ++tenprint_count;
	 }
      }
   }

   if (total_count == 0) {
      return;			/* nothing to check */
   }

   if ((NULL == cnt_opt) || (strcmp(cnt_opt->value, "auto") == 0)) {
      /* Score values from -1.0 to +1.0 are computed by the subtracting the
	 ratio of unwanted impressions to total impressions from the ratio
	 of found impressions to required impressions, for each type of
	 combination. */
      twoindex_score = 
	 (float)twoindex_count / twoindex_exp
	 - (float)(total_count - twoindex_count) / total_count;
      
      tenprint_score =
	 (float)tenprint_count / tenprint_exp
	 - (float)(total_count - tenprint_count) / total_count;
      
      segmented_score =
	 (float)segmented_count / segmented_exp
	 - (float)(total_count - segmented_count) / total_count;
      
      can_log(LOGL_DEBUG, LOGTP_CHECK, cfg, stats, "image-count auto scores: "
	      "twoindex %f, tenprint %f, segmented %f.\n",
	      twoindex_score, tenprint_score, segmented_score);

      top_score = -1.0;
      if (twoindex_score > top_score) {
	 top_score = twoindex_score;
	 image_count_type = TWOINDEX;
      }
      if (tenprint_score > top_score) {
	 top_score = tenprint_score;
	 image_count_type = TENPRINT;
      }
      if (segmented_score > top_score) {
	 top_score = segmented_score;
	 image_count_type = SEGMENTED;
      }
      if (top_score < 0.4) {
	 return;		/* doesn't look much like anything */
      }

      log_level = LOGL_WARNING;
      
   } else if (strcmp(cnt_opt->value, "twoindex") == 0) {
      image_count_type = TWOINDEX;
      log_level = LOGL_ERROR;
   } else if (strcmp(cnt_opt->value, "tenprint") == 0) {
      image_count_type = TENPRINT;
      log_level = LOGL_ERROR;
   } else if (strcmp(cnt_opt->value, "segmented") == 0) {
      image_count_type = SEGMENTED;
      log_level = LOGL_ERROR;
   } else {
      can_log(LOGL_ERROR, LOGTP_CHECK, cfg, stats,
	      "Unimplemented image-count option: %s.\n", cnt_opt->value);
      return;
   }

#define MAKE_CASE_FOR(WHAT)						\
   if (WHAT##_count < WHAT##_exp) {					\
      can_log(log_level, LOGTP_CHECK, cfg, stats,			\
	      "image-count " #WHAT ": %d missing impressions.\n",	\
	      WHAT##_exp - WHAT##_count);				\
   }									\
   if (total_count > WHAT##_count) {					\
      can_log(log_level, LOGTP_CHECK, cfg, stats,			\
	      "image-count " #WHAT ": %d extraneous impressions.\n",	\
	      total_count - WHAT##_count);				\
   }									\
   break

   switch(image_count_type) {
   case TWOINDEX:
      MAKE_CASE_FOR(twoindex);
   case TENPRINT:
      MAKE_CASE_FOR(tenprint);
   case SEGMENTED:
      MAKE_CASE_FOR(segmented);
   default:
      can_log(LOGL_FATAL, LOGTP_CHECK, cfg, stats,
	      "invalid image_count_type: %d\n", image_count_type);
      break;
   }
#undef MAKE_CASE_FOR
}
