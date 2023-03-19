/****************************************************************************
* VCGLib                                                            o o     *
* Visual and Computer Graphics Library                            o     o   *
*                                                                _   O  _   *
* Copyright(C) 2004                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/

/* This code has been adapted from the OFF importer and Gael's .pts loader from Expe */

/******************************************
* This file is modified by NIST - 06/2011 *
******************************************/

#ifndef __VCGLIB_IMPORT_NISTXYZ
#define __VCGLIB_IMPORT_NISTXYZ

#include <fstream>
#include <string>
#include <vector>
#include <cassert>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <vcg/space/color4.h>
#include <vcg/complex/trimesh/allocate.h>
#include <wrap/callback.h>
#include <wrap/io_trimesh/io_mask.h>
#include "../../common/nistLogo.h"
#include <stdio.h>


namespace vcg
{
namespace tri
{
namespace io
{

// /** \addtogroup  */
// /* @{ */
/**
This class encapsulate a filter for importing Expes's .pts point sets.
*/
template<class MESH_TYPE>
class ImporterNistXYZ
{
  public:

    typedef typename MESH_TYPE::VertexType      VertexType;
    typedef typename MESH_TYPE::VertexIterator  VertexIterator;
    typedef typename MESH_TYPE::VertexPointer    VertexPointer;
    typedef typename MESH_TYPE::CoordType        CoordType;
    typedef typename MESH_TYPE::ScalarType      ScalarType;

    enum ExpeCodes {NoError=0, CantOpen, InvalidFile, UnsupportedVersion};
 
    struct Options
    {
      Options()
        : onlyMaskFlag(false)
      {}
      bool onlyMaskFlag;
    };

    struct FileProperty
    {
      FileProperty(QByteArray _name, uint _size)
        : name(_name), size(_size)
      {}
      QByteArray name;    // name of the property
      uint size;          // size in byte (binary mode)
      bool hasProperty;    // true if the target mesh has the property
    };
    typedef std::vector<FileProperty> FileProperties;

   

    /*!
    *  Standard call for knowing the meaning of an error code
    * \param message_code  The code returned by <CODE>Open</CODE>
    *  \return              The string describing the error code
    */
    static const char* ErrorMsg(int message_code)
    {
      static const char* error_msg[] =
      {
        "No errors", "Can't open file", "Invalid file", "Unsupported version"
      };

      if(message_code>4 || message_code<0)
        return "Unknown error";
      else
        return error_msg[message_code];
    };

    /**
      * Load only the properties of the 3D objects.
      *
      * \param filename    the name of the file to read from
      * \param loadmask    the mask which encodes the properties
      * \return            the operation result
      */
    static bool LoadMask(const char *filename, int &loadmask)
    {
      // To obtain the loading mask all the file must be parsed
      // to distinguish between per-vertex and per-face color attribute.
      loadmask=0;
      MESH_TYPE dummyMesh;
      MeshModel dummym;
      return (Open(dummyMesh, dummym, filename, loadmask,0,true)==NoError);
    }

    static int Open(MESH_TYPE &mesh, const char *filename, CallBackPos *cb=0)
    {
      int loadmask;
      return Open(mesh,filename,loadmask,cb);
    }

    /*!
      *  Standard call for reading a mesh.
      *
      *  \param mesh         the destination mesh
      *  \param filename     the name of the file to read from
      *  \return             the operation result
      */
    static int Open(MESH_TYPE &mesh, MeshModel &m, const char *filename, int &loadmask,
      CallBackPos *cb=0, bool onlyMaskFlag=false)
    {
      Options opt;
      opt.onlyMaskFlag = onlyMaskFlag;
      return Open(mesh, m, filename, loadmask, opt, cb);
    }

