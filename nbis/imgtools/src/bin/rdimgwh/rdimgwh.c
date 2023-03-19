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

      FILE:    rdimgwh.c

      AUTHOR:  Joseph C. Konczal
      DATE:    07/28/2008
      UPDATED:  09/30/2008 by Kenenth Ko - add version option.
      UPDATED:  01/06/2009 by Kenenth Ko - add support for HPUX compile
      UPDATED:  02/04/2009 by JCK - move function definitions to preceed calls

#cat: rdimgwh - Reads an image or images from an old style image file
#cat:              or an ANSI/NIST file, and prints the width and
#cat:              height from the information embedded in the image
#cat:              data.  This should be the same as the width and
#cat:              height specified in any metadata associated with
#cat:              the image; there is an option to check this in AN2K
#cat:              files.  For non ANSI/NIST files, only the beginning
#cat:              of the file is read, and in either case only about
#cat:              enough is parsed to find the required width and
#cat:              height data.
***********************************************************************/

#include <stdio.h>
#include <limits.h>
#include <string.h>

#include <an2k.h>
#include <unistd.h>
#include <imgdecod.h>
#include <version.h>
#ifdef __NBIS_PNG__
#include <png_dec.h>
#endif

/* The following definition indicates the amount of data that will be
   read from the beginning of a file in order to determine its type
   and find the image width and height.  It is assumed that this will
   be enough, but it has not been proven. */

#define FILE_HEAD_BUFSIZE 4096

const char *program_name;
int verbose, debug;
extern char *optarg;
extern int optind, opterr, optopt;


/***********************************************************************/
void usage(void)
{
   (void) fprintf(stderr, "Usage:\n\
%s [options] image-file ...\n\
    AN2K Image Selection Options:\n\
	-f n[:i]	finger position n [and impression type i]\n\
	-i n		impression type\n\
	-t n		logical record type\n\
        -q n            pre-computed NIST quality metric\n\
        -n n		record number\n\
    Other Options:\n\
	-v		enable verbose mode\n\
", program_name);

   exit(EXIT_FAILURE);
}

/***********************************************************************/
void procargs(const int argc, char *const argv[], REC_SEL **rec_sel)
{
   int opt;
   char *option_spec = "f:i:t:q:n:v";
   const char *rest;		/* What follows a finger specifer, 
				   could be an impression type or nothing. */
   REC_SEL *fgp_sel = NULL, *imp_sel = NULL, *idc_sel = NULL,
      *lrt_sel = NULL, *nqm_sel = NULL;

   if ((argc == 2) && (strcmp(argv[1], "-version") == 0)) {
      getVersion();
      exit(0);
   }

   /* Select only the last part, the filename, from the program pathname. */
   program_name = strrchr(*argv, '/');
   if (program_name == NULL)    /* There are no directory components, */
      program_name = *argv;	/*  use the whole thing. */
   else				/* A slash was found, */
      program_name++;		/*  skip past the slash. */

   /* Process each argument, except the input files ... */
   while ((opt = getopt(argc, argv, option_spec)) != -1) {
      switch (opt) {
      case 'f':
	 if (parse_rec_sel_option(rs_fgplp, optarg, &rest, &fgp_sel, verbose))
	    exit(EXIT_FAILURE);
	 if (parse_rec_sel_option(rs_imp, rest, NULL, &imp_sel, verbose))
	    exit(EXIT_FAILURE);
	 break;
	 
      case 'i':
	 if (parse_rec_sel_option(rs_imp, optarg, NULL, &imp_sel, verbose))
	    exit(EXIT_FAILURE);
	 break;

      case 't':
	 if (parse_rec_sel_option(rs_lrt, optarg, NULL, &lrt_sel, verbose))
	    exit(EXIT_FAILURE);
	 break;
	 
      case 'q':
	 if (parse_rec_sel_option(rs_nqm, optarg, NULL, &nqm_sel, verbose))
	    exit(EXIT_FAILURE);
	 break;

      case 'n':
	 if (parse_rec_sel_option(rs_idc, optarg, NULL, &idc_sel, verbose))
	    exit(EXIT_FAILURE);
	 break;

      case 'v':
	 verbose = 1;
	 break;

      case '?':
	 usage();
	 break;

      default:
	 fprintf(stderr,
		 "ERROR: option '%c' specified but not implemented.\n", opt);
	 exit(EXIT_FAILURE);
      }
      
      if (new_rec_sel(rec_sel, rs_and, 5, 
		      fgp_sel, imp_sel, idc_sel, lrt_sel, nqm_sel) < 0)
	 exit(EXIT_FAILURE);
   }
}

