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

      FILE:    DPYAN2K.C

      AUTHORS: Michael D. Garris
               Stan Janet
      DATE:    03/07/2001
      UPDATE:  09/13/2004
      UPDATE:  05/10/2005 by MDG
      UPDATE:  10/29/2007 by Kenneth Ko
      UPDATE:  12/11/2007 by Kenneth Ko
      UPDATED: 01/31/2008 by Kenenth Ko
      UPDATE:  02/27/2008 by Joseph C. Konczal - added record selection
      UPDATE:  04/17/2008 by JCK - consolidated dpyan2k_record().
      UPDATE:  04/21/2008 by JCK - added display of SEG and ASEG data
      UPDATED: 09/03/2008 by Kenneth Ko

      ROUTINES:
               dpyan2k()
               dpyan2k_record()
               dpyan2k_binary_record()
               dpyan2k_tagged_record()
               *get_pix_per_mm()
               *find_matching_minutiae()
               *get_minutiae_pixel_coords()
               *get_nist_minutiae_pixel_coords()
               *get_iafis_minutiae_pixel_coords()
               *is_COF_zero()
               *get_segmentation_data()

***********************************************************************/

#include <stdio.h>
#include <sys/param.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <errno.h>
#include <dpyan2k.h>
#include <defs.h>

static int get_pix_per_mm(float *, float *, const int, const ANSI_NIST *);
static int find_matching_minutiae(RECORD **, int *, const int,
                  const ANSI_NIST *);
static int get_minutiae_pixel_coords(int **, int **, int *, const float,
                  const float, const int, const int, const int,
                  const ANSI_NIST *);
static int get_nist_minutiae_pixel_coords(int **, int **, int *,
                  const float, const float, const int, const int, const int,
                  const ANSI_NIST *);
static int get_iafis_minutiae_pixel_coords(int **, int **, int *,
                  const float, const float, const int, const ANSI_NIST *);
static int is_COF_zero(FIELD *);
static int get_segmentation_data(const RECORD *const, SEGMENTS **);

/*********************************************************************/
int dpyan2k(const char *fiffile, const REC_SEL *const rec_sel)
{
   int ret, num_images;
   int record_i;
   ANSI_NIST *ansi_nist;
   RECORD *record;

   if((ret = read_ANSI_NIST_file(fiffile, &ansi_nist))){
      fprintf(stderr, "ERROR : dpyan2k : reading ANSI_NIST file %s\n",
              fiffile);
      return(ret);
   }

   /* Initialize image displayed counter. */
   num_images = 0;
   /* Reset window offsets to display origin. */
   wx = 0;
   wy = 0;
   for(record_i = 1; record_i < ansi_nist->num_records; record_i++){
      /* Set current record. */
      record = ansi_nist->records[record_i];
      /* If image record ... */
      if(image_record(record->type)){

         /* Code change by MDG on 07/02/04 to gracefully handle unsupported */
         /* Type 7 & 16 image records */
         /* Code change by KKO on 10/29/07 to gracefully handle unsupported */
         /* Type 17 & 99 image records */

         if((record->type == TYPE_7_ID) || (record->type == TYPE_16_ID) ||
            (record->type == TYPE_99_ID)){
            fprintf(stderr, "WARNING : dpyan2k : unsupported user-defined "
		    "image record type = %u ignored\n", record->type);
         }

         else if (select_ANSI_NIST_record(record, rec_sel)) {
            /* Display image record in its own window. */
            if((ret = dpyan2k_record(record_i, ansi_nist))){
               free_ANSI_NIST(ansi_nist);
               return(ret);
            }
            /* Bump image displayed counter. */
            num_images++;
            if(verbose) {
               fprintf(stderr, "Forked: image record index [%d] [Type-%u]\n",
                       record_i+1, record->type);
	    }
         }
	 else if (verbose) {
	    fprintf(stderr, "INFO : dpyan2k : unselected record ignored,"
		    " index [%d], [Type-%u]\n", record_i+1, record->type);
	 }
      }
   }

   if (num_images == 0) {
      fprintf(stderr, "WARNING : dpyan2k : no images match the criteria\n");
      return -1;
   }

   /* Done forking children to display image records ... */
   /* must wait for all children to exit ...             */
   while(num_images--){
      if(verbose){
         fprintf(stderr, "Waiting for %d children to exit\n",
                 num_images+1);
      }
      if(wait(0) == -1){
         fprintf(stderr, "ERROR : dpyan2k : "
		 "wait failed with %d children left\n", num_images+1);
         free_ANSI_NIST(ansi_nist);
         return(-2);
      }
   }

   free_ANSI_NIST(ansi_nist);

   /* Return normally. */
   return(0);
}

