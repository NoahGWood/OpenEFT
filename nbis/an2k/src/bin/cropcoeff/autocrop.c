/************************************************************************
                               NOTICE
 
This MITRE-modified NIST code was produced for the U. S. Government
under Contract No. W15P7T-07-C-F700. Pursuant to Title 17 Section 105 
of the United States Code, this software is not subject to copyright 
protection and is in the public domain. NIST and MITRE assume no 
responsibility whatsoever for use by other parties of its source code 
or open source server, and makes no guarantees, expressed or implied,
about its quality, reliability, or any other characteristic.

This software has been determined to be outside the scope of the EAR
(see Part 734.3 of the EAR for exact details) as it has been created solely
by employees of the U.S. Government; it is freely distributed with no
licensing requirements; and it is considered public domain.  Therefore,
it is permissible to distribute this software as a free download from the
internet.
   
The algorithm and its benefits are briefly described in the
MITRE Technical Report "Fingerprint Recompression after Segmentation"
(MTR080005), available at 
http://www.mitre.org/work/tech_papers/tech_papers_08/08_0060/index.html

************************************************************************/

/***********************************************************************
      PACKAGE: 

      FILE:    AUTOCROP.C

      AUTHORS: Margaret Lepley (MITRE)
               Using previous AN2K dpyan2k code from
               Michael D. Garris
               Stan Janet
               
      DATE:    1/22/2008
      UPDATE:  2/7/2008  Better handling of small/zero segments.
      UPDATE:  10/29/2008 Output the wsq files to current directory.
      UPDATE:  12/11/2008 (Kenneth Ko) - add libgen.h which is required
                                         for function "basename".
      ROUTINES:
               autocrop()
               crop_record()
               ULcorner()
               cropcoeff()
***********************************************************************/

#include <stdio.h>
#include <an2k.h>
#include <an2kseg.h>
#include <wsq.h>
#include <img_io.h>
#include <string.h>
#ifndef __MSYS__
#include <libgen.h>
#endif

int crop_record(const int, const ANSI_NIST *, char *);
int ULcorner(SEG *, int, int, int);
int cropcoeff(unsigned char *, int, SEG *, int, char *);


/***************************************************************
Read AN2K file, and crop each Type-14 record.
***************************************************************/
int autocrop(char *fiffile)
{
   int ret;
   int record_i;
   ANSI_NIST *ansi_nist;
   RECORD *record;
   int finalret = 0;

   if((ret = read_ANSI_NIST_file(fiffile, &ansi_nist))){
      fprintf(stderr, "ERROR : autocrop : reading ANSI_NIST file %s\n",
              fiffile);
      return(ret);
   }
   fileroot(fiffile);

   for(record_i = 1; record_i < ansi_nist->num_records; record_i++){
     /* Set current record. */
     record = ansi_nist->records[record_i];
     /* If image record ... */
     if(image_record(record->type)){
       if(record->type == TYPE_14_ID){
          if((ret = crop_record(record_i, ansi_nist, fiffile))) {
             /* Continue processing any other images, but remember error */
             finalret = 1;
          }
       }
     }
   }

   free_ANSI_NIST(ansi_nist);

   /* Return normally. */
   return(finalret);
}

