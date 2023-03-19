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

#ifndef __VCGLIB_REFINE_LOOP
#define __VCGLIB_REFINE_LOOP

#include <math.h>
#include <vcg/complex/trimesh/base.h>
#include <vcg/complex/trimesh/refine.h>
#include <vcg/space/color4.h>
#include <vcg/container/simple_temporary_data.h>
#include <vcg/complex/trimesh/update/flag.h>
#include <vcg/complex/trimesh/update/color.h>


namespace vcg{
namespace tri{

/*
Metodo di Loop dalla documentazione "Siggraph 2000 course on subdivision"

		d4------d3							d4------d3
	 /	\		 / 	\						 /	\		 / 	\							u
	/		 \  /  	 \					/		e4--e3 	 \					 / \
 /	   	\/	 		\				 /	 / 	\/	\		\					/		\
d5------d1------d2	->	d5--e5--d1--e2--d2			 l--M--r
 \	   	/\	 		/				 \	 \ 	/\	/		/				  \	  /
	\		 /  \ 	 /					\		e6--e7	 /					 \ /
	 \	/		 \ 	/						 \	/		 \ 	/							d
		d6------d7							d6------d7

*******************************************************

*/

// Nuovi punti (e.g. midpoint), ossia odd vertices
//
template<class MESH_TYPE>
struct OddPointLoop : public std::unary_function<face::Pos<typename MESH_TYPE::FaceType> , typename MESH_TYPE::CoordType>
{
	void operator()(typename MESH_TYPE::VertexType &nv, face::Pos<typename MESH_TYPE::FaceType>  ep)	{

		face::Pos<typename MESH_TYPE::FaceType> he(ep.f,ep.z,ep.f->V(ep.z));
		typename MESH_TYPE::CoordType *l,*r,*u,*d;
		l = &he.v->P();
		he.FlipV();
		r = &he.v->P();

		if( MESH_TYPE::HasPerVertexColor())
			nv.C().lerp(ep.f->V(ep.z)->C(),ep.f->V1(ep.z)->C(),.5f);

		if (he.IsBorder()) {
			nv.P() = ((*l)*0.5 + (*r)*0.5);

		}
		else {
			he.FlipE();	he.FlipV();
			u = &he.v->P();
			he.FlipV();	he.FlipE();
			assert(&he.v->P()== r); // back to r
			he.FlipF();	he.FlipE();	he.FlipV();
			d = &he.v->P();

			// abbiamo i punti l,r,u e d per ottenere M in maniera pesata

			nv.P()=((*l)*(3.0/8.0)+(*r)*(3.0/8.0)+(*d)*(1.0/8.0)+(*u)*(1.0/8.0));
		}

	}

	Color4<typename MESH_TYPE::ScalarType> WedgeInterp(Color4<typename MESH_TYPE::ScalarType> &c0, Color4<typename MESH_TYPE::ScalarType> &c1)
	{
		Color4<typename MESH_TYPE::ScalarType> cc;
		return cc.lerp(c0,c1,0.5f);
	}

	template<class FL_TYPE>
	TexCoord2<FL_TYPE,1> WedgeInterp(TexCoord2<FL_TYPE,1> &t0, TexCoord2<FL_TYPE,1> &t1)
	{
		TexCoord2<FL_TYPE,1> tmp;
		tmp.n()=t0.n();
		tmp.t()=(t0.t()+t1.t())/2.0;
		return tmp;
	}
};

// vecchi punti, ossia even vertices
template<class MESH_TYPE>
struct EvenPointLoop : public std::unary_function<face::Pos<typename MESH_TYPE::FaceType> , typename MESH_TYPE::CoordType>
{

	void operator()(typename MESH_TYPE::CoordType &nP, face::Pos<typename MESH_TYPE::FaceType>  ep)	{

		face::Pos<typename MESH_TYPE::FaceType> he(ep.f,ep.z,ep.f->V(ep.z));
		typename MESH_TYPE::CoordType *r, *l,  *curr;
		curr = &he.v->P();

		if (he.IsBorder()) {//half edge di bordo
			he.FlipV();
			r = &he.v->P();
			he.FlipV();
			assert(&he.v->P()== curr); // back to curr
			he.NextB();
			if (&he.v->P() == curr)
				he.FlipV();
			l = &he.v->P();
			nP = ( *(curr) * (3.0)/(4.0)  + (*l)*(1.0/8.0) + (*r)*(1.0/8.0));
		}
		else {
			// compute valence of this vertex
 			int k = 0;
			face::Pos<typename MESH_TYPE::FaceType> heStart = he;
 			std::vector<typename MESH_TYPE::CoordType> otherVertVec;
			if(he.v->IsB())return ;
			do {
				he.FlipV();
				otherVertVec.push_back(he.v->P());
				he.FlipV();
				he.FlipE();	he.FlipF();
				k++;
			}	while(he.f!=heStart.f || he.z!=heStart.z || he.v!=heStart.v);
			//			while(he != heStart);

			float beta = 3.0 / 16.0;
			if(k > 3 )
				beta = (1.0/(float)k) * (5.0/8.0 - pow((3.0/8.0 + 0.25 * cos(2*M_PI/k)),2));

			*curr = *curr * (1 - k * beta) ;
			typename std::vector<typename MESH_TYPE::CoordType>::iterator iter;
			for (iter = otherVertVec.begin();
					 iter != otherVertVec.end();
					 ++iter) {
				*curr = *curr + (*iter) * beta;

			}
			nP = *curr;
		}
	} // end of operator()

