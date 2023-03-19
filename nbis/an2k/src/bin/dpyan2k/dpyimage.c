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

      FILE:    DPYIMAGE.C

      AUTHORS: Michael D. Garris
               Stan Janet
      DATE:    03/07/2001
      UPDATED: 05/10/2005 by MDG
      UPDATED: 04/09/2008 by Joseph C. Konczal - added record selection
      UPDATED: 04/23/2008 by Joseph C. Konczal - added display of SEG/ASEG data
      UPDATED: 09/10/2008 by Joseph C. Konczal

      ROUTINES:
               dpyimagepts()
               ImageBit8ToBit24Unit32()
               XMGetSubImageDataDepth24()
               event_handler()
               refresh_window()
               drag_image()
               move_image()
               button_release()
               button_press()

***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <an2k.h>
#include <dpyimage.h>

#define BITMAP_UNIT_24          4 /* 4 bytes ==> 32 bits */

/* X-Window global references. */
unsigned int dw, dh;
int window_up;
int got_click;
unsigned int depth;
unsigned int ww, wh, iw, ih;
int absx, absy, relx, rely;
int x_1, y_1;

/*********************************************************************/
/* X-Window global controls. */
char *program;
char *filename = "";
int accelerator = 1;
unsigned int init_ww=0, init_wh=0;
int nicevalue = -1;
int pointwidth = 3;
char *title = (char *)NULL;
int wx=0, wy=0;
int verbose = 0;
int debug = 0;
/* Deactivated from command line: defaults used only. */
int automatic = False;
unsigned int sleeptime = DEF_SLEEPTIME;

int nist_flag = True;
int iafis_flag = False;

