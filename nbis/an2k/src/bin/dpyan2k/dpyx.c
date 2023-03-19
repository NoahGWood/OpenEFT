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

      FILE:    DPYX.C

      AUTHORS: Michael D. Garris
               Stan Janet
      DATE:    12/30/1990
      UPDATED: 05/10/2005 by MDG
      UPDATED: 04/25/2008 by Joseph C. Konczal - added display of SEG/ASEG data

      ROUTINES:
               cleanup()
               xconnect()
               intiwin()
               set_gray_colormap()
               gray_colormap()

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <dpyx.h>

/* X-Window globals. */
Display *display;
char *display_name;
Window window, rw;
Visual *visual;
int screen;
Colormap def_cmap, cmap;
int cmap_size;
GC gc, boxgc, pointgc, seggc[3];
unsigned long bp, wp;
unsigned int border_width = DEF_BORDER_WIDTH;
int no_window_mgr = False;
int no_keyboard_input = False;

/******************************************************************/
void cleanup(void)
{
   int i;

   (void)XFreeGC(display,boxgc);
   (void)XFreeGC(display,pointgc);
   for (i = 0; i < (int)(sizeof(seggc)/sizeof(seggc[0])); i++)
      (void)XFreeGC(display,seggc[i]);

   (void)XUnmapWindow(display,window);
   (void)XDestroyWindow(display,window);

   if((visual->class != TrueColor) &&
      (cmap != def_cmap))
      (void)XFreeColormap(display,cmap);

   (void)XCloseDisplay(display);
}

/************************************************************************/
int xconnect(void)
{
   display = XOpenDisplay(display_name);
   if (display == (Display *) NULL) {
      fprintf(stderr, "ERROR : xconnect : ");
      fprintf(stderr, "cannot connect to X11 server %s\n",
              XDisplayName(display_name));
      return(-2);
   }

   /* Return normally. */
   return(0);
}

/*****************************************************************/
int initwin(int wx, int wy, unsigned int ww, unsigned int wh, 
	    unsigned int depth, unsigned long wp)
{
  int ret;

   if (depth == 1) {

      cmap = def_cmap;
      window = XCreateSimpleWindow(display,rw,wx,wy,ww,wh,border_width,bp,wp);

   } else if (depth == (unsigned int)CHAR_BIT) {
      /* if default visual is TrueColor, then no colormap to manipulate */
      if(visual->class == TrueColor){
         window = XCreateSimpleWindow(display,rw,wx,wy,ww,wh,
                                      border_width,bp,wp);
      }
      /* otherwise assume PsuedoColor or GrayScale Visual ... */
      else{

         unsigned long wmask;
         XSetWindowAttributes wattr;

         ret = gray_colormap(&cmap, display, &visual,depth);
         if (ret || (cmap == None)) {
            fprintf(stderr,"ERROR : initwin : cannot obtain gray colormap\n");
            return(-2);
         }

         cmap_size = 1 << depth;
         if((ret = set_gray_colormap(display,cmap,cmap_size,wp)))
            return(ret);

         wattr.colormap = cmap;
         wmask = CWColormap;
         window = XCreateWindow(display,rw,wx,wy,ww,wh,border_width,
                                CopyFromParent,CopyFromParent,
                                visual,wmask,&wattr);
      }
   }
   else if(depth == 24){
      /* if default visual is TrueColor, then no colormap to manipulate */
      if(visual->class == TrueColor){
	   window = XCreateSimpleWindow(display,rw,wx,wy,ww,wh,
                                       border_width,bp,wp);
      }
      /* otherwise assume PsuedoColor, which is not supported ... */
      else{
	   fprintf(stderr, "PsuedoColor is not currently supported for RGB\n");
           return(-3);
      }
   }
   else {
      fprintf(stderr, "ERORR : initwin : ");
      fprintf(stderr, "depth (%u) must be either 1, %d or 24\n",
              depth, CHAR_BIT);
      return(-4);
   }

   gc = XDefaultGC(display,screen);
   boxgc = XCreateGC(display,rw,(unsigned long)0L,(XGCValues *)NULL);
   XSetFunction(display,boxgc,GXinvert);
   pointgc = XCreateGC(display,rw,(unsigned long)0L,(XGCValues *)NULL);

   /* create GCs needed for display of segmentation boxes */
   XSetForeground(display,pointgc,0xFF0000);
   XSetFunction(display,pointgc,GXcopy);
   seggc[0] = XCreateGC(display,rw,(unsigned long)0L,(XGCValues *)NULL);
   XSetForeground(display,seggc[0],0x00FF00); /* green */
   XSetFunction(display,seggc[0],GXcopy);
   seggc[1] = XCreateGC(display,rw,(unsigned long)0L,(XGCValues *)NULL);
   XSetForeground(display,seggc[1],0x0000FF); /* blue */
   XSetFunction(display,seggc[1],GXcopy);
   seggc[2] = XCreateGC(display,rw,(unsigned long)0L,(XGCValues *)NULL);
   XSetForeground(display,seggc[2],0xFF00FF); /* magenta */
   XSetFunction(display,seggc[2],GXcopy);

   {
      XSizeHints size_hints;
      size_hints.x = wx;
      size_hints.y = wy;
      size_hints.width = ww;
      size_hints.height = wh;
      size_hints.flags = (USSize | USPosition);
      XSetStandardProperties(display,window,"","",None,(char **)NULL,
                             0,&size_hints);
   }

   if (no_window_mgr) {
      XSetWindowAttributes winattr;

      winattr.override_redirect = True;
      XChangeWindowAttributes(display,window,CWOverrideRedirect,&winattr);
   }
   else {
      XWMHints wmhints;

      wmhints.flags = InputHint;
      wmhints.input = True;
      XSetWMHints(display,window,&wmhints);
   }

   {
      unsigned long inputmask;

      inputmask = ( ExposureMask | ButtonPressMask | ButtonReleaseMask );
      if (! no_keyboard_input)
         inputmask |= ( KeyPressMask | KeyReleaseMask );
         XSelectInput(display,window,inputmask);
   }

   XMapRaised(display,window);

   /* Return normally. */
   return(0);
}

