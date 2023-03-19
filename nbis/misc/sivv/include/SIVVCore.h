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

/*******************************************************************************
	LIBRARY:	SIVVCore - Spectral Image Validation/Verification (Core)

	FILE:		SIVVCORE.H

	AUTHORS:	Joseph C. Konczal 
				John D. Grantham
	
	DATE:		01/05/2009 (JCK)
	UPDATED:	01/19/2009 (JDG)
				09/10/2009 (JDG)
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
					lps()									(JDG)
					generate_histogram()					(JDG)
					pad_image()								(JDG)

*******************************************************************************/

#ifndef _SIVVCORE_H
#define _SIVVCORE_H

/* Peak-Valley-Pair (extrenum) data structure */
struct extrenum {
	double min, max;
	int min_loc, max_loc;
};


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
void dump_image(const char *const name, IplImage *const img, int autosize, int verbose);


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
void init_blackman_1d_filter(CvArr *arr, const double alpha);

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
void init_tukey_1d_filter(CvArr *arr, const double alpha);

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
void apply_tukey_window(const IplImage *const src, IplImage *const dst);


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
void apply_blackman_window(const IplImage *const src, IplImage *const dst);

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
void polar_transform(const IplImage *const src, IplImage *const dst, const int flags);

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
void diagonal_shuffle_quadrants(const CvArr *const src, CvArr *const dst);

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
void log_power_spectrum(const IplImage *const src, IplImage *const dst);


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
double findmax(const IplImage *const src);

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
void sum_rows(const IplImage *const src, vector<double> &rowsums);

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
void sum_cols(const IplImage *const src, vector<double> &colsums);

/*******************************************************************************
FUNCTION NAME:	normalize_sums()

AUTHOR:			John D. Grantham

DESCRIPTION:	Normalizes the sums stored in a vector to the 0th term 

	INPUT:
		sums			- A vector of doubles containing sums to be normalized

	OUTPUT:
		sums  			- A vector of doubles, normalized to the 0th value

*******************************************************************************/
void normalize_sums(vector<double> &sums);


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
void smooth_sums(vector<double> &rowsums, int num_points);

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
void find_global_minmax(vector<double> &rowsums, extrenum &global_minmax);

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
CvPoint find_fingerprint_center(IplImage* src, int *xbound_min, int *xbound_max, int *ybound_min, int *ybound_max);

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
CvPoint find_fingerprint_center_morph(IplImage* src, int *xbound_min, int *xbound_max, int *ybound_min, int *ybound_max);


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
double cvp_distance(const CvPoint a, const CvPoint b);

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
void crop_image(const IplImage *const src, IplImage *const dst, int xbound_min, int xbound_max, int ybound_min, int ybound_max);

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
void peak_finder(vector<extrenum> &peaks, vector<double> &rowsums, extrenum global_minmax, double peak_threshold, int step);

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
//string sivv(IplImage *src, int window, int smoothscale, int verbose, int textonly, vector<double> *signal, string graphfile, int *fail);
string sivv(IplImage *src, int smoothscale, int verbose, int textonly, vector<double> *signal, string window, string graphfile, int *fail);

/* SIVV Function Overload for quick use with default values */
string sivv(IplImage *src);

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
vector<double> lps(IplImage *src, int window, int smoothscale, int verbose, int textonly);

/* LPS Function Overload for quick use with default values */
vector<double> lps(IplImage *src);

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
CvHistogram generate_histogram(IplImage* src, bool graph, string outfilepath);

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
IplImage *pad_image(IplImage* src, int new_width, int new_height, int color);


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
bool caseInsensitiveStringCompare(const std::string& str1, const std::string& str2);

#endif /* !_SIVVCORE_H */