/************************************************************************/
int dpyimagepts(char *fname, unsigned char *data,
		 unsigned int image_w, unsigned int image_h,
                 unsigned int d, unsigned int whitepix, int align, int *done,
		 int *xs, int *ys, int npts, const SEGMENTS *const segs)
{
   static int first = True;
   int ret;
   XImage *image = NULL;
   unsigned char *windata;
   char *data24;
   unsigned int max_whitepix = ((1 << CHAR_BIT) - 1);
   unsigned int windatasize;
   unsigned int new_ww, new_wh;

   if((ret = xconnect()))
      return(ret);
   screen = DefaultScreen(display);
   def_cmap = DefaultColormap(display,screen);
   dw = DisplayWidth(display,screen);
   dh = DisplayHeight(display,screen);
   rw = RootWindow(display,screen);
   visual = DefaultVisual(display,screen);
   bp = BlackPixel(display,screen);
   wp = WhitePixel(display,screen);

   absx = 0;
   absy = 0;
   relx = 0;
   rely = 0;

   filename = fname;
   iw = image_w;
   ih = image_h;

   if (first) {
      first = False;
      depth = d;
      if ((depth != 1) &&
          (depth != CHAR_BIT) &&
          (depth != 24)){
          (void) fprintf(stderr,
                 "%s: %s: depth (%u) must be 1, %d or 24\n",
                 program,filename,depth,CHAR_BIT);
          return(-2);
      }

      if (whitepix && (whitepix != max_whitepix)) {
         fprintf(stderr, "ERROR : dpyimagepts : ");
         fprintf(stderr, "whitepix (%u) must be either 0 or %u\n",
                 whitepix,max_whitepix);
         return(-3);
      }
      wp = (unsigned long) whitepix;
      bp = wp ? 0L : max_whitepix;

      ww = init_ww ? MIN(init_ww,dw) : MIN(iw,dw);
      wh = init_wh ? MIN(init_wh,dh) : MIN(ih,dh);
      if((ret = initwin(wx,wy,ww,wh,depth,wp)))
         return(ret);
   } else {
      if (d != depth) {
         fprintf(stderr, "ERROR : dpyimagepts : ");
         fprintf(stderr, "new depth %u (expected %u)\n", d, depth);
         return(-4);
      }
      if ((depth != 1) && (whitepix != wp)) {
         fprintf(stderr, "ERROR : dpyimagepts : ");
         fprintf(stderr, "new white pixel %u (expected %lu)\n",
                 whitepix, wp);
         return(-5);
      }
      new_ww = init_ww ? MIN(init_ww,dw) : MIN(iw,dw);
      new_wh = init_wh ? MIN(init_wh,dh) : MIN(ih,dh);
      if ((ww != new_ww) || (wh != new_wh)) {
         ww = new_ww;
         wh = new_wh;
         XResizeWindow(display,window,ww,wh);
      }
   }

   if (depth == 1)
      windatasize = howmany(ww,CHAR_BIT) * wh;
   else if (depth == 8)
      windatasize = ww * wh;
   else /* if (depth == 8) */
      windatasize = ww * wh * BITMAP_UNIT_24;

   windata = (unsigned char *) malloc(windatasize);
   if (windata == (unsigned char *) NULL) {
      fprintf(stderr, "ERROR : dpyimagepts : malloc(%u) failed\n", windatasize);
      return(-6);
   }

   if (! no_window_mgr)
      XStoreName(display,window, (title == (char *) NULL) ? filename : title );

   if (depth == 1) {
      XMGetSubImageData(((char *)data),relx,rely,iw,ih,
                        ((char *)windata),ww,wh);
      XMCreateBellImage(image,display,visual,(char *)windata,ww,wh,8);
      image->bitmap_bit_order = MSBFirst;
   }
   else if(depth == 8){
      if (visual->class == TrueColor) {
         XMGetSubImageDataDepth(((char *)data),relx,rely,iw,ih,
                                ((char *)windata),ww,wh);
         if((ret = ImageBit8ToBit24Unit32(&data24, (char *)windata, ww, wh))){
            free(windata);
            return(ret);
         }
         free(windata);
         image = XCreateImage(display,visual,DefaultDepth(display, screen),
                              ZPixmap,0,(char *)data24,ww,wh,align,
                              ww*BITMAP_UNIT_24);
         if (image == (XImage *) NULL) {
            (void) fprintf(stderr,"%s: cannot create %u x %u 24-bit image\n",
                           program,ww,wh);
            return(-7);
         }
      }
      else {
         XMGetSubImageDataDepth(((char *)data),relx,rely,iw,ih,
                                ((char *)windata),ww,wh);

         image = XCreateImage(display,visual,depth,ZPixmap,0,
			(char *)windata,ww,wh,align,ww);
         if (image == (XImage *) NULL) {
            (void) fprintf(stderr,"%s: cannot create %u x %u image\n",
                           program,ww,wh);
            return(-8);
         }
      }
   }
   else { /* if(depth == 24){ */
      if (visual->class == TrueColor) {
         XMGetSubImageDataDepth24(((char *)data),relx,rely,iw,ih,
                                  ((char *)windata),ww,wh);
         image = XCreateImage(display,visual,DefaultDepth(display, screen),
                              ZPixmap,0,(char *)windata,ww,wh,align,
                              ww*BITMAP_UNIT_24);
         if (image == (XImage *) NULL) {
            (void) fprintf(stderr,"%s: cannot create %u x %u 24-bit image\n",
                           program,ww,wh);
            return(-9);
         }
      }
      else {
         fprintf(stderr, "PsuedoColor is not currently supported for RGB\n");
         return(-10);
      }
   }

   if ( verbose ){
      switch ( image->bitmap_bit_order ) {
         case LSBFirst:
              (void) printf("bitmap bit order is LSBFirst\n"); break;
         case MSBFirst:
              (void) printf("bitmap bit order is MSBFirst\n"); break;
         default:
              (void) printf("bitmap bit order unknown\n"); break;
      }
   }

   if (window_up) {
      if (verbose)
         (void) printf("window is up, calling refresh()\n");
      refresh_window(image, xs, ys, npts, segs);
   }

   if((ret = event_handler(image,data,done,xs,ys,npts, segs)))
      return(ret);

   XClearWindow(display,window);
   XDestroyImage(image);

   cleanup();

   return(0);
}

/*******************************************************************/
int ImageBit8ToBit24Unit32(char **data24, char *data8, int ww, int wh)
{
   char *sptr, *dptr;
   int i, n, pixlen;

   pixlen=ww*wh;
   (*data24) = (char *)calloc(pixlen,BITMAP_UNIT_24);
   if(*data24 == (char *)NULL){
      fprintf(stderr, "ImageBit8ToBit24Unit32 : calloc : data24\n");
      return(-2);
   }

   sptr = data8;
   dptr = *data24;
   for(i = 0; i < pixlen; i++){
      for(n = 0; n < BITMAP_UNIT_24-1; n++)
         *dptr++ = *sptr;
      dptr++;
      sptr++;
   }

   return(0);
}

/*******************************************************************/
void XMGetSubImageDataDepth24(char *src, int x, int y, int srcw, int srch,
                              char *dst, int dstw, int dsth)
{
   int i, j, src_bytew;
   char *sptr, *ssptr, *dptr;

   if (x < 0)
      x = 0;
   else {
      if (x > srcw - dstw)
         x = (srcw - dstw);
   }

   if (y < 0)
      y = 0;
   else {
      if (y > srch - dsth)
         y = srch - dsth;
   }

   ssptr = src + (y*srcw*3) + (x*3);
   dptr = dst;
   src_bytew = (srcw*3);

   for(i = 0; i < dsth; i++){
      sptr = ssptr;
      for(j = 0; j < dstw; j++){
         /* SRC RGB -> DST BGR */
         *(dptr+2) = *sptr++;
         *(dptr+1) = *sptr++;
         *dptr = *sptr++;
         dptr += 3;
         *dptr++ = 0L;
      }
      ssptr += src_bytew;
   }
}

