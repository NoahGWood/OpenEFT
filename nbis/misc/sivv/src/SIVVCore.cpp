/*******************************************************************************

License: 
This software was developed at the National Institute of Standards and 
Technology (NIST) by employees of the Federal Government in the course 
of their official duties. Pursuant to title 17 Section 105 of the 
United States Code, this software is not subject to copyright protection 
and is in the public domain. NIST assumes no responsibility  whatsoever for 
its use by other parties, and makes no guarantees, expressed or implied, 
about its quality, reliability, or any other characteristic. 

This software has been determined to be outside the scope of the EAR
(see Part 734.3 of the EAR for exact details) as it has been created solely
by employees of the U.S. Government; it is freely distributed with no
licensing requirements; and it is considered public domain.  Therefore,
it is permissible to distribute this software as a free download from the
internet.

Disclaimer: 
This software was developed to promote biometric standards and biometric
technology testing for the Federal Government in accordance with the USA
PATRIOT Act and the Enhanced Border Security and Visa Entry Reform Act.
Specific hardware and software products identified in this software were used
in order to perform the software development.  In no case does such
identification imply recommendation or endorsement by the National Institute
of Standards and Technology, nor does it imply that the products and equipment
identified are necessarily the best available for the purpose.  

*******************************************************************************/

/*******************************************************************************
	LIBRARY:	SIVVCore - Spectral Image Validation/Verification (Core)

	FILE:		SIVVCORE.CPP

	AUTHORS:	Joseph C. Konczal 
				John D. Grantham
	
	DATE:		01/05/2009 (JCK)
	UPDATED:	01/19/2009 (JDG)
				09/10/2009 (JDG)
				01/25/2010 (JDG)
				09/03/2010 (JDG)
				07/07/2011 (JDG)

	DESCRIPTION:

		Contains the core set of functions necessary for the processing images
		using the SIVV method (as described in NISTIR 7599). 

********************************************************************************
	FUNCTIONS:
					FUNCTION NAME							(Author's initials)

					dump_image()							(JCK)
					init_blackman_1d_filter()				(JCK)
					apply_blackman_window()					(JCK)
					diagonal_shuffle_quadrants()			(JCK)
					polar_transform()						(JDG)
					log_power_spectrum()					(JCK/JDG)
					findmax()								(JDG)
					sum_rows()								(JDG)
					sum_cols()								(JDG)
					smooth_sums()							(JDG)
					find_global_minmax()					(JDG)
					normalize_sums()						(JDG)
					find_fingerprint_center()				(JDG)
					cvp_distance()							(JDG)
					crop_image()							(JDG)
					peak_finder()							(JDG)
					sivv()									(JDG)

*******************************************************************************/


/* Win32 includes */
#ifdef WIN32
/* Intentionally blank -- a placeholder for any Win32-specific includes */

/* Linux includes */
#else 
/* Intentionally blank -- a placeholder for any Linux-specific includes */

#endif

/* C++ Includes */
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

/* C includes */
#include <stdio.h>

/* OpenCV includes */
#include <opencv/cv.h>
#include <opencv/highgui.h>


/* SIVV Includes */
#include "SIVVCore.h"
#include "SIVVGraph.h"

/* PERFORMANCE MEASUREMENT INCLUDES */
#include <time.h>


/*******************************************************************************
FUNCTION NAME:	dump_image()

AUTHOR:			Joseph C. Konczal

DESCRIPTION:	Print image characteristics on stdout and create an
				image window displaying the image.

	INPUT:
		name			- A unique name used to label the image information
						printed on stdout and the window displaying the image
		img				- A pointer to the image to display
	    autosize		- A flag for turning autosizing of window on and off
        verbose			- A flag for turning printf's on and off

	OUTPUT:
		name			- An image window containing the given image (img)

*******************************************************************************/
void dump_image(const char *const name, IplImage *const img, int autosize, int verbose) 
{
   double min_val, max_val, xmin, xmax;
   CvPoint min_loc, max_loc;
   int channel;
   CvScalar mean, sd, sum;
   IplImage *disp_img;
   
   if (verbose != 0)
	   printf("%s: %d channels, %d bits, %dx%d\n", name, img->nChannels, img->depth, img->width, img->height);

   cvAvgSdv(img, &mean, &sd, NULL);
   sum = cvSum(img);
   double avg_sum = sum.val[0] / img->height;


   disp_img = cvCloneImage(img);
   
   	xmin = 1000000.0;
	xmax = -xmin;
	for (channel = 1; channel <= disp_img->nChannels; channel++) 
	{
		cvSetImageCOI(disp_img, channel);
	    cvMinMaxLoc(disp_img, &min_val, &max_val, &min_loc, &max_loc, NULL);
		if (verbose != 0)
			printf("  channel %d: min %0.10f, max %0.10f, mean %f, sd %f, avg_sum %f\n", channel, min_val, max_val, mean.val[channel-1], sd.val[channel-1], avg_sum);
		if (min_val < xmin)
			xmin = min_val;
		if (max_val > xmax)
			xmax = max_val;
	}
	cvSetImageCOI(disp_img, 0);		/* restore setting to all channels */
   
	if (disp_img->depth != (int)IPL_DEPTH_8S && disp_img->depth != (int)IPL_DEPTH_8U)
		cvScale(disp_img, disp_img, 1.0/(xmax-xmin), -xmin/(xmax-xmin));
    
	if (autosize != 0)
		cvNamedWindow(name, CV_WINDOW_AUTOSIZE); 	/* Auto-size window to image size */
	else
		cvNamedWindow(name, 0); 			/* User-resizable windows */

	cvShowImage(name, disp_img);
}

/*******************************************************************************
FUNCTION NAME:	init_blackman_1d_filter()

AUTHOR:			Joseph C. Konczal

DESCRIPTION:	Create an 1D Blackman filter of the specified size, using the 
				specified alpha, typically 0.16, and store it in the provided 
				space.

	INPUT:
		arr				- 1D array of double precision floating point values.  
						The size of which determines the size of the filter that
						will be created.
		alpha			- The Blackman alpha parameter.

	OUTPUT:
		arr				-  The filter values, 0.0 to 1.0, are depositied in 
						this array.

*******************************************************************************/
void init_blackman_1d_filter(CvArr *arr, const double alpha)
{
   const double a0 = (1.0 - alpha) / 2.0;
   const double a1 = 1.0/2.0;
   const double a2 = alpha / 2.0;
   const double f = 2.0 * CV_PI / (cvGetSize(arr).width - 1.0);
   double *const dptr = (double *const)cvPtr1D(arr, 0, NULL);
   int n;

   if (1 != cvGetSize(arr).height) {
      fprintf(stderr, "error: height != 1\n");
      exit(EXIT_FAILURE);
   }
   
   for (n = 0; n < cvGetSize(arr).width; n++) {
      dptr[n] = a0 - a1 * cos(f * (double)n) + a2 * cos(2.0 * f * (double)n);
   }
}

/*******************************************************************************
FUNCTION NAME:	init_tukey_1d_filter()

AUTHOR:			John D. Grantham

DESCRIPTION:	Create an 1D Tukey filter of the specified size, using the 
				specified alpha, typically 0.25, and store it in the provided 
				space.

	INPUT:
		arr				- 1D array of double precision floating point values.  
						The size of which determines the size of the filter that
						will be created.
		alpha			- The alpha parameter.

	OUTPUT:
		arr				-  The filter values, 0.0 to 1.0, are depositied in 
						this array.

*******************************************************************************/
void init_tukey_1d_filter(CvArr *arr, const double alpha)
{
	const double a1 = 0.5;
	const double a2 = (2.0 * CV_PI / alpha);
	const double a3 = (alpha / 2.0);
	double *const dptr = (double *const)cvPtr1D(arr, 0, NULL);
	double x;
   int n;

   if (1 != cvGetSize(arr).height) {
      fprintf(stderr, "error: height != 1\n");
      exit(EXIT_FAILURE);
   }

 
	for (n = 0; n < cvGetSize(arr).width; n++)
	{
		x = (double)n / (double)cvGetSize(arr).width;

		if ((double)x < a3)
		{
			dptr[n] = a1 * (1.0 + cos(a2 * ((double)x - a3)));
		}
		else if ((double)x < (1.0 - a3))
		{
			dptr[n] = 1.0;
		}
		else if ((double)x <= 1.0)
		{
			dptr[n] = a1 * (1.0 + cos(a2 * ((double)x - 1.0 + a3)));
		}
	}
}

