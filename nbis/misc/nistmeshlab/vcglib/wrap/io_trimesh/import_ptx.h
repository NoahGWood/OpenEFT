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
/****************************************************************************
History
$Log: not supported by cvs2svn $
Revision 1.6  2006/11/28 11:36:25  cignoni
Removed nonstandard  include io.h

Revision 1.5  2006/11/21 22:34:58  cignoni
small gcc compiling issues

Revision 1.4  2006/11/21 19:23:50  e_cerisoli
Added comments for documentation

****************************************************************************/

#ifndef __VCGLIB_IMPORT_PTX
#define __VCGLIB_IMPORT_PTX

#include <stdio.h>
#include <wrap/callback.h>
#include <vcg/complex/trimesh/allocate.h>
#include <vcg/complex/trimesh/clean.h>
#include <vcg/complex/trimesh/update/normal.h>
#include <vcg/complex/trimesh/update/bounding.h>

namespace vcg {
namespace tri {
namespace io {
	/** 
	This class encapsulate a filter for importing ptx meshes.
	*/
	template <class OpenMeshType>
	class ImporterPTX
	{
	public:
		typedef typename OpenMeshType::VertexPointer VertexPointer;
		typedef typename OpenMeshType::ScalarType ScalarType;
		typedef typename OpenMeshType::VertexType VertexType;
		typedef typename OpenMeshType::FaceType FaceType;
		typedef typename OpenMeshType::VertexIterator VertexIterator;
		typedef typename OpenMeshType::FaceIterator FaceIterator;
		
		class Info		//ptx file info
		{
		public:

			Info()
			{
				mask				= 0;
				meshnum			= 0;
				anglecull		= true;
				angle				= 89;
				savecolor		= true;
				pointcull		= true;
				pointsonly	= false;
				switchside	= false;
				flipfaces		= false;
			}

			/// a bit mask describing the field preesnt in the ply file
			int mask;  

			/// index of mesh to be imported
			int meshnum;

			/// if true use angle cull
			bool anglecull;
			/// culling angle, if angle culling is selected
			float angle;

			/// if true, remove invalid points
			bool pointcull;

			/// if true, only keeps points
			bool pointsonly;

			/// if true, color if saved. if no color is present, reflectancy is used instead
			bool savecolor;

			/// switch row-columns
			bool switchside;
			/// flip faces
			bool flipfaces;

		}; // end ptx file info class


		/// Standard call for knowing the meaning of an error code
		static const char *ErrorMsg(int error)
		{
			static const char * ptx_error_msg[] =
			{
				"No errors",
				"Can't open file",
				"Header not found",
				"Eof in header",
				"Format not found",
				"Syntax error on header",
			};
			if(error>6 || error<0) return "Unknown error";
			else return ptx_error_msg[error];
		};

		/// skip ONE range map inside the ptx file, starting from current position
		/// returns true if skipped, false if failed/eof
		static bool skipmesh(FILE* fp, CallBackPos *cb=NULL)
		{
			int colnum;
			int rownum;
			int skiplines;
			char linebuf;

			if(feof(fp))	return false;

			// getting mesh size;
			fscanf(fp,"%i\n",&colnum);
			fscanf(fp,"%i\n",&rownum);

			if ( ( colnum <=0 ) || ( rownum <=0 ) ) return false;
			if(feof(fp))	return false;

			// have to skip (col * row) lines plus 8 lines for the header
			skiplines = (colnum * rownum) + 8; 
			for(int ii=0; ii<skiplines; ii++)
			{
				fread(&linebuf,1,1,fp);
				while(linebuf != '\n')  fread(&linebuf,1,1,fp);
			} 

			if(cb) cb( 100, "Skipped preamble");
			return true;
		}

		///Standard call that reading a mesh
		static int Open( OpenMeshType &m, const char * filename, Info importparams, CallBackPos *cb=NULL)
		{
			FILE *fp;
			fp = fopen(filename, "rb");
			if(fp == NULL) return false;
			m.Clear();
			m.vn=0;
			m.fn=0;

			// if not exporting first one, skip meshes until desired one
			if (importparams.meshnum>0) 
				for (int i=0; i!=importparams.meshnum; ++i)  
					if(!skipmesh(fp, cb))
						return 1;

			if (!readPTX( m, fp, importparams, cb))
			{
				m.Clear();
				return 1;
			}

			return 0;
		}