/***********************************************************************
************************************************************************
#cat: get_wsq_wh - This routine does some of what wsq_decode_mem() in
#cat:              imgtools/src/lib/wsq/decoder.c does, but only about
#cat:              as much as necessary to get the image width and
#cat:              height.

   Input:
      idata      - Buffer containing image data.
      ilen       - Number of bytes of data in image data buffer.
   Output:
      ow         - width
      oh         - height
   Return Code:
      Zero       - successful completion
      Negative   - error
************************************************************************/
int get_wsq_wh(int *const ow, int *const oh,
	       unsigned char *const idata, const int ilen)
{
   int ret;
   const unsigned char *cbufptr, *ebufptr;
   unsigned short marker, tbl_size;
   FRM_HEADER_WSQ frm_header;	/* Here is a struct, where
				   get_jpegl_wh() uses a pointer to
				   malloc()ed heap space. */
   
   /* Set memory buffer pointers. */
   cbufptr = idata;
   ebufptr = idata + ilen;
   
   /* Verify and skip the SOI_WSQ marker. */
   if ((ret = getc_marker_wsq(&marker, SOI_WSQ, &cbufptr, ebufptr)))
      return ret;
   
   /* Skip tables preceding the start of frame (SOF_WSQ) marker. */
   while (1) {
      if ((ret = getc_marker_wsq(&marker, TBLS_N_SOF, &cbufptr, ebufptr)))
	 return ret;

      if (SOF_WSQ == marker)	/* frame found */
	 break;			/* exit while-loop */

      /* get table size and skip it */
      if ((ret = getc_ushort(&tbl_size, &cbufptr, ebufptr)))
	 return ret;
      /* table size includes the size field but not the marker */
      cbufptr += tbl_size - sizeof tbl_size;

   }

   /* read in the frame header */
   if ((ret = getc_frame_header_wsq(&frm_header, &cbufptr, ebufptr)))
      return ret;

   *ow = frm_header.width;
   *oh = frm_header.height;
   return 0;
}

/***********************************************************************
************************************************************************
#cat: get_jpegl_wh - This routine does some of what jpegl_decode_mem()
#cat:              in imgtools/src/lib/jpegl/decoder.c does, but only
#cat:              about as much as necessary to get the image width
#cat:              and height. (Neither this function, nor
#cat:              jpegl_decode_mem() appear to be able to handle all
#cat:              possible jpegl files, but they do handle all the
#cat:              jpegl files that the NBIS code creates.)

   Input:
      idata      - Buffer containing image data.
      ilen       - Number of bytes of data in image data buffer.
   Output:
      ow         - width
      oh         - height
   Return Code:
      Zero       - successful completion
      Negative   - error
************************************************************************/
int get_jpegl_wh(int *const ow, int *const oh,
		 unsigned char *const idata, const int ilen)
{
   int ret;
   const unsigned char *cbufptr, *ebufptr;
   unsigned short marker, tbl_size;
   FRM_HEADER_JPEGL *frm_header_ptr; /* Here is a pointer to
					malloc()ed heap space, where
					get_wsq_wh() uses a structure */

   /* Set memory buffer pointers. */
   cbufptr = idata;
   ebufptr = idata + ilen;

   /* Skip SOI and APP0 markers */
   if ((ret = getc_marker_jpegl(&marker, SOI, &cbufptr, ebufptr)))
      return ret;
   if ((ret = getc_marker_jpegl(&marker, APP0, &cbufptr, ebufptr)))
      return ret;

   /* Skip JFIF header */
   if ((ret = getc_ushort(&tbl_size, &cbufptr, ebufptr)))
      return ret;
   cbufptr += tbl_size - sizeof tbl_size;

   /* Skip tables preceeding the start of frame (SOF3) marker. */
   while (1) {
      if ((ret = getc_marker_jpegl(&marker, TBLS_N_SOF, &cbufptr, ebufptr)))
	 return ret;

      if (SOF3 == marker)	/* frame found */
	 break;			/* exit while-loop */

      /* get table size and skip it */
      if ((ret = getc_ushort(&tbl_size, &cbufptr, ebufptr)))
	 return ret;
      /* table size includes the size field but not the marker */
      cbufptr += tbl_size - sizeof tbl_size;
   }
   /* Read in the frame header. */
   if ((ret = getc_frame_header_jpegl(&frm_header_ptr, &cbufptr, ebufptr)))
      return ret;

   /* The setup_IMG_DAT_decode() routine in
      imgtools/src/lib/jpegl/imgdat.c assigns img_dat->max_width and
      max_height the values from frm_header->x and y. */
   *ow = frm_header_ptr->x;
   *oh = frm_header_ptr->y;
   free(frm_header_ptr);
   return 0;
}

