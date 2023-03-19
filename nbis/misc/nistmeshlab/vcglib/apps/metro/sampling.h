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
Revision 1.24  2007/05/04 16:50:23  ganovelli
added plus types version (#ifdef _PLUS_TYPES_ to use it ).

Revision 1.23  2006/10/25 12:40:19  fiorin
Added possibility to use Octree as search structure:

Revision 1.22  2006/04/20 08:30:24  cignoni
small GCC compiling issues

Revision 1.21  2006/01/22 10:05:43  cignoni
Corrected use of Area with the unambiguous DoubleArea

Revision 1.20  2005/11/12 06:44:29  cignoni
Changed GetClosest -> GetClosestFace

Revision 1.19  2005/10/02 23:11:50  cignoni
Changed the core for distance computation.
Current version uses the lib flexible search structures.
Now the comparison can be done exploiting a static uniform grid,
a hashed grid or a hierarchy of AA box.

Revision 1.18  2005/09/16 11:55:18  cignoni
Partial adding of AABB trees, still not working...

Revision 1.17  2005/08/26 10:42:47  cignoni
Added scalar type specification in the typedef of MetroMeshGrid

Revision 1.16  2005/04/04 10:47:26  cignoni
Release 4.05
Added saving of Error Histogram

Revision 1.15  2005/01/26 22:45:34  cignoni
Release 4.04
final updates for gcc compiling issues

Revision 1.14  2005/01/24 15:37:14  cignoni
updated from MinDistPoint to Closest (and removed some warnings)

Revision 1.13  2004/09/20 16:17:46  ponchio
Floating except fixed (happened on meshes with less than 100 faces :P)

Revision 1.12  2004/09/09 22:59:15  cignoni
Removed many small warnings

Revision 1.11  2004/08/25 15:28:15  ponchio
Comma at end of enum.

Revision 1.10  2004/08/25 15:15:22  ganovelli
minor changes to comply gcc compiler (typename's and stuff)

Revision 1.9  2004/07/18 10:13:34  cignoni
NewUserBit -> NewBitFlag

Revision 1.8  2004/06/24 09:08:31  cignoni
Official Release of Metro 4.00

Revision 1.7  2004/05/14 13:49:07  ganovelli
created

Revision 1.6  2004/05/14 00:38:01  ganovelli
a bit of cleaning:
SamplingFlags struct added
optional treatment for unreferred vertices.
Note: unref vertices are tested against unref vertices without
using the grid...it is n^2 with n number of unreferred vertices. To make it
with the grid in the proper way :
derive face and vertex from a simplexClass,
instantiate GridStaticPtr on the simplexClass template.



****************************************************************************/
#ifndef __VCGLIB__SAMPLING
#define __VCGLIB__SAMPLING

#include <time.h>
//#include "min_dist_point.h"
#include <vcg/complex/trimesh/closest.h>
#include <vcg/space/box3.h>
#include <vcg/math/histogram.h>
#include <vcg/space/color4.h>
#include <vcg/simplex/face/distance.h>
#include <vcg/complex/trimesh/update/color.h>
#include <vcg/space/index/grid_static_ptr.h>
#include <vcg/space/index/aabb_binary_tree/aabb_binary_tree.h>
#include <vcg/space/index/octree.h>
#include <vcg/space/index/spatial_hashing.h>
namespace vcg
{

struct SamplingFlags{
			 enum{
						HIST														= 0x0001,
						VERTEX_SAMPLING									= 0x0002,
						EDGE_SAMPLING										= 0x0004,
						FACE_SAMPLING										= 0x0008,
						MONTECARLO_SAMPLING							= 0x0010,
						SUBDIVISION_SAMPLING						= 0x0020,
						SIMILAR_SAMPLING			          = 0x0040,
						NO_SAMPLING     			          = 0x0070,
						SAVE_ERROR                      = 0x0100,
						INCLUDE_UNREFERENCED_VERTICES		= 0x0200,
            USE_STATIC_GRID                 = 0x0400,
            USE_HASH_GRID                   = 0x0800,
            USE_AABB_TREE                   = 0x1000,
						USE_OCTREE                      = 0x2000
				};
	};
// -----------------------------------------------------------------------------------------------
template <class MetroMesh>
class Sampling
{
public:

private:
	  typedef typename MetroMesh::CoordType				CoordType;
    typedef typename MetroMesh::ScalarType			ScalarType;
		typedef typename MetroMesh::VertexType			VertexType;
    typedef typename MetroMesh::VertexPointer		VertexPointer;
    typedef typename MetroMesh::VertexIterator	VertexIterator;
    typedef typename MetroMesh::FaceIterator		FaceIterator;
    typedef typename MetroMesh::FaceType				FaceType;
    typedef typename MetroMesh::FaceContainer		FaceContainer;
	  
		typedef GridStaticPtr				<FaceType, typename MetroMesh::ScalarType >									MetroMeshGrid;
	  typedef SpatialHashTable		<FaceType, typename MetroMesh::ScalarType >									MetroMeshHash;
    typedef AABBBinaryTreeIndex	<FaceType, typename MetroMesh::ScalarType, vcg::EmptyClass>	MetroMeshAABB;
		typedef Octree							<FaceType, typename MetroMesh::ScalarType >                 MetroMeshOctree;

    typedef Point3<typename MetroMesh::ScalarType> Point3x;




    // data structures
    MetroMesh       &S1;
    MetroMesh       &S2;
    MetroMeshGrid   gS2;
    MetroMeshHash   hS2;
    MetroMeshAABB   tS2;
		MetroMeshOctree oS2;


		unsigned int n_samples_per_face    ;
		float n_samples_edge_to_face_ratio ;
		float bbox_factor                  ;
		float inflate_percentage			     ;
		unsigned int min_size					     ;
		int n_hist_bins                    ;
		int print_every_n_elements         ;
		int referredBit                    ;
    // parameters
    double          dist_upper_bound;
    double					n_samples_per_area_unit;
    unsigned long   n_samples_target;
    int             Flags;

    // results
    Histogram<double>            hist;
    unsigned long   n_total_samples;
    unsigned long   n_total_area_samples;
    unsigned long   n_total_edge_samples;
    unsigned long   n_total_vertex_samples;
    double          max_dist;
    double          mean_dist;
    double          RMS_dist;
    double          volume;
    double          area_S1;

    // globals
    int             n_samples;

    // private methods
    inline double   ComputeMeshArea(MetroMesh & mesh);
    float           AddSample(const Point3x &p);
    inline void     AddRandomSample(FaceIterator &T);
    inline void     SampleEdge(const Point3x & v0, const Point3x & v1, int n_samples_per_edge);
    void            VertexSampling();
    void            EdgeSampling();
    void            FaceSubdiv(const Point3x & v0, const Point3x &v1, const Point3x & v2, int maxdepth);
    void            SimilarTriangles(const Point3x &v0, const Point3x &v1, const Point3x &v2, int n_samples_per_edge);
    void            MontecarloFaceSampling();
    void            SubdivFaceSampling();
    void            SimilarFaceSampling();

public :
    // public methods
    Sampling(MetroMesh &_s1, MetroMesh &_s2);
		~Sampling();
    void            Hausdorff();
    double          GetArea()                   {return area_S1;}
    double          GetDistMax()                {return max_dist;}
    double          GetDistMean()               {return mean_dist;}
    double          GetDistRMS()                {return RMS_dist;}
    double          GetDistVolume()             {return volume;}
    unsigned long   GetNSamples()               {return n_total_samples;}
    unsigned long   GetNAreaSamples()           {return n_total_area_samples;}
    unsigned long   GetNEdgeSamples()           {return n_total_edge_samples;}
    unsigned long   GetNVertexSamples()         {return n_total_vertex_samples;}
    double					GetNSamplesPerAreaUnit()    {return n_samples_per_area_unit;}
    unsigned long   GetNSamplesTarget()         {return n_samples_target;}
    Histogram<double> &GetHist()                  {return hist;}
    void            SetFlags(int flags)         {Flags = flags;}
    void            ClearFlag(int flag)         {Flags &= (flag ^ -1);}
    void            SetParam(double _n_samp)    {n_samples_target = _n_samp;}
    void            SetSamplesTarget(unsigned long _n_samp);
    void            SetSamplesPerAreaUnit(double _n_samp);
};

// -----------------------------------------------------------------------------------------------

// constructor
template <class MetroMesh>
Sampling<MetroMesh>::Sampling(MetroMesh &_s1, MetroMesh &_s2):S1(_s1),S2(_s2)
{
    Flags = 0;
    area_S1 = ComputeMeshArea(_s1);
		// set default numbers
		n_samples_per_face             =	10;
		n_samples_edge_to_face_ratio   = 0.1f;
		bbox_factor                    = 0.1f;
		inflate_percentage			       = 0.02f;
		min_size					             = 125;		/* 125 = 5^3 */
		n_hist_bins                    = 256;
		print_every_n_elements         = S1.fn/100;

		if(print_every_n_elements <= 1)
		  print_every_n_elements = 2;

			referredBit = VertexType::NewBitFlag();
			// store the unreferred vertices
			FaceIterator fi; VertexIterator vi; int i;
			for(fi = _s1.face.begin(); fi!= _s1.face.end(); ++fi)
				for(i=0;i<3;++i) (*fi).V(i)->SetUserBit(referredBit);
}

template <class MetroMesh>
Sampling<MetroMesh>::~Sampling()
{
	VertexType::DeleteBitFlag(referredBit);
}


// set sampling parameters
template <class MetroMesh>
void Sampling<MetroMesh>::SetSamplesTarget(unsigned long _n_samp)
{
    n_samples_target        = _n_samp;
    n_samples_per_area_unit =  n_samples_target / (double)area_S1;
}

template <class MetroMesh>
void Sampling<MetroMesh>::SetSamplesPerAreaUnit(double _n_samp)
{
    n_samples_per_area_unit = _n_samp;
    n_samples_target        = (unsigned long)((double) n_samples_per_area_unit * area_S1);
}


// auxiliary functions
template <class MetroMesh>
inline double Sampling<MetroMesh>::ComputeMeshArea(MetroMesh & mesh)
{
    FaceIterator    face;
    double                  area = 0.0;

    for(face=mesh.face.begin(); face != mesh.face.end(); face++)
			if(!(*face).IsD())
        area += DoubleArea(*face);

    return area/2.0;
}

template <class MetroMesh>
float Sampling<MetroMesh>::AddSample(const Point3x &p )
{
    FaceType   *f=0;
    Point3x             normf, bestq, ip;
		ScalarType              dist;

    dist = dist_upper_bound;

    // compute distance between p_i and the mesh S2
    if(Flags & SamplingFlags::USE_AABB_TREE)
      f=tri::GetClosestFace<MetroMesh,MetroMeshAABB>(S2, tS2, p, dist_upper_bound, dist, normf, bestq, ip);
    if(Flags & SamplingFlags::USE_HASH_GRID)
      f=tri::GetClosestFace<MetroMesh,MetroMeshHash>(S2, hS2, p, dist_upper_bound, dist, normf, bestq, ip);
    if(Flags & SamplingFlags::USE_STATIC_GRID)
      f=tri::GetClosestFace<MetroMesh,MetroMeshGrid>(S2, gS2, p, dist_upper_bound, dist, normf, bestq, ip);
		if (Flags & SamplingFlags::USE_OCTREE)
			f=tri::GetClosestFace<MetroMesh,MetroMeshOctree>(S2, oS2, p, dist_upper_bound, dist, normf, bestq, ip);

    // update distance measures
    if(dist == dist_upper_bound)
        return -1.0;

    if(dist > max_dist)
        max_dist = dist;        // L_inf
    mean_dist += dist;	        // L_1
    RMS_dist  += dist*dist;     // L_2
    n_total_samples++;

    if(Flags &  SamplingFlags::HIST)
        hist.Add((float)fabs(dist));

    return (float)dist;
}


// -----------------------------------------------------------------------------------------------
// --- Vertex Sampling ---------------------------------------------------------------------------

template <class MetroMesh>
void Sampling<MetroMesh>::VertexSampling()
{
    // Vertex sampling.
    int   cnt = 0;
    float error;

    printf("Vertex sampling\n");
    VertexIterator vi;
		typename std::vector<VertexPointer>::iterator vif;
    for(vi=S1.vert.begin();vi!=S1.vert.end();++vi)
			if(  (*vi).IsUserBit(referredBit) || // it is referred
					((Flags&SamplingFlags::INCLUDE_UNREFERENCED_VERTICES) != 0) ) //include also unreferred
    {
        error = AddSample((*vi).cP());

        n_total_vertex_samples++;

        // save vertex quality
        if(Flags & SamplingFlags::SAVE_ERROR)  (*vi).Q() = error;

        // print progress information
        if(!(++cnt % print_every_n_elements))
            printf("Sampling vertices %d%%\r", (100 * cnt/S1.vn));
    }
    printf("                       \r");
}


// -----------------------------------------------------------------------------------------------
// --- Edge Sampling -----------------------------------------------------------------------------

template <class MetroMesh>
inline void Sampling<MetroMesh>::SampleEdge(const Point3x & v0, const Point3x & v1, int n_samples_per_edge)
{
    // uniform sampling of the segment v0v1.
    Point3x     e((v1-v0)/(double)(n_samples_per_edge+1));
    int         i;

    for(i=1; i <= n_samples_per_edge; i++)
    {
        AddSample(v0 + e*i);
        n_total_edge_samples++;
    }
}


template <class MetroMesh>
void Sampling<MetroMesh>::EdgeSampling()
{
    // Edge sampling.
		typedef std::pair<VertexPointer, VertexPointer> pvv;
		std::vector< pvv > Edges;

    printf("Edge sampling\n");

    // compute edge list.
    FaceIterator fi;
    for(fi=S1.face.begin(); fi != S1.face.end(); fi++)
        for(int i=0; i<3; ++i)
        {
            Edges.push_back(make_pair((*fi).V0(i),(*fi).V1(i)));
            if(Edges.back().first > Edges.back().second)
                swap(Edges.back().first, Edges.back().second);
        }
    sort(Edges.begin(), Edges.end());
		typename std::vector< pvv>::iterator edgeend = unique(Edges.begin(), Edges.end());
    Edges.resize(edgeend-Edges.begin());

    // sample edges.
		typename std::vector<pvv>::iterator   ei;
    double                  n_samples_per_length_unit;
    double                  n_samples_decimal = 0.0;
    int                     cnt=0;
    if(Flags & SamplingFlags::FACE_SAMPLING)
        n_samples_per_length_unit = sqrt((double)n_samples_per_area_unit);
    else
        n_samples_per_length_unit = n_samples_per_area_unit;
    for(ei=Edges.begin(); ei!=Edges.end(); ++ei)
    {
        n_samples_decimal += Distance((*ei).first->cP(),(*ei).second->cP()) * n_samples_per_length_unit;
        n_samples          = (int) n_samples_decimal;
        SampleEdge((*ei).first->cP(), (*ei).second->cP(), (int) n_samples);
        n_samples_decimal -= (double) n_samples;

        // print progress information
        if(!(++cnt % print_every_n_elements))
            printf("Sampling edge %d%%\r", (100 * cnt/Edges.size()));
    }
    printf("                     \r");
}


// -----------------------------------------------------------------------------------------------
// --- Face Sampling -----------------------------------------------------------------------------

// Montecarlo sampling.
template <class MetroMesh>
inline void Sampling<MetroMesh>::AddRandomSample(FaceIterator &T)
{
    // random sampling over the input face.
    double      rnd_1, rnd_2;

    // vertices of the face T.
    Point3x p0(T->V(0)->cP());
    Point3x p1(T->V(1)->cP());
    Point3x p2(T->V(2)->cP());
    // calculate two edges of T.
    Point3x v1(p1 - p0);
    Point3x v2(p2 - p0);

    // choose two random numbers.
    rnd_1 = (double)rand() / (double)RAND_MAX;
    rnd_2 = (double)rand() / (double)RAND_MAX;
    if(rnd_1 + rnd_2 > 1.0)
    {
        rnd_1 = 1.0 - rnd_1;
        rnd_2 = 1.0 - rnd_2;
    }

    // add a random point on the face T.
    AddSample (p0 + (v1 * rnd_1 + v2 * rnd_2));
    n_total_area_samples++;
}

template <class MetroMesh>
void Sampling<MetroMesh>::MontecarloFaceSampling()
{
    // Montecarlo sampling.
    int     cnt = 0;
    double  n_samples_decimal = 0.0;
    FaceIterator fi;

    srand(clock());
 //   printf("Montecarlo face sampling\n");
    for(fi=S1.face.begin(); fi != S1.face.end(); fi++)
		if(!(*fi).IsD())
    {
        // compute # samples in the current face.
        n_samples_decimal += 0.5*DoubleArea(*fi) * n_samples_per_area_unit;
        n_samples          = (int) n_samples_decimal;

        // for every sample p_i in T...
        for(int i=0; i < n_samples; i++)
            AddRandomSample(fi);

        n_samples_decimal -= (double) n_samples;

        // print progress information
//        if(!(++cnt % print_every_n_elements))
 //           printf("Sampling face %d%%\r", (100 * cnt/S1.fn));
    }
 //   printf("                     \r");
}


// Subdivision sampling.
template <class MetroMesh>
void Sampling<MetroMesh>::FaceSubdiv(const Point3x & v0, const Point3x & v1, const Point3x & v2, int maxdepth)
{
    // recursive face subdivision.
    if(maxdepth == 0)
    {
        // ground case.
        AddSample((v0+v1+v2)/3.0f);
        n_total_area_samples++;
        n_samples++;
        return;
    }

    // compute the longest edge.
    double  maxd01 = SquaredDistance(v0,v1);
    double  maxd12 = SquaredDistance(v1,v2);
    double  maxd20 = SquaredDistance(v2,v0);
    int     res;
    if(maxd01 > maxd12)
        if(maxd01 > maxd20)     res = 0;
        else                    res = 2;
    else
        if(maxd12 > maxd20)     res = 1;
        else                    res = 2;

    // break the input triangle along the median to the the longest edge.
    Point3x  pp;
    switch(res)
    {
     case 0 :    pp = (v0+v1)/2;
                 FaceSubdiv(v0,pp,v2,maxdepth-1);
                 FaceSubdiv(pp,v1,v2,maxdepth-1);
                 break;
     case 1 :    pp = (v1+v2)/2;
                 FaceSubdiv(v0,v1,pp,maxdepth-1);
                 FaceSubdiv(v0,pp,v2,maxdepth-1);
                 break;
     case 2 :    pp = (v2+v0)/2;
                 FaceSubdiv(v0,v1,pp,maxdepth-1);
                 FaceSubdiv(pp,v1,v2,maxdepth-1);
                 break;
    }
}

template <class MetroMesh>
void Sampling<MetroMesh>::SubdivFaceSampling()
{
    // Subdivision sampling.
    int     cnt = 0, maxdepth;
    double  n_samples_decimal = 0.0;
    typename MetroMesh::FaceIterator fi;

    printf("Subdivision face sampling\n");
    for(fi=S1.face.begin(); fi != S1.face.end(); fi++)
    {
        // compute # samples in the current face.
        n_samples_decimal += 0.5*DoubleArea(*fi) * n_samples_per_area_unit;
        n_samples          = (int) n_samples_decimal;
        if(n_samples)
        {
            // face sampling.
            maxdepth = ((int)(log((double)n_samples)/log(2.0)));
            n_samples = 0;
            FaceSubdiv((*fi).V(0)->cP(), (*fi).V(1)->cP(), (*fi).V(2)->cP(), maxdepth);
        }
        n_samples_decimal -= (double) n_samples;

        // print progress information
        if(!(++cnt % print_every_n_elements))
            printf("Sampling face %d%%\r", (100 * cnt/S1.fn));
    }
    printf("                     \r");
}


// Similar Triangles sampling.
template <class MetroMesh>
void Sampling<MetroMesh>::SimilarTriangles(const Point3x & v0, const Point3x & v1, const Point3x & v2, int n_samples_per_edge)
{
    Point3x     V1((v1-v0)/(double)(n_samples_per_edge-1));
    Point3x     V2((v2-v0)/(double)(n_samples_per_edge-1));
    int         i, j;

    // face sampling.
    for(i=1; i < n_samples_per_edge-1; i++)
        for(j=1; j < n_samples_per_edge-1-i; j++)
        {
            AddSample( v0 + (V1*(double)i + V2*(double)j) );
            n_total_area_samples++;
            n_samples++;
        }
}

template <class MetroMesh>
void Sampling<MetroMesh>::SimilarFaceSampling()
{
    // Similar Triangles sampling.
    int     cnt = 0, n_samples_per_edge;
    double  n_samples_decimal = 0.0;
    FaceIterator fi;

    printf("Similar Triangles face sampling\n");
    for(fi=S1.face.begin(); fi != S1.face.end(); fi++)
    {
        // compute # samples in the current face.
        n_samples_decimal += 0.5*DoubleArea(*fi) * n_samples_per_area_unit;
        n_samples          = (int) n_samples_decimal;
        if(n_samples)
        {
            // face sampling.
            n_samples_per_edge = (int)((sqrt(1.0+8.0*(double)n_samples) +5.0)/2.0);
            n_samples = 0;
            SimilarTriangles((*fi).V(0)->cP(), (*fi).V(1)->cP(), (*fi).V(2)->cP(), n_samples_per_edge);
        }
        n_samples_decimal -= (double) n_samples;

        // print progress information
        if(!(++cnt % print_every_n_elements))
            printf("Sampling face %d%%\r", (100 * cnt/S1.fn));
    }
    printf("                     \r");
}


// -----------------------------------------------------------------------------------------------
// --- Distance ----------------------------------------------------------------------------------

template <class MetroMesh>
void Sampling<MetroMesh>::Hausdorff()
{
		Box3< ScalarType> bbox;

    typedef typename std::vector<FaceType>::iterator  FaceVecIterator;
    // set grid meshes.
    if(Flags & SamplingFlags::USE_HASH_GRID)   hS2.Set(S2.face.begin(),S2.face.end());
    if(Flags & SamplingFlags::USE_AABB_TREE)   tS2.Set(S2.face.begin(),S2.face.end());
    if(Flags & SamplingFlags::USE_STATIC_GRID) gS2.Set(S2.face.begin(),S2.face.end());
		if(Flags & SamplingFlags::USE_OCTREE)      oS2.Set(S2.face.begin(),S2.face.end());

    // set bounding box
    bbox = S2.bbox;
    dist_upper_bound = /*bbox_factor * */bbox.Diag();
    if(Flags &  SamplingFlags::HIST)
    	hist.SetRange(0.0, dist_upper_bound/100.0, n_hist_bins);

    // initialize sampling statistics.
    n_total_area_samples = n_total_edge_samples = n_total_vertex_samples = n_total_samples = n_samples = 0;
		max_dist             = -HUGE_VAL;
		mean_dist = RMS_dist = 0;

    // Vertex sampling.
    if(Flags & SamplingFlags::VERTEX_SAMPLING)
        VertexSampling();
    // Edge sampling.
    if(n_samples_target > n_total_samples)
			{		
				n_samples_target -= (int) n_total_samples;
        n_samples_per_area_unit  = n_samples_target / area_S1;
				if(Flags & SamplingFlags::EDGE_SAMPLING)
        {
            EdgeSampling();
           if(n_samples_target > n_total_samples) n_samples_target -= (int) n_total_samples;
           else n_samples_target=0;
        }
        // Face sampling.
        if((Flags & SamplingFlags::FACE_SAMPLING) && (n_samples_target > 0))
        {
            n_samples_per_area_unit  = n_samples_target / area_S1;
            if(Flags & SamplingFlags::MONTECARLO_SAMPLING)        MontecarloFaceSampling();
            if(Flags & SamplingFlags::SUBDIVISION_SAMPLING)       SubdivFaceSampling();
            if(Flags & SamplingFlags::SIMILAR_SAMPLING) SimilarFaceSampling();
        }
    }

    // compute vertex colour
    if(Flags & SamplingFlags::SAVE_ERROR)
      vcg::tri::UpdateColor<MetroMesh>::VertexQualityRamp(S1);

    // compute statistics
    n_samples_per_area_unit = (double) n_total_samples / area_S1;
    volume     = mean_dist / n_samples_per_area_unit / 2.0;
    mean_dist /= n_total_samples;
    RMS_dist   = sqrt(RMS_dist / n_total_samples);
}
}
#endif