/*******************************************************************************
FUNCTION NAME:	apply_tukey_window()

AUTHOR:			John D. Grantham

DESCRIPTION:	Create an appropriately sized 2D Tukey window, and apply 
				it to the image.

	INPUT:
		src				- Points to an allocated image structure containing the
						input image.

	OUTPUT:
		dst				- Points to allocated image structure of the same size 
						and type as the input (src) to receive the filtered 
						result. Src and dst may be equal.

*******************************************************************************/
void apply_tukey_window(const IplImage *const src, IplImage *const dst)
{
   IplImage *hor, *ver, *tka;

   hor = cvCreateImage(cvSize(src->width, 1), src->depth, 1);
   ver = cvCreateImage(cvSize(src->height, 1), src->depth, 1);


   init_tukey_1d_filter(hor, 0.25);
   init_tukey_1d_filter(ver, 0.25);

	// DEBUG
	//dump_image("Window: Horizontal", hor, 0, 0);
	//dump_image("Window: Vertical", ver, 0, 0);

   tka = cvCreateImage(cvGetSize(src), src->depth, 1);

   cvGEMM(ver, hor, 1.0, NULL, 0.0, tka, CV_GEMM_A_T);

	// DEBUG
	//dump_image("Window: Combined", tka, 0, 0);

   cvMul(src, tka, dst, 1.0);
}


/*******************************************************************************
FUNCTION NAME:	apply_blackman_window()

AUTHOR:			Joseph C. Konczal

DESCRIPTION:	Create an appropriately sized 2D Blackman window, and apply 
				it to the image.

	INPUT:
		src				- Points to an allocated image structure containing the
						input image.

	OUTPUT:
		dst				- Points to allocated image structure of the same size 
						and type as the input (src) to receive the filtered 
						result. Src and dst may be equal.

*******************************************************************************/
void apply_blackman_window(const IplImage *const src, IplImage *const dst)
{
   IplImage *hor, *ver, *bwa;

   hor = cvCreateImage(cvSize(src->width, 1), src->depth, 1);
   ver = cvCreateImage(cvSize(src->height, 1), src->depth, 1);

   init_blackman_1d_filter(hor, 0.16);
   init_blackman_1d_filter(ver, 0.16);

   bwa = cvCreateImage(cvGetSize(src), src->depth, 1);

   cvGEMM(ver, hor, 1.0, NULL, 0.0, bwa, CV_GEMM_A_T);

   cvMul(src, bwa, dst, 1.0);
}

/*******************************************************************************
FUNCTION NAME:	polar_transform()

AUTHOR:			John D. Grantham

DESCRIPTION:	Performs a polar transformation to the input array (src) and
				writes the result into the output array (dst). Both arrays must 
				be two dimentional and the values must be 32-bit floating point. 

	INPUT:
		src				- Points to an array structure containing the 2D, 
						32-bit floating point input data.
		flags			- OpenCV Interpolation flag (ex: CV_INTER_CUBIC), 
						determines the type of interpolation used in remapping 

	OUTPUT:
		dst				- Points to allocated array structure of the same size
						and type as the input (src) to receive the transformed 
						result. Src and dst may not be equal.

*******************************************************************************/
void polar_transform(const IplImage *const src, IplImage *const dst, const int flags)
{
	IplImage *mapx = cvCreateImage(cvSize(src->width, src->height), IPL_DEPTH_32F, 1);
	IplImage *mapy = cvCreateImage(cvSize(src->width, src->height), IPL_DEPTH_32F, 1);

   if (dst == src) 
   {
	printf("ERROR: polar_transform input arrays cannot be equal!\n");
	exit(EXIT_FAILURE);      
   }

    long width, height;
	
	int x1, y1, x2, y2;
	int xdiff, ydiff;
	double x, y, xm, ym, r, rmax, m, xx, yy;
	double xmax, ymax;
	double t;
	double phi;
	double phi2;
	double angl;
	short row, col;
	
	
	long circle = 100;
	int channels;
	
	width     = src->width;
	height    = src->height;
	channels	= src->nChannels;

    /* define x1 and x2 */
	x1 = 0;
	x2 = width;
	y1 = 0;
	y2 = height;

	/* calculate distance of x and y */
	xdiff = x2 - x1;
	ydiff = y2 - y1;

	/* calculate center points */
	xm = xdiff / 2.0;
	ym = ydiff / 2.0;

	/* angl conversion factor */
	angl = 0;


	for (row = y1; row < y2; row++)
	{
		for (col = x1; col < x2; col++)
		{
			phi = (2 * CV_PI) * (col - x1) / xdiff;

			phi = fmod (phi + angl, 2 * CV_PI);

			if (phi >= 1.5 * CV_PI)
				phi2 = 2 * CV_PI - phi;
			else
			if (phi >= CV_PI)
				phi2 = phi - CV_PI;
			else
			if (phi >= 0.5 * CV_PI)
				phi2 = CV_PI - phi;
			else
				phi2 = phi;

			xx = tan (phi2);
			if (xx != 0)
				m = (double) 1.0 / xx;
			else
				m = 0;

			if (m <= ((double)(ydiff) / (double)(xdiff)))
			{
				if (phi2 == 0)
				{
					xmax = 0;
					ymax = ym - y1;
				}
				else
				{
					xmax = xm - x1;
					ymax = m * xmax;
				}
			}
			else
			{
				ymax = ym - y1;
				xmax = ymax / m;
			}
			
			rmax = sqrt ((double)((xmax*xmax) + (ymax*ymax)));
			
			t = ((ym - y1) < (xm - x1)) ? (ym - y1) : (xm - x1);

			rmax = (rmax - t) / 100.0 * (100 - circle) + t;

			r = rmax * (double)((y2 - row) / (double)(ydiff));

			xx = r * sin (phi2);
			yy = r * cos (phi2);
			
			if (phi >= 1.5 * CV_PI)
			{
				x = (double)xm - xx;
				y = (double)ym - yy;
			}
			else
			if (phi >= CV_PI)
			{
				x = (double)xm - xx;
				y = (double)ym + yy;
			}
			else
			if (phi >= 0.5 * CV_PI)
			{
				x = (double)xm + xx;
				y = (double)ym + yy;
			}
			else
			{
				x = (double)xm + xx;
				y = (double)ym - yy;
			}

			/* insert new x and y values into maps */
			((float *)(mapx->imageData + mapx->widthStep * row))[col] = x; /* float pointer because image is IPL_DEPTH_32F */
			((float *)(mapy->imageData + mapy->widthStep * row))[col] = y; 
	
		}
	}
	cvRemap(src, dst, mapx, mapy, flags, cvScalarAll(0));
}

/*******************************************************************************
FUNCTION NAME:	diagonal_shuffle_quadrants()

AUTHOR:			Joseph C. Konczal

DESCRIPTION:	Rearrange the quadrants of a 2D array by swapping them 
				diagonally.

	INPUT:
		src				- Points to an array structure containing the input data

	OUTPUT:
		dst				- Points to allocated structure of the same size and
						type as the input (src) to receive the shuffled result.
						Src and dst may be equal.

*******************************************************************************/
void diagonal_shuffle_quadrants(const CvArr *const src, CvArr *const dst)
{
   CvMat srcq_hdr[4], dstq_hdr[4], *srcq[4], *dstq[4], *tmp;
   const int width = cvGetSize(src).width, height = cvGetSize(src).height;
   const int wx = width/2, hy = height/2;
   
   if (cvGetSize(dst).width != width || cvGetSize(dst).height != height) {
      fprintf(stderr, "Unmatched sizes: src %d x %d != dst %d x %d\n", cvGetSize(src).width, cvGetSize(src).height, cvGetSize(dst).width, cvGetSize(dst).height);
      exit(EXIT_FAILURE);
   }

   /* Prepare to automate the repetition of nearly the same code, with
      a few different details each time, using macros.

      "Quarter" is used here as a verb, where the array is quartered
      into boustrophedonically numbered quadrants.

      Since C starts counting at 0, we need to rearrange the order of
      the quadrants in these C arrays from 0123 to 2301, i.e., swap 0
      and 2, and 1 and 3.
   */
#define Q(a, n, x, y, h, w)						\
   a##q[n] = cvGetSubRect(a, &a##q_hdr[n], cvRect(x, y, h, w))

#define QUARTER(a)			\
   Q(a, 0,  0,  0, wx, hy),		\
   Q(a, 1, wx,  0, wx, hy),		\
   Q(a, 2, wx, hy, wx, hy),		\
   Q(a, 3,  0, hy, wx, hy)

   QUARTER(src);

   if (src == dst) {
      tmp = cvCreateMat(hy, wx, cvGetElemType(src));
#define SWAP(a, b) cvCopy(b, tmp, 0), cvCopy(a, b, 0), cvCopy(tmp, a, 0)
      SWAP(srcq[0], srcq[2]);
      SWAP(srcq[1], srcq[3]);
#undef SWAP
   } else {
      QUARTER(dst);
      cvCopy(srcq[0], dstq[2], 0);
      cvCopy(srcq[1], dstq[3], 0);
      cvCopy(srcq[2], dstq[0], 0);
      cvCopy(srcq[3], dstq[1], 0);
   }
#undef Q
#undef QUARTER
}

