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
      DATE:    02/06/2009

***********************************************************************
               ROUTINES:
 	                *format_field()
                        *chk_itm_size()
                        *chk_fld_size()
                        *ordinal_century_suffix()
                        *chk_date_fld()
                        *parse_item_numeric()
                        *chk_itm_num()
                        *chk_itm_img()
                        *chk_itm_str()
                        lookup_item_type_desc_by_name();
                        lookup_item_type_desc_by_num();
                        *check_item()
                        *check_field()
                        *chk_item_count()
                        *check_record()
                        check_ansi_nist()

***********************************************************************/

#include <stdio.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>

#include "an2k.h"
#include "chkan2k.h"

/*************************************************************************
**************************************************************************
   format_field - Format field contents into a user readable string in the
                  caller supplied buffer.  Designed for printing out field
                  and item contents in log messages.  Truncates results to
                  fit the user supplied buffer.

   Input:
      buf      - user supplied character buffer
      size     - size of the user supplied buffer
      field    - a pointer to the field to format
      one_sub  - if not equal to UNSET, the index of a single sub-field to
                 format instead of the whole field
      one_item - if not equal to UNSET, the index of a single item to format
                 instead of the whole field
   Output:
      buf   - formatted field contents, truncated with ellipsis if necessary
   Return:
      address of caller supplied buffer
**************************************************************************/
static char *
format_field(char *const buf, const int size, const FIELD *const field, 
	     const int one_sub, const int one_item)
{
   char *const end = buf + size;
   const char *ip;
   char *bp = buf;
   int sub_i, itm_i;
   int sub_start_i, sub_finish_i;
   int itm_start_i, itm_finish_i;

   if (UNSET != one_sub) {
      sub_start_i = one_sub;
      sub_finish_i = one_sub + 1;
   } else {
      sub_start_i = 0;
      sub_finish_i = field->num_subfields;
   }
   for (sub_i = sub_start_i; sub_i < sub_finish_i; sub_i++) {
      if ((sub_i > 0) && (sub_i != one_sub)) {
	 if (end - bp < 8) {
	    goto trunc;
	 }
	 strcpy(bp, "<RS>");
	 bp += 4;
      }
      
      if (one_item != UNSET) {
	 itm_start_i = one_item;
	 itm_finish_i = one_item + 1;
      } else {
	 itm_start_i = 0;
	 itm_finish_i = field->subfields[sub_i]->num_items;
      }
      for (itm_i = itm_start_i; itm_i < itm_finish_i; itm_i++) {
	 /* terminate each preceeding item before proceeding */
	 if ((itm_i > 0) && (itm_i != one_item)) {
	    if (end - bp < 8) {
	       goto trunc;
	    }
	    strcpy(bp, "<US>");
	    bp += 4;
	 }

	 /* generate a printable representation of an item's value */
	 for (ip = (char *)field->subfields[sub_i]->items[itm_i]->value;
	      *ip != '\0';
	      ip++) {
	    if (end - bp < 5) {
	       goto trunc;
	    } else if (isprint(*ip)) {
	       *bp++ = *ip;
	    } else if (end - bp < 9) {
	       goto trunc;
	    } else {
	       bp += sprintf(bp, "<%02x>", 0xff&(unsigned int)*ip);
	    }
	 }
      }
   }

   if (end - bp < 5) {
      goto trunc;
   }
   strcpy(bp, "<GS>");
   return buf;

 trunc:
   strcpy(bp, "...");
   return buf;
}

/*************************************************************************
**************************************************************************
   chk_itm_size - Check whether the size of the given item content, not
              including the terminators, conforms to the byte limits from
              the configuration file, which are loaded into the given item
              descriptor.  Log issues and update statistics.

   Input:
      cfg       - configuration data structure
      field     - field containing the item to check
      item      - the item to check
      rec_i     - record index, zero based
      fld_i     - field index, zero based
      sub_i     - subfield index, zero based
      itm_i     - item index, zero based
      isp       - item specification containing size limits
      fsp       - field specification for the containing field
      rsp       - record specification for the containing record
      ssp       - standard descriptor for the applicable standard
      stats     - counts of things checked, types of issues found, etc.
   Output:
      stats - updated statistics
**************************************************************************/
static void
chk_itm_size(const CAN_CONFIG *const cfg, const FIELD *field,
	     const int rec_i, const int fld_i, const int sub_i, const int itm_i,
	     const ITEM_SPEC *const isp, const FIELD_SPEC *const fsp,
	     const RECORD_SPEC *rsp, const CAN_STANDARD *ssp, 
	     CAN_CONTEXT *const stats)
{
   int size;
   char errbuf[40];
   const SUBFIELD *sub = field->subfields[sub_i];
   const ITEM *item = sub->items[itm_i];

   if (RDT_BINARY == rsp->data_type) {
      size = item->num_bytes; 	/* number of bytes read from the file */
   } else {
      size = item->num_chars;	/* # of bytes in value, excluding terminator */
   }
   
   if (((UNSET != isp->size.min) && (size < isp->size.min)) ||
       ((UNSET != isp->size.max) && (size > isp->size.max))) {
      if (isp->size.min == isp->size.max) {
	 log_chk(LOGL_ERROR, cfg, field, fsp, rec_i, fld_i, sub_i, itm_i,
		 stats, "item size %d bytes, not %d as required: '%s'\n",
		 size, isp->size.min, 
		 format_field(errbuf, sizeof errbuf, field, sub_i, itm_i));
      } else {
	 log_chk(LOGL_ERROR, cfg, field, fsp, rec_i, fld_i, sub_i, itm_i,
		 stats, "item size %d bytes, not %d to %d as required: '%s'\n",
		 size, isp->size.min, isp->size.max,
		 format_field(errbuf, sizeof errbuf, field, sub_i, itm_i));
      }
   }
}