/*****************************************************************
Joe Konczal combined dpyan2k_binary_record() and dpyan2k_tagged_record(),
which were very similar, in preparation for adding the ability to
display segmentation information in one place instead of two.
*****************************************************************/
int dpyan2k_record(const int imgrecord_i, const ANSI_NIST *ansi_nist)
{
   int ret;
   RECORD *imgrecord, *minrecord;
   int minrecord_i;
   unsigned char *imgdata;
   float hpix_per_mm, vpix_per_mm;
   int iw, ih, id;
   double ppmm;
   unsigned int whitepix = 0, align;
   int done = 0;
   char wintitle[2 * MAXPATHLEN];
   int *xs, *ys, npts;
   SEGMENTS *segments;

   imgrecord = ansi_nist->records[imgrecord_i];

  /* Set pixel aligment to single byte boundaries. */
   align = 8;

   /* Initialize number of points to 0. */
   npts = 0;

   /* Get and decode (if necessary) image field data. */
   ret = decode_ANSI_NIST_image(&imgdata, &iw, &ih, &id, &ppmm,
				ansi_nist, imgrecord_i, 1 /*interleave flag*/);
   /* If ERROR or IGNORE ... */
   if(ret <= 0)
      return(ret);

   /* New code - believes image attributes returned by the decode process. */
   if (verbose) {
      fprintf(stderr, "%s:\n", filename);
      fprintf(stderr, "\timage size: %d x %d\n", iw, ih);
      fprintf(stderr, "\tdepth: %d\n", id);
      fprintf(stderr, "\tpoints: %d\n", npts);
   }
   
   if(id == 1)
      whitepix = 0;
   else if(id == 8 || id == 24)
      whitepix = 255;
   else{
      fprintf(stderr, "ERROR : dpyan2k_record : "
	      "image in record index [%d] [Type-%u] has depth %d != 1 or 8\n",
	      imgrecord_i+1, imgrecord->type, id);
      return(-5);
   }
   

   /* Get image scan frequency. */
   ret = get_pix_per_mm(&hpix_per_mm, &vpix_per_mm, imgrecord_i, ansi_nist);
   /* If ERROR getting pix/mm ... */
   if(ret < 0)
      return(ret);
   /* If pix/mm set AND matching minutiae record found ... */
   if(ret != IGNORE){
      ret = find_matching_minutiae(&minrecord, &minrecord_i,
                                   imgrecord_i, ansi_nist);
      /* If ERROR ... */
      if(ret < 0)
         return(ret);
      /* If matching minutiae record found ... */
      if(ret){
         /* Extract minutiae coordinates converting from micrometer */
         /* units to pixels. */
         if((ret = get_minutiae_pixel_coords(&xs, &ys, &npts,
                         hpix_per_mm, vpix_per_mm, iw, ih,
                         minrecord_i, ansi_nist)))
            return(ret);
      }
   }

   /* Retrieve segmentation data from SEG or ASEG records. */
   if (TYPE_14_ID == imgrecord->type) {
      ret = get_segmentation_data(imgrecord, &segments);
      if (ret < 0)
	 return ret;
   }

   fflush(stdout);
   fflush(stderr);

   switch(fork()){
      /* On ERROR ... */
      case -1:
           fprintf(stderr, "ERROR : dpyan2k_record : fork failed\n");
           return(-8);
      /* Child process. */
      case 0:
           /* Compose title for window. */
	   if (sprintf(wintitle, "%s : Record %d : Type-%u",
		       filename, imgrecord_i+1, imgrecord->type)
	       >= sizeof(wintitle)) {
	      fprintf(stderr, "ERROR : dpyan2k_record : "
		      "sprintf result overflows %lu byte buffer\n",
		      (unsigned long)sizeof(wintitle));
	      return(-81);
	   }
           /* Display the image record's contents. */
           dpyimagepts(wintitle, imgdata, (unsigned int)iw, (unsigned int)ih,
		       (unsigned int)id, whitepix, (int)align, &done,
                       xs, ys, npts, segments);
           free(imgdata);
           if(npts > 0){
              free(xs);
              free(ys);
           }
           if(verbose)
              fprintf(stderr, "Child: %s exiting\n", wintitle);
           _exit(0);
      /* Parent process. */
      default:
           /* Bump next window offsets. */
           wx += WIN_XY_INCR;
           wy += WIN_XY_INCR;
           break;
   }

   return(0);
}