/*******************************************************************************
FUNCTION NAME:	log_power_spectrum()

AUTHORS:		Joseph C. Konczal and John D. Grantham

DESCRIPTION:	Compute the log power spectrum of the DFT image. 

	INPUT:
		src				- Points to an array structure containing the input data

	OUTPUT:
		dst				- Points to allocated structure of the same size and
						type as the input (src) to receive the transformed result.
						Src and dst may be equal.

*******************************************************************************/
void log_power_spectrum(const IplImage *const src, IplImage *const dst)
{
   IplImage *dft_imgy, *dft_real, *dft_comb, *dft_dpy, *dft_zero;

   /*   Prepare the real and empty imaginary planes */
   dft_imgy = cvCreateImage(cvGetSize(src), src->depth, 1);
   dft_comb = cvCreateImage(cvGetSize(src), src->depth, 2);
   cvZero(dft_imgy);
   cvMerge(src, dft_imgy, NULL, NULL, dft_comb);

   /* Perform the Discrete Fourier Transform */
   cvDFT(dft_comb, dft_comb, CV_DXT_FORWARD, 0);

   /* Separate the real and imgy planes in the result */
   dft_real = cvCreateImage(cvGetSize(src), src->depth, 1);
   cvSplit(dft_comb, dft_real, dft_imgy, NULL, NULL);

   /*   Combine real and imaginary planes, along with a plane of
        zeros, to produce a color image to display.  The default
        interpretation of the color planes appears to be BGR. */
   dft_dpy = cvCreateImage(cvGetSize(src), src->depth, 3);
   dft_zero = cvCreateImage(cvGetSize(src), src->depth, 1);
   cvZero(dft_zero);
   cvMerge(dft_zero, dft_real, dft_imgy, NULL, dft_dpy);   

   diagonal_shuffle_quadrants(dft_dpy, dft_dpy);

   cvSplit(dft_dpy, dft_zero, dft_real, dft_imgy, NULL);

   /* Compute the magnitude of the spectrum Mag = sqrt(Re^2 + Im^2) */
   cvPow(dft_real, dft_real, 2.0);
   cvPow(dft_imgy, dft_imgy, 2.0);
   cvAdd(dft_real, dft_imgy, dft_real, NULL);
   cvPow(dft_real, dft_real, 0.5);

   /* Compute cartesian log power spectrum */
   cvCopy(dft_real, dft_imgy, NULL);
   cvAddS(dft_imgy, cvScalarAll(1.0), dft_imgy, NULL);
   cvLog(dft_imgy, dft_imgy);

   /* Write results out to dst */
   cvConvertScale(dft_imgy, dst, 1.0, 0);
}

/*******************************************************************************
FUNCTION NAME:	findmax()

AUTHOR:			John D. Grantham

DESCRIPTION:	Finds and returns the maximum value of an image. 

	INPUT:
		src				- Points to an array structure containing the input data

	OUTPUT:
		return  		- A double containing the maximum value found in the 
						input image.

*******************************************************************************/
double findmax(const IplImage *const src)
{
	/* Deprecated */
	/* double max = 0.0, val = 0.0; */
	double min_val, max_val;
	CvPoint min_loc, max_loc;
	cvMinMaxLoc(src, &min_val, &max_val, &min_loc, &max_loc, NULL);

	return max_val;
}

/*******************************************************************************
FUNCTION NAME:	sum_rows()

AUTHOR:			John D. Grantham

DESCRIPTION:	Sums each of the rows in a given image and stores the sums in a
				given vector of doubles. 

	INPUT:
		src				- Points to an array structure containing the input data

	OUTPUT:
		rowsums  		- A vector of doubles containing the sum of each row of
						the input image, stored in top-to-bottom order

*******************************************************************************/
void sum_rows(const IplImage *const src, vector<double> &rowsums)
{
	CvMat *row = cvCreateMat(1, src->width, src->depth);
	CvScalar sum;
	int i, num_rows = src->height; 
	
	for (i = 0; i < num_rows; i++)
	{
		row = cvGetRow(src, row, i); /* top to bottom */
		sum = cvSum(row);
		rowsums[i] = sum.val[0];
	}
}

/*******************************************************************************
FUNCTION NAME:	sum_cols()

AUTHOR:			John D. Grantham

DESCRIPTION:	Sums each of the columns in a given image and stores the sums in
				a given vector of doubles. 

	INPUT:
		src				- Points to an array structure containing the input data

	OUTPUT:
		rowsums  		- A vector of doubles containing the sum of each column
						of the input image, stored in left-to-right order

*******************************************************************************/
void sum_cols(const IplImage *const src, vector<double> &colsums)
{
	CvMat *col = cvCreateMat(src->height, 1, src->depth);
	CvScalar sum;
	int i, num_cols = src->width; 
	
	for (i = 0; i < num_cols; i++)
	{
		col = cvGetCol(src, col, i); /* left-to-right */
		sum = cvSum(col);
		colsums[i] = sum.val[0];
	}
}

/*******************************************************************************
FUNCTION NAME:	normalize_sums()

AUTHOR:			John D. Grantham

DESCRIPTION:	Normalizes the sums stored in a vector to the 0th term 

	INPUT:
		sums			- A vector of doubles containing sums to be normalized

	OUTPUT:
		sums  			- A vector of doubles, normalized to the 0th value

*******************************************************************************/
void normalize_sums(vector<double> &sums)
{
	double normterm;
	int i, num_sums = sums.size();
	
	for (i = 0; i < num_sums; i++)
	{
		if (i == 0)
			normterm = sums[i]; /* (max or 0th harmonic) */
		sums[i] = sums[i] / normterm;
	}
}