/*******************************************************************
Only acts on WSQ compressed Type-14 record, with fingerprint segments.
Crop segments are read/adjusted to have UL corner on 32x32 grid, and  
then all segment areas are segmented out using CropCoeff technique.
*******************************************************************/
int crop_record(const int imgrecord_i, const ANSI_NIST *ansi_nist, char *fiffile)
{
   int ret;
   RECORD *imgrecord;
   FIELD *field;
   int field_i;
   unsigned char *idata;
   int ilen;
   unsigned char *img_comp;
   int nsgs;
   SEG *segs;
   int iw, ih;
   double scale, shift;

   /* Type-10,13,14,15,&16 records. */
   imgrecord = ansi_nist->records[imgrecord_i];

   if (imgrecord->type == TYPE_14_ID) {
      ret = lookup_type14_segments(&segs, &nsgs, imgrecord);
      if(ret < 0)
         return(ret);
   }
   else
     return(0);
   
   /* Lookup grayscale compression algorithm (TAG_CA_ID). */
   if(!lookup_ANSI_NIST_field(&field, &field_i, TAG_CA_ID, imgrecord)){
      fprintf(stderr, "ERROR : crop_record : ");
      fprintf(stderr, "TAG_CA field not found in ");
      fprintf(stderr, "record index [%d] [Type-%d.%03d]\n",
              imgrecord_i+1, imgrecord->type, TAG_CA_ID);
      return(-4);
   }
   img_comp = field->subfields[0]->items[0]->value;

   if(strcmp((char *)img_comp, COMP_NONE) == 0) {
      fprintf(stderr, "ERROR : Input data must be WSQ compressed\n");
      return(-5);
   }
   
   /* Set image pointer to last field value in record. */
   field = imgrecord->fields[imgrecord->num_fields-1];
   idata = field->subfields[0]->items[0]->value;
   ilen = field->subfields[0]->items[0]->num_bytes;

   /* Find actual WSQ width/height (sometimes different from ANSI_NIST info) */
   if((ret = read_wsq_frame_header(idata, ilen, &iw, &ih, &scale, &shift))) {
      return(ret);
   }

   /* Crop box adjustments.  Snap UL to 32x32 grid plus others */
   if((ret = ULcorner(segs, nsgs, iw, ih))) {
      return(ret);
   }

   /* Do all the crops on this image */
   ret = cropcoeff(idata, ilen, segs, nsgs, fiffile);
   
   free (segs);

   return(ret);
}

/* Box edges that are within BSIZE distance of the original image
   border, move out to the original image border, to improve 
   image quality. */
#define BSIZE 24
/* Box dimensions that are less than MSIZE are increased using centering.
   This prevents lower quality box edge areas from dominating the image. 
*/
#define MSIZE 100

/**********************************************************************
Adjust crop box boundaries to work well with CropCoeff. Some of these
adjustments are required, others are just best practice.
Segments with zero on-image area or nonsensical UL/LR corners are passed 
through without change. The cropping library function will skip them.
For all other segmentss:
  UL/LR corners that are outside image bounds are moved in. (REQUIRED)
  UL/LR corners within a small distance of the original image edge,
        are moved out to align with the edge. (best practice)
  Small crop areas are enlarged with centering (ensures reasonable
                                  size central area of high quality)
  UL is shifted +16 in both dimensions to get a nearest neighbor grid.
      (Arbitrary choice.  Alternative is no shift.)
  UL corner is snapped to lower 32x32 gridpoint (REQUIRED)

The 3 steps not labelled REQUIRED might optionally be dropped or 
modified, without causing major cropcoeff errors. 

BSIZE and MSIZE choices are first guesses and may vary by applicaiton.  
If MSIZE is used, it must be at least 16.

[Note: lrx,lry is just outside the box.]
***********************************************************************/
int ULcorner(SEG *segs, int nsgs, int wid, int hgt)
{
   int n;
   int ulx, uly, lrx, lry;
   int border;
       
   /* For each SEG ... */
   for(n = 0; n < nsgs; n++){
     ulx = segs[n].left;
     uly = segs[n].top;
     lrx = segs[n].right;
     lry = segs[n].bottom;

     /* Move UL and LR onto image (REQUIRED). */
     if (ulx < 0) ulx = 0;
     if (uly < 0) uly = 0;
     if (lrx > wid) lrx = wid;
     if (lry > hgt) lry = hgt;

     /* UL corner must be above/left of LR corner 
        and UL/LR corner must be on the image */
     /* These are zero intersect with image, or illogical segments 
        (LR doesn't have greater coordinates than UL) */
     if(lrx-ulx<=0 || lry-uly<=0 || 
        lrx <= 0   || lry <= 0   || 
        ulx >= wid || uly >= hgt ) continue;

     /* Move points very near the edge to the edge for higher accuracy */
     /* Recommended.  BSIZE can be adjusted */
     if (ulx < BSIZE) ulx = 0;
     if (uly < BSIZE) uly = 0;
     if (lrx > wid - BSIZE) lrx = wid;
     if (lry > hgt - BSIZE) lry = hgt;

     /* Expand small segmentss, using centering */
     /* Suggested if task is not harmed by bigger segments */
     if( lrx - ulx < MSIZE ) {
       border = (MSIZE - (lrx - ulx))/2;
       ulx -= border;
       lrx += border;
       if (ulx < 16) ulx = 0;
       if (lrx > wid) lrx = wid;
     }
     if( lry - uly < MSIZE ) {
       border = (MSIZE - (lry - uly))/2;
       uly -= border;
       lry += border;
       if (uly < 16) uly = 0;
       if (lry > hgt) lry = hgt;
     }

     /* Large enough segments can snap to closest gridpoint */
     /* Not required. Based on judgement that fingerprint segmentations
        often already include significant surrounding background area,
        so removing at most 16 pixels on an edge is not a problem. */
     if( lrx - ulx > MSIZE ) 
       ulx += 16;
     if( lry - uly > MSIZE ) 
       uly += 16;

     /* Snap UL corner outward to 32x32 gridpoint (REQUIRED) */
     ulx /= 32; ulx *= 32;
     uly /= 32; uly *= 32;

     segs[n].left = ulx;
     segs[n].top = uly;
     segs[n].right = lrx;
     segs[n].bottom = lry;
   }

   /* Return normally. */
   return(0);
}