/***************************************************************/
int get_pix_per_mm(float *ohpix_per_mm, float *ovpix_per_mm,
			  const int imgrecord_i, const ANSI_NIST *ansi_nist)
{
   RECORD *imgrecord;
   FIELD *field;
   int field_i;
   float hpix_per_mm, vpix_per_mm;
   int slc, hps, vps;
   char *nep;

   *ohpix_per_mm = 0.0;
   *ovpix_per_mm = 0.0;
   imgrecord = ansi_nist->records[imgrecord_i];

   if(binary_image_record(imgrecord->type)){
      /* Get pix/mm from NTR Type-1.012 field. */
      if(!lookup_ANSI_NIST_field(&field, &field_i, NTR_ID,
                                 ansi_nist->records[0])){
         fprintf(stderr, "ERROR : get_pix_per_mm : "
		 "NTR field [Type-1.%03d] not found\n", NTR_ID);
         return(-2);
      }

      /* Beware of strtof(), it doesn't seem to work right on Fedora
	 Core 7, but (float)strtod() works. -- jck */
      errno = 0;
      hpix_per_mm = 
	 (float)strtod((char *)field->subfields[0]->items[0]->value, &nep);
      if (errno) {
	 fprintf(stderr, "ERROR : get_pix_per_mm : "
		 "converting NTR value '%s' to floating point: %s.\n",
		 (char *)field->subfields[0]->items[0]->value, strerror(errno));
	 return(-23);

      } else if (nep == (char *)field->subfields[0]->items[0]->value
		 || *nep != 0) {
	 fprintf(stderr, "ERROR : gep_pix_per_mm : "
		 "NTR value '%s' is not a floating point number.\n",
		 (char *)field->subfields[0]->items[0]->value);
	 return(-24);
      }
      vpix_per_mm = hpix_per_mm;
   }
   else if(tagged_image_record(imgrecord->type)){
      /* Get SLC field (ID=008) in tagged image record. */
      if(!lookup_ANSI_NIST_field(&field, &field_i, SLC_ID, imgrecord)){
         fprintf(stderr, "ERROR : get_pix_per_mm : "
		 "SLC field [Type-%u.%03d] not found\n",
		 imgrecord->type, SLC_ID);
         return(-3);
      }
      slc = (int)strtol((char *)field->subfields[0]->items[0]->value, &nep, 10);
      if (nep == (char *)field->subfields[0]->items[0]->value
	  || *nep != '\0') {
	 fprintf(stderr, "ERROR : get_pix_per_mm : "
		 "SLC field [Type-%u.%03d] : invalid integer value '%s'\n",
		 imgrecord->type, SLC_ID,
		 (char *)field->subfields[0]->items[0]->value);
	 return(-34);
      }
      /* If not scale specified ... then, can't plot minutiae. */
      if(slc == 0){
         fprintf(stderr, "WARNING : get_pix_per_mm : "
		 "SLC field [Type-%u.%03d] = 0, "
		 "so scanning frequency not available\n",
		 imgrecord->type, SLC_ID);
         return(IGNORE);
      }

      /* Get HPS field in tagged image record. */
      if(!lookup_ANSI_NIST_field(&field, &field_i, HPS_ID, imgrecord)){
         fprintf(stderr, "ERROR : get_pix_per_mm : "
		 "HPS field [Type-%u.%03d] not found\n",
                 imgrecord->type, HPS_ID);
         return(-4);
      }
      hps = (int)strtol((char *)field->subfields[0]->items[0]->value, &nep, 10);
      if (nep == (char *)field->subfields[0]->items[0]->value
	  || *nep != '\0') {
	 fprintf(stderr, "ERROR : get_pix_per_mm : "
		 "HPS field [Type-%u.%03d] : invalid integer value '%s'\n",
		 imgrecord->type, HPS_ID,
		 (char *)field->subfields[0]->items[0]->value);
	 return(-45);
      }

      /* Get VPS field in tagged image record. */
      if(!lookup_ANSI_NIST_field(&field, &field_i, VPS_ID, imgrecord)){
         fprintf(stderr, "ERROR : get_pix_per_mm : "
		 "VPS field [Type-%u.%03d] not found\n",
                 imgrecord->type, VPS_ID);
         return(-5);
      }
      vps = (int)strtol((char *)field->subfields[0]->items[0]->value, &nep, 10);
      if (nep == (char *)field->subfields[0]->items[0]->value
	  || *nep != '\0') {
	 fprintf(stderr, "ERROR : get_pix_per_mm : "
		 "VPS field [Type-%u.%03d] : invalid integer value '%s'\n",
		 imgrecord->type, VPS_ID,
		 (char *)field->subfields[0]->items[0]->value);
	 return(-56);
      }

      /* If hps and vps are pix/inch ... */
      if(slc == 1){
         /* Compute pix/mm. */
         hpix_per_mm = hps / MM_PER_INCH;
         vpix_per_mm = vps / MM_PER_INCH;
      }
      /* If hps and vps are pix/cm ... */
      else if(slc == 2){
         /* Compute pix/mm. */
         hpix_per_mm = hps / 10.0;
         vpix_per_mm = vps / 10.0;
      }
      else{
         fprintf(stderr, "ERROR : get_pix_per_mm : SLC field ");
         fprintf(stderr, "[Type-%u.%03d] has illegal value, ",
                 imgrecord->type, SLC_ID);
         fprintf(stderr, "%d != {0,1,2}\n", slc);
         return(-6);
      }
   }
   else{
      fprintf(stderr, "ERROR : get_pix_per_mm : ");
      fprintf(stderr, "record Type-%u not supported\n",
              imgrecord->type);
      return(-7);
   }

   /* Assign output pointers. */
   *ohpix_per_mm = hpix_per_mm;
   *ovpix_per_mm = vpix_per_mm;

   /* Return normally. */
   return(0);
}