/*******************************************************************************
FUNCTION NAME:	smooth_sums()

AUTHOR:			John D. Grantham

DESCRIPTION:	Smooths the signal using an n-point moving average filter, where 
				n is an odd number greater than 1 (if given an even number, the
				function will choose an odd number 1 lower than the given value).
				The filtering algorithm performs zero-phase digital filtering by
				processing the input data in both forward and reverse directions
				as described in NISTIR 7599.


	INPUT:
		rowsums			- A vector of doubles containing sums which represent 
						the spectrum/signal to be smoothed
		numpoints		- The number of points over which to the signal will be
						smoothed (higher values mean more smoothing)

	OUTPUT:
		rowsums  			- A vector of doubles, smoothed by n points

*******************************************************************************/
void smooth_sums(vector<double> &rowsums, int num_points)	
{
	int num_rows = rowsums.size();
	double smoothsum, smoothavg;
	vector<double> firstpass(num_rows), firstreverse(num_rows), secondpass(num_rows);
	int i, j, distance, maxindex = num_rows - 1;

	if (num_points % 2 == 0)
		distance = num_points / 2;
	else
		distance = (num_points - 1) / 2;

	/* First pass */
	for (i = 0; i <= maxindex; i++)
	{
		smoothsum = 0;
		if (i == -1)
		{
			/* Do not change 0th term */
			firstpass[i] = rowsums[i];
		}
		else if (i < distance)
		{
			for (j = i+1; j < (i + distance); j++)
			{
				smoothsum += rowsums[j]; 
			}
			for (j = 0; j < i; j++)
			{
				smoothsum += rowsums[j];
			}
			for (j = i+1; j <= distance; j++)
			{
				smoothsum += rowsums[j];
			}
		}
		else if (i > (maxindex - distance))
		{
			for (j = i-1; j > (i - distance); j--)
			{
				smoothsum += rowsums[j]; 
			}
			for (j = maxindex; j > i; j--)
			{
				smoothsum += rowsums[j];
			}
			for (j = i-1; j >= maxindex - distance; j--)
			{
				smoothsum += rowsums[j];
			}
		}
		else
		{
			for (j = (i - distance); j < (i + distance); j++)
			{
				if (j != i)
					smoothsum += rowsums[j];
			}
		}
		if (i != -1)
		{
			smoothavg = smoothsum / (distance * 2);
			firstpass[i] = smoothavg;
		}
	}

	/* Reverse signal for second pass */
	for (i = 0; i <= maxindex; i++)
	{
		firstreverse[i] = firstpass[maxindex-i];
	}


	/* Second pass */
	for (i = 0; i <= maxindex; i++)
	{
		smoothsum = 0;
		if (i == -1)
		{
			/* Do not change 0th term */
			secondpass[i] = firstreverse[i];
		}
		else if (i < distance)
		{
			for (j = i+1; j < (i + distance); j++)
			{
				smoothsum += firstreverse[j]; 
			}
			for (j = 0; j < i; j++)
			{
				smoothsum += firstreverse[j];
			}
			for (j = i+1; j <= distance; j++)
			{
				smoothsum += firstreverse[j];
			}
		}
		else if (i > (maxindex - distance))
		{
			for (j = i-1; j > (i - distance); j--)
			{
				smoothsum += firstreverse[j]; 
			}
			for (j = maxindex; j > i; j--)
			{
				smoothsum += firstreverse[j];
			}
			for (j = i-1; j >= maxindex - distance; j--)
			{
				smoothsum += firstreverse[j];
			}
		}
		else
		{
			for (j = (i - distance); j < (i + distance); j++)
			{
				if (j != i)
					smoothsum += firstreverse[j];
			}
		}
		if (i != -1)
		{
			smoothavg = smoothsum / (distance * 2);
			secondpass[i] = smoothavg;
		}
	}

	/* Write results of secondpass to rowsums (undo reversal) */
	for (i = 0; i <= maxindex; i++)
	{
		rowsums[i] = secondpass[maxindex-i];
	}
}

/*******************************************************************************
FUNCTION NAME:	find_global_minmax()

AUTHOR:			John D. Grantham

DESCRIPTION:	Finds the global minimum and maximum of a given vector of 
				doubles, and stores the values (along with their locations) in 
				a given extrenum structure.


	INPUT:
		rowsums			- A vector of doubles containing sums 
		global_minmax	- The extrenum structure in which to store the global
						minimum and maximum points found in the vector of sums

	OUTPUT:
		global_minmax	- An extrenum structure containing the global minimum
						and maximum values, along with their locations (points)

*******************************************************************************/
void find_global_minmax(vector<double> &rowsums, extrenum &global_minmax)
{
	int i, num_rows = rowsums.size();

	/* Find Global Minimum and Maximum */
	global_minmax.min = rowsums[0];
	global_minmax.max = rowsums[0];
	global_minmax.min_loc = 0;
	global_minmax.max_loc = 0;

	for (i = 0; i < num_rows; i++)
	{
		if (rowsums[i] <= global_minmax.min)
		{
			global_minmax.min = rowsums[i];
			global_minmax.min_loc = i;
		}
		else if (rowsums[i] >= global_minmax.max)
		{
			global_minmax.max = rowsums[i];
			global_minmax.max_loc = i;
		}
	}
}

/*******************************************************************************
FUNCTION NAME:	cvp_distance()

AUTHOR:			John D. Grantham

DESCRIPTION:	Calculates and returns the distance between two points


	INPUT:
		a				- A CvPoint (origin)
		b				- Another CvPoint (destination)

	OUTPUT:
		return			- The distance between the two given CvPoints (from
						origin to destination)

*******************************************************************************/
double cvp_distance(const CvPoint a, const CvPoint b)
{
	return sqrt((double)((b.x - a.x)*(b.x - a.x)) + ((b.y - a.y)*(b.y - a.y)));
}

/*******************************************************************************
FUNCTION NAME:	crop_image()

AUTHOR:			John D. Grantham

DESCRIPTION:	Crops the specified area from the source image (src) into the 
				destination image (dst). The destination image must already be
				properly sized and cannot be equal to the source image.


	INPUT:
		src				- The source image
		dst				- The destination image
		xbound_min		- The minimum x-value bounding the area to be cropped 
		xbound_max		- The maximum x-value bounding the area to be cropped 
		ybound_min		- The minimum y-value bounding the area to be cropped 
		ybound_max		- The maximum y-value bounding the area to be cropped 

	OUTPUT:
		dst				- An image containing the cropped area of the source
						image

*******************************************************************************/
void crop_image(const IplImage *const src, IplImage *const dst, int xbound_min, int xbound_max, int ybound_min, int ybound_max)
{
	CvMat *crop_mat = cvCreateMat(dst->width, dst->height, dst->depth); 
	cvGetSubRect(src, crop_mat, cvRect(xbound_min, ybound_min, dst->width, dst->height));
	cvCopy(crop_mat, dst, NULL);
}

/*******************************************************************************
FUNCTION NAME:	peak_finder()

AUTHOR:			John D. Grantham

DESCRIPTION:	Finds peak-valley-pairs (PVP's) within a given signal above the 
				given peak size threshold and within a specified distance 
				between the	valley and peak


	INPUT:
		peaks			- A vector in which to store any PVP's found
		rowsums			- A vector containing the signal to be processed
		global_minmax	- An extrenum structure containing the global minimum
						and maximum values of the signal
		peak_threshold	- The given peak size threshold (the minimum relative
						size of peaks to be found in the signal)
		step			- The maximum distance allowable between a peak and 
						its preceeding valley

	OUTPUT:
		peaks			- A vector containing any PVP's found (within step and
						above peak_threshold)

*******************************************************************************/
void peak_finder(vector<extrenum> &peaks, vector<double> &rowsums, extrenum global_minmax, double peak_threshold, int step)
{
	/* Deprecated */
	/*
	double gdlmin = 0, gdlmax = 0, diff = 0;
	int min_loc = global_minmax.min_loc, max_loc = global_minmax.max_loc;
	*/
	
	/* Find Local Minimum and Maximum with min before max within step */
	double min = global_minmax.min, max = global_minmax.max, lmin, lmax;
	int i, j, lmin_loc = 0, lmax_loc = 0, num_rows = rowsums.size();
	extrenum pvp;
	
	for (i = 0; i < (num_rows - step); i++)
	{
		lmin = max;
		lmax = min;
		if (rowsums[i] < lmin && i > lmax_loc)
		{
			lmin = rowsums[i];
			lmin_loc = i;
			for (j = i; j < (i + step); j++)
			{
				if (rowsums[j] > lmax)
				{
					lmax = rowsums[j];
					lmax_loc = j;
				}
			}
			if (lmax != min)
			{
				for (j = i; j < (i+step); j++)
				{
					if (rowsums[j] < lmin && j < lmax_loc)
					{
						lmin = rowsums[j];
						lmin_loc = j;
					}
				}
			}
		}	
				
		if ((lmin != max && lmax != min) && (lmax - lmin) > peak_threshold) // with threshold
		{
			/* Add peak to vector of PVP's */
			pvp.min_loc = lmin_loc;
			pvp.max_loc = lmax_loc;
			pvp.min = lmin;
			pvp.max = lmax;
			peaks.push_back(pvp);
		}
	}
}