/*****************************************************************/
/* Sets the colormap intensities to be gray (equal r,g&b values) */
/*	and evenly spread out between black (0) and white        */
/*	(the maximum value for a unsigned short).                       */
/*****************************************************************/
int set_gray_colormap(Display *display, Colormap cmap,
                      unsigned int cmap_size, unsigned long wp)
{
   register unsigned long i;
   unsigned short m;
   unsigned short flags = (DoRed | DoGreen | DoBlue);
   XColor *cmap_defs;

   if (! cmap_size){
      fprintf(stderr, "ERROR : set_gray_colormap : colormap size is zero\n");
      return(-2);
   }

   if (wp && (wp != (unsigned long)cmap_size - 1L)){
      fprintf(stderr, "ERROR : set_gray_colormap : ");
      fprintf(stderr, "white pixel must be zero or (colormap size - 1)\n");
      return(-3);
   }

   cmap_defs = (XColor *) malloc((unsigned int) (cmap_size * sizeof(XColor)));
   if (cmap_defs == (XColor *) NULL){
      fprintf(stderr, "ERROR : set_gray_colormap : malloc failed\n");
      return(-4);
   }

   for (i=0; i < cmap_size; i++) {
      cmap_defs[i].pixel = i;
      m = ((wp ? i : ((cmap_size-1)-i)) / (double) (cmap_size - 1)) *
           ((unsigned short) ~0) + 0.5;
#ifdef DEBUG
      (void) printf("m[%d] = %u\n",i,m);
#endif
      cmap_defs[i].red   = m;
      cmap_defs[i].green = m;
      cmap_defs[i].blue  = m;
      cmap_defs[i].flags = flags;
   }

   XStoreColors(display,cmap,cmap_defs,cmap_size);

   free((char *) cmap_defs);

   return(0);
}

/************************************************************************/
/* Returns a gray colormap of the specified depth, setting *bp and *wp  */
/* to the correct values for the blackest and whitest pixels.           */
/* Visual should be set to the DefaultVisual. If the class for this     */
/*	visual is non-static (GrayScale or PseudoColor), a suitable     */
/*	visual must be found. If it is, visual is reset; otherwise      */
/*	returns None.                                                   */
/* A fatalerr error occurs if XCreateColormap() fails.                  */
/************************************************************************/
int gray_colormap(Colormap *ocmap, Display *display,
                  Visual **visual, unsigned int depth)
{
   XVisualInfo vinfo;
   Colormap cmap;
   int screen;

   screen = DefaultScreen(display);

   if (((*visual)->class != GrayScale) && ((*visual)->class != PseudoColor)) {
      if ((! XMatchVisualInfo(display,screen,depth,GrayScale,&vinfo)) &&
         (! XMatchVisualInfo(display,screen,depth,PseudoColor,&vinfo))) {
#ifdef DEBUG
         (void) fprintf(stderr,"Cannot find a suitable visual\n");
#endif
         *ocmap = (Colormap) None;
      }
      *visual = vinfo.visual;
   }

   cmap = XCreateColormap(display,RootWindow(display,screen),*visual,AllocAll);
   if (cmap == None){
      fprintf(stderr, "ERROR : gray_colormap : XCreateColormap failed\n");
      return(-2);
   }

   *ocmap = cmap;

   return(0);
}