/***************************************************************/
int find_matching_minutiae(RECORD **ominrecord, int *ominrecord_i,
                  const int imgrecord_i, const ANSI_NIST *ansi_nist)
{
   RECORD *imgrecord, *record;
   int record_i;
   int img_idc, t9_idc;
   char *valstr, *nep;

   imgrecord = ansi_nist->records[imgrecord_i];

   /* Get image record's IDC. */
   if(imgrecord->fields[IDC_ID-1]->field_int != IDC_ID){
      fprintf(stderr, "ERROR : find_matching_minutiae : "
	      "IDC field [Type-%u.%03d] not found\n", imgrecord->type, IDC_ID);
      return(-2);
   }
   valstr = (char *)imgrecord->fields[IDC_ID-1]->subfields[0]->items[0]->value;
   img_idc = (int)strtol(valstr, &nep, 10);
   if (nep == valstr || *nep != '\0') {
      fprintf(stderr, "ERROR : find_matching_minutiae : "
	      "IDC field [Type-%u.%03d] : invalid integer value '%s'\n",
	      imgrecord->type, IDC_ID, valstr);
      return(-23);
   }

   /* Search for 1st Type-9 record with matching IDC. */
   for(record_i = 1; record_i < ansi_nist->num_records; record_i++){
      /* If we are not on the input image record ... */
      if(record_i != imgrecord_i){
         record = ansi_nist->records[record_i];
         if(record->type == TYPE_9_ID){
            /* Get Type-9 record's IDC. */
            if(record->fields[IDC_ID-1]->field_int != IDC_ID){
               fprintf(stderr, "ERROR : find_matching_minutiae : "
		       "IDC field [Type-%u.%03d] not found\n",
                       record->type, IDC_ID);
               return(-3);
            }
	    valstr = 
	       (char *)record->fields[IDC_ID-1]->subfields[0]->items[0]->value;
	    t9_idc = (int)strtol(valstr, &nep, 10);
	    if (nep == valstr || *nep != '\0') {
	       fprintf(stderr, "ERROR : find_matching_minutiae : "
		       "IDC field [Type-%u.%03d] : invalid integer value '%s'.\n",
		       record->type, IDC_ID, valstr);
	       return(-34);
	    }

            /* Do we have an IDC match? */
            if(img_idc == t9_idc){
               *ominrecord = record;
               *ominrecord_i = record_i;
               return(TRUE);
            } /* Else, IDCs don't match. */
         } /* Else, record not Type-9. */
      } /* Else, on input record. */
   } /* END For ... */

   /* If we get here, then no matching minutiae found ... */
   return(FALSE);
}