/*************************************************************************
**************************************************************************
   chk_fld_size - Check whether the size of the given field content, not
              including the terminators, conforms to the byte limits from
              the configuration file, which are loaded into the field
              descriptor.  This actually checks the sizes of the subfields,
              since multiple instances of a field, when allowed by the
              standard, correspond to multiple subfields in the RECORD
              structure.  Log issues and update statistics.

   Input:
      cfg     - configuration data structure
      record  - record containing field to check
      rec_i   - record index, zero based
      fld_i   - field index, zero based
      fsp     - field descriptor containing size limits
      stats   - counts of things checked, types of issues found, etc.
   Output:
      stats   - updated statistics
**************************************************************************/
static void
chk_fld_size(const CAN_CONFIG *const cfg, const RECORD *record, 
	     const int rec_i, const int fld_i, const FIELD_SPEC *const fsp,
	     CAN_CONTEXT *const stats)
{
   char errbuf[40];
   int sub_i, size;
   const FIELD *field = record->fields[fld_i];
   
   for (sub_i = 0; sub_i < field->num_subfields; sub_i++) {
      size = field->subfields[sub_i]->num_bytes;
      if ((sub_i < field->num_subfields-1) && (fld_i < record->num_fields-1)) {
	 --size;		/* don't count subfield separator */
      }
      if (((UNSET != fsp->size.min) && (size < fsp->size.min)) ||
	  ((UNSET != fsp->size.max) && (size > fsp->size.max))) {

	 if (fsp->size.min == fsp->size.max) {
	    log_chk(LOGL_ERROR,
		    cfg, field, fsp, rec_i, fld_i, sub_i, UNSET, stats,
		    "field size %d bytes, not %d as required: '%s'\n",
		    size, fsp->size.max, 
		    format_field(errbuf, sizeof errbuf, field, sub_i, UNSET));
	 } else {
	    log_chk(LOGL_ERROR,
		    cfg, field, fsp, rec_i, fld_i, sub_i, UNSET, stats,
		    "field size %d bytes, not %d to %d as required: '%s'\n",
		    size, fsp->size.min, fsp->size.max,
		    format_field(errbuf, sizeof errbuf, field, sub_i, UNSET));
	 }
      }
   }
}

/*************************************************************************
**************************************************************************
   ordinal_century_suffix - Determine what ordinal suffix to add to the
              two-digit century part of a date.  Used by chk_itm_date.

   Input:
      cardinal - the cardinal number of the century, e.g., CC from CCYYMMDD
   Return:
      appropriate suffix string to append to the numarical century
**************************************************************************/
static char *
ordinal_century_suffix(int cardinal)
{
   if (cardinal < 0) {
      cardinal = -cardinal;
   }
   ++cardinal;
   
   if ((cardinal%10 == 1) && (cardinal != 11)) {
      return "st";
   } else if ((cardinal%10 == 2) && (cardinal != 12)) {
      return "nd";
   } else if ((cardinal%10 == 3) && (cardinal != 13)) {
      return "rd";
   } else {
      return "th";
   }
}