/*******************************************************************************
FUNCTION NAME:	sivv()

AUTHOR:			John D. Grantham

DESCRIPTION:	Performs the entire standard SIVV process, described in NISTIR
				7599, on a given image, using either default (see overload 
				below) or given parameters


	INPUT:
		img				- An image to be processed by SIVV
		window			- A flag for turning on/off the windowing step in the
						SIVV process
		smoothscale		- A flag for setting the number of points to be used
						in the signal smoothing algorithm (see the
						smooth_sums() function definition above)
		verbose			- A flag for turning on/off "verbose" mode, useful for
						debugging the SIVV process
		textonly		- A flag for turning on/off the "textonly mode", which 
						determines whether the output is displayed graphically
						or as text only

	OUTPUT:
		return			- A string containing the results of the SIVV process,
						separated by commas in the following order:
							(1) Number of peak-valley-pairs (PVP's) found
							(2) Ordinal number of largest PVP found
							(3) Power difference between the valley and peak
							(4) Frequency difference between the valley and peak
							(5) Slope between the valley and peak
							(6) Frequency of the midpoint between the valley
								and peak


*******************************************************************************/
//string sivv(IplImage *src, int window, int smoothscale, int verbose, int textonly, vector<double> *signal = 0, string graphfile = "", int *fail = NULL)
string sivv(IplImage *src, int smoothscale, int verbose, int textonly, vector<double> *signal = 0, string window = "", string graphfile = "", int *fail = NULL)
{
	IplImage *img, *img_dfp, *img_polar, *polar_trans;
	double grey_scale_factor;
	string results;

	//bool caseInsensitiveStringCompare(const string& str1, const string& str2)
	if (caseInsensitiveStringCompare(window, "blackman"))
	{
		window = "blackman";
	}
	else if (caseInsensitiveStringCompare(window, "tukey"))
	{
		window = "tukey";
	}
	else
	{
		window = "";
	}
	// OLD
	//if (window != 0)
	//	window = 1;

	if (smoothscale < 1)
		smoothscale = 1;

	if (verbose != 0)
		verbose = 1;

	if (textonly != 0)
		textonly = 1;

	img = cvCreateImage(cvGetSize(src), src->depth, 1);

	/* Check if source image is in color and convert to grayscale if so, otherwise copy to working image */
	if (src->nChannels > 1)
	{
		cvCvtColor(src, img, CV_RGB2GRAY);
	}
	else
	{
		cvCopy(src, img);
	}


    /* Convert image data to double precision floating point values 0.0 to 1.0 */
	img_dfp = cvCreateImage(cvGetSize(img), IPL_DEPTH_64F, 1);
	grey_scale_factor = 1.0 / 255.0;
	cvConvertScale(img, img_dfp, grey_scale_factor, 0.0);

    /* Apply 2D Blackman Window Function (if specified at command line or by default) */
	
	if (window == "blackman")
	{
		apply_blackman_window(img_dfp, img_dfp);
		if (textonly == 0)
		{
			dump_image("Blackman Window", img_dfp, 0, verbose);
		}
	}
	/* Apply 2D Tukey Window Function (if specified at command line or by default) */
	else if (window == "tukey")
	{
		apply_tukey_window(img_dfp, img_dfp);
		if (textonly == 0)
		{
			dump_image("Tukey Window", img_dfp, 0, verbose);
		}
	}

	
	/*	Compute log polar power spectrum */
	IplImage *img_lps = cvCreateImage(cvGetSize(img_dfp), img_dfp->depth, img_dfp->nChannels);
	log_power_spectrum(img_dfp, img_lps);
	if (textonly == 0)
	{
		dump_image("Log Power Spectrum", img_lps, 0, verbose);
	}

	img_polar = cvCreateImage(cvGetSize(img_lps), IPL_DEPTH_32F, img_lps->nChannels);
	cvConvertScale(img_lps, img_polar, (1.0 / findmax(img_lps)), 0);
			
	/* Polar transform using bicubic interpolation and no filling of outliers */
	polar_trans = cvCreateImage(cvGetSize(img_polar), img_polar->depth, img_polar->nChannels);
	polar_transform(img_polar, polar_trans, CV_INTER_CUBIC);
	
	if (textonly == 0)
	{
		dump_image("Polar Transform", polar_trans, 0, verbose);
	}

	/* Reduce LogPolar to angles 0 - 180 */
	IplImage *polar_trans_half = cvCreateImage(cvSize(polar_trans->width / 2 , polar_trans->height), polar_trans->depth, polar_trans->nChannels);
	CvMat *polar_trans_half_mat = cvCreateMat(polar_trans->height, polar_trans->width / 2, polar_trans->depth); 
	cvGetSubRect(polar_trans, polar_trans_half_mat, cvRect(0, 0, polar_trans->width / 2, polar_trans->height));
	cvCopy(polar_trans_half_mat, polar_trans_half, NULL);
	

    /* Sum rows of polar transform (0 - 180) */
	int num_rows = polar_trans_half->height;
	vector<double> rowsums(num_rows);
	cvFlip(polar_trans_half, polar_trans_half, 0);
	sum_rows(polar_trans_half, rowsums);
	
	/*	Smooth sums */
	if (smoothscale > 1)
		smooth_sums(rowsums, smoothscale); 

	/* Normalize sums to DC (0th) term */
	normalize_sums(rowsums);

	/* Find global min and max */
	extrenum global_minmax;
	find_global_minmax(rowsums, global_minmax);

	if (verbose != 0)
		printf("Global maximum: %0.10f, Global minimum: %0.10f\nGlobal max location: %d, Global min location: %d\n", global_minmax.max, global_minmax.min, global_minmax.max_loc, global_minmax.min_loc);

	/* Find peaks */
	vector<extrenum> peaks;
	peak_finder(peaks, rowsums, global_minmax, 0.005, (num_rows / 5)); 	/* Here, peak threshold is 0.005 and "step" is numrows / 5 */

	double cpp_scale = 1 / (2 * (double)num_rows);

	/* Process peaks */
	double maxdiff = 0.0, diff = 0.0;
	extrenum largestpvp;
	int lpvp_peaknum = 0;

	for (int i = 0; i < (int)peaks.size(); i++)
	{
		diff = peaks[i].max - peaks[i].min;
		if ((diff > maxdiff))
		{
			maxdiff = diff;
			largestpvp.min = peaks[i].min;
			largestpvp.max = peaks[i].max;
			largestpvp.min_loc = peaks[i].min_loc;
			largestpvp.max_loc = peaks[i].max_loc;
			lpvp_peaknum = i+1;
		}
	}

	/* At this point, we have identified the peak/valley pair with the greatest difference (ie, the largest peak feature across the signal) */
	if (verbose != 0)
		printf("Peak value: %10f, Valley value: %10f\nPeak location (freq): %f, Valley location (freq): %f\n", largestpvp.max, largestpvp.min, (largestpvp.max_loc * cpp_scale), (largestpvp.min_loc * cpp_scale));

	/* Calculate peak statistics */
	double dx = 0;
	double dy = 0;
	double slope = 0;
	double center_freq = 0;
	double peak_freq = 0;
	int num_peaks = 0;

	if (peaks.size() > 0)
	{
		num_peaks = peaks.size();
		dx = (largestpvp.max_loc - largestpvp.min_loc) * cpp_scale;
		dy = (largestpvp.max - largestpvp.min);
		center_freq = ((largestpvp.min_loc * cpp_scale) + (dx / 2));
		peak_freq = largestpvp.max_loc * cpp_scale;
		
		if (dy == 0.0 || dx == 0.0)
			slope = 0.0;
		else
			slope = dy / dx;
	}

	// DEBUG
	if (dy == 0)
		*fail = 1;
	else
		*fail = 0;

	stringstream out;

	out << lpvp_peaknum << ", " << num_peaks << ", " << dy << ", " << dx << ", " << slope << ", " << center_freq << ", " << peak_freq;

	results = out.str();

	/* Graph */
	if (textonly == 0 || graphfile != "")
	{
		IplImage *graph = cvCreateImage(cvSize(500, 500), IPL_DEPTH_32F, 3);
		
		graph_sums(graph, rowsums, &largestpvp, &global_minmax, cvScalar(255,0,0));
		if (textonly == 0)
			dump_image("1D Power Spectrum Graph", graph, 1, 0);
		if (graphfile != "")
			cvSaveImage(graphfile.c_str(), graph);
	}

	if (signal != 0)
	{
		*signal = rowsums;
	}

	return results;
}

/* SIVV Function Overload for quick use with default values */
string sivv(IplImage *src)
{
	int fail = 0;
	string results = sivv(src, 7, 0, 1, NULL, "blackman", "", &fail); /* primary default values */
	
	if (fail == 1)
	{
		results = sivv(src, 7, 0, 1, NULL, "tukey", "", &fail); /* secondary default values */
	}

	return results;
}

