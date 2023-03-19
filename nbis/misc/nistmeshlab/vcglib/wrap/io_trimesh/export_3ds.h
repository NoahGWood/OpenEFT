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
 Revision 1.7  2007/03/12 16:40:16  tarini
 Texture coord name change!  "TCoord" and "Texture" are BAD. "TexCoord" is GOOD.

 Revision 1.6  2006/11/30 22:48:06  cignoni
 Corrected bug in exporting mesh with deleted vertices

 Revision 1.5  2006/11/08 15:48:50  cignoni
 Corrected management of capabilities and masks

 Revision 1.4  2006/03/29 09:25:49  zifnab1974
 extra includes necessary for compilation of meshlab on AMD 64 with gcc 3.4.5

 Revision 1.3  2006/03/07 13:20:40  cignoni
 changed include of io_material

 Revision 1.2  2006/02/17 23:53:36  fmazzant
 added relative Path of the Lib3ds library

 Revision 1.1  2006/02/16 19:28:36  fmazzant
 transfer of Export_3ds.h, Export_obj.h, Io_3ds_obj_material.h from Meshlab to vcg

 Revision 1.14  2006/02/06 11:26:26  fmazzant
 bug-fix comment

 Revision 1.13  2006/02/06 11:04:39  fmazzant
 added file material.h. it include struct Material, CreateNewMaterial(...) and MaterialsCompare(...)

 Revision 1.12  2006/02/04 10:18:46  fmazzant
 clean code

 Revision 1.11  2006/02/03 12:36:53  fmazzant
 not significant updating

 Revision 1.10  2006/02/03 10:04:41  fmazzant
 no significant updated

 Revision 1.9  2006/02/02 16:08:27  fmazzant
  updated comment code

 Revision 1.8  2006/02/02 16:05:18  fmazzant
 deleted bug when saving mesh without textures.

 Revision 1.7  2006/02/02 15:35:15  fmazzant
 updated comment code

 Revision 1.6  2006/02/02 13:08:10  fmazzant
 cleaned & commented[italian] code

 Revision 1.5  2006/02/02 10:50:46  fmazzant
 deleted a big bug of exporter 3ds

 Revision 1.4  2006/01/31 09:34:30  fmazzant
 bug-fix on savemaskexporter, when press cancel returns -1.

 Revision 1.3  2006/01/29 23:52:43  fmazzant
 correct a small bug

 Revision 1.2  2006/01/29 18:33:42  fmazzant
 added some comment to the code

 Revision 1.1  2006/01/29 16:33:03  fmazzant
 moved export_obj and export_3ds from test/io into meshio/

 Revision 1.30  2006/01/26 15:57:20  fmazzant
 deleted a small bug

 Revision 1.29  2006/01/26 15:48:49  fmazzant
 added control on the maximum number of vertices allowed by the 3DS file format

 Revision 1.28  2006/01/23 14:07:39  fmazzant
 deleted bug when saving a face color.

 Revision 1.27  2006/01/22 23:59:01  fmazzant
 changed default value of diffuse. 1.0 -> 0.8

 Revision 1.26  2006/01/22 10:42:18  fmazzant
 cleaned code & optimized code of material's name 3ds

 Revision 1.25  2006/01/22 01:26:52  fmazzant
 deleted bug on saving name material 3ds

 Revision 1.24  2006/01/21 15:19:51  fmazzant
 changed:
 inserting coord texture in to map from key = index, value=coord
 to key=coord, value=index_of_vertix.

 Revision 1.23  2006/01/20 14:15:52  fmazzant
 added texture filename on material 3ds and coordtexture on face

 Revision 1.22  2006/01/19 09:36:29  fmazzant
 cleaned up history log

 Revision 1.21  2006/01/18 16:14:52  fmazzant
 update small error

 Revision 1.20  2006/01/18 16:12:58  fmazzant
 updated material

 Revision 1.19  2006/01/18 15:09:45  fmazzant
 added texture base

 Revision 1.18  2006/01/18 14:57:26  fmazzant
 added Lib3dsNode in export_3ds

****************************************************************************/

#ifndef __VCGLIB_EXPORT_3DS
#define __VCGLIB_EXPORT_3DS

#include <map>
#include <wrap/callback.h>
#include <vcg/complex/trimesh/allocate.h>
#include <wrap/io_trimesh/io_mask.h>