/***********************************************************************
************************************************************************
#cat: get_jpegb_wh - This routine does some of what jpegb_decode_mem()
#cat:              in imgtools/src/lib/ijg/src/lib/jpegb/decoder.c
#cat:              does, but only about as much as necessary to get
#cat:              the image width and height.

   Input:
      idata      - Buffer containing image data.
      ilen       - Number of bytes of data in image data buffer.
   Output:
      ow         - width
      oh         - height
   Return Code:
      Zero       - successful completion
      Negative   - error
************************************************************************/
/*   */

int get_jpegb_wh(int *const ow, int *const oh,
		 unsigned char *const idata, const int ilen)
{
   struct jpeg_decompress_struct cinfo;
   struct jpeg_error_mgr jerr;

   cinfo.err = jpeg_std_error(&jerr);

   jpeg_create_decompress(&cinfo);
   
   jpeg_membuf_src(&cinfo, (JOCTET *)idata, (size_t)ilen);
   (void) jpeg_read_header(&cinfo, TRUE);
   
   *ow = cinfo.image_width;
   *oh = cinfo.image_height;
   return 0;
}

/***********************************************************************
************************************************************************
#cat: get_ihead_wh_file - Gets the width and height of an IHEAD by reading 
#cat:              the values out of an IHEAD struct.

   Input:
      ifile      - Path to the IHEAD file
   Output:
      ow         - width
      oh         - height
   Return Code:
      Zero       - successful completion
      Negative   - error
************************************************************************/
int get_ihead_wh_file(int *const ow, int *const oh, char *ifile) {
   FILE *fp;
   IHEAD *ihead;

   if (ifile == NULL) {
      fprintf(stderr, "ERROR : get_ihead_wh: File path is NULL\n");
      return -1;
   }

   fp = fopen(ifile, "rb");
   if (fp == NULL) {
      fprintf(stderr, "ERROR : get_ihead_wh: File does not exist at path\n");
      return -2;
   }

   ihead = readihdr(fp);
   if (ihead == NULL) {
      fprintf(stderr, "ERROR : get_ihead_wh: IHEAD created is NULL\n");
      fclose(fp);
      return -3;
   }
   fclose(fp);

   *ow = get_width(ihead);
   *oh = get_height(ihead);

   free(ihead);
   return 0;
}