/*******************************************************************************
FUNCTION NAME:	lps()

AUTHOR:			John D. Grantham

DESCRIPTION:	Performs a generalized version of the standard SIVV process on 
				a given image, using either default (see overload below) or 
				given parameters. This generalized version of the process is
				intended for other applications which are not specific to the
				intended validation and verification purposes for which SIVV
				was originally designed. 


	INPUT:
		img				- An image to be processed by SIVV
		window			- A flag for turning on/off the windowing step in the
						SIVV process
		smoothscale		- A flag for setting the number of points to be used
						in the signal smoothing algorithm (see the
						smooth_sums() function definition above)
		verbose			- A flag for turning on/off "verbose" mode, useful for
						debugging the SIVV process
		textonly		- A flag for turning on/off the "textonly mode", which 
						determines whether the output is displayed graphically
						or as text only

	OUTPUT:
		return			- A vector of values comprising the 1D signal resulting
						from the image


*******************************************************************************/
vector<double> lps(IplImage *src, int window, int smoothscale, int verbose, int textonly)
{
	vector<double> signal;

	IplImage *img, *img_dfp, *img_polar, *polar_trans;
	double grey_scale_factor;

	if (window != 0)
		window = 1;

	if (smoothscale < 1)
		smoothscale = 1;

	if (verbose != 0)
		verbose = 1;

	if (textonly != 0)
		textonly = 1;

	img = cvCreateImage(cvGetSize(src), src->depth, 1);

	/* Check if source image is in color and convert to grayscale if so, otherwise copy to working image */
	if (src->nChannels > 1)
	{
		cvCvtColor(src, img, CV_RGB2GRAY);
	}
	else
	{
		cvCopy(src, img);
	}


    /* Convert image data to double precision floating point values 0.0 to 1.0 */
	img_dfp = cvCreateImage(cvGetSize(img), IPL_DEPTH_64F, 1);
	grey_scale_factor = 1.0 / 255.0;
	cvConvertScale(img, img_dfp, grey_scale_factor, 0.0);


    /* Apply 2D Blackman Window Function (if specified at command line or by default) */
	if (window == 1)
	{
		apply_blackman_window(img_dfp, img_dfp);
		if (verbose != 0)
		{
			dump_image("Blackman Window", img_dfp, 0, verbose);
		}
	}
	
	/*	Compute log polar power spectrum */
	IplImage *img_lps = cvCreateImage(cvGetSize(img_dfp), img_dfp->depth, img_dfp->nChannels);
	log_power_spectrum(img_dfp, img_lps);
	if (verbose != 0)
	{
		dump_image("Log Power Spectrum", img_lps, 0, verbose);
	}

	img_polar = cvCreateImage(cvGetSize(img_lps), IPL_DEPTH_32F, img_lps->nChannels);
	cvConvertScale(img_lps, img_polar, (1.0 / findmax(img_lps)), 0);
			
	/* Polar transform using bicubic interpolation and no filling of outliers */
	polar_trans = cvCreateImage(cvGetSize(img_polar), img_polar->depth, img_polar->nChannels);
	polar_transform(img_polar, polar_trans, CV_INTER_CUBIC);
	
	if (verbose != 0)
	{
		dump_image("Polar Transform", polar_trans, 0, verbose);
	}

	/* Reduce LogPolar to angles 0 - 180 */
	IplImage *polar_trans_half = cvCreateImage(cvSize(polar_trans->width / 2 , polar_trans->height), polar_trans->depth, polar_trans->nChannels);
	CvMat *polar_trans_half_mat = cvCreateMat(polar_trans->height, polar_trans->width / 2, polar_trans->depth); 
	cvGetSubRect(polar_trans, polar_trans_half_mat, cvRect(0, 0, polar_trans->width / 2, polar_trans->height));
	cvCopy(polar_trans_half_mat, polar_trans_half, NULL);
	

    /* Sum rows of polar transform (0 - 180) */
	int num_rows = polar_trans_half->height;
	vector<double> rowsums(num_rows);
	cvFlip(polar_trans_half, polar_trans_half, 0);
	sum_rows(polar_trans_half, rowsums);
	
	/*	Smooth sums */
	if (smoothscale > 1)
		smooth_sums(rowsums, smoothscale); 

	/* Normalize sums to DC (0th) term */
	normalize_sums(rowsums);
	signal = rowsums;

	/* Find global min and max */
	extrenum global_minmax;
	find_global_minmax(rowsums, global_minmax);

	extrenum *nopeaks = NULL; /* NULL extrenum for graphing function */
	
	/* Graph */
	if (textonly == 0)
	{
		IplImage *graph = cvCreateImage(cvSize(500, 500), IPL_DEPTH_32F, 3);
		
		graph_sums(graph, rowsums, nopeaks, &global_minmax, cvScalar(255,0,0));
		dump_image("1D Power Spectrum Graph", graph, 1, 0);
	}

	return signal;
}

/* LPS Function Overload for quick use with default values */
vector<double> lps(IplImage *src)
{
	vector<double> signal = lps(src, 0, 7, 0, 1); /* default values */
	return signal;
}


/*******************************************************************************
FUNCTION NAME:	generate_histrogram()

AUTHOR:			John D. Grantham

DESCRIPTION:	Generates a histogram of the image.

	INPUT:
		src				- An image to be processed
		graph			- A boolean switch to turn graphing on/off
		outfile			- An optional string containing the path of a file to
						  store histrogram values such as "histogram.txt"

	OUTPUT:
		hist			- The resulting histogram data structure

*******************************************************************************/
CvHistogram generate_histogram(IplImage* src, bool graph, string outfilepath="")
{
	const string label = "Histogram";
	IplImage* mask = NULL;
	ofstream outfile;

	int hist_size = 256;
	float range_0[]={0,256};
	float* ranges[] = { range_0 };
	IplImage *image, *hist_image = 0;
	CvHistogram *hist;
	int bin_w, i; 
	float max_value = 0;

	hist_image = cvCreateImage(cvSize(512, 200), 8, 1);
	hist = cvCreateHist(1, &hist_size, CV_HIST_ARRAY, ranges, 1);

	image = cvCreateImage(cvGetSize(src), src->depth, 1);

	/* Check if source image is in color and convert to grayscale if so, otherwise copy to working image */
	if (src->nChannels > 1)
	{
		cvCvtColor(src, image, CV_RGB2GRAY);
	}
	else
	{
		cvCopy(src, image);
	}

	if (graph)
	{
		cvNamedWindow(label.c_str(), 1);
	}

	cvCalcHist(&image, hist, 0, mask);
	cvGetMinMaxHistValue(hist, 0, &max_value, 0, 0);
	
	cvSet(hist_image, cvScalarAll(255), 0);
	bin_w = cvRound((double)hist_image->width/hist_size);
	double total_val = 0;
	double scale_factor = (double)hist_image->height/(double)max_value;

	if (graph)
	{
		/* Draw histogram */
		for(i = 0; i < hist_size; i++)
		{
			double bin_val = cvGetReal1D(hist->bins,i);
			cvRectangle(hist_image, cvPoint(i*bin_w, hist_image->height),  cvPoint((i+1)*bin_w, hist_image->height - cvRound(cvGetReal1D(hist->bins,i) * scale_factor)), cvScalarAll(0), -1, 8, 0);
			total_val += bin_val;
		}

		cvShowImage(label.c_str(), hist_image );
	}

	// Write to output file... (TO-DO)

	if (outfilepath != "")
	{
		outfile.open(outfilepath.c_str());

		if (outfile.fail())
		{
			fprintf(stderr, "WARNING: Cannot write to file \"");
			fprintf(stderr, outfilepath.c_str());
			fprintf(stderr, "\"!\nHistogram output will not be written to file. Continuing to run SIVV wihout hisogram output...\n");
		}
		if (outfile.good())
		{
			outfile << "Bin,Count\n";
			for(i = 0; i < hist_size; i++)
			{
				outfile << i << "," << cvGetReal1D(hist->bins,i) << endl;
			}
			outfile.close();
		}
	}

	return *hist;
}