#include "io_material.h"

/*
	3DS export needs the Lib3ds library.

	lib3ds is a free ANSI-C library for working with the popular "3ds" 3D model format.

	Supported platforms include GNU (autoconf, automake, libtool, make, GCC) on Unix and 
	Cygwin, and MS Visual C++ 6.0. lib3ds loads and saves Atmosphere settings, Background 
	settings, Shadow map settings, Viewport setting, Materials, Cameras, Lights, Meshes, 
	Hierarchy, Animation keyframes. It also contains useful matrix, vector and quaternion 
	mathematics tools. lib3ds usually integrates well with OpenGL. In addition, some 
	diagnostic and conversion tools are included. 
	
	lib3ds is distributed under the terms of the GNU Lesser General Public License.

	this information has been taken by the official site.

	runable from http://lib3ds.sourceforge.net/

*/
#include <lib3ds/file.h>
#include <lib3ds/io.h>
#include <lib3ds/mesh.h>
#include <lib3ds/types.h>
#include <lib3ds/material.h>

#include <vector>
#include <iostream>
#include <fstream>

#include <QString>
#include <QMessageBox>

#define MAX_POLYGONS 65535

namespace vcg {
namespace tri {
namespace io {

	template <class SaveMeshType>
	class Exporter3DS
	{
	public:	
		typedef typename SaveMeshType::FaceIterator FaceIterator;
		typedef typename SaveMeshType::VertexIterator VertexIterator;
		typedef typename SaveMeshType::VertexType VertexType;

		//int: old index vertex
		//TexCoord2: tex coord with vertex's index i
		typedef std::pair<int,vcg::TexCoord2<float> > Key;
	
		/*
			enum of all the types of error
		*/
		enum SaveError
		{
			E_NOERROR,					// 0
			E_CANTOPENFILE,				// 1
			E_CANTCLOSEFILE,			// 2
			E_UNESPECTEDEOF,			// 3
			E_ABORTED,					// 4
			E_NOTDEFINITION,			// 5
			E_NOTVEXTEXVALID,			// 6
			E_NOTFACESVALID,			// 7
			E_NOTEXCOORDVALID,			// 8
			E_NOTNUMBERVERTVALID		// 9
		};

		/*
			this function takes an index and the relative error message gets back
		*/
		static const char* ErrorMsg(int error)
		{
			static const char* obj_error_msg[] =
			{
					"No errors",													// 0
					"Can't open file",												// 1
					"can't close file",												// 2
					"Premature End of file",										// 3
					"File saving aborted",											// 4
					"Function not defined",											// 5
					"Vertices not valid",											// 6
					"Faces not valid",												// 7
					"Texture Coord not valid",										// 8
					"You cannot save more than 65535 vertices for the 3DS format"	// 9
				};

			if(error>9 || error<0) return "Unknown error";
			else return obj_error_msg[error];
		};

		/*
			returns mask of capability one define with what are the saveable information of the format.
		*/
		static int GetExportMaskCapability()
		{
			int capability = 0;

			//camera
			//capability |= vcg::tri::io::Mask::IOM_CAMERA;

			//vert
			//capability |= vcg::tri::io::Mask::IOM_VERTTEXCOORD;

			//face
			//capability |= vcg::tri::io::Mask::IOM_FACEFLAGS;
			capability |= Mask::IOM_FACECOLOR;
			capability |= Mask::IOM_FACENORMAL;

			//wedg
			capability |= Mask::IOM_WEDGTEXCOORD;
			capability |= Mask::IOM_WEDGNORMAL;

			return capability;
		}