/*************************************************************************
**************************************************************************
   chk_itm_date - Additional item check function to check the value of the
              common type of ANSI/NIST date: an 8-byte non-GMT date field
              without time-of-day, log issues, and update statistics.

   Input:
      cfg       - configuration data structure
      field     - field to check
      rec_i     - record index
      fld_i     - field index
      sub_i     - subfield index
      itm_i     - item index
      isp       - item specification containing size limits
      fsp       - field specification for the containing field
      rsp       - record specification for the containing record
      ssp       - standard descriptor for the applicable standard

      stats     - current counts of things checked, types of issues found, etc.
   Output:
      stats     - updated statistics
**************************************************************************/
static void 
chk_itm_date(const CAN_CONFIG *const cfg, const FIELD *field,
	     const int rec_i, const int fld_i, const int sub_i, const int itm_i,
	     const ITEM_SPEC *const isp, const FIELD_SPEC *const fsp,
	     const RECORD_SPEC *rsp, const ANSI_NIST *const ansi_nist,
	     CAN_CONTEXT *const stats)
{
   const char *const datestr = 
      (char *)field->subfields[sub_i]->items[itm_i]->value;
   int count, cen, year, mon, day, last_day, errors = 0;
   char end[11];
   
   /* first verify that we can parse the date, the length has already been
      checked and any appropriate error message issued */
   count = sscanf(datestr, "%2d%2d%2d%2d%10s", &cen, &year, &mon, &day, end);
   if (count != 4) {
      ++errors;
      if (count == 5) {
	 log_chk(LOGL_ERROR, cfg, field, fsp,
		 rec_i, fld_i, sub_i, itm_i, stats,
		 "extraneous data '%s' at end of date '%s'\n", end, datestr);
      } else {
	 if (count == EOF) {
	    count = 0;
	 }
	 log_chk(LOGL_ERROR, cfg, field, fsp,
		 rec_i, fld_i, sub_i, itm_i, stats,
		 "four 2-digit fields required (CCYYMMDD), %d found in '%s'\n",
		 count, datestr); 
      }
   } else {
      /* check for a reasonable century, i.e., 20th or 21st (1900s or 2000s) */
      if (cen < 19) {
	 ++errors;
	 log_chk(LOGL_WARNING,
		 cfg, field, fsp, rec_i, fld_i, sub_i, itm_i, stats,
		 "date in the %d%s century%s: '%s'\n", 
		 (cen < 0) ? -cen+1 : cen+1 , ordinal_century_suffix(cen),
		 (*datestr == '-') ? " BC" : "", datestr);
      } else {
	 const time_t tt_now = time((time_t *)NULL);
	 const struct tm *tm_now = localtime(&tt_now);
	 const int year_now  = tm_now->tm_year % 100;
	 const int cen_now   = (tm_now->tm_year - year_now + 1900) / 100;
	 const int mon_now   = tm_now->tm_mon + 1;
	 const int mday_now  = tm_now->tm_mday + 1;
	 
	 if ((cen_now < cen)
	     || ((cen_now == cen) && (year_now < year))
	     || ((cen_now == cen) && (year_now == year) && (mon_now < mon))
	     || ((cen_now == cen) && (year_now == year) && (mon_now == mon)
		 && mday_now < day)) {
	    log_chk(LOGL_ERROR,
		    cfg, field, fsp, rec_i, fld_i, sub_i, itm_i, stats,
		    "date in the future (it's now %02d%02d%02d%02d): '%s'\n", 
		    cen_now, year_now, mon_now, mday_now, datestr);
	 }
      }

      /* any year 00-99 seems reasonable enough */
      if ((mon < 1) || (mon > 12)) {
	 ++errors;
	 log_chk(LOGL_ERROR, cfg, field, fsp, rec_i, fld_i, sub_i, itm_i,
		 stats, "month %d out of range 1-12 in '%s'\n", mon, datestr);
      }

      /* determine the last day of the particular month */
      if (2 == mon) {
	 /* February: extra day every 4 years, except years divisible by
	    100 but not 400.  (However, the variable 'year' means year 0-99
	    of century 'cen'.) */
	 last_day = (((year%4 == 0) && (year != 0))
		     || ((year == 0) && (cen%4 == 0))) ? 29 : 28;
      } else if ((4 == mon) || (6 == mon) || (9 == mon) || (11 == mon)) {
	 /* April, June, September, and November are also short */
	 last_day = 30;
      } else {
	 /* The other seven are longer. */
	 last_day = 31;
      }

      if ((day < 1) || (day > last_day)) {
	 ++errors;
	 log_chk(LOGL_ERROR, cfg, field, fsp, rec_i, fld_i, sub_i, itm_i,
		 stats, "day %d out of range 1-%d for month %d in '%s'\n",
		 day, last_day, mon, datestr);
      }
   }
   
   if (errors == 0) {
      log_chk(LOGL_DEBUG, cfg, field, fsp, rec_i, fld_i, sub_i, itm_i, stats,
	      "is a reasonable date '%s'\n", datestr);
   }
}

/*************************************************************************
**************************************************************************
   parse_item_numeric - Check whether the value of an item can be parsed
              into a valid number within a specified range, log issues,
              and update statistics.

   Input:
      cfg     - configuration data structure
      field   - field to check
      fsp      - field descriptor
      rec_i   - current record index
      fld_i   - current field index
      sub_i   - index of subfield to check
      stats   - current counts of things checked, types of issues found, etc.
   Output:
      res     - the result is stored where this points
      stats   - updated statistics
   Return:
      zero - success
      negative - failure
**************************************************************************/
int
parse_item_numeric(const CAN_CONFIG *const cfg, const FIELD *const field,
		   const FIELD_SPEC *const fsp, const int rec_i, const int fld_i, 
		   const int sub_i, const int itm_i, int *const res, 
		   CAN_CONTEXT *const stats)
{
   SUBFIELD *sub;
   ITEM *itm;
   char *valstr, *end;
   long valnum;

   if (lookup_ANSI_NIST_subfield(&sub, sub_i, field) == FALSE) {
      log_chk(LOGL_ERROR, cfg, field, fsp, rec_i, fld_i, sub_i, UNSET, stats,
	      "subfield not found\n");
      return -__LINE__;
   }

   if (lookup_ANSI_NIST_item(&itm, itm_i, sub) == FALSE) {
      log_chk(LOGL_ERROR, cfg, field, fsp, rec_i, fld_i, sub_i, itm_i, stats,
	      "item not found\n");
      return -__LINE__;
   }

   valstr = (char *)itm->value;

   errno = 0;
   valnum = strtol(valstr, &end, 10);
   if (errno != 0) {
      log_chk(LOGL_ERROR, cfg, field, fsp, rec_i, fld_i, sub_i, itm_i, stats,
	      "cannot convert %s value '%s' to a number: %s\n",
	      fsp->tag, valstr, strerror(errno));
      return -__LINE__;
   }
   if (*end != '\0') {
      log_chk(LOGL_ERROR, cfg, field, fsp, rec_i, fld_i, sub_i, itm_i, stats,
	      "unexpected non-numeric character '%c' (0x%x) in number\n",
	      *end, (unsigned int)*end);
      return -__LINE__;
   }
   if ((valnum < INT_MIN) || (valnum > INT_MAX)) {
      log_chk(LOGL_ERROR, cfg, field, fsp, rec_i, fld_i, sub_i, itm_i, stats,
	      "numeric value %ld outside supported range %d to %d\n",
	      valnum, 0, INT_MAX);
      return -__LINE__;
   } 

   *res = (int)valnum;
   return 0;
}

