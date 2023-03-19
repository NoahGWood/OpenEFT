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
	LIBRARY:	SIVVGraph - Spectral Image Validation/Verification (Graphing)

	FILE:		SIVVGRAPH.CPP

	AUTHOR:		John D. Grantham
	
	DATE:		01/19/2009
	UPDATED:	09/10/2009 (JDG)
				01/25/2010 (JDG)


	DESCRIPTION:

		Contains a function for manually graphing the results of the SIVV
		algorithm, utilizing the OpenCV library's basic drawing functions.


********************************************************************************
	FUNCTIONS:
					graph_sums()

*******************************************************************************/

/* C++ Includes */
#include <vector>

using namespace std;

/* OpenCV includes */
#include <opencv/cv.h>
#include <opencv/highgui.h>

/* SIVV includes */
#include "SIVVCore.h"
#include "SIVVGraph.h"


/*******************************************************************************
FUNCTION NAME:	graph_sums()

DESCRIPTION:	Manually draws a graph of the results of the SIVV algorithm 
				onto an image (IplImage). 

	INPUT:
		dst_graph		- An IplImage on which the graph will be drawn
		rowsums			- A vector containing the data to be graphed
		largestpvp		- An extrenum (struct containing point values and 
						locations) representing the points of the largest 
						peak-valley-pair (PVP) to be identified on the graph
		global_minmax	- An extrenum representing the points of the global
						minimum and maximum of the results
		graph_color		- A CvScalar value specifying the color of the line of
						the graphed data points

	OUTPUT:
		dst_graph		- A graph of the results of the SIVV algorithm

*******************************************************************************/
void graph_sums(IplImage *const dst_graph, vector<double> &rowsums, extrenum *largestpvp, extrenum *global_minmax, CvScalar graph_color)
{
	/* Initialize Graph Image */
	IplImage *graph = cvCreateImage(cvGetSize(dst_graph), dst_graph->depth, dst_graph->nChannels);
	int i, num_rows = rowsums.size();
	
	/* Set  background to white */
	cvSet(graph, cvScalar(255,255,255,255), NULL);

	/* Set graph window size */
	int gwin_xmin = 20;
	int gwin_ymin = 20;
	int gwin_xmax = graph->width-20;
	int gwin_ymax = graph->height-20;
	int gwin_xrange = gwin_xmax - gwin_xmin;
	int gwin_yrange = gwin_ymax - gwin_ymin;
	double freq_scale = 0.5 / (double)gwin_xrange;
	
	/* Draw axes */
	cvLine(graph, cvPoint(gwin_xmin, gwin_ymin), cvPoint(gwin_xmin, gwin_ymax), cvScalar(0,0,0,0), 2, 8, 0);
	cvLine(graph, cvPoint(gwin_xmin, gwin_ymin), cvPoint(gwin_xmax, gwin_ymin), cvScalar(0,0,0,0), 2, 8, 0);

	/* Draw x-axis grid lines (with labels) */
	CvFont font;
	cvInitFont(&font, CV_FONT_HERSHEY_PLAIN, 1.0, 1.0, 0.0, 1, 8);
	char label[] = {'0', '.', '0', '0', '\0'};
	double x_pt;
	for (i = 0; i <= gwin_xrange; i++)
	{
		x_pt = (double)i * freq_scale;

		if (x_pt == 0.0 || x_pt == 0.05 || x_pt == 0.10 || x_pt == 0.15 || x_pt == 0.2 || x_pt == 0.25 || x_pt == 0.3 || x_pt == 0.35 || x_pt == 0.4 || x_pt == 0.45 || x_pt == 0.5)
		{
			/* Draw grid line */
			if (x_pt != 0)
				cvLine(graph, cvPoint((int)i+gwin_xmin, gwin_ymin), cvPoint((int)i+gwin_xmin, gwin_ymax), cvScalar(128,128,128,0), 1, 8, 0);

			/* Label x-axis grid values */
			if (x_pt == 0.0) {
				label[2] = '0';
				label[3] = '0'; }
			else if (x_pt == 0.05) {
				label[2] = '0';
				label[3] = '5';}
			else if (x_pt == 0.1) {
				label[2] = '1';
				label[3] = '0';}
			else if (x_pt == 0.15) {
				label[2] = '1';
				label[3] = '5';}
			else if (x_pt == 0.2) {
				label[2] = '2';
				label[3] = '0';}
			else if (x_pt == 0.25) {
				label[2] = '2';
				label[3] = '5';}
			else if (x_pt == 0.3) {
				label[2] = '3';
				label[3] = '0';}
			else if (x_pt == 0.35) {
				label[2] = '3';
				label[3] = '5';}
			else if (x_pt == 0.4) {
				label[2] = '4';
				label[3] = '0';}
			else if (x_pt == 0.45) {
				label[2] = '4';
				label[3] = '5';}
			else if (x_pt == 0.5) {
				label[2] = '5';
				label[3] = '0';}

			cvFlip(graph, graph, 0);
			cvPutText(graph, label, cvPoint((int)i+gwin_xmin-20, gwin_ymax+15), &font, cvScalar(0,0,0,0));
			cvFlip(graph, graph, 0);
		}
	}


	/* Find Global Minimum and Maximum */
	double max = global_minmax->max;
	/* Deprecated */
	/*
	double min = global_minmax->min;
	int min_loc = global_minmax->min_loc, max_loc = global_minmax->max_loc;
	*/
	
	/* Manual Plotting */
	double x = 0;
	double y = 0;
	double x2 = 0;
	double y2 = 0;
	double xscale = (double)gwin_xrange / (double)num_rows;
	double yscale = (double)gwin_yrange / max;
	
	for	(i = 0; i < (num_rows - 1); i++)
	{
		x = i;
		y = rowsums[(int)x];
		x2 = x+1;
		y2 = rowsums[(int)x2];

		/* Scale values */
		x *= xscale;
		x2 *= xscale;
		y *= yscale;
		y2 *= yscale;

		cvLine(graph, cvPoint((int)x+gwin_xmin, (int)y+gwin_ymin), cvPoint((int)x2+gwin_xmin,(int)y2+gwin_ymin), graph_color, 1, 8, 0);
	}
	
	/* Identify greatest distance local maximum and minimum in graph: Max = Red, Min = Green (only draw if peaks found) */
	if (largestpvp != NULL)
	{
		/* Draw and scale points */
		cvCircle(graph, cvPoint((int)(largestpvp->max_loc * xscale)+gwin_xmin, (int)(largestpvp->max * yscale)+gwin_ymin), 1, cvScalar(0,0,255,0), 2, 8, 0);
		cvCircle(graph, cvPoint((int)(largestpvp->min_loc * xscale)+gwin_xmin, (int)(largestpvp->min * yscale)+gwin_ymin), 1, cvScalar(0,255,0,0), 2, 8, 0);
	}

	/* Write to dst_graph (must be flipped for display purposes) */
	cvFlip(graph, dst_graph, 0);
}