/*******************************************************************************
FUNCTION NAME:	pad_image()

AUTHOR:			John D. Grantham

DESCRIPTION:	Pads input image up to a specified size using a specified
				grayscale value


	INPUT:
		src				- An image to be processed
		new_width		- An int to specify the width of the output image
		new_height		- An int to specify the height of the output image
		color			- An int (between 0 and 255) to specify the grayscale
						value to use when padding the image

	OUTPUT:
		return			- The resulting padded image

*******************************************************************************/
IplImage *pad_image(IplImage* src, int new_width, int new_height, int color)
{
	IplImage *image = cvCreateImage(cvGetSize(src), src->depth, 1);

	/* Check if source image is in color and convert to grayscale if so, otherwise copy to working image */
	if (src->nChannels > 1)
	{
		cvCvtColor(src, image, CV_RGB2GRAY);
	}
	else
	{
		cvCopy(src, image);
	}
	
	IplImage* padded = cvCreateImage(cvSize(new_width, new_height), image->depth, 1);

	int pad_left, pad_right, pad_top, pad_bottom;

	pad_left = (padded->width - image->width) / 2;
	pad_right = padded->width - image->width - pad_left;
	pad_top = (padded->height - image->height) / 2;
	pad_bottom = padded->height - image->height - pad_top;

	// Loop to write new image
	int x, y, val;

	int *rows = new int[padded->height];
	int *cols = new int[padded->width];
		
	for (y = 0; y < padded->height; y++)
	{
		rows[y] = 0;
		for (x = 0; x < padded->width; x++)
		{
			if (y == 0)
			{
				cols[x] = 0;
			}

			if ((x <= pad_left || x >= (pad_left + image->width)) || (y <= pad_top || y >= (pad_top + image->height)))
			{
				((uchar *)(padded->imageData + padded->widthStep * y))[x] = color;
			}	
			else
			{
				val = ((uchar *)(image->imageData + image->widthStep * (y-pad_top)))[(x-pad_left)];
				((uchar *)(padded->imageData + padded->widthStep * y))[x] = val;

				// DEBUG
				//cout << "(" << x << "," << y << "): " << val << endl;
			}
		}
	}

	// DEBUG
	//dump_image("Padded Image", padded, 0, 0);
	//cvWaitKey(0);

	// Clean-up
	cvReleaseImage(&image);
	delete [] rows;
	delete [] cols;

	return padded;	
	cvReleaseImage(&padded);
}

/*******************************************************************************
FUNCTION NAME:	find_fingerprint_center()

AUTHOR:			John D. Grantham

DESCRIPTION:	Finds the center of the area of highest density of dark pixels in
				an image, with the assumption that if the image contains a 
				fingerprint, this point will also be the center of the fingerprint 


	INPUT:
		src				- An image to be processed
		xbound_min		- A pointer to a location in which to store the
						minimum x-value bounding the area of highest edge
						density (if found)
		xbound_max		- A pointer to a location in which to store the
						maximum x-value bounding the area of highest edge
						density (if found)
		ybound_min		- A pointer to a location in which to store the
						minimum y-value bounding the area of highest edge
						density (if found)
		ybound_max		- A pointer to a location in which to store the
						maximum y-value bounding the area of highest edge
						density (if found)

	OUTPUT:
		xbound_min		- The minimum x-value bounding the area of highest 
						edge density (if found)
		xbound_max		- The maximum x-value bounding the area of highest 
						edge density (if found)
		ybound_min		- The minimum y-value bounding the area of highest 
						edge density (if found)
		ybound_max		- The maximum y-value bounding the area of highest 
						edge density (if found)

*******************************************************************************/
CvPoint find_fingerprint_center(IplImage* src, int *xbound_min, int *xbound_max, int *ybound_min, int *ybound_max)
{
	int center_x = 0, center_y = 0, i;

	IplImage *image = cvCreateImage(cvGetSize(src), src->depth, 1);

	/* Check if source image is in color and convert to grayscale if so, otherwise copy to working image */
	if (src->nChannels > 1)
	{
		cvCvtColor(src, image, CV_RGB2GRAY);
	}
	else
	{
		cvCopy(src, image);
	}

	// Row and column-wise processing of color values
	int x, y, val;

	int *rows = new int[image->height];
	int *cols = new int[image->width];
	
	for (y = 0; y < image->height; y++)
	{
		rows[y] = 0;
		for (x = 0; x < image->width; x++)
		{
			if (y == 0)
			{
				cols[x] = 0;
			}

			val = ((uchar *)(image->imageData + image->widthStep * y))[x];
			//cout << "(" << x << "," << y << "): " << val << endl;

			if (val <= 128)
			{
				rows[y]++;
				cols[x]++;
			}
		}
	}

	// Find center (maximum range)

	int rowmax_idx = 0, colmax_idx = 0, sum = 0;
	double max = 0, avg = 0;

	for (y = 0; y < (image->height - 400); y++)
	{
		sum = 0;
		for (i = y; i < (y + 400); i++)
		{
			sum += rows[i];
		}
		avg = (double)sum / 400;

		//cout << "[" << y << " -> " << (y+400)

		if (avg >= max)
		{
			max = avg;
			rowmax_idx = y;
		}
	}

	max = 0;
	for (x = 0; x < (image->width - 400); x++)
	{
		sum = 0;
		for (i = x; i < (x + 400); i++)
		{
			sum += cols[i];
		}
		avg = (double)sum / 400;
		
		if (avg >= max)
		{
			max = avg;
			colmax_idx = x;
		}
	}

	// Set boundaries
	*xbound_min = colmax_idx;
	*xbound_max = colmax_idx + 400;
	*ybound_min = rowmax_idx;
	*ybound_max = rowmax_idx + 400;

	// Set center point
	center_x = colmax_idx + 200;
	center_y = rowmax_idx + 200;

	
	// DEBUG
	//cout << "x_min: " << *xbound_min << "\tx_max: " << *xbound_max << "\ny_min : " << *ybound_min << "\ty_max: " << *ybound_max << endl;
	//cout << "New center: (" << center_x << "," << center_y << ")\n";
	//cvWaitKey(0);

	// Clean-up
	cvReleaseImage(&image);
	delete [] rows;
	delete [] cols;

	return cvPoint(center_x, center_y);
}