/*************************************************************************
**************************************************************************
   chk_itm_num - Check whether the value of an item can be parsed
              into a valid number within a specified range, log issues,
              and update statistics.

   Input:
      cfg       - configuration data structure
      field     - field to check
      rec_i     - current record index
      fld_i     - current field index
      sub_i     - index of subfield to check
      itm_i     - index of item to check
      isp       - item specification containing size limits
      fsp       - field specification for the containing field
      rsp       - record specification for the containing record
      ansi_nist - entire ANSI/NIST file structure
      stats     - current counts of things checked, types of issues found, etc.
   Output:
      stats     - updated statistics
**************************************************************************/
static void
chk_itm_num(const CAN_CONFIG *const cfg, const FIELD *const field,
	    const int rec_i, const int fld_i, const int sub_i, const int itm_i,
	    const ITEM_SPEC *const isp, const FIELD_SPEC *const fsp, 
	    const RECORD_SPEC *const rsp, const ANSI_NIST *const ansi_nist,
	    CAN_CONTEXT *const ctx)
{   
   int valnum;
   ITEM_SPEC_VAL *val;

   if (parse_item_numeric(cfg, field, fsp, rec_i, fld_i, sub_i, itm_i, 
			  &valnum, ctx) != 0) {
      return;
   }
   
   if ((NULL != isp) 
       && (((NULL != isp->min) && (UNSET != isp->min->u.num) 
	    && (valnum < isp->min->u.num))
	   || ((NULL != isp->max) && (UNSET != isp->max->u.num)
	       && (valnum > isp->max->u.num)))) {
      log_chk(LOGL_ERROR, 
	      cfg, field, fsp, rec_i, fld_i, sub_i, itm_i, ctx,
	      "numeric value %ld outside valid range %d to %d\n",
	      valnum, (NULL != isp->min) ? isp->min->u.num : UNSET,
	      (NULL != isp->max) ? isp->max->u.num : UNSET);
   }

   if ((NULL != isp) && (NULL != isp->enum_vals)) {
      val = lookup_val_in_lst_by_num_flat(isp->enum_vals, valnum);
      if (NULL == val) {
	 log_chk(LOGL_ERROR, cfg, field, fsp, rec_i, fld_i, sub_i, itm_i,
		 ctx, "unexpected numerical item value: %s.\n",
		 (char *)field->subfields[sub_i]->items[itm_i]->value);
      }
   }
}


/*************************************************************************
**************************************************************************
   chk_itm_img - Additional field check function to check the data, verify
              that image data can be uncompressed, and that specified image
              characteristics correspond to what is found in the data, log
              issues, and update statistics.

   Input:
      cfg       - configuration data structure
      field     - field to check
      rec_i     - current record index
      fld_i     - current field index
      sub_i     - index of subfield to check
      itm_i     - index of item to check
      isp       - item specification containing size limits
      fsp       - field specification for the containing field
      rsp       - record specification for the containing record
      ansi_nist - entire ANSI/NIST file structure
      stats     - current counts of things checked, types of issues found, etc.
   Output:
      stats     - updated statistics
**************************************************************************/
static void 
chk_itm_img(const CAN_CONFIG *const cfg, const FIELD *const field,
	    const int rec_i, const int fld_i, const int sub_i, const int itm_i,
	    const ITEM_SPEC *const isp, const FIELD_SPEC *const fsp,
	    const RECORD_SPEC *const rsp, const ANSI_NIST *ansi_nist,
	    CAN_CONTEXT *const stats)
{
   FIELD *fp;
   int fi, hll, vll, bpx, iw, ih, id, ret;
   double ippmm;
   unsigned char *idata;

   if ((lookup_ANSI_NIST_field(&fp, &fi, HLL_ID,
			       ansi_nist->records[rec_i]) != 0)
       || (parse_item_numeric(cfg, fp, fsp, rec_i, HLL_ID, 0, 0,
			      &hll, stats) != 0)) {
      hll = UNSET;
   }

   if ((lookup_ANSI_NIST_field(&fp, &fi, VLL_ID,
			       ansi_nist->records[rec_i]) != 0)
       || (parse_item_numeric(cfg, fp, fsp, rec_i, VLL_ID, 0, 0, 
			      &vll, stats) != 0)) {
      vll = UNSET;
   }

   if (field->record_type == 10) {
      bpx = UNSET;
   } else if ((field->record_type == 3) || (field->record_type == 4)) {
      bpx = 8;
   } else if ((field->record_type == 5) || (field->record_type == 6)) {
      bpx = 1;
   } else if ((lookup_ANSI_NIST_field(&fp, &fi, BPX_ID,
				      ansi_nist->records[rec_i]) != 0)
	      || (parse_item_numeric(cfg, fp, fsp, rec_i, BPX_ID, 0, 0,
				     &bpx, stats) != 0)) {
      bpx = UNSET;
   }
   
   ret = decode_ANSI_NIST_image(&idata, &iw, &ih, &id, &ippmm,
				ansi_nist, rec_i, 0);

   if (ret <= 0) {
      log_chk(LOGL_ERROR, cfg, field, fsp, rec_i, fld_i, UNSET, UNSET,
	      stats, "cannot decode image, error code %d\n", ret);
      return;
   }
   
   if ((hll != UNSET) && (hll != iw)) {
      log_chk(LOGL_WARNING, cfg, field, fsp, rec_i, fld_i, sub_i, itm_i, 
	      stats, "HLL %d does not match width %d from image\n", hll, iw);
   }

   if ((vll != UNSET) && (vll != ih)) {
      log_chk(LOGL_WARNING, cfg, field, fsp, rec_i, fld_i, sub_i, itm_i,
	      stats, "VLL %d does not match height %d from image\n", vll, ih);
   }

   if ((bpx != UNSET) && (bpx != id)) {
      log_chk(LOGL_WARNING, cfg, field, fsp, rec_i, fld_i, sub_i, itm_i,
	      stats, "BPX %d does not match depth %d from image\n", bpx, id);
   }

   free(idata);
}