		/*
			function which saves in 3DS file format
		*/
		static int SaveBinary(SaveMeshType &m, const char * filename, const int &mask, CallBackPos *cb=0)
		{
			if(m.vn > MAX_POLYGONS)//check max polygons
				return E_NOTNUMBERVERTVALID;

			if(m.vn == 0)
				return E_NOTVEXTEXVALID;
			if(m.fn == 0)
				return E_NOTFACESVALID;

			/*
				<<concetto:>>
				si tiene in considerazione una mappa ListOfDuplexVert<Key,int>, alla quale gli viene associato il seguente significato:
					Key:e' una coppia (int,TexCoord) formata da un int che rappresenta l'indice del vettore nella mesh originale e la sua 
						coordinata di texture. tale coppia rappresenta una chiave, essendo univoca in tutta la mesh. Non e' possibile che 
						si incontrino due vertici che hanno solito indice di vertice e solite coordinate di texture, se un vertice di 
						questo tipo esistesse allora i due vertici rappresenterebbero lo stesso vertice.
					int:e' l'indice del vertice inserito all'interno del vettore VectorOfVertexType<VertexType>
					
					Nel vertice VectorOfVertexType vengono inseriti tutti i vertici appartenenti alla mesh + i k vertici dublicati. la scelta 
					di tali vertici va in base alla seguente regola:
						se un vertice con indice x(originale) ha piu' di una coordinata di texture allora tale vertice viene duplicato e 
						inserito in ListOfDuplexVert e in VectorOfVertexType(in VectorOfVertexType, l'inserimento del doppio vertice non 
						sarebbe necessario, pero' viene fatto per comodita', in caso contrario dovremmo cercare il vertice dentro il vettore).

					rappresentazione grafica:

					ListOfDuplexVert						VectorOfVertexType
						------									---------
						|key1| -> index1 ---------              |vertex1|
						------				 	 |	            ---------
						|key2| -> index2 ----	 -------------> |vertex2|
						------				|					---------
						|key3|				|					|vertex3|
						------				|					---------
						|key4|				------------------>	|vertex4|
						------									---------
						|key5|			        --------------> |vertex5|
						------					|				---------
						  .						|					.
						  .						|					.
						  .						|					.
						------					|				---------
						|keyn| -> indexn --------				|vertexn|
						------									---------


				questo tipo di struttura permette di selezionare l'indice del vertice in VectorOfVertexType con costo O(1).

				<<code:>>
				questo pezzo di codice itera su tutte le facce della mesh per riempire la mappa e il vettore.
				per ogni faccia e per ogni vertice di faccia costruisce la coppia (indice,texture), controlla se
				all'interno di ListOfDuplexVert esiste gia' in tal caso non fa niente, in caso contrario aggiunte la
				coppia in ListOfDuplexVert e l'oggetto VertexType in VectorOfVertexType associando al valore della
				chiave (indice,texture) l'indice del vertice a cui punta.

				alla fine vengono duplicati solamente quei vertici che hanno piu' coordinate di texture.

				c'e' da tenere presente che il codice appena descritto viene eseguito SOLAMENTE se la mesh contiene texture e
				se dalla dialog di exporter viene spuntato il salvataggio delle texture. In caso contrario non esegue niente e tratta
				solamente i vertici che sono presenti nella mesh senza creare duplicati. Le informazioni presenti in assenza di
				texture sono piu' che sufficienti.


																										Federico Mazzanti
			*/			
			std::map<Key,int> ListOfDuplexVert;
			std::vector<VertexType> VectorOfVertexType;
      std::vector<int> VertRemap; // VertRemap[i] keep the final position of m.vert[i] inside the 3ds vertex list. used for remapping the pointers to vertex in the faces
			int count = 1;
			int nface = 0;
			if(HasPerWedgeTexCoord(m) && (mask & vcg::tri::io::Mask::IOM_WEDGTEXCOORD) )
			{
				FaceIterator fi;
				for(fi=m.face.begin(); fi!=m.face.end(); ++fi) if( !(*fi).IsD() )
				{
					for(unsigned int k=0;k<3;k++)
					{
						int i = GetIndexVertex(m, (*fi).V(k));
						vcg::TexCoord2<float> t = (*fi).WT(k);
						if(!m.vert[i].IsD())
						{
							if(AddDuplexVertexCoord(ListOfDuplexVert,Key(i,t)))
							{
								VectorOfVertexType.push_back((*(*fi).V(k)));
								ListOfDuplexVert[Key(i,t)] = VectorOfVertexType.size()-1;
								count++;
							}
						}
					}

					if (cb !=NULL)
						(*cb)(100.0 * (float)++nface/(float)m.face.size(), "calc duplex vertex ...");
					else
						return E_ABORTED;
				}
			}

			int number_vertex_to_duplicate = 0;
			
			if(HasPerWedgeTexCoord(m) && (mask & vcg::tri::io::Mask::IOM_WEDGTEXCOORD ))
				number_vertex_to_duplicate = (count-1) - m.vn;

			Lib3dsFile *file = lib3ds_file_new();//creates new file
			Lib3dsMesh *mesh = lib3ds_mesh_new("mesh");//creates a new mesh with mesh's name "mesh"		

			QString qnamematerial = "Material - %1";
			std::vector<Material> materials;
			
			int current = 0;
			int max = m.vn+m.fn+number_vertex_to_duplicate;
			
			lib3ds_mesh_new_point_list(mesh, m.vn + number_vertex_to_duplicate);// set number of vertexs
	
			if(HasPerWedgeTexCoord(m) && (mask & vcg::tri::io::Mask::IOM_WEDGTEXCOORD ))
				lib3ds_mesh_new_texel_list(mesh,m.vn + number_vertex_to_duplicate); //set number of textures

			int v_index = 0;
			VertexIterator vi;
			//saves vert
			if(HasPerWedgeTexCoord(m) && (mask & vcg::tri::io::Mask::IOM_WEDGTEXCOORD ))
			{
				for(unsigned int i=0; i< VectorOfVertexType.size();i++)
				{
					Lib3dsPoint point;
					point.pos[0] = VectorOfVertexType[i].P()[0];
					point.pos[1] = VectorOfVertexType[i].P()[1];
					point.pos[2] = VectorOfVertexType[i].P()[2];
		
					mesh->pointL[i] = point;		

					if (cb !=NULL)
						(*cb)(100.0 * (float)++current/(float)max, "writing vertices ");
					else
						return E_ABORTED;
				}
			}
			else
			{
        VertRemap.resize(m.vert.size(),-1);
				for(vi=m.vert.begin(); vi!=m.vert.end(); ++vi) if( !(*vi).IsD() )
				{
					Lib3dsPoint point;
					point.pos[0] = (*vi).P()[0];
					point.pos[1] = (*vi).P()[1];
					point.pos[2] = (*vi).P()[2];

					mesh->pointL[v_index] = point;		
          VertRemap[vi-m.vert.begin()]=v_index;
					if (cb !=NULL)
						(*cb)(100.0 * (float)++current/(float)max, "writing vertices ");
					else
						return E_ABORTED;
					v_index++;
				}
			}

			lib3ds_mesh_new_face_list (mesh, m.face.size());//set number of faces
			int f_index = 0;//face index
			//int t_index = 0;//texture index
			FaceIterator fi;
			for(fi=m.face.begin(); fi!=m.face.end(); ++fi) if( !(*fi).IsD() )
			{
				vcg::TexCoord2<float> t0,t1,t2;
				int i0 = GetIndexVertex(m, (*fi).V(0));
				int i1 = GetIndexVertex(m, (*fi).V(1));
				int i2 = GetIndexVertex(m, (*fi).V(2));
				if(HasPerWedgeTexCoord(m) && (mask & vcg::tri::io::Mask::IOM_WEDGTEXCOORD ) )
				{
					t0 = (*fi).WT(0);
					t1 = (*fi).WT(1);
					t2 = (*fi).WT(2);
				}

				Lib3dsFace face;
				if(HasPerWedgeTexCoord(m) && (mask & vcg::tri::io::Mask::IOM_WEDGTEXCOORD ))
				{
					face.points[0] = GetIndexDuplexVertex(ListOfDuplexVert,Key(i0,t0));
					face.points[1] = GetIndexDuplexVertex(ListOfDuplexVert,Key(i1,t1));
					face.points[2] = GetIndexDuplexVertex(ListOfDuplexVert,Key(i2,t2));
				}
				else
				{
					face.points[0] = VertRemap[i0];
					face.points[1] = VertRemap[i1];
					face.points[2] = VertRemap[i2];
				}
				
				//saves coord textures
				if(HasPerWedgeTexCoord(m) && (mask & vcg::tri::io::Mask::IOM_WEDGTEXCOORD ) )
				{
					mesh->texelL[face.points[0]][0] = t0.u();
					mesh->texelL[face.points[0]][1] = t0.v();
					mesh->texelL[face.points[1]][0] = t1.u();
					mesh->texelL[face.points[1]][1] = t1.v();
					mesh->texelL[face.points[2]][0] = t2.u();
					mesh->texelL[face.points[2]][1] = t2.v();
				}

				if(mask & vcg::tri::io::Mask::IOM_FACEFLAGS)
					face.flags = 0;
				
				face.smoothing = 10;

				if((mask & vcg::tri::io::Mask::IOM_FACENORMAL) | (mask & vcg::tri::io::Mask::IOM_WEDGNORMAL) )
				{
					face.normal[0] = (*fi).N()[0];
					face.normal[1] = (*fi).N()[1];
					face.normal[2] = (*fi).N()[2];
				}

				if((mask & vcg::tri::io::Mask::IOM_FACECOLOR) | (mask & vcg::tri::io::Mask::IOM_WEDGTEXCOORD))
				{
					int material_index = vcg::tri::io::Materials<SaveMeshType>::CreateNewMaterial(m, materials, 0, fi);
					if(material_index == (int)materials.size())
					{
						Lib3dsMaterial *material = lib3ds_material_new();//creates a new material
						
						std::string name = qnamematerial.arg(material_index-1).toStdString();
						strcpy(material->name,name.c_str());//copy new name of material

						if(mask & vcg::tri::io::Mask::IOM_FACECOLOR)
						{
							//ambient
							material->ambient[0] = materials[materials.size()-1].Ka[0];
							material->ambient[1] = materials[materials.size()-1].Ka[1];
							material->ambient[2] = materials[materials.size()-1].Ka[2];
							material->ambient[3] = materials[materials.size()-1].Tr;

							//diffuse
							material->diffuse[0] = materials[materials.size()-1].Kd[0];
							material->diffuse[1] = materials[materials.size()-1].Kd[1];
							material->diffuse[2] = materials[materials.size()-1].Kd[2];
							material->diffuse[3] = materials[materials.size()-1].Tr;

							//specular
							material->specular[0] = materials[materials.size()-1].Ks[0];
							material->specular[1] = materials[materials.size()-1].Ks[1];
							material->specular[2] = materials[materials.size()-1].Ks[2];
							material->specular[3] = materials[materials.size()-1].Tr;

							//shininess
							material->shininess = materials[materials.size()-1].Ns;
						}
											
						//texture
				    if(HasPerWedgeTexCoord(m) && (mask & vcg::tri::io::Mask::IOM_WEDGTEXCOORD ) )
							strcpy(material->texture1_map.name,materials[materials.size()-1].map_Kd.c_str());

						lib3ds_file_insert_material(file,material);//inserts the material inside the file
						strcpy(face.material,name.c_str());
					}
					else
					{	
						std::string name = qnamematerial.arg(material_index).toStdString();
						strcpy(face.material,name.c_str());//set name of material
					}
				}

				mesh->faceL[f_index]=face;

				if (cb !=NULL)
					(*cb)(100.0 * (float)++current/(float)max, "writing faces ");
				else 
					return E_ABORTED;
				f_index++;
				
			}

			lib3ds_file_insert_mesh(file, mesh);//inserts the Mesh into file
			
			Lib3dsNode *node = lib3ds_node_new_object();//creates a new node
			strcpy(node->name,mesh->name);
			node->parent_id = LIB3DS_NO_PARENT;	
			lib3ds_file_insert_node(file,node);//inserts the node into file

			bool result = lib3ds_file_save(file, filename); //saves the file
			if(result)
				return E_NOERROR; 
			else 
				return E_ABORTED;
		}
		
		/*
			function which saves in 3DS format
		*/
		static int Save(SaveMeshType &m, const char * filename, const int &mask, CallBackPos *cb=0)
		{
			return SaveBinary(m,filename,mask,cb);	
		}

		/*
			returns index of the vertex
		*/
		inline static int GetIndexVertex(SaveMeshType &m, VertexType *p)
		{
			return p-&*(m.vert.begin());
		}

		/*
			added pair Key,int into map
		*/
		inline static bool AddDuplexVertexCoord(std::map<Key,int> &m,Key key)
		{
			int index = m[key];
			if(index==0)
				return true;
			return false;
		}

		/*
			returns value of key key into map. this value is vertex's index into list all duplicate vertex
		*/
		inline static int GetIndexDuplexVertex(std::map<Key,int> &m,Key key)
		{
			return m[key];
		}

	}; // end class

} // end Namespace tri
} // end Namespace io
} // end Namespace vcg

#endif