/***************************************************************/
int get_minutiae_pixel_coords(int **oxs, int **oys, int *onpts,
                 const float hpix_per_mm, const float vpix_per_mm,
                 const int iw, const int ih,
                 const int minrecord_i, const ANSI_NIST *ansi_nist)
{
   int ret;

   if(nist_flag){
      if((ret = get_nist_minutiae_pixel_coords(oxs, oys, onpts,
                      hpix_per_mm, vpix_per_mm, 
                      iw, ih, minrecord_i, ansi_nist)))
         return(ret);
   }
   else if (iafis_flag){
      if((ret = get_iafis_minutiae_pixel_coords(oxs, oys, onpts,
                      hpix_per_mm, vpix_per_mm, minrecord_i, ansi_nist)))
         return(ret);
   }
   else{
      fprintf(stderr, "ERROR : get_minutiae_pixel_coords : ");
      fprintf(stderr, "neither NIST or IAFIS field flags set\n");
      return(-2);
   }

   return(0);
}

/***************************************************************/
int get_nist_minutiae_pixel_coords(int **oxs, int **oys, int *onpts,
                 const float hpix_per_mm, const float vpix_per_mm,
                 const int iw, const int ih,
                 const int minrecord_i, const ANSI_NIST *ansi_nist)
{
   RECORD *minrecord;
   FIELD *field;
   ITEM *item;
   int field_i, subfield_i;
   int *xs, *ys, npts, micro_mm;
   unsigned char c;
   char *valstr, *nep;

   minrecord = ansi_nist->records[minrecord_i];

   /* Find minutiae count field ... */
   if(!lookup_ANSI_NIST_field(&field, &field_i, MIN_ID, minrecord)){
      fprintf(stderr, "ERROR : get_nist_minutiae_pixel_coords : "
	      "MIN field in record index [%d] [Type-%u.%03d] not found\n",
              minrecord_i+1, minrecord->type, MIN_ID);
      return(-2);
   }
   valstr = (char *)field->subfields[0]->items[0]->value;
   npts = (int)strtol(valstr, &nep, 10);
   if (nep == valstr || *nep != '\0') {
      fprintf(stderr, "ERROR : get_nist_minutiae_pixel_coord : "
	      "IDC field in record index [%d] [Type-%u.%03d] : "
	      "invalid integer value '%s'.\n",
	      minrecord_i, minrecord->type, MIN_ID, valstr);
      return(-20);
   }


   /* !!!! No minutiea test added 09-13-04 !!! */
   /* If no minutiea recorded in Type-9 record ... */
   if(npts == 0){
      *oxs = (int *)NULL;
      *oys = (int *)NULL;
      *onpts = npts;
      return(0);
   }

   xs = (int *)malloc(npts * sizeof(int));
   if(xs == (int *)NULL){
      fprintf(stderr,
              "ERROR : get_nist_minutiae_pixel_coords : malloc : xs\n");
      return(-3);
   }
   ys = (int *)malloc(npts * sizeof(int));
   if(ys == (int *)NULL){
      fprintf(stderr,
              "ERROR : get_nist_minutiae_pixel_coords : malloc : ys\n");
      free(xs);
      return(-4);
   }
   /* Find minutiae attributes field ... */
   if(!lookup_ANSI_NIST_field(&field, &field_i, MRC_ID, minrecord)){
      fprintf(stderr, "ERROR : get_nist_minutiae_pixel_coords : ");
      fprintf(stderr, "MRC field in record index [%d] [Type-%u.%03d] ",
              minrecord_i+1, minrecord->type, MRC_ID);
      fprintf(stderr, "not found\n");
      free(xs);
      free(ys);
      return(-5);
   }
   /* If wrong number of subfields ... */
   if(field->num_subfields != npts){
      fprintf(stderr, "ERROR : get_nist_minutiae_pixel_coords : ");
      fprintf(stderr, "number of subfields %d != %d ",
              field->num_subfields, npts);
      fprintf(stderr, "in MRC field index [%d.%d] [Type-%u.%03d]\n",
              minrecord_i+1, field_i+1, minrecord->type, MRC_ID);
      free(xs);
      free(ys);
      return(-6);
   }

   /* For each subfield in MRC field ... */
   for(subfield_i = 0; subfield_i < npts; subfield_i++){
      /* Set location/direction item. */
      item = field->subfields[subfield_i]->items[1];
      /* If location/direction wrong length ... */
      if(strlen((char *)item->value) != 11){
         fprintf(stderr, "ERROR : get_nist_minutiae_pixel_coords : "
		 "location/direction value \"%s\" has wrong length in "
		 "MRC subfield index [%d.%d.%d] [Type-%u.%03d]\n",
		 (char *)item->value, minrecord_i+1, field_i+1, subfield_i+1,
		 minrecord->type, MRC_ID);
         free(xs);
         free(ys);
         return(-7);
      }
      /* Get x-coord. */
      c = item->value[4];
      item->value[4] = '\0';
      micro_mm = (int)strtol((char *)item->value, &nep, 10);
      if (nep == (char *)item->value || *nep != '\0') {
	 fprintf(stderr, "ERROR : get_nist_minutiae_pixel_coords : "
		 "MRC subfield [%d.%d.%d] [Type-%u.%03d] : "
		 "x-coord is not a valid integer value '%s'\n.",
		 minrecord_i+1, field_i+1, subfield_i+1,
		 minrecord->type, MRC_ID, (char *)item->value);
	 return(-78);
      }
      xs[subfield_i] = sround(micro_mm * hpix_per_mm / 100.0);
      item->value[4] = c;
      /* Get y-coord. */
      c = item->value[8];
      item->value[8] = '\0';

      micro_mm = (int)strtol((char *)&(item->value[4]), &nep, 10);
      if (nep == (char *)item->value || *nep != '\0') {
	 fprintf(stderr, "ERROR : get_nist_minutiae_pixel_coords : "
		 "MRC subfield [%d.%d.%d] [Type-%u.%03d] : "
		 "y-coord is not a valid integer value '%s'\n.",
		 minrecord_i+1, field_i+1, subfield_i+1,
		 minrecord->type, MRC_ID, (char *)item->value);
	 return(-79);
      }
      ys[subfield_i] = sround(micro_mm * vpix_per_mm / 100.0);
      /* Flip y-coord. */
      ys[subfield_i] = ih - ys[subfield_i] - 1;
      item->value[8] = c;
   }

   /* Set output pointers. */
   *oxs = xs;
   *oys = ys;
   *onpts = npts;

   /* Return normally. */
   return(0);
}

