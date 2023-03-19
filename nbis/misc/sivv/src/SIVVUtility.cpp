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
	PACKAGE:	SIVVUtility - Spectral Image Validation/Verification Utility

	FILE:		SIVVUTILITY.CPP

	AUTHORS:	John D. Grantham
	
	DATE:		01/19/2009 
	UPDATED:	09/10/2009 (JDG)
				01/22/2010 (JDG)
				07/21/2010 (JDG)
				09/03/2010 (JDG)
				06/10/2011 (JDG)
				07/07/2011 (JDG)

	DESCRIPTION:

		An application written to demonstrate the use of the SIVV library 
		functions for the processing of images.

*******************************************************************************/

/* Win32 includes */
#ifdef WIN32
/* Intentionally blank -- a placeholder for any Win32-specific includes */

/* Linux includes */
#else 
/* Uncomment if using automatic alignment in linux (using GTK) -- see below */
/* #include <gtk/gtk.h> */

#endif

/* C++ Includes */
#include <vector>
#include <iostream>
#include <fstream>
#include <string>

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
FUNCTION NAME:	print_usage()

AUTHOR:			John D. Grantham

DESCRIPTION:	Prints the usage instructions to the screen (stderr). 

	INPUT:
		VOID			- (No input)

	OUTPUT:
		VOID			- Usage instructions are printed to stderr

*******************************************************************************/
void print_usage(void)
{
	fprintf(stderr, "\nUSAGE: [EXECUTABLE] image_filename [FLAGS]\n\n");
	fprintf(stderr, "FLAGS:\n\n-nowindow\t\t = Turn windowing off (on by default)\n");
	fprintf(stderr, "-windowtype [name]\t = Set window type to either \"blackman\" or \"tukey\"\n\t\t\t\   (default is blackman)\n");
	fprintf(stderr, "-smooth [value]\t\t = Turn smoothing off (value <= 1) \n\t\t\t   or set width of moving average (value > 1)\n\t\t\t   (smoothing value default is 7)\n");
	fprintf(stderr, "-verbose\t\t = Turn \"verbose\" mode on (off by default)\n");
	fprintf(stderr, "-nocenter\t\t = Turn \"autocentering\" off (on by default)\n");
	fprintf(stderr, "-textonly\t\t = Turn on \"textonly\" mode for text-only output\n\t\t\t   (off by default)\n");
	fprintf(stderr, "-quiet\t\t\t = Turn on \"quiet\" mode for no screen output\n\t\t\t   (implies \"textonly\" and overrides \"verbose\",\n\t\t\t   off by default)\n");
	fprintf(stderr, "-ROI [X1] [Y1] [X2] [Y2] = Set Region Of Interest to a rectangular area\n\t\t\t   specified by (X1,Y1) and (X2,Y2)\n\t\t\t   (X1,Y1) and (X2,Y2) can be any two diagonally\n\t\t\t   opposite corners of the rectangular area\n\t\t\t   Setting the ROI flag implies the \"-nocenter\" flag\n");
	fprintf(stderr, "-signaltofile [FILE]\t = Write full 1D signal to specified file\n\t\t\t   (comma-separated values)\n");
	fprintf(stderr, "-graphtofile [FILE]\t = Save graph of 1D signal to specified file\n\t\t\t   (must be BMP, TIFF, PNG, or JPEG -- PNG recommended)\n");
	fprintf(stderr, "-histtofile [FILE]\t = Write full histogram of image to specified file\n\t\t\t   (comma-separated values with header row)\n");
	fprintf(stderr, "-faildir \"[DIRECTORY]\"\t = Path to directory in which to place copies of images\n\t\t\t   which produce no peaks in SIVV signal\n\t\t\t   (annotated with box indicating ROI processed)\n\t\t\t   NOTE: Path should be placed inside quotes (\"\")\n");
}