    static int Open(MESH_TYPE &mesh, MeshModel &m, const char *filename, int &loadmask,
      const Options& options, CallBackPos *cb=0)
    {
      QFile device(filename);
      if ((!device.open(QFile::ReadOnly)))
        return CantOpen;
      QTextStream stream(&device);

      loadmask = 0;

      QString buf;
      QStringList line;
      QString::iterator i;
      std::vector<CoordType> pos;
      std::vector<CMeshO::VertexType::ColorType> colors;
      CoordType p;
      CMeshO::VertexType::ColorType c;
      int status;

      loadmask |= Mask::IOM_VERTCOORD;
      loadmask |= Mask::IOM_VERTCOLOR;

      while(!stream.atEnd())
      {
         status = 0;
         line = (buf = stream.readLine(1024)).split(QRegExp("[ |\t]"));

         for(i = buf.begin(); i != buf.end(); ++i)
         {
             if (*i == '#')
             {
                m.headerList.append(line[0]);
                status == 1;
             }
         }	
         if (status == 0)
         {
            if (line.size() == 6)
            {
               for (int k=0; k<3; ++k)
               {
                  p[k] = line[k].toDouble();
                  c[k] = line[3+k].toInt();
               }
               pos.push_back(p);
               colors.push_back(c);
            }
            else if (line.size() == 4)
            {
               for (int k=0; k<3; ++k)
               {
                  p[k] = line[k].toDouble();
                  c[k] = line[3].toInt();
               }
               pos.push_back(p);
               colors.push_back(c);
            }
            else if (line.size() == 3)
            {
               for (int k=0; k<3; ++k)
               {
                  p[k] = line[k].toDouble();
                  c[k] = 255;
               }
               pos.push_back(p);
               colors.push_back(c);
            }    
         }
      }
      device.close();
/*
      // Add NIST Logo
      int count, k, j = 0;
      for (int i=0; i<nistLogoCoordSize; i++)
      {
         for (k=0; k<3; k++)
         {
            p[k] = nistLogo[count+k];
         }
         count = count + k;

         for (j=0; j<3; j++)
         {  
            c[j] = nistLogo[count];
         }
         count++;

         pos.push_back(p);
         colors.push_back(c);
       }
*/
       VertexIterator v_iter = Allocator<MESH_TYPE>::AddVertices(mesh,pos.size());
       for (int i=0; i<pos.size(); ++i)
       {
          v_iter->P() = pos[i];
          v_iter->C() = colors[i];
          ++v_iter;
       }

       return 0;
    } // end Open