/***************************************************************
Use SEG info (segs) to automatically crop WSQ memory (idata)
and write out new WSQ files with names that begin with fiffile.
***************************************************************/
int cropcoeff(unsigned char *idata, int ilen, SEG *segs, int nsgs, 
	      char *fiffile) 
{
   unsigned char  *cdata;
   short *qdata;
   int ret, n, clen, hgt_pos, huff_pos;
   int ulx,uly,lrx,lry,fgp;
   int iw, ih, nh, nw;
   char *str;
   char *ofile;

   /* Set pointers to NULL so that first call to wsq_cropcoeff_mem decodes
      original data and allocates output memory */
   cdata = NULL;
   qdata = NULL;

   ofile = (char *)malloc(strlen(fiffile)+10);

   /* Crop each box */
   for(n = 0; n < nsgs; n++) {
     ulx = segs[n].left;
     uly = segs[n].top;
     lrx = segs[n].right;
     lry = segs[n].bottom;
     fgp = segs[n].finger;

     /* To avoid extra processing, skip over illogical segments */
     if(lrx-ulx<=0 || lry-uly<=0 || lrx <= 0 || lry <= 0) {
         fprintf(stderr, "WARNING: Crop area for FGP %d invalid, no image generated\n", fgp);
         fprintf(stderr, "         UL (%d,%d)  LR (%d,%d)\n\n", 
                 ulx, uly, lrx, lry);
         continue;
     }

     if ((ret = wsq_cropcoeff_mem(&cdata,&clen,&nw,&nh,ulx,uly,lrx,lry,&iw,&ih,idata,ilen, 
                  &qdata, &hgt_pos, &huff_pos))) {
       fprintf(stderr, "ERROR: CropCoeff failed on FGP %d and must skip the rest of this image\n\n", fgp);
       if(cdata != NULL)
           free(cdata);
       if(qdata != NULL)
           free(qdata);
       free(ofile);
       return(ret);  
     }

     if (nw > 0 && nh > 0) {
       /* Cropping succeeded in generating output WSQ memory */
       /* WRITE SEGMENTED DATA */
       str = basename(fiffile);
       sprintf(ofile,"%s.%d.unk%c", str, fgp, 0);
       newext(ofile, strlen(ofile), "wsq");
       if((ret = write_raw_from_memsize(ofile, cdata, clen))) {
          fprintf(stderr, "\n\nERROR: Writing output file %s\n\n", ofile);
       }
     } else {
         /* This shouldn't happen.  If it does then there was an unexpected error in processing */
         fprintf(stderr, "\n\nERROR: Crop area for FGP %d didn't create an image\n\n", fgp);
     }
   }

   /* Free reused decode memory */
   if(cdata != NULL)
     free(cdata);
   if(qdata != NULL)
     free(qdata);
   free(ofile);

   return(0);
}