/*******************************************************************************
FUNCTION NAME:	main()

AUTHOR:			John D. Grantham

DESCRIPTION:	Accepts command-line arguments and runs the SIVV Utility. 

	INPUT:
		argc			- The number of command line arguments
		argv			- An array of command line arguments

	OUTPUT:
		return			- Return code indicating execution success or failure

*******************************************************************************/
int main (int argc, char **argv)
{
	try
	{
		IplImage *img;
   
		int i, value, usage = 0, window = 1, verbose = 0, smoothscale = 7, autocenter = 1, textonly = 0, roi = 0, roi_x1 = 0, roi_x2 = 0, roi_y1 = 0, roi_y2 = 0, fail = 0, quiet = 0; //default values
		int fxmin, fxmax, fymin, fymax;
		string results, graphfilename = "", histfilename = "", failfile = "", windowtype = "blackman";
		ofstream outfile, graphfile, histfile, testfile;
		vector<double> signal;

		// Performance test
		//clock_t total_start, total_end;
		//total_start = clock();

		/* Parse command-line arguments */
   		if (argc == 1)
		{
			fprintf(stderr, "ERROR: You must specify an image filename to process\n");
			print_usage();
			exit(EXIT_FAILURE);
		}
		else // Check command line args
		{
			if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "-help") || !strcmp(argv[1], "?"))
			{
				print_usage();
				exit(EXIT_SUCCESS);
			}
			FILE *file;
			file = fopen(argv[1], "r");
			if (file == NULL)
			{
				fprintf(stderr, "ERROR: Cannot read file \"");
				fprintf(stderr, argv[1]);
				fprintf(stderr, "\"!\n");
				print_usage();
				exit(EXIT_FAILURE);
			}
			else
				fclose(file);

			if (argc > 2 && argc <= 21) 
			{
				for (i = 2; i <= argc - 1; i++)
				{
					if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "-help") || !strcmp(argv[i], "?"))
					{
						print_usage();
						exit(EXIT_SUCCESS);
					}
					else if (!strcmp(argv[i], "-nowindow"))
					{
						window = 0;
						windowtype = "";
					}
					else if (!strcmp(argv[i], "-windowtype"))
					{
						if ((i+1) >= argc)
						{
							print_usage();
							exit(EXIT_FAILURE);
						}
						else
						{
							string tempname = argv[i+1];
							if (caseInsensitiveStringCompare(tempname, "blackman"))
							{
								windowtype = "blackman";
							}
							else if (caseInsensitiveStringCompare(tempname, "tukey"))
							{
								windowtype = "tukey";
							}
							i++;
						}
					}
					else if (!strcmp(argv[i], "-smooth"))
					{
						if ((i+1) >= argc)
						{
							print_usage();
							exit(EXIT_FAILURE);
						}
						else
						{
							value = atoi(argv[i+1]);
							if (value > 1)
								smoothscale = atoi(argv[i+1]);
							else
								smoothscale = 1;
							i++;
						}
					}
					else if (!strcmp(argv[i], "-verbose"))
					{
						verbose = 1;
					}
					else if (!strcmp(argv[i], "-nocenter"))
					{
						autocenter = 0;
					}
					else if (!strcmp(argv[i], "-textonly"))
					{
						textonly = 1;
					}
					else if (!strcmp(argv[i], "-quiet"))
					{
						quiet = 1;
						textonly = 1;
						verbose = 0;
					}
					else if (!strcmp(argv[i], "-ROI") || !strcmp(argv[i], "-roi"))
					{
						roi = 1;

						if ((i+4) >= argc)
						{
							print_usage();
							exit(EXIT_FAILURE);
						}
						else
						{
							roi_x1 = atoi(argv[i+1]);
							roi_y1 = atoi(argv[i+2]);
							roi_x2 = atoi(argv[i+3]);
							roi_y2 = atoi(argv[i+4]);

							i += 4;
						}
					}
					else if (!strcmp(argv[i], "-signaltofile"))
					{
						if ((i+1) >= argc)
						{
							print_usage();
							exit(EXIT_FAILURE);
						}
						else
						{
							outfile.open(argv[i+1]);
							if (outfile.fail())
							{
								fprintf(stderr, "ERROR: Cannot write to file \"");
								fprintf(stderr, argv[i+1]);
								fprintf(stderr, "\"!\n");
								print_usage();
								exit(EXIT_FAILURE);
							}
							i++;
						}
					}
					else if (!strcmp(argv[i], "-graphtofile"))
					{
						if ((i+1) >= argc)
						{
							print_usage();
							exit(EXIT_FAILURE);
						}
						else
						{
							graphfile.open(argv[i+1]);
							if (graphfile.fail())
							{
								fprintf(stderr, "ERROR: Cannot write to file \"");
								fprintf(stderr, argv[i+1]);
								fprintf(stderr, "\"!\n");
								print_usage();
								exit(EXIT_FAILURE);
							}
							else
							{
								graphfile.close();
								graphfilename = argv[i+1];
							}
							i++;
						}
					}
					else if (!strcmp(argv[i], "-histtofile"))
					{
						if ((i+1) >= argc)
						{
							print_usage();
							exit(EXIT_FAILURE);
						}
						else
						{
							histfile.open(argv[i+1]);
							if (histfile.fail())
							{
								fprintf(stderr, "ERROR: Cannot write to file \"");
								fprintf(stderr, argv[i+1]);
								fprintf(stderr, "\"!\n");
								print_usage();
								exit(EXIT_FAILURE);
							}
							else
							{
								histfile.close();
								histfilename = argv[i+1];
							}
							i++;
						}
					}
					else if (!strcmp(argv[i], "-faildir"))
					{
						if ((i+1) >= argc)
						{
							print_usage();
							exit(EXIT_FAILURE);
						}
						else
						{
							failfile = argv[i+1];
							i++;
						}
					}
					else
					{
						fprintf(stderr, "ERROR: Command-line option not recognized: ");
						fprintf(stderr, argv[i]);
						fprintf(stderr, "\n\n");
						usage = 1;
					}
				}
			}
			else if (argc > 21)
			{
				print_usage();
				exit(EXIT_FAILURE);
			}
			if (usage == 1)
			{
				print_usage();
				exit(EXIT_FAILURE);
			}
		}

		try
		{
			/* Load Image */
			img = cvLoadImage(argv[1], 0);
			/*img = cvLoadImage(argv[1], CV_LOAD_IMAGE_GRAYSCALE | CV_LOAD_IMAGE_ANYDEPTH);*/
		}
		catch (...)
		{
 			//string error = e.what();
			//cerr << "ERROR: " << error << endl;
			cerr << "ERROR: Could not load file: " << argv[1] << endl;
			exit(EXIT_FAILURE);
		}

	   // Check to ensure that image was loaded properly
	   if (img == 0)
	   {
			fprintf(stderr, "ERROR: Could not load file: ");
			fprintf(stderr, argv[1]);
			fprintf(stderr, "\n\n");
			exit(EXIT_FAILURE);
	   }

      
		/* ROI Validation */
		if (roi == 1)
		{
			if (roi_x1 > img->width || roi_x1 < 0 || roi_x2 > img->width || roi_x2 < 0 || roi_y1 > img->height || roi_y1 < 0 || roi_y2 > img->height || roi_y2 < 0)
			{
				fprintf(stderr, "ERROR: ROI points must be within the dimensions of the image!\n");
				print_usage();
				exit(EXIT_FAILURE);
			}
			else if (roi_x1 == roi_x2 || roi_y1 == roi_y2) 
			{
				fprintf(stderr, "WARNING: ROI points must form a rectangle with an area greater than 0; ROI has been ignored.\n");
				roi = 0;
			}
			else
			{
				if (roi_x1 < roi_x2)
				{
					fxmin = roi_x1;
					fxmax = roi_x2;
				}
				else
				{
					fxmin = roi_x2;
					fxmax = roi_x1;
				}
				if (roi_y1 < roi_y2)
				{
					fymin = roi_y1;
					fymax = roi_y2;
				}
				else
				{	
					fymin = roi_y2;
					fymax = roi_y1;
				}
			}
		}

		/* Resize image for display */
		double aspect_scale = (double)img->width / (double)img->height;
		int new_height = 500;
		double new_width = new_height * aspect_scale;
		if (textonly == 0)
		{
			IplImage *img_copy = cvCreateImage(cvSize((int)new_width, new_height), img->depth, img->nChannels);
			cvResize(img, img_copy, CV_INTER_CUBIC);
			dump_image("Original (resized for display)", img_copy, 1, verbose);
		}

		/* Generate Histogram */
		if (textonly == 0 || histfilename != "")
		{
			bool graph = (textonly == 0 ? 1 : 0);
			generate_histogram(img, graph, histfilename);
		}
	
		/* Find center of fingerprint (if exists) and determine if image should be cropped */
		CvPoint newcenter = cvPoint(0,0);
		if (autocenter != 0 && roi == 0) 
		{
			IplImage *img_cropped;
			if (img->width < 400 || img->height < 400)
			{
				IplImage *temp = pad_image(img, 400, 400, 128);

				// Set boundaries
				fxmin = 0;
				fxmax = img->width;
				fymin = 0;
				fymax = img->height;

				// Set center point
				newcenter.x = img->width / 2;
				newcenter.y = img->width / 2;

				img_cropped = temp;
			}
			else
			{
				newcenter = find_fingerprint_center_morph(img, &fxmin, &fxmax, &fymin, &fymax);

								if (verbose != 0 && autocenter != 0)
					printf("New Center: (%d, %d)\nX-boundary min: %d, X-boundary max: %d\nY-boundary min: %d, Y-boundary max: %d\n", newcenter.x, newcenter.y, fxmin, fxmax, fymin, fymax);

				img_cropped = cvCreateImage(cvSize(abs(fxmax-fxmin), abs(fymax-fymin)), img->depth, img->nChannels);

				// DEBUG
				//cout << "img size: (" << img->width << "," << img->height << ")\n";
				//cout << "img_cropped size: (" << img_cropped->width << "," << img_cropped->height << ")\n";
				
				crop_image(img, img_cropped, fxmin, fxmax, fymin, fymax);

				if (img_cropped->width < 400 || img_cropped->height < 400)
				{
					IplImage *temp = pad_image(img_cropped, 400, 400, 128);

					img_cropped = temp;
	
					// Set boundaries
					fxmin = 0;
					fxmax = img_cropped->width;
					fymin = 0;
					fymax = img_cropped->height;

					// Set center point
					newcenter.x = img_cropped->width / 2;
					newcenter.y = img_cropped->width / 2;
				}
			}
			
			if (textonly == 0)
			{
				dump_image("Cropped", img_cropped, 0, verbose);
			}
	
			/* Call SIVV on cropped */
			if (window == 0)
			{
				results = sivv(img_cropped, smoothscale, verbose, textonly, &signal, "", graphfilename, &fail);
			}
			else
			{
				results = sivv(img_cropped, smoothscale, verbose, textonly, &signal, windowtype, graphfilename, &fail);
			}
			if (failfile != "" && fail == 1)
			{
				// Try again with opposite window
				if (windowtype == "blackman")
					windowtype = "tukey";
				else if (windowtype == "tukey")
					windowtype = "blackman";
				results = sivv(img_cropped, smoothscale, verbose, textonly, &signal, windowtype, graphfilename, &fail);
			}
		}
		else if (roi == 1)
		{
			// Crop from ROI
			IplImage *img_cropped = cvCreateImage(cvSize(abs(fxmax-fxmin), abs(fymax-fymin)), img->depth, img->nChannels);
			crop_image(img, img_cropped, fxmin, fxmax, fymin, fymax);

			if (textonly == 0)
			{
				dump_image("Cropped", img_cropped, 0, verbose);
			}
	
			/* Call SIVV on cropped */
			if (window == 0)
			{
				results = sivv(img_cropped, smoothscale, verbose, textonly, &signal, "", graphfilename, &fail);
			}
			else
			{
				results = sivv(img_cropped, smoothscale, verbose, textonly, &signal, windowtype, graphfilename, &fail);
			}
			if (failfile != "" && fail == 1)
			{
				// Try again with opposite window
				if (windowtype == "blackman")
					windowtype = "tukey";
				else if (windowtype == "tukey")
					windowtype = "blackman";
				results = sivv(img_cropped, smoothscale, verbose, textonly, &signal, windowtype, graphfilename, &fail);
			}
		}
		else
		{
			/* Call SIVV on original */
			if (window == 0)
			{
				results = sivv(img, smoothscale, verbose, textonly, &signal, "", graphfilename, &fail);
			}
			else
			{
				results = sivv(img, smoothscale, verbose, textonly, &signal, windowtype, graphfilename, &fail);
			}
			if (failfile != "" && fail == 1)
			{
				// Try again with opposite window
				if (windowtype == "blackman")
					windowtype = "tukey";
				else if (windowtype == "tukey")
					windowtype = "blackman";
				results = sivv(img, smoothscale, verbose, textonly, &signal, windowtype, graphfilename, &fail);
			}
		}

		if (quiet == 0)
			cout << argv[1] << ", " << results << endl;

		// DEBUG
		/*if (fail == 1)
		{
			cout << "Fail detected...\n";
		}*/


		// If still fails...
		if (failfile != "" && fail == 1)
		{
			if (failfile.substr(failfile.length() - 1,1) != "\\")
			{
				failfile += "\\";
				failfile += argv[1];
			}
			else
			{
				failfile +=  argv[1];
			}

			testfile.open(failfile.c_str());
			if (testfile.fail())
			{
				fprintf(stderr, "ERROR: Cannot write to directory \"");
				fprintf(stderr, argv[i+1]);
				fprintf(stderr, "\"!\n");
			}
			else
			{
				testfile.close();
			}
		
			// DEBUG
			//cout << "\n\nDY FAIL (DY=0)\nfxmin: " << fxmin << "\nfymin: " << fymin << "\nfxmax: " << fxmax << "\nfymax: " << fymax << endl;

			IplImage *img_roi = cvCreateImage(cvGetSize(img), img->depth, 3);
			cvCvtColor(img, img_roi, CV_GRAY2RGB);

			cvRectangle(img_roi, cvPoint(fxmin, fymin), cvPoint(fxmax, fymax), cvScalar(0,255,0,0), 1, 8, 0);
			// DEBUG
			//dump_image("Image with ROI", img_roi, 1, verbose);

			cvSaveImage(failfile.c_str(), img_roi);
		}

		if (outfile.good())
		{
			for(vector<double>::const_iterator s = signal.begin(); s != signal.end(); s++)
			{
				if (s == signal.begin())
				{
					outfile << *s;
				}
				else
				{
					outfile << "," << *s;
				}
			}
			outfile.close();
		}

		/* 	Graph */
		if (textonly == 0)
		{
 
	/* Align windows nicely */
	#ifdef WIN32		
			HWND window;
			RECT rectWindow;
			int bottom;
		
			cvMoveWindow("Original (resized for display)", 0, 0);
			window = (HWND)cvGetWindowHandle("Original (resized for display)");
			GetWindowRect(window, &rectWindow);
			bottom = rectWindow.bottom + 3;
			cvMoveWindow("1D Power Spectrum Graph", rectWindow.right+3, 0);
		
			// DEBUG
			//cvMoveWindow("Blackman Window", 0, bottom);
			cvMoveWindow("Tukey Window", 0, bottom);

			window = (HWND)cvGetWindowHandle("Blackman Window");
			GetWindowRect(window, &rectWindow);
			cvMoveWindow("Log Power Spectrum", rectWindow.right+3, bottom);
			window = (HWND)cvGetWindowHandle("Log Power Spectrum");
			GetWindowRect(window, &rectWindow);
			cvMoveWindow("Polar Transform", rectWindow.right+3, bottom);
			window = (HWND)cvGetWindowHandle("Polar Transform");
			GetWindowRect(window, &rectWindow);
			cvMoveWindow("Cropped", rectWindow.right+3, bottom);
			window = (HWND)cvGetWindowHandle("Log Power Spectrum");
			GetWindowRect(window, &rectWindow);
			cvMoveWindow("Histogram", rectWindow.left, rectWindow.bottom+3);


	#else
			/* Manual alignment in linux */
		
			cvMoveWindow("Cropped", 975, (int)new_height+50);
			cvMoveWindow("Blackman Window", 0, (int)new_height+50);
			cvMoveWindow("Log Power Spectrum", 325, (int)new_height+50);
			cvMoveWindow("Polar Transform", 650, (int)new_height+50);
			cvMoveWindow("Original (resized for display)", 0, 0);
			cvMoveWindow("1D Power Spectrum Graph", (int)new_width + 5, 0);
			cvMoveWindow("Histogram", 325, 875);
		
	
			/* Uncomment for Automatic alignment in linux (using GTK) */
			/*
			GtkWidget *widget;
			GtkRequisition req;
			gint x, y, w, h, bottom;

			cvMoveWindow("Original (resized for display)", 0, 0);
			widget = (GtkWidget*)cvGetWindowHandle("Original (resized for display)");
			gdk_window_get_deskrelative_origin(widget->window, &x, &y);
			x += (widget->style->xthickness * 2);
			y += (widget->style->ythickness * 2);
			gtk_widget_size_request(widget, &req);
			w = req.width + x;
			h = req.height + y;
			bottom = h;
			cvMoveWindow("1D Power Spectrum Graph", w, 0);
			cvMoveWindow("Blackman Window", 0, bottom);
			widget = (GtkWidget*)cvGetWindowHandle("Blackman Window");
			gdk_window_get_deskrelative_origin(widget->window, &x, &y);
			x += (widget->style->xthickness * 2);
			w = 320 + x;
			cvMoveWindow("Log Power Spectrum", w, bottom);
			widget = (GtkWidget*)cvGetWindowHandle("Log Power Spectrum");
			gdk_window_get_deskrelative_origin(widget->window, &x, &y);
			x += (widget->style->xthickness * 2);
			w = 320 + x;
			cvMoveWindow("Polar Transform", w, bottom);
			widget = (GtkWidget*)cvGetWindowHandle("Polar Transform");
			gdk_window_get_deskrelative_origin(widget->window, &x, &y);
			x += (widget->style->xthickness * 2);
			w = 320 + x;
			cvMoveWindow("Cropped", w, bottom);
			*/
		
	#endif
			
			// Performance test
			//total_end = clock();
			//cout << "Total SIVV CPU Time: " << total_end - total_start << endl;
			
		   /* Pause until dismissed by user */
		   cvWaitKey(0);
		}

	   exit(EXIT_SUCCESS);
	}
	catch(cv::Exception& e)
	{
		string error = e.what();
		cerr << "OPENCV ERROR: " << error << endl;
		exit(EXIT_FAILURE);
    }
	catch (std::exception &e)
	{
 		string error = e.what();
		cerr << "ERROR: " << error << endl;
		exit(EXIT_FAILURE);
	}
}