/*************************************************************************
**************************************************************************
   chk_itm_str - Check whether the content of the given item conforms to
              any restrictions given by an enumeration of allowable values
              in the configuration file, which are loaded into the given
              item descriptor.  Log issues and update statistics.

   Input:
      cfg       - configuration data structure
      field     - field containing the item to check
      rec_i     - current record index
      fld_i     - current field index
      sub_i     - index of subfield to check
      itm_i     - index of item to check
      isp       - item specification containing size limits
      fsp       - field specification for the containing field
      rsp       - record specification for the containing record
      ansi_nist - entire ANSI/NIST file structure
      stats     - counts of things checked, types of issues found, etc.
   Output:
      stats     - updated statistics
**************************************************************************/
static void
chk_itm_str(const CAN_CONFIG *const cfg, const FIELD *field,
	    const int rec_i, const int fld_i, const int sub_i, const int itm_i,
	    const ITEM_SPEC *const isp, const FIELD_SPEC *const fsp,
	    const RECORD_SPEC *rsp, const ANSI_NIST *ansi_nist,
	    CAN_CONTEXT *const stats)
{
   ITEM_SPEC_VAL *val;
   ITEM *const item = field->subfields[sub_i]->items[itm_i];

   if (ITM_STR == isp->type) {
      if (NULL !=  isp->enum_vals) {
	 val = lookup_val_in_lst_by_str_flat(isp->enum_vals, (char *)item->value);
	 if (NULL == val) {
	    log_chk(LOGL_ERROR, cfg, field, fsp, rec_i, fld_i, sub_i, itm_i,
		    stats, "unexpected item value: %s.\n", (char *)item->value);
	 }
      }
   }
}

const ITEM_SPEC_TYPE_DESC item_types[] = {
   { ITM_NUM,   "num",   &chk_itm_num },
   { ITM_SNUM,  "snum",  &chk_itm_num },
   { ITM_CNUM,  "cnum",  NULL },
   { ITM_HEX,   "hex",   NULL },
   { ITM_FP,    "fp",    NULL },
   { ITM_STR,   "str",   &chk_itm_str },
   { ITM_BIN,   "bin",   NULL },
   { ITM_DATE,  "date",  &chk_itm_date },
   { ITM_IMAGE, "image", &chk_itm_img }
};

const int num_item_types = sizeof item_types / sizeof *item_types;

/*************************************************************************
**************************************************************************
   lookup_item_type_desc_by_name - lookup an item descriptor, in the
                 'item_types' list defined above, corresponding to the item
                 name specified in the parameter.

   Input:
      name - the name of item type whose descriptor is desired
   Output:
      NULL - failure
      pointer to an ITEM_SPEC_TYPE_DESC - success
**************************************************************************/
const ITEM_SPEC_TYPE_DESC *
lookup_item_type_desc_by_name(const char *const name)
{
   const ITEM_SPEC_TYPE_DESC *is;

   for (is = item_types; is - item_types < num_item_types; is++) {
      if (strcmp(is->name, name) == 0) {
	 return is;
      }
   }
   return NULL;
}


