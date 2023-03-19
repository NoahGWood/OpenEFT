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
      PACKAGE: NIST Image Display

      FILE:    DPYIMAGE.H

      AUTHORS: Stan Janet
               Michael D. Garris
      DATE:    12/03/1990
      UPDATED: 05/10/2005 by MDG

***********************************************************************/
#ifndef _DPYIMAGE_H
#define _DPYIMAGE_H

#include <dpyx.h>

/* X-Window global references. */
extern unsigned int dw, dh;
extern int window_up;
extern int got_click;
extern unsigned int depth;
extern unsigned int ww, wh, iw, ih;
extern int absx, absy, relx, rely;
extern int x_1, y_1;

/* X-Window Contols & command line globals. */
extern char *program;
extern char *filename;
extern int accelerator;
extern unsigned int init_ww, init_wh;
extern int nicevalue;
extern int pointwidth;
extern char *title;
extern unsigned int wx, wy;
extern int verbose;
extern int debug;
extern int errors;

extern int automatic;
extern unsigned int sleeptime;
extern int dpy_mode;
extern int raw;
extern unsigned int raw_w, raw_h, raw_depth, raw_whitepix;
extern char def_tmpdir[];
extern char *tmpdir;

extern int nist_flag;
extern int iafis_flag;

/************************************************************************/
/* dpyimage.c */
extern int dpyimage(char *, register unsigned char *, unsigned int,
                    unsigned int, unsigned int, unsigned int, int, int *);
extern int ImageBit8ToBit24Unit32(char **, char *, int, int);
extern void XMGetSubImageDataDepth24(char *, int, int, int, int,
                    char *, int, int);
extern int event_handler(register XImage *, register unsigned char *, int *);
extern void refresh_window(register XImage *);
extern int drag_image(register XImage *, register unsigned char *, int, int);
extern int move_image(register XImage *, register unsigned char *, int, int);
extern int button_release(XEvent *, register XImage *,
                    register unsigned char *);
extern void button_press(XEvent *);

/* dpyio.c */
extern int readfile(char *, unsigned char **, int *, unsigned int *,
                    unsigned int *, unsigned int *, unsigned int *, int *);
extern int createfile(char *);
extern void unlinkfile(char *);
extern void buildheader(struct header_t *, char *, unsigned int, unsigned int,
                    unsigned int, unsigned int, int);
extern int writeheader(FILE *, struct header_t *);
extern int readheader(FILE *, struct header_t *);
extern int writedata(FILE *, unsigned char *, int);
extern int readdata(FILE *, unsigned char *, int);
extern int fdclose(int, char *);

/* dpynorm.c */
extern int dpynorm(int, char **);

/* dpypipe.c */
extern int pipecomm(int, char **);
extern int pipe_parent(register FILE *);
extern int pipe_child(int, char **, register FILE *);

/* dpytmp.c */
extern int tmpcomm(int, char **);
extern int tmp_parent(pid_t);
extern int tmp_child(int, char **, pid_t);

/* tally.c */
extern int bitcount(register unsigned int);
extern void bytecount(register unsigned char *, register unsigned int,
                      register unsigned int *);
extern void pixelcount(register unsigned char *, register unsigned int,
                       register unsigned int *, register unsigned int *);

#endif