/***************************************************************/
int get_iafis_minutiae_pixel_coords(int **oxs, int **oys, int *onpts,
                 const float hpix_per_mm, const float vpix_per_mm,
                 const int minrecord_i, const ANSI_NIST *ansi_nist)
{
   int ret;
   RECORD *minrecord;
   FIELD *field;
   ITEM *item;
   int field_i, subfield_i;
   int *xs, *ys, npts, micro_mm;
   char c;

   minrecord = ansi_nist->records[minrecord_i];

   /* Find minutiae COF field ... */
   if(lookup_ANSI_NIST_field(&field, &field_i, COF_ID, minrecord)){
      ret = is_COF_zero(field);
      /* If ERROR ... */
      if(ret < 0)
         return(ret);
      /* IF COF field not zero ... */
      if(!ret){
         fprintf(stderr, "WARNING : get_iafis_minutiae_pixel_coords : ");
         fprintf(stderr, "COF field non-zero in record index ");
         fprintf(stderr, "[%d] [Type-%d.%03d], so minutiae ignored\n",
                 minrecord_i+1, minrecord->type, COF_ID);
         *onpts = 0;
         /* Return normally. */
         return(0);
      }
   }

   /* Find minutiae count field ... */
   if(!lookup_ANSI_NIST_field(&field, &field_i, NMN_ID, minrecord)){
      fprintf(stderr, "ERROR : get_iafis_minutiae_pixel_coords : ");
      fprintf(stderr, "NMN field in record index [%d] [Type-%d.%03d] ",
              minrecord_i+1, minrecord->type, NMN_ID);
      fprintf(stderr, "not found\n");
      return(-2);
   }
   npts = atoi((char *)field->subfields[0]->items[0]->value);
   xs = (int *)malloc(npts * sizeof(int));
   if(xs == (int *)NULL){
      fprintf(stderr,
              "ERROR : get_iafis_minutiae_pixel_coords : malloc : xs\n");
      return(-3);
   }
   ys = (int *)malloc(npts * sizeof(int));
   if(ys == (int *)NULL){
      fprintf(stderr,
              "ERROR : get_iafis_minutiae_pixel_coords : malloc : ys\n");
      free(xs);
      return(-4);
   }
   /* Find minutiae attributes field ... */
   if(!lookup_ANSI_NIST_field(&field, &field_i, MAT_ID, minrecord)){
      fprintf(stderr, "ERROR : get_iafis_minutiae_pixel_coords : ");
      fprintf(stderr, "MAT field in record index [%d] [Type-%d.%03d] ",
              minrecord_i+1, minrecord->type, MAT_ID);
      fprintf(stderr, "not found\n");
      free(xs);
      free(ys);
      return(-5);
   }
   /* If wrong number of subfields ... */
   if(field->num_subfields != npts){
      fprintf(stderr, "ERROR : get_iafis_minutiae_pixel_coords : ");
      fprintf(stderr, "number of subfields %d != %d ",
              field->num_subfields, npts);
      fprintf(stderr, "in MAT field index [%d.%d] [Type-%d.%03d]\n",
              minrecord_i+1, field_i+1, minrecord->type, MAT_ID);
      free(xs);
      free(ys);
      return(-6);
   }

   /* For each subfield in MAT field ... */
   for(subfield_i = 0; subfield_i < npts; subfield_i++){
      /* Set location/direction item. */
      item = field->subfields[subfield_i]->items[1];
      /* If location/direction wrong length ... */
      if(strlen((char *)item->value) != 11){
         fprintf(stderr, "ERROR : get_iafis_minutiae_pixel_coords : ");
         fprintf(stderr, "location/direction value \"%s\" has ", item->value);
         fprintf(stderr, "wrong length in MAT subfield ");
         fprintf(stderr, "index [%d.%d.%d] [Type-%d.%03d]\n",
                          minrecord_i+1, field_i+1, subfield_i+1,
                          minrecord->type, MAT_ID);
         free(xs);
         free(ys);
         return(-7);
      }
      /* Get x-coord. */
      c = item->value[4];
      item->value[4] = '\0';
      micro_mm = atoi((char *)item->value);
      xs[subfield_i] = sround(micro_mm * hpix_per_mm / 100.0);
      item->value[4] = c;
      /* Get y-coord. */
      c = item->value[8];
      item->value[8] = '\0';
      micro_mm = atoi((char *)&(item->value[4]));
      ys[subfield_i] = sround(micro_mm * vpix_per_mm / 100.0);
      item->value[8] = c;
   }

   /* Set output pointers. */
   *oxs = xs;
   *oys = ys;
   *onpts = npts;

   /* Return normally. */
   return(0);
}