	Color4<typename MESH_TYPE::ScalarType> WedgeInterp(Color4<typename MESH_TYPE::ScalarType> &c0, Color4<typename MESH_TYPE::ScalarType> &c1)
	{
		Color4<typename MESH_TYPE::ScalarType> cc;
		return cc.lerp(c0,c1,0.5f);
	}
	Color4b WedgeInterp(Color4b &c0, Color4b &c1)
	{
		Color4b cc;
		cc.lerp(c0,c1,0.5f);
		return cc;
	}

	template<class FL_TYPE>
	TexCoord2<FL_TYPE,1> WedgeInterp(TexCoord2<FL_TYPE,1> &t0, TexCoord2<FL_TYPE,1> &t1)
	{
		TexCoord2<FL_TYPE,1> tmp;
		// assert(t0.n()== t1.n());
		tmp.n()=t0.n();
		tmp.t()=(t0.t()+t1.t())/2.0;
		return tmp;
	}

};

template<class CoordType> struct EvenParam {
	CoordType sum;
	bool border;
	int k;
} ;


template<class MESH_TYPE,class ODD_VERT, class EVEN_VERT>
bool RefineOddEven(MESH_TYPE &m, ODD_VERT odd, EVEN_VERT even,float length,
                    bool RefineSelected=false, CallBackPos *cbOdd = 0, CallBackPos *cbEven = 0)
{
  EdgeLen <MESH_TYPE, typename MESH_TYPE::ScalarType> ep(length);
  return RefineOddEvenE(m, odd, even, ep, RefineSelected, cbOdd, cbEven);
}

template<class MESH_TYPE, class ODD_VERT, class EVEN_VERT, class PREDICATE>
bool RefineOddEvenE(MESH_TYPE &m, ODD_VERT odd, EVEN_VERT even, PREDICATE edgePred,
										bool RefineSelected=false, CallBackPos *cbOdd = 0, CallBackPos *cbEven = 0)
{

	// n = numero di vertici iniziali
	int n = m.vn;

	// refine dei vertici odd, crea dei nuovi vertici in coda
	RefineE< MESH_TYPE,OddPointLoop<MESH_TYPE> > (m, odd, edgePred, RefineSelected, cbOdd);
	// momentaneamente le callback sono identiche, almeno cbOdd deve essere passata
	cbEven = cbOdd;

	vcg::tri::UpdateFlags<MESH_TYPE>::FaceBorderFromFF(m);
	// aggiorno i flag perche' IsB funzioni
	vcg::tri::UpdateFlags<MESH_TYPE>::VertexBorderFromFace (m);
	//vcg::tri::UpdateColor<MESH_TYPE>::VertexBorderFlag(m);

	// marco i vertici even [ i primi n ] come visitati
	int evenFlag = MESH_TYPE::VertexType::NewBitFlag();
	for (int i = 0; i < n ; i++ ) {
		m.vert[i].SetUserBit(evenFlag);
	}


	int j = 0;
    // di texture per wedge (uno per ogni edge)

	typename MESH_TYPE::VertexIterator vi;
	typename MESH_TYPE::FaceIterator fi;
	for (fi = m.face.begin(); fi != m.face.end(); fi++) if(!(*fi).IsD()){ //itero facce
		for (int i = 0; i < 3; i++) { //itero vert
			if ( (*fi).V(i)->IsUserBit(evenFlag) && ! (*fi).V(i)->IsD() ) {
				if (RefineSelected && !(*fi).V(i)->IsS() )
					break;
				face::Pos<typename MESH_TYPE::FaceType>aux (&(*fi),i);
				if( MESH_TYPE::HasPerVertexColor() ) {
					(*fi).V(i)->C().lerp((*fi).V0(i)->C() , (*fi).V1(i)->C(),0.5f);
				}

				if (cbEven) {
					(*cbEven)(int(100.0f * (float)j / (float)m.fn),"Refining");
					j++;
				}
				even((*fi).V(i)->P(), aux);
			}
		}
	}

	return true;
}



} // namespace tri
} // namespace vcg




#endif