/*******************************************************************************
FUNCTION NAME:	find_fingerprint_center_morph()

AUTHOR:			John D. Grantham 
				(with credit to Bruce Bandini for his morphology code)

DESCRIPTION:	Finds the center of the area of the center of a potential 
				fingerprint, based on several morphology operations


	INPUT:
		src				- An image to be processed
		xbound_min		- A pointer to a location in which to store the
						minimum x-value bounding the area of highest edge
						density (if found)
		xbound_max		- A pointer to a location in which to store the
						maximum x-value bounding the area of highest edge
						density (if found)
		ybound_min		- A pointer to a location in which to store the
						minimum y-value bounding the area of highest edge
						density (if found)
		ybound_max		- A pointer to a location in which to store the
						maximum y-value bounding the area of highest edge
						density (if found)

	OUTPUT:
		xbound_min		- The minimum x-value bounding the area of highest 
						edge density (if found)
		xbound_max		- The maximum x-value bounding the area of highest 
						edge density (if found)
		ybound_min		- The minimum y-value bounding the area of highest 
						edge density (if found)
		ybound_max		- The maximum y-value bounding the area of highest 
						edge density (if found)

*******************************************************************************/
CvPoint find_fingerprint_center_morph(IplImage* src, int *xbound_min, int *xbound_max, int *ybound_min, int *ybound_max)
{
	cv::Mat cvmatStrEl;
	cv::Mat cvmatInputImg;
	cv::Mat cvmatTopHatImg;
	cv::Mat cvmatBlackWhiteImg;
	cv::Mat cvmatClosedOutputImg;
	cv::Mat cvmatDistanceTransImg;
	cv::Mat cvmatDistanceTransImg_8u;
	cv::Mat cvmatLaplacianImg;
	cv::Mat cvmatThinnedImg;
	cv::Mat cvmatThinnedImg_8u;
	cv::Mat cvmatThresholdThinImg;
	cv::Mat cvmatAdapThreshedImg;
	

	// Performance Test
    //clock_t total_start, total_end, strel_start, strel_end, tophat_start, tophat_end, binary_start, binary_end, close_start, close_end, multiply_start, multiply_end, moments_start, moments_end;
  
	// Performance Test
    //total_start = clock();

	double cv_status;
	
	cv::Size sz;

	int i, num_rows, num_cols, ilen;

	num_rows = src->height;
	num_cols = src->width;
	ilen = num_rows * num_cols;
  
	// John's way
	cvmatInputImg = cv::cvarrToMat(src);
	
	// Bruce's way
	//cvmatInputImg = cv::Mat(num_rows, num_cols, CV_8UC1, originalImageData);

  cvmatTopHatImg = cv::Mat(num_rows, num_cols, CV_8UC1, cv::Scalar(0));
   
  for(i=0; i<ilen; i++) {
    cvmatTopHatImg.data[i] = 0;
  }

  cvmatBlackWhiteImg = cv::Mat(num_rows, num_cols, CV_8UC1, cv::Scalar(0));
  for(i=0; i<ilen; i++) {
    cvmatBlackWhiteImg.data[i] = 0;
  }

  cvmatClosedOutputImg = cv::Mat(num_rows, num_cols, CV_8UC1, cv::Scalar(0));
  for(i=0; i<ilen; i++) {
    cvmatClosedOutputImg.data[i] = 0;
  }

  cvmatDistanceTransImg = cv::Mat(num_rows, num_cols, CV_8UC1, cv::Scalar(0));
  for(i=0; i<ilen; i++) {
    cvmatDistanceTransImg.data[i] = 0;
  }

  cvmatThinnedImg = cv::Mat(num_rows, num_cols, CV_8UC1, cv::Scalar(0));
  for(i=0; i<ilen; i++) {
    cvmatThinnedImg.data[i] = 0;
  }

  cvmatThinnedImg_8u = cv::Mat(num_rows, num_cols, CV_8UC1, cv::Scalar(0));
  for(i=0; i<ilen; i++) {
    cvmatThinnedImg_8u.data[i] = 0;
  }

  /* create the structuring element */
  // Performance Test
  //strel_start = clock();
  sz = cv::Size();
  //DEBUG
  //sz.width = 80; // Hard-coded, based on Bruce's constants
  //sz.height = 80; // Hard-coded, based on Bruce's constants
  sz.width = 14;
  sz.height = 14;
  cvmatStrEl = cv::getStructuringElement(CV_SHAPE_ELLIPSE, sz);
  // Performance Test
  //strel_end = clock();


  /***************************************************************************************************/
  /* now that the input and output matrices have been created and initialized,
     run the openCV morphology to "Top Hat Filter" the original, input image */
//    printf("cvmatTopHatImg.data POINTER: 0x%p\n", cvmatTopHatImg.data);
	// Performance Test
	//tophat_start = clock();
  try {
    cv::morphologyEx(cvmatInputImg, cvmatTopHatImg, CV_MOP_TOPHAT, cvmatStrEl);
//     printf("cv::morphologyEx, Top Hat Filter\n");
  }
  catch(cv::Exception &ex) {
    //    printf("Caught cv::Exception: %s\n", ex.err);
    printf("Caught cv::Exception:\n");
  }
  catch(std::exception &e) {
    printf("Caught Exception: '%s'\n", e.what());
  }
  // Performance Test
  //tophat_end = clock();


  /***************************************************************************************************/
  /* run the OpenCV threshold to convert image to binary, black & white */
//    printf("cvmatBlackWhiteImg.data POINTER: 0x%p\n", cvmatBlackWhiteImg.data);
    // Performance Test
	//binary_start = clock();
	cv_status = cv::threshold(cvmatTopHatImg, cvmatBlackWhiteImg, 128, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
//   printf("cv::threshold, convert to black & white: %f\n", cv_status);
//   printf("PTR cvmatTopHatImg.data: 0x%p\n", cvmatTopHatImg.data);
//   printf("PTR cvmatBlackWhiteImg.data: 0x%p\n", cvmatBlackWhiteImg.data);
	// Performance Test
	//binary_end = clock();


  /***************************************************************************************************/
  /* run the OpenCV morphology to generate mask from binary, black & white image  */
//    printf("cvmatClosedOutputImg.data POINTER: 0x%p\n", cvmatClosedOutputImg.data);
	// Performance Test
	//close_start= clock();
  cv::morphologyEx(cvmatBlackWhiteImg, cvmatClosedOutputImg, CV_MOP_CLOSE, cvmatStrEl);
//   printf("cv::morphologyEx #2\n");
	// Performance Test
	//close_end = clock();


  /***************************************************************************************************/
  /* now that the mask has been generated, multiply the mask, pixel by pixel, by the binarized, raw
     image; if the mask value is 0, then multiply, if not, skip to next pixel */
  /* in order to work with OpenCV, the foreground is 0 and background is 255, so the image is also
     inverted in this step (while the mask is set to BLACK) */
	// Performance Test
	//multiply_start = clock();
  int v;
  for(v=0; v<ilen; v++) {
//     if(cvmatClosedOutputImg.data[v] != 0) {
//       cvmatBlackWhiteImg.data[v] = cvmatBlackWhiteImg.data[v];
//     }
/* set mask and invert image values */
    if(cvmatClosedOutputImg.data[v] != 0) {
      cvmatBlackWhiteImg.data[v] = abs(cvmatBlackWhiteImg.data[v] - 255);
    }
    else {
      cvmatBlackWhiteImg.data[v] = 0;  /* set mask to BLACK */
//       cvmatBlackWhiteImg.data[v] = 255;  /* set mask to WHITE */
    }
  }
	// Performance Test
	//multiply_end = clock();

	// Performance Test
	//moments_start = clock();

	// Copy binarized MAT to IMG
  	IplImage bwimg = cvmatClosedOutputImg;

	//alternate "moments" method
	CvMoments *moments = (CvMoments*)malloc(sizeof(CvMoments));
	cvMoments(&bwimg, moments);

	double moment10 = cvGetSpatialMoment(moments, 1, 0);
    double moment01 = cvGetSpatialMoment(moments, 0, 1);
    double area = cvGetCentralMoment(moments, 0, 0);

	static int posX = 0;
    static int posY = 0;
 
    int lastX = posX;
    int lastY = posY;
 
    posX = moment10/area;
    posY = moment01/area;
	
	//cout << "Position according to moments: (" << posX << "," << posY << ")\n";
	delete moments;

	// Performance Test
	//moments_end = clock();

	// This is deprecated... but must remain for now
	*xbound_min = posX - 200;
	*xbound_max = posX + 200;
	*ybound_min = posY - 200;
	*ybound_max = posY + 200;

	if (*xbound_min < 0)
	{
		*xbound_min = 0;
	}
	if (*xbound_max > src->width)
	{
		*xbound_max = src->width;
	}
	if (*ybound_min < 0)
	{
		*ybound_min = 0;
	}
	if (*ybound_max > src->height)
	{
		*ybound_max = src->height;
	}
	

	// DEBUG show results
	/*
    cv::namedWindow("cvmatBlackWhiteImg", 0);
	cv::imshow("cvmatBlackWhiteImg", cvmatBlackWhiteImg);
	
	cv::namedWindow("cvmatClosedOutputImg", 0);
	cv::imshow("cvmatClosedOutputImg", cvmatClosedOutputImg);

	// DEBUG
	cout << "xbound_min = " << *xbound_min << endl;
	cout << "xbound_max = " << *xbound_max << endl;
	cout << "ybound_min = " << *ybound_min << endl;
	cout << "ybound_max = " << *ybound_max << endl;
	cout << "PosX: " << posX << " PosY: " << posY << endl;
	cvWaitKey(0);
	*/

	// Performance Test
	//total_end = clock();

	//cout << "Structure Element CPU time: " << strel_end - strel_start << endl;
	//cout << "TopHat CPU time: " << tophat_end - tophat_start << endl;
	//cout << "Binarize CPU time: " << binary_end - binary_start << endl;
	//cout << "Closing CPU time: " << close_end - close_start << endl;
	//cout << "Multiply CPU time: " << multiply_end - multiply_start << endl;
	//cout << "Moments CPU time: " << moments_end - moments_start << endl;
	//cout << "Centering Total CPU time: " << total_end - total_start << endl;

	return cvPoint(posX, posY);
}


/*******************************************************************************
FUNCTION NAME:	caseInsensitiveStringCompare

AUTHOR:			John D. Grantham

DESCRIPTION:	Provides a case-insensitive comparison of strings


	INPUT:
		str1			- The first string to be compared
		str2			- The second string to be compared

	OUTPUT:
		return			- A boolean value indicating whether or not the strings
						are equal (returns true if strings are equal)

*******************************************************************************/
bool caseInsensitiveStringCompare(const string& str1, const string& str2)
{
    //std::string str1Cpy(str1);
    //std::string str2Cpy(str2);

    //std::transform(str1Cpy.begin(), str1Cpy.end(), str1Cpy.begin(), ::tolower);
    //std::transform(str2Cpy.begin(), str2Cpy.end(), str2Cpy.begin(), ::tolower);

	string str1Cpy(str1);
    string str2Cpy(str2);

    transform(str1Cpy.begin(), str1Cpy.end(), str1Cpy.begin(), ::tolower);
    transform(str2Cpy.begin(), str2Cpy.end(), str2Cpy.begin(), ::tolower);

 
	return (str1Cpy == str2Cpy);
}
