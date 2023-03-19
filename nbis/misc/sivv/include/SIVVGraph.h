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
	LIBRARY:	SIVVGraph - Spectral Image Validation/Verification (Graphing)

	FILE:		SIVVGRAPH.H

	AUTHOR:		John D. Grantham
	
	DATE:		01/19/2009
	UPDATED:	09/10/2009 (John Grantham)

	DESCRIPTION:

		Contains a function for manually graphing the results of the SIVV
		algorithm, utilizing the OpenCV library's basic drawing functions.


********************************************************************************
	FUNCTIONS:
					graph_sums()

*******************************************************************************/


#ifndef _SIVVGRAPH_H
#define _SIVVGRAPH_H


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
void graph_sums(IplImage *const dst_graph, vector<double> &rowsums, extrenum *largestpvp, extrenum *global_minmax, CvScalar graph_color);


#endif /* !_SIVVGRAPH_H */