/*************************************************************************
**************************************************************************
   lookup_item_type_desc_by_num - lookup an item descriptor, in the
                 'item_types' list defined above, corresponding to the item
                 number specified in the parameter.

   Input:
     num - the number of the item type whose descriptor is desired
   Output:
      NULL - failure
      pointer to an ITEM_SPEC_TYPE_DESC - success
**************************************************************************/
const ITEM_SPEC_TYPE_DESC *
lookup_item_type_desc_by_num(const int num)
{
   const ITEM_SPEC_TYPE_DESC *is;

   for (is = item_types; is - item_types < num_item_types; is++) {
      if (is->type == num) {
	 return is;
      }
   }
   return NULL;
}

/*************************************************************************
**************************************************************************
   check_item - Check various generic and specific aspects of the given
                 ITEM, log issues, and update statistics.

   Input:
      cfg       - configuration data structure
      field     - field containing the item to check
      rec_i     - current record index
      fld_i     - current field index
      sub_i     - current subfield index
      itm_i     - current item index
      fsp       - field specification for the containing field
      rsp       - record specification for the containing record
      ssp       - standard descriptor for the applicable standard
      ansi_nist - ANSI/NIST structure containing the data that includes
                  this item
      stats     - current counts of things checked, types of issues found, etc.
   Output:
      stats     - updated statistics
**************************************************************************/
static void
check_item(const CAN_CONFIG *const cfg, const FIELD *const field,
	   const int rec_i, const int fld_i, const int sub_i, const int itm_i,
	   const FIELD_SPEC *fsp, const RECORD_SPEC *rsp, 
	   const CAN_STANDARD *ssp, const ANSI_NIST *const ansi_nist,
	   CAN_CONTEXT *const stats)
{
   char buf[40];
   const ITEM_SPEC *loop_isp, *isp = NULL;
   const ITEM_SPEC_TYPE_DESC *itd;
   int spec_i, max_items = 0;

   ++stats->itm_total;

   /* We have the index of the item in its subfield, and we want to match
      it up with an item specification.  Optional items that are not
      present require empty placeholders if there are any following items.
      Item parameters may apply to multiple instances.  When execution
      exits this loop, 'isp' will point to the correct item descriptor if
      possible, otherwise NULL. */
   isp = NULL;
   for (spec_i = 0; spec_i < fsp->num_items; spec_i++) {
      loop_isp = fsp->items[spec_i];
      if (loop_isp->occ.max == UNSET) { 
	 /* indefinite item count */
	 if (spec_i+1 == fsp->num_items) { /* last item spec */
	    max_items = INT_MAX;
	    break;
	 } else {		/* cannot check item */
	    log_chk(LOGL_WARNING, cfg, field, fsp,
		    rec_i, fld_i, sub_i, itm_i, stats,
		    "Cannot check item, unclear which criteria to use.\n");
	    return;
	 }
      }
      
      if ((itm_i >= max_items) && (itm_i < max_items+loop_isp->occ.max)) {
	 isp = loop_isp;
      }
      max_items += loop_isp->occ.max;
   }
   
   if (0 == fsp->num_items) { /* no item criteria for the field */
      log_chk(LOGL_WARNING, cfg, field, fsp,
	      rec_i, fld_i, sub_i, itm_i, stats,
	      "No criteria found, item not checked: %s\n",
	      format_field(buf, sizeof buf, field, sub_i, itm_i));
   } else if (NULL != isp) {
      /* check the size of the item */
      chk_itm_size(cfg, field, rec_i, fld_i, sub_i, itm_i, isp, fsp, rsp, ssp,
		   stats);
      
      /* check details specific to a particular type of item */
      itd = lookup_item_type_desc_by_num(isp->type);
      if ((NULL != itd) && (NULL != itd->func)) {
	 (itd->func)(cfg, field, rec_i, fld_i, sub_i, itm_i,
		     isp, fsp, rsp, ansi_nist, stats);
      }
   }
}

/*************************************************************************
**************************************************************************
   chk_item_count - Check whether the number of items in a subfield falls
              within the specified limits, log issues, and update
              statistics.

   Input:
      cfg     - configuration data structure
      field   - field to check
      fsp      - field descriptor
      rec_i   - current record index
      fld_i   - current field index
      sub_i   - index of subfield to check
      stats   - current counts of things checked, types of issues found, etc.
      min     - the fewest items allowed
      max     - the most items allowed
   Output:
      stats   - updated statistics
**************************************************************************/
static void
chk_item_count(const CAN_CONFIG *const cfg, const FIELD *const field,
	       const FIELD_SPEC *const fsp, const int rec_i, const int fld_i,
	       const int sub_i, CAN_CONTEXT *const stats)
{
   int min = 0, max = 0, isp_i;
   const int icnt = field->subfields[sub_i]->num_items;
   ITEM_SPEC *isp;

   if (0 == fsp->num_items) {
      log_chk(LOGL_WARNING, cfg, field, fsp, rec_i, fld_i, sub_i, UNSET, stats,
	      "No item criteria, cannot determine correct number of items.\n");
      return;
   }
   
   for (isp_i = 0; isp_i < fsp->num_items; isp_i++) {
      isp = fsp->items[isp_i];

      if ((UNSET != max) && (UNSET != isp->occ.min)) {
	 min = max + isp->occ.min;
      }

      if (UNSET == isp->occ.max) {
	 max = UNSET;
      } else if (UNSET != max) {
	 max += isp->occ.max;
      }
   }

   
   if (UNSET == max) {
      max = MAX_ITEMS;
   }
   if ((icnt < min) || (icnt > max)) {
      if (min == max) {
	 log_chk(LOGL_ERROR,
		 cfg, field, fsp, rec_i, fld_i, sub_i, UNSET, stats,
		 "number of items %d is not %d as required\n", icnt, max);
      } else {
	 log_chk(LOGL_ERROR,
		 cfg, field, fsp, rec_i, fld_i, sub_i, UNSET, stats,
		 "number of items %d is not %d to %d as required\n",
		 icnt, min, max);
      }
   }
}

