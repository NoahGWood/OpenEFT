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

      FILE:    DPYTMP.C

      AUTHORS: Stan Janet
               Michael D. Garris
      DATE:    12/03/1990
      UPDATED: 05/10/2005 by MDG

      ROUTINES:
               tmpcomm()
               tmp_parent()
               tmp_child()

***********************************************************************/

#include <usebsd.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/file.h>
#include <sys/param.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <dpyimage.h>

extern int kill(pid_t, int);
extern int fileno(FILE *);

/*****************************************************************/
int tmpcomm(int argc, char **argv)
{
   int ret;
   pid_t pid, p;
   char dir[MAXPATHLEN];

   if (verbose)
      (void) printf("In tmpcomm()\n");

   p = getpid();
   (void) sprintf(dir,OUTFILE_DIRFMT,tmpdir,(int)p);
   if (mkdir(dir,OUTFILE_DIRMODE) < 0) {
      perror(dir);
      return(-2);
   }

   pid = fork();
   if ((int)pid < 0) {
      perror("Fork failed");
      return(-3);
   }

   if ((int)pid) {
      char cmd[2*MAXPATHLEN];

      if((ret = tmp_parent(p)))
         return(ret);
      if((ret =  kill(pid,SIGKILL)))
         return(ret);
      (void) sprintf(cmd,"/bin/rm -f %s/*",dir);
      if (verbose)
         (void) printf("%s\n",cmd);
         (void) system(cmd);
      if (rmdir(dir) < 0)
         perror(dir);
   } else {
      extern int nicevalue;
      extern char *program;

      program = "[child]";
      if((nicevalue >= 0) && (nice(nicevalue) < 0))
         perror("Nice failed");
      if((ret = fdclose(0,"stdin")))
         return(ret);
      if(open("/dev/null",O_RDWR) < 0) {
         perror("Cannot open /dev/null");
         return(-4);
      }
      if((ret = fdclose(ConnectionNumber(display),"X11 connection")))
         return(ret);
      if((ret = tmp_child(argc,argv,p)))
         return(ret);
      return(0);
   }

   return(0);
}

/************************************************************************/
int tmp_parent(pid_t pid)
{
   int ret;
   register FILE *fp;
   int done=False, bytes, filenumber=0;
   unsigned char *data;
   unsigned int iw, ih, depth;
   char outfile[MAXPATHLEN], ctrlfile[MAXPATHLEN];
   struct header_t header;
   struct stat s;

   for (;;) {
      (void) sprintf(outfile,OUTFILE_FMT,tmpdir,(int)pid,filenumber++);
      (void) strcpy(ctrlfile,outfile);
      (void) strcat(ctrlfile,OUTFILE_EXT);

      /* wait for creation of control file */
      while (access(ctrlfile,F_OK) < 0)
         sleep((unsigned int)1);

      if (verbose)
         (void) printf("\tcontrol file %s exists\n",ctrlfile);

      fp = fopen(outfile,"rb");
      if (fp == (FILE *) NULL) {
         (void) fprintf(stderr,"dpyimage: cannot open %s\n",outfile);
         return(-2);
      }

      unlinkfile(outfile);
      unlinkfile(ctrlfile);

      if (fstat(fileno(fp),&s) < 0) {
         perror(outfile);
         return(-3);
      }
      if (s.st_size == 0)
         break;

      if((ret = readheader(fp,&header)))
         return(ret);

      iw = header.iw;
      ih = header.ih;
      depth = header.depth;
      if (depth == 1)
         bytes = howmany(iw,CHAR_BIT) * ih;
      else if (depth == 8)
         bytes = iw * ih;
      else /* if(depth == 24) */
         bytes = iw * ih * 3;

      data = (unsigned char *) malloc((unsigned int) bytes);
      if (data == (unsigned char *) NULL) {
         (void) fprintf(stderr,"dpyimage: malloc(%d) failed\n", bytes);
         return(-4);
      }

      if (verbose > 2) {
         unsigned int zero, one;

         (void) printf("%s:\n",header.filename);
         (void) printf("\timage size: %u x %u (%d bytes)\n", iw,ih,bytes);
         (void) printf("\tdepth: %u\n",depth);
         pixelcount(data,bytes,&zero,&one);
         (void) printf("\tpixel breakdown: %u zero, %u one\n\n",
                       zero,one);
      }

      if((ret = readdata(fp,data,bytes))){
         free((char *)data);
         return(ret);
      }
      if((ret = fclose(fp))){
         free((char *)data);
         return(ret);
      }
      if((ret = dpyimage(header.filename,data,iw,ih,depth,
                        header.whitepix,header.align,&done))){
         free((char *)data);
         return(ret);
      }
      free((char *)data);

      if (done)
         break;
   } /* for */

   return(0);
}

/*************************************************************/
int tmp_child(int argc, char **argv, pid_t ppid)
{
   int ret;
   char outfile[MAXPATHLEN], ctrlfile[MAXPATHLEN];
   register FILE *fp;
   int align, bpi, bytes, filenumber=0;
   unsigned char *data;
   unsigned int iw, ih, depth, whitepix;
   struct header_t header;
   extern int optind;

   while ( optind <= argc ) {

      (void) sprintf(outfile,OUTFILE_FMT,tmpdir,(int)ppid,filenumber++);
      (void) strcpy(ctrlfile,outfile);
      (void) strcat(ctrlfile,OUTFILE_EXT);

      fp = fopen(outfile,"wb");
      if (fp == (FILE *) NULL) {
         (void) fprintf(stderr,"dpyimage: cannot open %s\n",outfile);
         return(-2);
      }

      if (optind != argc) {
         if((ret = readfile(argv[optind],&data,&bpi,&iw,&ih,&depth,
                           &whitepix,&align)))
            return(ret);
         buildheader(&header,argv[optind],iw,ih,depth,whitepix,align);
         if((ret = writeheader(fp,&header))){
            free((char *) data);
            return(ret);
         }
         if (depth == 1)
            bytes = howmany(iw,CHAR_BIT) * ih;
         else if (depth == 8)
            bytes = iw * ih;
         else /* if (depth == 24) */
            bytes = iw * ih * 3;

         if (verbose)
            (void) printf("(child) %d bytes\n",bytes);
         if((ret = writedata(fp,data,bytes))){
            free((char *) data);
            return(ret);
         }
      } /* If */

      if (optind != argc)
         free((char *) data);
      if((ret = fclose(fp)))
         return(ret);
      if((ret = createfile(ctrlfile)))
         return(ret);

      optind++;
   } /* for */

   return(0);
}
