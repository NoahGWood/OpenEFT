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
Revision 1.3  2007/04/20 10:11:51  cignoni
Corrected bug in selectionVertexFromFaceStrict

Revision 1.2  2007/02/01 06:37:05  cignoni
Added FaceFromBorder

Revision 1.1  2006/10/16 08:50:58  cignoni
First Working Version

****************************************************************************/
#ifndef __VCG_TRI_UPDATE_SELECTION
#define __VCG_TRI_UPDATE_SELECTION

#include <queue>
#include <vcg/complex/trimesh/update/flag.h>

namespace vcg {
namespace tri {

/// \ingroup trimesh 

/// \headerfile selection.h vcg/complex/trimesh/update/selection.h

/// \brief Management, updating and computation of per-vertex and per-face normals.
/** 
This class is used to compute or update the normals that can be stored in the vertex or face component of a mesh.
*/

template <class ComputeMeshType>
class UpdateSelection
{

public:
typedef ComputeMeshType MeshType; 
typedef	typename MeshType::ScalarType			ScalarType;
typedef typename MeshType::VertexType     VertexType;
typedef typename MeshType::VertexPointer  VertexPointer;
typedef typename MeshType::VertexIterator VertexIterator;
typedef typename MeshType::FaceType       FaceType;
typedef typename MeshType::FacePointer    FacePointer;
typedef typename MeshType::FaceIterator   FaceIterator;
typedef typename vcg::Box3<ScalarType>  Box3Type;

static size_t AllVertex(MeshType &m)
{
	VertexIterator vi;
	for(vi = m.vert.begin(); vi != m.vert.end(); ++vi)
		if( !(*vi).IsD() )	(*vi).SetS();
  return m.vn;
}
  
static size_t AllFace(MeshType &m)
{
	FaceIterator fi;
	for(fi = m.face.begin(); fi != m.face.end(); ++fi)
    if( !(*fi).IsD() )	(*fi).SetS();
  return m.fn;  
}

static size_t ClearVertex(MeshType &m)
{
	VertexIterator vi;
	for(vi = m.vert.begin(); vi != m.vert.end(); ++vi)
		if( !(*vi).IsD() )	(*vi).ClearS();
  return 0;
}

static size_t ClearFace(MeshType &m)
{
	FaceIterator fi;
	for(fi = m.face.begin(); fi != m.face.end(); ++fi)
		if( !(*fi).IsD() )	(*fi).ClearS();
  return 0;
}

static void Clear(MeshType &m)
{
  ClearVertex(m);
  ClearFace(m);
}

static size_t CountFace(MeshType &m)
{
  size_t selCnt=0;
	FaceIterator fi;
  for(fi=m.face.begin();fi!=m.face.end();++fi)
    if(!(*fi).IsD() && (*fi).IsS()) ++selCnt;
  return selCnt;
}

static size_t CountVertex(MeshType &m)
{
  size_t selCnt=0;
	VertexIterator vi;
  for(vi=m.vert.begin();vi!=m.vert.end();++vi)
    if(!(*vi).IsD() && (*vi).IsS()) ++selCnt;
  return selCnt;
}

static size_t InvertFace(MeshType &m)
{
  size_t selCnt=0;
	FaceIterator fi;
  for(fi=m.face.begin();fi!=m.face.end();++fi)
      if(!(*fi).IsD()) 
      {
        if((*fi).IsS()) (*fi).ClearS(); 
        else {
          (*fi).SetS(); 
          ++selCnt;
        }
      }
  return selCnt;
} 

static size_t InvertVertex(MeshType &m)
{
  size_t selCnt=0;
	VertexIterator vi;
  for(vi=m.vert.begin();vi!=m.vert.end();++vi)
      if(!(*vi).IsD()) 
      {
        if((*vi).IsS()) (*vi).ClearS(); 
        else {
          (*vi).SetS(); 
          ++selCnt;
        }
      }
  return selCnt;
} 

/// \brief Select all the vertices that are touched by at least a single selected faces
static size_t VertexFromFaceLoose(MeshType &m)
{
  size_t selCnt=0;
	ClearVertex(m);
  FaceIterator fi;
	for(fi = m.face.begin(); fi != m.face.end(); ++fi)
		if( !(*fi).IsD() && (*fi).IsS())	
    {
      if( !(*fi).V(0)->IsS()) { (*fi).V(0)->SetS(); ++selCnt; }
      if( !(*fi).V(1)->IsS()) { (*fi).V(1)->SetS(); ++selCnt; }
      if( !(*fi).V(2)->IsS()) { (*fi).V(2)->SetS(); ++selCnt; }
    }
  return selCnt;
}

/// \brief Select ONLY the vertices that are touched ONLY by selected faces
/** In other words all the vertices having all the faces incident on them selected.
 \warning Isolated vertices will not selected.
*/
static size_t VertexFromFaceStrict(MeshType &m)
{
	VertexFromFaceLoose(m);
  FaceIterator fi;
	for(fi = m.face.begin(); fi != m.face.end(); ++fi)
		if( !(*fi).IsD() && !(*fi).IsS())	
    {
     (*fi).V(0)->ClearS(); 
     (*fi).V(1)->ClearS(); 
     (*fi).V(2)->ClearS(); 
    }
  return CountVertex(m);
}

/// \brief Select ONLY the faces with ALL the vertices selected 
static size_t FaceFromVertexStrict(MeshType &m)
{
  size_t selCnt=0;
	ClearFace(m);
  FaceIterator fi;
	for(fi = m.face.begin(); fi != m.face.end(); ++fi)
		if( !(*fi).IsD())	
    {
      if((*fi).V(0)->IsS() && (*fi).V(1)->IsS() && (*fi).V(2)->IsS())
      { 
        (*fi).SetS();
        ++selCnt;
      }
    }
  return selCnt;
}

static size_t FaceFromVertexLoose(MeshType &m)
{
  size_t selCnt=0;
	ClearFace(m);
  FaceIterator fi;
	for(fi = m.face.begin(); fi != m.face.end(); ++fi)
		if( !(*fi).IsD() && !(*fi).IsS())	
    {
      if((*fi).V(0)->IsS() || (*fi).V(1)->IsS() || (*fi).V(2)->IsS())
      { 
        (*fi).SetS();
        ++selCnt;
      }
    }
  return selCnt;
}

static size_t VertexFromBorderFlag(MeshType &m)
{
  size_t selCnt=0;
  ClearVertex(m);
  VertexIterator vi;
  for(vi = m.vert.begin(); vi != m.vert.end(); ++vi)
    if( !(*vi).IsD() )
    {
      if((*vi).IsB() )
      {
        (*vi).SetS();
        ++selCnt;
      }
    }
  return selCnt;
}


static size_t FaceFromBorderFlag(MeshType &m)
{
  size_t selCnt=0;
	ClearFace(m);
  FaceIterator fi;
	for(fi = m.face.begin(); fi != m.face.end(); ++fi)
		if( !(*fi).IsD() )	
    {
      if((*fi).IsB(0) || (*fi).IsB(1) || (*fi).IsB(2))
      { 
        (*fi).SetS();
        ++selCnt;
      }
    }
  return selCnt;
} 
/// \brief This function expand current selection to cover the whole connected component. 
static size_t FaceConnectedFF(MeshType &m)
{
	// it also assumes that the FF adjacency is well computed. 
	assert (HasFFAdjacency(m));
	UpdateFlags<MeshType>::FaceClearV(m);
	
	std::deque<FacePointer> visitStack;
  size_t selCnt=0;
  FaceIterator fi;
	for(fi = m.face.begin(); fi != m.face.end(); ++fi)
		if( !(*fi).IsD() && (*fi).IsS() && !(*fi).IsV() )	
				visitStack.push_back(&*fi);
				
	while(!visitStack.empty())
    {
			FacePointer fp = visitStack.front();
			visitStack.pop_front();
			assert(!fp->IsV());
			fp->SetV();
			for(int i=0;i<3;++i) { 
				FacePointer ff = fp->FFp(i);
        if(! ff->IsS()) 
						{
							ff->SetS();
							++selCnt;
							visitStack.push_back(ff);
							assert(!ff->IsV());
						}
      }
    }
  return selCnt;
} 

/// \brief Select ONLY the vertices whose quality is in the specified closed interval. 
static size_t VertexFromQualityRange(MeshType &m,float minq, float maxq)
{
  size_t selCnt=0;
	ClearVertex(m);
	VertexIterator vi;
	assert(HasPerVertexQuality(m));
  for(vi=m.vert.begin();vi!=m.vert.end();++vi)
      if(!(*vi).IsD()) 
      {
        if( (*vi).Q()>=minq &&  (*vi).Q()<=maxq )
					{
						(*vi).SetS(); 
						++selCnt;
					}
      }
  return selCnt;
}

static int VertexInBox( MeshType & m, const Box3Type &bb)
{
  int selCnt=0;
  for (VertexIterator vi = m.vert.begin(); vi != m.vert.end(); ++vi) if(!(*vi).IsD())
  {
    if(bb.IsIn((*vi).cP()) ) {
      (*vi).SetS();
      ++selCnt;
    }
  }
  return selCnt;
}


void VertexNonManifoldEdges(MeshType &m)
{
  assert(HasFFTopology(m));

  VertexClear(m);
  for (FaceIterator fi = m.face.begin(); fi != m.face.end(); ++fi)	if (!fi->IsD())
    {
      for(int i=0;i<3;++i)
      if(!IsManifold(*fi,i)){
        (*fi).V0(i)->SetS();
        (*fi).V1(i)->SetS();
        }
    }
}

}; // end class

}	// End namespace
}	// End namespace


#endif