#ifdef __NBIS_JASPER__
/***********************************************************************
************************************************************************
#cat: get_jp2_wh_stream - Return the size of a jp2 format image
#cat:              accessed through a JasPer stream.  This function
#cat:              provides the core functionality for the other two
#cat:              "get_jp2_wh" functions, which each create the
#cat:              required kind of JasPer stream before calling it.

   Input:
      jin        - JasPer input stream containing the image
   Output:
      ow         - width
      oh         - height
   Return Code:
      Zero       - successful completion
      Negative   - error
************************************************************************/
int get_jp2_wh_stream(int *const ow, int *const oh, jas_stream_t *const jin)
{
   int fmt;
   jas_image_t *jimg;
   int compno, width, height;

   fmt = jas_image_getfmt(jin);
   jimg = jas_image_decode(jin, fmt, NULL);
   (void) jas_stream_close(jin);

   if (!jimg) {
      fprintf(stderr,
	      "ERROR : get_jp2_wh_stream: JasPer failed to decode image.\n");
      return -1;
   }

   width = height = 0;
   for (compno = 0; compno < jas_image_numcmpts(jimg); ++compno) {
      int neww = jas_image_cmptwidth(jimg, compno);
      int newh = jas_image_cmptheight(jimg, compno);
      if (neww > width)
	 width = neww;
      if (newh > height)
	 height = newh;
   }

   (void) jas_image_destroy(jimg);

   *ow = width;
   *oh = height;
   return 0;
}

/***********************************************************************
************************************************************************
#cat: get_jp2_wh - Return the size of a jp2 format image in a buffer.

   Input:
      idata      - Buffer containing jp2 image data.
      ilen       - Number of bytes of data in image data buffer.
   Output:
      ow         - width
      oh         - height
   Return Code:
      Zero       - successful completion
      Negative   - error
************************************************************************/
int get_jp2_wh(int *const ow, int *const oh,
	       unsigned char *const idata, const int ilen)
{
   jas_stream_t *jin;

   if (jas_init()){
      fprintf(stderr, "ERROR : get_jp2_wh : JasPer init failed.\n");
      return -2;
   }

   if (!(jin = jas_stream_memopen((char *const)idata, (unsigned int)ilen))){
      fprintf(stderr, "ERROR : get_jp2_wh: JasPer failed to open stream.\n");
      return -3;
   }

   return get_jp2_wh_stream(ow, oh, jin);
}

/***********************************************************************
************************************************************************
#cat: get_jp2_wh_file - Return the size of a jp2 image in a file.
#cat:              

   Input:
      ifile      - filename.
   Output:
      ow         - width
      oh         - height
   Return Code:
      Zero       - successful completion
      Negative   - error
************************************************************************/
int get_jp2_wh_file(int *const ow, int *const oh, const char *const ifile)
{
   jas_stream_t *jin;

   if (jas_init()){
      fprintf(stderr, "ERROR : get_jp2_wh_file : JasPer init failed.\n");
      return -2;
   }

   if (!(jin = jas_stream_fopen(ifile, "rb"))){
      fprintf(stderr, "ERROR : get_jp2_wh_file: "
	      "JasPer failed to open stream from file: %s.\n", ifile);
      return -3;
   }

   return get_jp2_wh_stream(ow, oh, jin);
}
#endif	/* __NBIS_JASPER__ */

#ifdef __NBIS_PNG__
/***********************************************************************
************************************************************************
#cat: get_png_wh_file - Open a PNG file and read enough to find the
#cat:              image width and height.

   Input:
      ifile      - filename.
   Output:
      ow         - width
      oh         - height
   Return Code:
      Zero       - successful completion
      Negative   - error
************************************************************************/
int get_png_wh_file(int *const ow, int *const oh, const char *const ifile)
{
   long width, height;
   FILE *fp;
   png_structp png_ptr;
   png_infop info_ptr;

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr) {
      return -1;
   }

   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr) {
      png_destroy_read_struct(&png_ptr, NULL, NULL);
      return -2;
   }

   fp = fopen(ifile, "rb");
   if (!fp) {
      fprintf(stderr, "ERROR : cannot open input file: '%s': %s\n",
	      ifile, strerror(errno));
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      return -3;
   }

   if (setjmp(png_jmpbuf(png_ptr))) {
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      fclose(fp);
      return -4;
   }

   png_init_io(png_ptr, fp);

   png_read_info(png_ptr, info_ptr);
   width  = png_get_image_width(png_ptr, info_ptr);
   height = png_get_image_height(png_ptr, info_ptr);

   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   fclose(fp);
   
   if (width > INT_MAX) {
      fprintf(stderr, "ERROR : get_png_wh_file : "
	      "image width %ld > implementation limit %d\n", width, INT_MAX);
      return -5;
   }
   if (height > INT_MAX) {
      fprintf(stderr, "ERROR : get_png_wh_file : "
	      "image height %ld > implementation limit %d\n", height, INT_MAX);
      return -6;
   }

   *ow = (int)width;
   *oh = (int)height;
   return 0;
}

