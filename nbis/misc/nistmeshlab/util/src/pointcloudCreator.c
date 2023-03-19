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

/************************************************************************

      PACKAGE:  IMAGE ENCODER/DECODER TOOLS

      FILE:     pointcloudCreator.c

      AUTHORS:  Kenneth Ko

      DATE:     06/16/2011

      pointcloudCreator - This is an implemention to generate 2D fingerprint
                          to a 3D from a RAW grayscale image file.

*************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
   FILE *infile, *outfile;
   int fileLen;
   uint8_t *buffer;
   float grayRatio, z;
   int g_flag, bytes_read, res_w, res_h;
   int i, r, c;

   if (argc != 6)
   {
      printf("Usage: %s <input file> <output file> width height [0|1]\n", 
             argv[0]);
      printf("           infile      Read RAW gray image file to be read.\n");
      printf("           outfile     Write the XYZ formated file to be written.\n"); 
      printf("           width       The width of the RAW image.\n");
      printf("           height      The height of the RAW image.\n");
      printf("           gray        With grayscale value in the XYZ(Yes/No)\n");
      printf("                       Yes: Set to 1\n");
      printf("                       No : Set to 0\n");  
      return -1;
   }
   
   /* Set widt and heigth */
   res_w = atoi(argv[3]);
   res_h = atoi(argv[4]);
   g_flag = atoi(argv[5]);

   // We assume argv[1] is a filename to open
   infile = fopen(argv[1], "rb");
   fileLen = 0;

   /* fopen returns 0, the NULL pointer, on failure */
   if (infile == 0)
   {
       printf("Could not open file\n");
       fclose(infile);
       return -2;
   }
   else 
   {
      /* Get file size */
      fseek(infile, 0, SEEK_END); 
      fileLen = ftell(infile); 
      fseek(infile, 0, SEEK_SET);

      if (fileLen == 0)
      {
         printf("File contain zero byte\n");
         fclose(infile);
         return -3;
      }
      else if (fileLen != (res_w * res_h))
      {
         printf("File size not equal to user input width and height\n");
         fclose(infile);
         return -4;
      }
      else
      {
         grayRatio = 3.642857;

         /* allocate buffer for image file */
         buffer = (uint8_t *)malloc(sizeof(uint8_t) * fileLen);

         if (buffer == 0)
         {
            printf("Failed to alocate memory\n");
            fclose(infile);
            return -5;
         }   
         else
         {
            bytes_read = fread(buffer, sizeof(uint8_t), fileLen, infile);
              
            if (bytes_read == 0)
            {
               printf("Can't read file to buffer\n");
               fclose(infile);
               return -6;
            }
            else
            {
               /* Convert 2D to 3D */
               outfile = fopen(argv[2], "w");
               if (outfile == 0)
               {
                  printf("Could not open file\n");
                  fclose(infile);
                  return -7;
               }
               else
               {
                  /* add header format */
                  fprintf(outfile, "#Created by PointClound Creator - Version 1.0\n");
                  if (g_flag == 0)
                  {
                     fprintf(outfile, "#xyz\n");
                  }
                  else if (g_flag == 1)
                  {
                     fprintf(outfile, "#xyz_g\n");
                  }
                  fprintf(outfile, "#N/A\n");
                  fprintf(outfile, "#N/A\n");
                  fprintf(outfile, "#N/A\n");
                  fprintf(outfile, "#%d\n", res_w);
                  fprintf(outfile, "#%d\n", res_h);
                  fprintf(outfile, "#N/A\n");
                  fprintf(outfile, "#N/A\n");
		  
                  /* convert to 3D */
                  i = 0;
                  for (r = 0; r < res_h; r++)
                  {
                     for (c = 0; c < res_w; c++)
                     {
                        z = (buffer[i] * grayRatio) * (0.013);
                        if (g_flag == 0)
                        {
                           fprintf(outfile, "%d %d %f\n", c, r * -1, z);
                        }
                        else if (g_flag == 1)
                        {                 
                           fprintf(outfile, "%d %d %f %d\n", 
                                   c, r * -1 , z, buffer[i]);
                        }
                        i++;
                     }
                  }
               }
            }
         }
         free(buffer);
      }
      fclose(infile);
      fclose(outfile);
   }
   return 0;
}