/*************************************************************************
**************************************************************************
   check_field - Check various generic and specific aspects of the given
                 FIELD, log issues, and update statistics.

   Input:
      cfg       - configuration data structure
      field     - field to check
      rec_i     - current record index
      fld_i     - current field index
      rsp       - record specification for the containing record
      ssp       - standard descriptor for the applicable standard
      ansi_nist - ANSI/NIST structure containing the data that includes
                  this field
      stats     - current counts of things checked, types of issues found, etc.
   Output:
      stats     - updated statistics
**************************************************************************/
static void
check_field(const CAN_CONFIG *const cfg, const FIELD *const field, 
	    const int rec_i, const int fld_i, const RECORD_SPEC *rsp,
	    const CAN_STANDARD *ssp, const ANSI_NIST *const ansi_nist, 
	    CAN_CONTEXT *const stats)
{
   const FIELD_SPEC *fsp;
   char errbuf[80];
   int dup_fld;
   int sub_i, itm_i;

   ++stats->fld_total;

   /* Find the parameters for this field. */
   fsp = lookup_field_in_cfg_by_tf_ids_deep(cfg, rsp->idnum, field->field_int);

   /* Check for duplicate fields */
   dup_fld = check_for_duplicate_fields(stats, field, fld_i);
   if (dup_fld != UNSET) {
      log_chk(LOGL_ERROR,
	      cfg, field, fsp, rec_i, fld_i, UNSET, UNSET, stats,
	      "duplicate field, index of first instance [%d.%d]\n",
	      rec_i+1, dup_fld+1);
   }

   /* Check validity of field ID */
   if (NULL == fsp) {
      ++stats->fld_skip;
      if (((field->record_type >= 10) && (field->field_int >= 200)
	   && (field->field_int <= 998))
	  || ((field->record_type == 2) && (field->field_int >= 3)
	      && ((field->field_int <= 999) || (ansi_nist->version > 400)))) {
	 log_chk(LOGL_WARNING,
		 cfg, field, (FIELD_SPEC *)NULL, rec_i, fld_i, UNSET, UNSET,
		 stats, "User-defined field (UDF): '%s'\n", 
		 format_field(errbuf, sizeof errbuf, field, UNSET, UNSET));
      } else {
	 log_chk((field->record_type == 2) ? LOGL_WARNING : LOGL_ERROR,
		 cfg, field, (FIELD_SPEC *)NULL, rec_i, fld_i, UNSET, UNSET,
		 stats, "unknown field: '%s'\n", 
		 format_field(errbuf, sizeof errbuf, field, UNSET, UNSET));
      }
      return;
   }
   ++stats->fld_check;

   /* check ANSI/NIST standard version */
   if ((strcmp(fsp->std->name, "ANSI/NIST") == 0)
       && (ansi_nist->version < fsp->std->ver.u.num)) {
      log_chk(LOGL_ERROR, cfg, field, fsp, 
	      rec_i, fld_i, UNSET, UNSET, stats,
	      "requires ANSI/NIST version %d, file indicates version %d.\n",
	      fsp->std->ver.u.num, ansi_nist->version);
   }

   /* check for unwanted fields, or the wrong number of subfields */
   if (fsp->occ.max == 0) {
      log_chk(LOGL_ERROR, cfg, field, fsp, rec_i, fld_i, UNSET, UNSET, stats,
	      "forbidden field\n");
   } else if ((fsp->occ.max != UNSET) && (field->num_subfields > fsp->occ.max)) {
      log_chk(LOGL_ERROR, cfg, field, fsp, rec_i, fld_i, UNSET, UNSET,
	      stats, "too many subfields %d, max = %d\n",
	      field->num_subfields, fsp->occ.max);
   } else if ((fsp->occ.min != UNSET) && (field->num_subfields < fsp->occ.min)) {
      log_chk(LOGL_ERROR, cfg, field, fsp, rec_i, fld_i, UNSET, UNSET, 
	      stats, "too few subfields %d, min = %d\n",
	      field->field_int, fsp->occ.min);
   }

   /* check size */
   chk_fld_size(cfg, ansi_nist->records[rec_i], rec_i, fld_i, fsp, stats);

      
   /* check specific field requirements */
   if (NULL != fsp->check) {
      (*fsp->check)(cfg, field, fsp, rec_i, fld_i, ansi_nist, stats);
   }

   /* check the items in each subfield */
   for (sub_i = 0; sub_i < field->num_subfields; sub_i++) {
      /* check for the wrong number of items */
      chk_item_count(cfg, field, fsp, rec_i, fld_i, sub_i, stats);

      /* check each item */
      for (itm_i = 0; itm_i < field->subfields[sub_i]->num_items; itm_i++) {
	 check_item(cfg, field, rec_i, fld_i, sub_i, itm_i, fsp, rsp, ssp, 
		    ansi_nist, stats);
      }
   }

   return;
}