/***********************************************************************
************************************************************************
#cat: get_png_wh - Parse a PNG file in memory and find the image width
#cat:              and height.

   Input:
      idata      - data buffer containing PNG image data
      ilen       - number of bytes of data in the PNG imaga buffer
   Output:
      ow         - width
      oh         - height
   Return Code:
      Zero       - successful completion
      Negative   - error
************************************************************************/
int get_png_wh(int *const ow, int *const oh,
	       unsigned char *const idata, const int ilen)
{
   long width, height;
   png_structp png_ptr;
   png_infop info_ptr;
   struct png_mem_io_struct ios;

   ios.cur = idata;
   ios.end = idata+ilen ;
   

   png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr) {
      return -1;
   }

   info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr) {
      png_destroy_read_struct(&png_ptr, NULL, NULL);
      return -2;
   }


   if (setjmp(png_jmpbuf(png_ptr))) {
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      return -4;
   }

   png_set_read_fn(png_ptr, &ios, png_mem_read_data);

   png_read_info(png_ptr, info_ptr);
   width  = png_get_image_width(png_ptr, info_ptr);
   height = png_get_image_height(png_ptr, info_ptr);

   png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
   
   if (width > INT_MAX) {
      fprintf(stderr, "ERROR : get_png_wh_file : "
	      "image width %ld > implementation limit %d\n", width, INT_MAX);
      return -5;
   }
   if (height > INT_MAX) {
      fprintf(stderr, "ERROR : get_png_wh_file : "
	      "image height %ld > implementation limit %d\n", height, INT_MAX);
      return -6;
   }

   *ow = (int)width;
   *oh = (int)height;
   return 0;
}
#endif	/* __NBIS_PNG__ */

/***********************************************************************
************************************************************************
#cat: get_wh - It would be easy get the width and height by calling
#cat:              read_and_decode_image() instead of using all the
#cat:              code below, but it would read and decode the entire
#cat:              image, which is way more I/O and computing than
#cat:              necessary, since the width and height are typically
#cat:              found in the image header.

   Input:
      idata
      ilen
   Output:
      ow         - width
      oh         - height
   Return Code:
      Zero - success
      Negative - failure
************************************************************************/

int get_wh(int *const ow, int *const oh,
	   unsigned char *const idata, const int ilen, const char *const ifile)
{
   int img_type;
   
   if (image_type(&img_type, idata, ilen) < 0) {
      return -2;
   }

   switch(img_type) {
   case UNKNOWN_IMG:
      *ow = *oh = -1;
      fprintf(stderr, "ERROR : get_wh : raw or unknown image type\n");
      return -1;

   case WSQ_IMG:
      return get_wsq_wh(ow, oh, idata, ilen);

   case JPEGL_IMG:
      return get_jpegl_wh(ow, oh, idata, ilen);

   case JPEGB_IMG:
      return get_jpegb_wh(ow, oh, idata, ilen);

   case IHEAD_IMG:
      return get_ihead_wh_file(ow, oh, ifile);

   case ANSI_NIST_IMG:
      fprintf(stderr, "ERROR : get_wh : ANSI/NIST is not a valid image type, "
	      "use the individual images within an ANSI/NIST file.\n");
      *ow = *oh = -1;
      break;

#ifdef __NBIS_JASPER__
   case JP2_IMG:
      if (!ifile)
	 return get_jp2_wh(ow, oh, idata, ilen);
      else
	 return get_jp2_wh_file(ow, oh, ifile);
#endif

#ifdef __NBIS_PNG__
   case PNG_IMG:
      if (!ifile)
	 return get_png_wh(ow, oh, idata, ilen);
      else
	 return get_png_wh_file(ow, oh, ifile);
#endif
      
   case RAW_IMG:
      fprintf(stderr, "RAW_IMG not yet implemented.\n");
      *ow = *oh = -1;
      break;

   default:
      fprintf(stderr,
	      "ERROR : get_wh : image type %d not implemented.\n", img_type);
      return -3;
   }
  
   return -4;
}