/***************************************************************/
int is_COF_zero(FIELD *coffield)
{
   ITEM *item;

   if((coffield->num_subfields != 1) ||
      (coffield->subfields[0]->num_items < 1)){
      fprintf(stderr, "ERROR : is_COF_zero : field format error\n");
      return(-2);
   }
   else{
      item = coffield->subfields[0]->items[0];
      if(strcmp((char *)item->value, "00000000") != 0)
         return(FALSE);
   }

   if(coffield->subfields[0]->num_items >= 2){
      item = coffield->subfields[0]->items[1];
      if(strcmp((char *)item->value, "00000000") != 0)
         return(FALSE);
   }

   if(coffield->subfields[0]->num_items >= 3){
      item = coffield->subfields[0]->items[2];
      if(strcmp((char *)item->value, "000.0000") != 0)
         return(FALSE);
   }

   if(coffield->subfields[0]->num_items >= 4){
      item = coffield->subfields[0]->items[3];
      if(strcmp((char *)item->value, "00000000") != 0)
         return(FALSE);
   }

   if(coffield->subfields[0]->num_items == 5){
      item = coffield->subfields[0]->items[4];
      if(strcmp((char *)item->value, "00000000") != 0)
         return(FALSE);
   }

   return(TRUE);
}

/***************************************************************/
int get_segmentation_data(const RECORD *const imgrecord,
				 SEGMENTS **segments)
{
   FIELD *sfield, *afield;
   SUBFIELD *subf;
   int num_polygons = 0, space_needed = 0;
   int sfield_i, afield_i, subf_i, item_i, *next;
   SEGMENTS *seg;
   POLYGON *pol;

   /* First determine how much space is needed. */
   if (lookup_ANSI_NIST_field(&sfield, &sfield_i, SEG_ID, imgrecord)) {
      num_polygons += sfield->num_subfields;
      space_needed += 
	 (sfield->num_subfields * (sizeof(POLYGON) + 2 * 4 * sizeof(int)));
   } else {
      sfield = NULL;
   }

   if (lookup_ANSI_NIST_field(&afield, &afield_i, ASEG_ID, imgrecord)) {
      num_polygons += afield->num_subfields;
      space_needed += (afield->num_subfields * sizeof(POLYGON));
      for (subf_i = 0; subf_i < afield->num_subfields; subf_i++) {
	 subf = afield->subfields[subf_i];
	 space_needed += ((subf->num_items - 1) * sizeof(int));
      }
   } else {
      afield = NULL;
   }

   /* If no segmentation was found, return empty-handed. */
   if (0 == space_needed) {
      *segments = NULL;
      return IGNORE;
   }

   /* Allocate the space. */
   space_needed += sizeof(SEGMENTS);
   if (NULL == (seg = (SEGMENTS*)malloc(space_needed))) {
      fprintf(stderr, "ERROR : get_segmentation_data : "
	      "cannot allocate %d bytes for segmentation data\n", 
	      space_needed);
      return -1;
   }

   /* Initial initialization. */
   seg->num_polygons = 0;
   seg->polygons = (POLYGON*)(seg + 1);
   next = (int*)(seg->polygons + num_polygons);

   /* Retrieve the data. */
   if (NULL != sfield) {      
      for (subf_i = 0; subf_i < sfield->num_subfields; subf_i++) {
	 subf = sfield->subfields[subf_i];

	 pol = seg->polygons + seg->num_polygons++;
	 pol->num_points = 4;
	 pol->x = next;
	 next += pol->num_points;
	 pol->y = next;
	 next += pol->num_points;

	 if (subf->num_items != 5) {
	    fprintf(stderr, "ERROR : get_segmentation_data : "
		    "expected 5 items, found %d\n", subf->num_items);
	    return -2;
	 }
	 pol->fgp = atoi((char *)subf->items[0]->value);
	 pol->x[0] = pol->x[3] = atoi((char *)subf->items[1]->value);
	 pol->x[1] = pol->x[2] = atoi((char *)subf->items[2]->value);
	 pol->y[0] = pol->y[1] = atoi((char *)subf->items[3]->value);
	 pol->y[2] = pol->y[3] = atoi((char *)subf->items[4]->value);
      }
   }
   if (NULL != afield) {
      for (subf_i = 0; subf_i < afield->num_subfields; subf_i++) {
	 subf = afield->subfields[subf_i];

	 pol = seg->polygons + seg->num_polygons++;
	 pol->fgp = atoi((char *)subf->items[0]->value);
	 pol->num_points = atoi((char *)subf->items[1]->value);
	 pol->x = next;
	 next += pol->num_points;
	 pol->y = next;
	 next += pol->num_points;
	 
	 for (item_i = 2; item_i < subf->num_items; item_i++) {
	    if (0 == (item_i % 2))    /* even */
	       pol->x[(item_i-2)/2] = atoi((char *)subf->items[item_i]->value);
	    else		      /* odd */
	       pol->y[(item_i-2)/2] = atoi((char *)subf->items[item_i]->value);
	 }
      }
   }

   *segments = seg;
   return 0;
}