/*************************************************************************
**************************************************************************
   check_record - Check the top level details of the contents of a RECORD,
              recurse into the fields, log issues, and update statistics.

   Input:
      cfg       - configuration data structure
      rec       - record to check
      rec_i     - current record index
      ssp       - standard descriptor for the applicable standard
      ansi_nist - ANSI/NIST structure containing the data that includes
                  this record
      stats     - counts of things checked, types of issues found, etc.
   Output:
      stats     - updated statistics
**************************************************************************/
static void
check_record(const CAN_CONFIG *const cfg, const RECORD *const rec,
	     const int rec_i, const CAN_STANDARD *ssp, 
	     const ANSI_NIST *const ansi_nist, CAN_CONTEXT *const stats)
{
   const RECORD_SPEC *rsp; 
   FIELD_SPEC *const *fsp;
   FIELD *fld;
   FIELD missing_field;
   int fld_i;

   log_chk(LOGL_DEBUG, cfg, (FIELD *)NULL, (FIELD_SPEC *)NULL, rec_i,
	   UNSET, UNSET, UNSET, stats, "[Type-%d] Check record.\n", rec->type);
   ++stats->rec_check;

   rsp = lookup_record_in_cfg_by_type_num_deep(cfg, rec->type);
   if (NULL == rsp) {
      ++stats->rec_skip;
      can_log(LOGL_WARNING, LOGTP_CHECK, cfg, stats, 
	      "Unknown record type %d, record %d in %s.\n",
	      rec->type, rec_i+1, cfg->name);
      return;
   }

   /* check all the fields that are in the record  */
   reset_record_field_accumulator(stats);
   for (fld_i = 0; fld_i < rec->num_fields; fld_i++) {
      (void)check_field(cfg, rec->fields[fld_i], rec_i, fld_i,
			rsp, ssp, ansi_nist, stats);
   }

   /* verify all the required fields are present */
   for (fsp = rsp->fields; fsp - rsp->fields < rsp->num_fields; fsp++) {
      if ((*fsp)->occ.min > 0) {
	 if ((lookup_ANSI_NIST_field(&fld, &fld_i, (*fsp)->idnum, rec) == 0)
	     && ((strcmp((*fsp)->std->name, "ANSI/NIST") != 0)
		 || ((*fsp)->std->ver.u.num <= ansi_nist->version))) {
	    /* field not found,
	       but required by the ANSI/NIST version in the type-1 record */
	    missing_field.record_type = rec->type;
	    missing_field.field_int = (*fsp)->idnum;
	    log_chk(LOGL_ERROR, cfg, &missing_field, *fsp, rec_i, 
		    UNSET, UNSET, UNSET, stats, "required field missing\n");
	 }
      }
   }
}

/*************************************************************************
**************************************************************************
   check_ansi_nist - Check the top level details of the contents of an
              ANSI_NIST file structure, recurse into records, and fields,
              log issues, and update statistics.

   Input:
      cfg       - configuration data structure
      ansi_nist - pointer to ANSI_NIST structure to check
      stats     - current counts of things checked, types of issues found, etc.
   Output:
      stats     - updated statistics
**************************************************************************/
void
check_ansi_nist(const CAN_CONFIG *const cfg, const ANSI_NIST *const ansi_nist,
		CAN_CONTEXT *const stats)
{
   int i;		   /* internal index, add one when printing */
   RECORD *rec;
   CAN_STANDARD *ssp;

   log_chk(LOGL_INFO, cfg, (FIELD *)NULL, (FIELD_SPEC *)NULL, UNSET,
	   UNSET, UNSET, UNSET, stats, "Check file.\n");

   ssp = lookup_standard_in_cfg_by_anver_deep(cfg, ansi_nist->version);
   if (NULL == ssp) {
      ssp = lookup_standard_in_cfg_by_anver_deep(cfg, 400);
      if (NULL == ssp) {
	 can_log(LOGL_ERROR, LOGTP_CHECK, cfg, stats,
		 "Unknown ANSI/NIST version: %d.\n", ansi_nist->version);
	 return;
      } else {
	 can_log(LOGL_WARNING, LOGTP_CHECK, cfg, stats,
		 "Unknown ANSI/NIST version: %d, checking file against %d.\n",
		 ansi_nist->version, ssp->ver.u.num);
      }
   }
   stats->rec_total = ansi_nist->num_records;

   for(i = 0; i < ansi_nist->num_records; i++) {
      rec = ansi_nist->records[i];

      if (rec->type >= NUM_RECORD_TYPE_SLOTS) {
	 fprintf(stderr, "WARNING : record type %d > %d, counting as type 0\n",
		 rec->type, NUM_RECORD_TYPE_SLOTS-1);
	 ++stats->record[0];
      } else {
	 ++stats->record[rec->type];
      }
      
      check_record(cfg, rec, i, ssp, ansi_nist, stats);
   }
   check_record_combinations(cfg, ansi_nist, stats);
}