/***********************************************************************
************************************************************************
#cat: get_wh_from_file - Opens and reads in the beginning of an image
#cat:              file, so that the file data in memory can be passed
#cat:              to get_wh().  This approach works for the old-style
#cat:              single-image files but not for AN2K files, which
#cat:              may contain multiple images.

   Input:
      ifile      - the image file name
   Output:
      ow
      oh
   Return Code:
      Zero       - successful completion
      Negative   - error
************************************************************************/

int get_wh_from_file(int *const ow, int *const oh, const char *const ifile)
{
   FILE *imgfp;
   const int requested_count = FILE_HEAD_BUFSIZE;
   unsigned char imgdata[FILE_HEAD_BUFSIZE];
   int read_count;
   
   if (NULL == (imgfp = fopen(ifile, "rb"))) {
      fprintf(stderr, "ERROR : get_wh_from_file : "
	      "opening image file %s : %s.\n", ifile, strerror(errno));
      return -1;
   }
   read_count = fread((char*)imgdata, 1, requested_count, imgfp);
   if (read_count != requested_count) {
      fprintf(stderr, "WARNING : get_wh_from_file : "
	      "'%s', %d bytes requested, %d bytes read : %s.\n",
	      ifile, requested_count, read_count, strerror(errno));
   }
   return get_wh(ow, oh, imgdata, read_count, ifile);
}

/***********************************************************************/
int main(int argc, char *argv[])
{
   char *filename;
   REC_SEL *rec_sel = NULL;
   int ret;
   int error_count = 0;

   int width, height;

   /* In case of errors encountered by ANSI_NIST functions, we assume
      that an appropriate error message is printed to stderr before
      the function returns an error code, which is less than zero. */
	 
   procargs(argc, argv, &rec_sel);

   if (optind == argc)
      usage();

   while (optind < argc) {
      filename = argv[optind++];

      ret = is_ANSI_NIST_file(filename);
      if (ret < 0) {		/* error */
	 ++error_count;

      } else if (ret > 0) {	/* ANSI NIST file, look inside for images */
	 ANSI_NIST *ansi_nist;
	 RECORD *rec;
	 int rec_i;

	 if (read_ANSI_NIST_file(filename, &ansi_nist)) {
	    ++error_count;
	    continue;
	 }
	 
	 for (rec_i = 1;
	      (ret = lookup_ANSI_NIST_image(&rec, &rec_i, rec_i, ansi_nist))
		 > 0; 
	      rec_i++) {
	    
	    ITEM *img_item;

	    if (select_ANSI_NIST_record(rec, rec_sel) <= 0) {
	       ++error_count;
	       continue;
	    }

	    printf("%s:%d ", filename, rec_i+1);
	    fflush(stdout);
	    img_item = rec->fields[rec->num_fields-1]->subfields[0]->items[0];
	    if (get_wh(&width, &height, 
		       img_item->value, img_item->num_chars, NULL) < 0) {
	       
	       ++error_count;
	       continue;
	    }
	    
	    printf("w=%d h=%d\n", width, height);
	 }	    

	 free_ANSI_NIST(ansi_nist);

      } else {	      /* non-ANSI NIST image file */
	 printf("%s ", filename);
	 fflush(stdout);
	 if (get_wh_from_file(&width, &height, filename)) {
	    ++error_count;
	    continue;
	 }
	 printf("w=%d h=%d\n", width, height);
      }
   }

   exit(error_count);
}