		///Call that load a mesh
		static bool readPTX( OpenMeshType &m, FILE *fp, Info importparams, CallBackPos *cb=NULL)
		{
			int numtokens;
			int colnum;
			int rownum;
			float xx,yy,zz;		// position
			float rr,gg,bb;		// color
			float rf;					// reflectance
			Matrix44f currtrasf;

			bool hascolor;
			bool savecolor   =  importparams.savecolor &&  VertexType::HasColor();
			bool onlypoints  =  importparams.pointsonly;
			bool switchside  =  importparams.switchside;
			bool flipfaces   =  importparams.flipfaces;

			int total = 50;
			if (onlypoints) total = 100;  
			char linebuf[256];
			
			fscanf(fp,"%i\n",&colnum);					
			fscanf(fp,"%i\n",&rownum);
			
			if ( ( colnum <=0 ) || ( rownum <=0 ) ) return false;
			// initial 4 lines [still don't know what is this :) :)]
			if ( !fscanf(fp,"%f %f %f\n", &xx, &yy, &zz) ) return false;
			if ( !fscanf(fp,"%f %f %f\n", &xx, &yy, &zz) ) return false;
			if ( !fscanf(fp,"%f %f %f\n", &xx, &yy, &zz) ) return false;
			if ( !fscanf(fp,"%f %f %f\n", &xx, &yy, &zz) ) return false;
									// now the transformation matrix
			if ( !fscanf(fp,"%f %f %f %f\n", &(currtrasf.ElementAt(0,0)), &(currtrasf.ElementAt(0,1)), &(currtrasf.ElementAt(0,2)), &(currtrasf.ElementAt(0,3))) )return false;
			if ( !fscanf(fp,"%f %f %f %f\n", &(currtrasf.ElementAt(1,0)), &(currtrasf.ElementAt(1,1)), &(currtrasf.ElementAt(1,2)), &(currtrasf.ElementAt(1,3))) )return false;
			if ( !fscanf(fp,"%f %f %f %f\n", &(currtrasf.ElementAt(2,0)), &(currtrasf.ElementAt(2,1)), &(currtrasf.ElementAt(2,2)), &(currtrasf.ElementAt(2,3))) )return false;
			if ( !fscanf(fp,"%f %f %f %f\n", &(currtrasf.ElementAt(3,0)), &(currtrasf.ElementAt(3,1)), &(currtrasf.ElementAt(3,2)), &(currtrasf.ElementAt(3,3))) )return false;
			    	
			//now the real data begins
			// first line, we should know if the format is
			// XX YY ZZ RF
			// or it is
			// XX YY ZZ RF RR GG BB
			// read the entire first line and then count the spaces. it's rude but it works :)
			int ii=0;
			fread(&(linebuf[ii++]),1,1,fp);
			while(linebuf[ii-1] != '\n')  if ( fread(&(linebuf[ii++]),1,1,fp)==0 ) return false;
			linebuf[ii-1] = '\0'; // terminate the string
			numtokens=1;
			for(ii=0; ii<(int)strlen(linebuf); ii++) if(linebuf[ii] == ' ') numtokens++;
			if(numtokens == 4)  hascolor = false;
			else if(numtokens == 7)  hascolor = true;
			else  return false;
			
			// PTX transformation matrix is transposed
			currtrasf.transposeInPlace();
			
			// allocating vertex space
			int vn = rownum*colnum;
			VertexIterator vi = Allocator<OpenMeshType>::AddVertices(m,vn); 
			m.vn = vn;
			m.bbox.SetNull();

			// parse the first line....
			if(hascolor)
			{
				printf("\n hascolor ");
				sscanf(linebuf,"%f %f %f %f %f %f %f", &xx, &yy, &zz, &rf, &rr, &gg, &bb);
			}
			else
			{
				printf("\n no color ");
				sscanf(linebuf,"%f %f %f %f", &xx, &yy, &zz, &rf);
			}
			
			//addthefirstpoint
			(*vi).P()[0]=xx;
			(*vi).P()[1]=yy;
			(*vi).P()[2]=zz;
			// updating bbox
			m.bbox.Add( (*vi).P() );

			if(VertexType::HasQuality())
			{
				(*vi).Q()=rf;
			}

			if(hascolor && savecolor)
			{
				(*vi).C()[0]=rr;
				(*vi).C()[1]=gg;
				(*vi).C()[2]=bb;
			}
			else if(!hascolor && savecolor)
			{
				(*vi).C()[0]=rf*255;
				(*vi).C()[1]=rf*255;
				(*vi).C()[2]=rf*255;
			}
			vi++;

			// now for each line until end of mesh (row*col)-1
			for(ii=0; ii<((rownum*colnum)-1); ii++)
			{
				char tmp[255];
				sprintf(tmp, "PTX Mesh Loading...");
				if(cb) cb((ii*total)/vn, tmp);	

				// read the stream
				if(hascolor)   
					fscanf(fp,"%f %f %f %f %f %f %f", &xx, &yy, &zz, &rf, &rr, &gg, &bb);
				else  
					fscanf(fp,"%f %f %f %f", &xx, &yy, &zz, &rf);

				// add the point
				(*vi).P()[0]=xx;
				(*vi).P()[1]=yy;
				(*vi).P()[2]=zz;
					
				m.bbox.Add( (*vi).P() );

				if(VertexType::HasQuality())
				{
					(*vi).Q()=rf;
				}

				if(hascolor && savecolor)
				{
					(*vi).C()[0]=rr;
					(*vi).C()[1]=gg;
					(*vi).C()[2]=bb;
				}
				else if(!hascolor && savecolor)
				{
					(*vi).C()[0]=rf*255;
					(*vi).C()[1]=rf*255;
					(*vi).C()[2]=rf*255;
				}

				vi++;
			}

			if(! onlypoints)
			{
				// now i can triangulate
				int trinum = (rownum-1) * (colnum-1) * 2;
				typename OpenMeshType::FaceIterator fi= Allocator<OpenMeshType>::AddFaces(m,trinum);
				m.fn = trinum;
				int v0i,v1i,v2i, t;
				t=0;
				for(int rit=0; rit<rownum-1; rit++)
					for(int cit=0; cit<colnum-1; cit++)
					{
						t++;
						if(cb) cb(50 + (t*50)/(rownum*colnum),"PTX Mesh Loading");	

						if(!switchside)
						{
							v0i = (rit  ) + ((cit  ) * rownum);
							v1i = (rit+1) + ((cit  ) * rownum);
							v2i = (rit  ) + ((cit+1) * rownum);
						}
						else
						{
							v0i = (cit  ) + ((rit  ) * colnum);
							v1i = (cit+1) + ((rit  ) * colnum);
							v2i = (cit  ) + ((rit+1) * colnum);
						}
						
						// upper tri
						(*fi).V(2) = &(m.vert[v0i]);
						(*fi).V(1) = &(m.vert[v1i]);
						(*fi).V(0) = &(m.vert[v2i]);
						if(flipfaces)
						{
							(*fi).V(2) = &(m.vert[v1i]);
							(*fi).V(1) = &(m.vert[v0i]);
						}

						fi++;
						if(!switchside)
						{
							v0i = (rit+1) + ((cit  ) * rownum);
							v1i = (rit+1) + ((cit+1) * rownum);
							v2i = (rit  ) + ((cit+1) * rownum);
						}
						else
						{
							v0i = (cit+1) + ((rit  ) * colnum);
							v1i = (cit+1) + ((rit+1) * colnum);
							v2i = (cit  ) + ((rit+1) * colnum);
						}
						// lower tri
						(*fi).V(2) = &(m.vert[v0i]);
						(*fi).V(1) = &(m.vert[v1i]);
						(*fi).V(0) = &(m.vert[v2i]);
						if(flipfaces)
						{
							(*fi).V(2) = &(m.vert[v1i]);
							(*fi).V(1) = &(m.vert[v0i]);
						}

						fi++;
					}
			}	

			// remove unsampled points
			if(importparams.pointcull)
			{
				if(cb) cb(40,"PTX Mesh Loading - remove invalid vertices");	
				for(VertexIterator vi = m.vert.begin(); vi != m.vert.end(); vi++)
				{
					if((*vi).P() == Point3f(0.0, 0.0, 0.0)) 
						Allocator<OpenMeshType>::DeleteVertex(m,*vi);						
				}

				if(! importparams.pointsonly)
				{
					if(cb) cb(60,"PTX Mesh Loading - remove invalid faces");	
					for(typename OpenMeshType::FaceIterator fi = m.face.begin(); fi != m.face.end(); fi++)
					{
						if( ((*fi).V(0)->IsD()) || ((*fi).V(1)->IsD()) || ((*fi).V(2)->IsD()) )
								Allocator<OpenMeshType>::DeleteFace(m,*fi);						
					}
				}
			}

			// eliminate high angle triangles
			if(! importparams.pointsonly)
			{				
				if(importparams.anglecull)
				{
					float limit = cos( double(importparams.angle)*3.14159265358979323846/180.0 );
					Point3f raggio;

					if(cb) cb(85,"PTX Mesh Loading - remove steep faces");	
					vcg::tri::UpdateNormals<OpenMeshType>::PerFaceNormalized(m);
					for(typename OpenMeshType::FaceIterator fi = m.face.begin(); fi != m.face.end(); fi++)
						if(!(*fi).IsD())
						{
							raggio = -((*fi).V(0)->P() + (*fi).V(1)->P() + (*fi).V(2)->P()) / 3.0;
							raggio.Normalize();
							if((raggio.dot((*fi).N())) < limit)
									Allocator<OpenMeshType>::DeleteFace(m,*fi);	
							
						}
				}
			}

			for(typename OpenMeshType::VertexIterator vi = m.vert.begin(); vi != m.vert.end(); vi++)
			{
				if(!(*vi).IsD())
					(*vi).P() = currtrasf * (*vi).P();
			}

			// deleting unreferenced vertices
			vcg::tri::Clean<OpenMeshType>::RemoveUnreferencedVertex(m);
			vcg::tri::UpdateNormals<OpenMeshType>::PerFaceNormalized(m);
			vcg::tri::UpdateBounding<CMeshO>::Box(m);
			if(cb) cb(100,"PTX Mesh Loading finish!");
			return true;
		}
	
	}; // end class

} // end Namespace tri
} // end Namespace io
} // end Namespace vcg
#endif