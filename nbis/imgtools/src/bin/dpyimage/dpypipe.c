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

      FILE:    DPYPIPE.C

      AUTHORS: Stan Janet
               Michael D. Garris
      DATE:    12/03/1990
      UPDATED: 05/10/2005 by MDG

      ROUTINES:
               pipecomm()
               pipe_parent()
               pipe_child()

***********************************************************************/

#include <usebsd.h>
#include <stdlib.h>
#include <limits.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <dpyimage.h>

extern FILE *fdopen (int, const char *);

/****************************************************************/
int pipecomm(int argc, char **argv)
{
   int ret, display_fd;
   register FILE *fp;
   int pid, cmdfd[2], datafd[2];
   int status, sig;

   if (verbose)
      (void) printf("In pipecomm()\n");

   if ((pipe(cmdfd) < 0) || (pipe(datafd) < 0)) {
      perror("Pipe failed");
      return(-2);
   }

   display_fd = ConnectionNumber(display);

   pid = fork();
   if (pid < 0) {
      perror("Fork failed");
      return(-3);
   }

   if (pid) {
      /* parent */

      if((ret = fdclose(cmdfd[0],"pipe")))
         return(ret);
      if((ret = fdclose(datafd[1],"pipe")))
         return(ret);
      fp = fdopen(datafd[0],"r");
      if (fp == (FILE *) NULL) {
         (void) fprintf(stderr,"%s: fdopen failed\n",program);
         return(-4);
      }
      if((ret = pipe_parent(fp)))
         return(ret);
        
      (void) fclose(fp);

      if(waitpid(pid, &status, 0) < 0){
         perror("Waitpid failed");
         return(-5);
      }

      if(WIFEXITED(status) == 0){
         if(WIFSIGNALED(status)){
            sig = WTERMSIG(status);
            fprintf(stderr, "Child pid = %d died with Signal %d\n",
                    pid, sig);
            return(-6);
         }
      }

/*
      (void) kill(pid,SIGKILL);
*/
   }
   else {
      /* child */

      extern int nicevalue;
      extern char *program;

      program = "[child]";
      if((nicevalue >= 0) && (nice(nicevalue) < 0))
         perror("Nice failed");
      if((ret = fdclose(0,"stdin")))
         return(-4);
      if(open("/dev/null",O_RDWR) < 0) {
         perror("Cannot open /dev/null");
         return(-7);
      }
      if((ret = fdclose(display_fd,"X11 connection")))
         return(ret);
      if((ret = fdclose(cmdfd[1],"pipe")))
         return(ret);
      if((ret = fdclose(datafd[0],"pipe")))
         return(ret);
      fp = (FILE *)fdopen(datafd[1],"w");
      if(fp == (FILE *) NULL) {
         (void) fprintf(stderr,"%s: fdopen failed\n",program);
         return(-8);
      }

      /* Note that child process MUST exit and not return a code */
      if((ret = pipe_child(argc,argv,fp)))
         exit(ret);
      (void) fclose(fp);
      exit(0);
   }

   return(0);
}

/****************************************************************/
int pipe_parent(register FILE *fp)
{
   int ret;
   int done=False, n, bytes;
   unsigned int iw, ih, depth;
   unsigned char *data;
   struct header_t header;

   while (! done) {
      n = fread((char *)&header,1,HEADERSIZE,fp);
      if (n == 0)
         break;
      if (n != HEADERSIZE) {
         (void) fprintf(stderr,
                        "%s: header fread returned %d, expected %d\n",
                        program,n,HEADERSIZE);
         return(-2);
      }

      iw = header.iw;
      ih = header.ih;
      depth = header.depth;
      if (depth == 1)
         bytes = howmany(iw,CHAR_BIT) * ih;
      else if (depth == 8)
         bytes = iw * ih;
      else /* if (depth == 24) */
         bytes = iw * ih * 3;

      if (verbose) {
         (void) printf("%s:\n",header.filename);
         (void) printf("\timage size: %u x %u (%d bytes)\n",
                       iw,ih,bytes);
         (void) printf("\tdepth: %u\n",depth);
      }

      data = (unsigned char *) malloc((unsigned int) bytes);
      if (data == (unsigned char *) NULL) {
         (void) fprintf(stderr,"%s: malloc(%d) failed\n",
                        program,bytes);
         return(-3);
      }

      if((ret = readdata(fp,data,bytes)))
         return(ret);

      if (verbose > 2) {
         unsigned int zero, one;
         pixelcount(data,bytes,&zero,&one);
         (void) printf("\tpixel breakdown: %u zero, %u one\n\n",
                       zero,one);
      }

      if((ret = dpyimage(header.filename,data,iw,ih,depth,
                        header.whitepix,header.align,&done)))
         return(ret);

      free((char *) data);
   } /* While */

   return(0);
}

/*******************************************************************/
int pipe_child(int argc, char **argv, register FILE *fp)
{
   int ret;
   int done = False, align, bpi, bytes;
   unsigned int iw, ih, depth, whitepix;
   unsigned char *data;
   extern int optind;
   struct header_t header;

   while ( !done && (optind < argc)) {
      if((ret = readfile(argv[optind],&data,&bpi,&iw,&ih,
                        &depth,&whitepix,&align)))
         return(ret);

      buildheader(&header,argv[optind],iw,ih,depth,whitepix,align);
      if((ret = writeheader(fp,&header)))
         return(ret);
      if (depth == 1)
         bytes = howmany(iw,CHAR_BIT) * ih;
      else if(depth == 8)
         bytes = iw * ih;
      else /* if(depth == 24) */
         bytes = iw * ih * 3;

      if (verbose)
         (void) printf("(child) %d bytes\n",bytes);
      if((ret = writedata(fp,data,bytes)))
         return(ret);
      (void) fflush(fp);
      free((char *) data);
      optind++;
   } /* While */

   /* Exit successfully */
   return(0);
}