/*******************************************************************/
int event_handler(XImage *image, unsigned char *data,
		  int *done, int *xs, int *ys, int npts,
		  const SEGMENTS *const segs)
{
   int ret;
   XEvent event;
   int quit;

   if (automatic) {
      if (! window_up) {
         window_up = True;
         XMaskEvent(display,(unsigned long)ExposureMask,&event);
         refresh_window(image, xs, ys, npts, segs);
      }
      sleep(sleeptime);
      return(0);
   }

   quit = False;
   got_click = False;
   while ( ! quit ) {

      XNextEvent(display,&event);

      switch( event.type ) {

         case ConfigureNotify:
              ww = event.xconfigure.width;
              wh = event.xconfigure.height;
              if (verbose)
                 (void) printf("\twindow resized to %u x %u\n",ww,wh);
              break;

         case ButtonPress:
              button_press(&event);
              break;

         case ButtonRelease:
	      if((ret = button_release(&event,image,data,xs,ys,npts,segs)))
                 return(ret);
              break;

         case KeyPress:
              {
                 KeySym keysym;

                 if (verbose)
                    (void) printf("KeyPress event received\n");

                 keysym = XLookupKeysym((XKeyEvent *)&event,ShiftMapIndex);
                 switch (keysym) {
                    case XK_Alt_L:
                    case XK_Alt_R:
                    case XK_F1:
                    case XK_F3:
                         break;

                    case XK_X:
                    case XK_x:
                         quit = True;
                         *done = True;
                        break;
                    default:
                         quit = True;
                         break;
                 } /* switch */
              }
              break;

         case Expose:
              window_up = True;

              while(XCheckTypedWindowEvent(display, event.xexpose.window,
                                           Expose, &event)){
                 if (verbose)
                    printf("   Window %d expose event %d ignored\n",
                           event.xexpose.count, event.xexpose.count);
              }

              if (verbose)
                 (void) printf("Expose event received, calling refresh()\n");
              refresh_window(image, xs, ys, npts, segs);

              break;
      } /* switch */
   } /* while */

   return(0);
}

/****************************************************************/
void refresh_window(XImage *image, int *xs, int *ys, int npts,
		    const SEGMENTS *const segs)
{
   int fgp_count[20];
   int i, tx, ty, pol_i, vtx_i, fgp_i;
   int point_wh;
   int point_wh2;
   POLYGON *pol;
   int xpts_bytes = 0;
   XPoint *xpts = NULL;

   /* Pointwidth is globally defined and potentially set by procargs. */
   point_wh2 = pointwidth>>1;
   point_wh = (point_wh2<<1) + 1;
   if(point_wh <= 0){
      point_wh2 = 0;
      point_wh = 1;
   }

   XClearWindow(display,window);
   XPutImage(display,window,gc,image,0,0,0,0,ww,wh);

   /* Draw minutiae */
   for(i = 0; i < npts; i++){
      if(((tx = xs[i]-absx-point_wh2) >= 0) &&
         (tx < ww) &&
         ((ty = ys[i]-absy-point_wh2) >= 0) &&
         (ty < wh)){
            XFillRectangle(display,window,pointgc,tx, ty, point_wh, point_wh);
       }
   }

   /* Draw segmentation boxes */
   if (segs != NULL) {
      for (fgp_i = 0; fgp_i < sizeof(fgp_count)/sizeof(fgp_count[0]); fgp_i++)
	 fgp_count[fgp_i] = 0;
      
      for (pol_i = 0; pol_i < segs->num_polygons; pol_i++) {
	 pol = segs->polygons + pol_i;

	 /* allocate more space if needed */
	 if (((pol->num_points + 1) * sizeof(XPoint)) > xpts_bytes) {
	    xpts_bytes = (pol->num_points + 1) * sizeof(XPoint);
	    xpts = (XPoint*)realloc(xpts, xpts_bytes);
	    if (NULL == xpts) {
	       fprintf(stderr, "ERROR : refresh_window : "
		       "cannot allocate %d bytes for data points\n",
		       pol->num_points * sizeof(XPoint));
	       return;
	    }
	    /*	 fprintf(stderr, "INFO : refresh_window : "
		 "allocated %d bytes for segment data points\n",
		 xpts_bytes); */
	 }
	 
	 /* define a cyclic path */
	 for (vtx_i = 0; vtx_i < pol->num_points; vtx_i++) {
	    xpts[vtx_i].x = (short)pol->x[vtx_i];
	    xpts[vtx_i].y = (short)pol->y[vtx_i];
	 }
	 xpts[pol->num_points].x = xpts[0].x;
	 xpts[pol->num_points].y = xpts[0].y;
	 
	 XDrawLines(display, window, 
		    seggc[fgp_count[pol->fgp]++%(sizeof seggc/sizeof seggc[0])],
		    xpts, pol->num_points+1, CoordModeOrigin);
      }
   }
}