    static int Save(MeshModel &m, const char * filename, CallBackPos *cb=0)
    {
       FILE * fpout;
       VertexPointer  vp;
       VertexIterator vi;
       double x, y, z;
       int j, r, g, b;
       double xMin, xMax, yMin, yMax, w, h;
       int index = 9;
       int grayScale = 0, status = 0, vertSize = 0;
       QList<double> xlist;
       QList<double> ylist;

       // Write all the information to output file
       fpout = fopen(filename,"w");
       if(fpout==NULL)
       {
          return 1;
       }       
       else
       {  
          if (m.headerList.size() == index)
          {
             status = 1;
             // Set grayScale to save the new header
             if (m.headerList[1].compare("#xyz") == 0)
             {
                grayScale = 0;
             }
             else if (m.headerList[1].compare("#xyz_g") == 0)
             {
                grayScale = 1;
             }
             else if (m.headerList[1].compare("#xyz_rgb") == 0)
             {
                grayScale = 2;
             }
             else
             {
                grayScale = 3;
             }
          }

          // Get the width and height for the active vert
          xMin = 0;
          xMax = 0;
          yMin = 0;
          yMax = 0;

          for(j=0,vi=m.cm.vert.begin();vi!=m.cm.vert.end();++vi)
          {
             vp=&(*vi);
             //if (j < (m.cm.vert.size() - nistLogoCoordSize))
             //{
                if( ! vp->IsD() )
                {
                   x = double(vp->P()[0]);
                   xlist.append(x);
                   y = double(vp->P()[1]);
                   ylist.append(y);
                }
             //}         
             j++;                                 
          }
          qSort(xlist.begin(), xlist.end());
          qSort(ylist.begin(), ylist.end());

          if (!xlist.isEmpty() && !ylist.isEmpty())
          {
             xMin = xlist.first();
             xMax = xlist.last();
             yMin = ylist.first();
             yMax = ylist.last();   

             w = sqrt((pow((xMax-xMin), 2))+(pow((yMax-yMax), 2))) + 1;
             h = sqrt((pow((xMin-xMin), 2))+(pow((yMin-yMax), 2))) + 1;
          }
          else
          {
             w = 0;
             h = 0;
          }

          // Set file header information
          if (status == 1)
          {
             fprintf(fpout, "#Created by MeshLab (NIST modified version) - Verson 1.0\n");

//             QByteArray q_str0 = m.headerList[0].toLocal8Bit();
//             const char * c_str0 = q_str0.data();
//             fprintf(fpout, "%s\n", c_str0);
 
             QByteArray q_str1 = m.headerList[1].toLocal8Bit();
             const char * c_str1 = q_str1.data();
             fprintf(fpout, "%s\n", c_str1);

             QByteArray q_str2 = m.headerList[2].toLocal8Bit();
             const char * c_str2 = q_str2.data();
             fprintf(fpout, "%s\n", c_str2);

             QByteArray q_str3 = m.headerList[3].toLocal8Bit();
             const char * c_str3 = q_str3.data();
             fprintf(fpout, "%s\n", c_str3);

             QByteArray q_str4 = m.headerList[4].toLocal8Bit();
             const char * c_str4 = q_str4.data();
             fprintf(fpout, "%s\n", c_str4);

             fprintf(fpout, "#%g\n", w);

             fprintf(fpout, "#%g\n", h);

             QByteArray q_str5 = m.headerList[5].toLocal8Bit();
             const char * c_str5 = q_str5.data();
             fprintf(fpout, "%s\n", c_str5);

             QByteArray q_str6 = m.headerList[6].toLocal8Bit();
             const char * c_str6 = q_str6.data();
             fprintf(fpout, "%s\n", c_str6);
          }
          else
          {
             fprintf(fpout, "#Created by MeshLab (NIST modified version) - Verson 1.0\n");
             fprintf(fpout, "%s\n", "#xyz");
             fprintf(fpout, "%s\n", "#N/A");
             fprintf(fpout, "%s\n", "#N/A");
             fprintf(fpout, "%s\n", "#N/A");
             fprintf(fpout, "#%g\n", w);
             fprintf(fpout, "#%g\n", h);
             fprintf(fpout, "%s\n", "#N/A");
             fprintf(fpout, "%s\n", "#N/A");
          }


          // Write vertex to file   
          for(j=0,vi=m.cm.vert.begin();vi!=m.cm.vert.end();++vi)
          {
             vp=&(*vi);
             //if (j < (m.cm.vert.size() - nistLogoCoordSize))
             //{
                if( ! vp->IsD() )
                {
                   x = double(vp->P()[0]);
                   y = double(vp->P()[1]);
                   z = double(vp->P()[2]);
                   r = g= vp->C()[0];
                   g = vp->C()[1];
                   b = vp->C()[2];
                   if (grayScale == 0 || grayScale == 3)
                   {
                      fprintf(fpout, "%g %g %g\n", x, y, z); 
                   } 
                   else if (grayScale == 1)
                   {
                      fprintf(fpout, "%g %g %g %d\n", x, y, z, g);
                   }
                   else if (grayScale == 2)
                   {
                      fprintf(fpout, "%g %g %g %d %d %d\n", x, y, z, r, g, b);
                   }
                }
             //}   
             j++;                                            
          }
          fclose(fpout);
          return 0;
       }
   }
   protected:

};
// /*! @} */

} //namespace io
} //namespace tri
} // namespace vcg

#endif //__VCGLIB_IMPORT_NISTXYZ