/************************************************************/
int drag_image(XImage *image, unsigned char *data,
	       int dx, int dy, int *xs, int *ys, int npts,
	       const SEGMENTS *const segs)
{
   int ret;
   int px, py;

   if (accelerator != 1) {
      dx *= accelerator;
      dy *= accelerator;
   }

   dx = (dx / CHAR_BIT) * CHAR_BIT;
   dy = (dy / CHAR_BIT) * CHAR_BIT;

   if (!dx && !dy) {
      if (verbose)
         (void) printf("\tdrag_image: no change\n");
      return(0);
   }

   px = absx + dx;
   py = absy + dy;
   if((ret = move_image(image,data,px,py,xs,ys,npts,segs)))
      return(ret);

   return(0);
}

/************************************************************/
int move_image(XImage *image, unsigned char *data,
	       int px, int py, int *xs, int *ys, int npts,
	       const SEGMENTS *const segs)
{
   int ret;
   char *data8, *data24;

   if (verbose)
      (void) printf("\tmove_image: %d %d\n",px,py);

   if (depth == 1) {
      XMGetSubImageData(((char *)data), px,py,iw,ih,image->data,ww,wh);
   }
   else if(depth == 8){
      if (visual->class == TrueColor){
         data8 = (char *)malloc(ww*wh);
         if(data8 == (char *)NULL){
            fprintf(stderr, "move_image : malloc : data8\n");
            return(-2);
         }
         XMGetSubImageDataDepth(((char *)data), px,py,iw,ih,data8,ww,wh);
         if((ret = ImageBit8ToBit24Unit32(&data24, data8, ww, wh))){
            free(data8);
            return(ret);
         }
         free(data8);
         image->data = data24;
      } else {
         XMGetSubImageDataDepth(((char *)data), px,py,iw,ih,image->data,ww,wh);
      }
   }
   else { /* if(depth == 24){ */
      if (visual->class == TrueColor) {
	 XMGetSubImageDataDepth24(((char *)data),px,py,iw,ih,
                                  image->data,ww,wh);
      }
      else {
         fprintf(stderr, "PsuedoColor is not currently supported for RGB\n");
         return(-3);
      }
   }

   absx = px;
   absy = py;
   refresh_window(image,xs,ys,npts,segs);

   return(0);
}

/************************************************************/
int button_release(XEvent *event, XImage *image,
		   unsigned char *data, int *xs, int *ys, int npts,
		   const SEGMENTS *const segs)
{
   int ret;
   int x, y, on;
   unsigned int state;

   if (! got_click)
      return(0);

   state = event->xbutton.state & ALL_BUTTONS;
   for (on=0; state; state >>= 1){
      if (state & 1)
         on++;
   }
   if (on != 1) {
      if (verbose)
         (void) printf("\tbutton_release: rejected (buttons still down)\n");
      got_click = False;
      return(0);
   }

   x = event->xbutton.x;
   y = event->xbutton.y;

   if (! PT(x,y,iw,ih)) {
      if (verbose)
         (void) printf("\tbutton_release: rejected (bad point)\n");
      return(0);
   }

   got_click = False;
   if (verbose)
      (void) printf("Button Release at (%d,%d)\n",x,y);
   if((ret = drag_image(image,data,x_1-x,y_1-y,xs,ys,npts,segs)))
      return(ret);

   return(0);
}

/*************************************************************/
void button_press(XEvent *event)
{
   if (got_click) {
      if (verbose)
         (void) printf("\tbutton_press: already had a click\n");
      got_click = False;
      return;
   }

   if (event->xbutton.state & ALL_BUTTONS) {
      if (verbose)
         (void) printf("\tbutton_press: rejected (buttons already down\n");
      got_click = False;
      return;
   }

   x_1 = event->xbutton.x;
   y_1 = event->xbutton.y;

   if (! PT(x_1,y_1,iw,ih)) {
      if (verbose)
         (void) printf("\tbutton_press: rejected (bad point)\n");
      got_click = False;
      return;
   }

   if (verbose)
      (void) printf("Button Press at (%d,%d)\n",x_1,y_1);
   got_click = True;
}

