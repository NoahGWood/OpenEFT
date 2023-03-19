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
Revision 1.33  2007/06/07 15:16:39  fiorin
Added IntersectionSphereTriangle

Revision 1.32  2007/05/29 14:33:29  fiorin
Added IntersectionSegmentSphere

Revision 1.31  2007/04/16 09:08:15  cignoni
commented out non compiling intersectionSpherePlane

Revision 1.30  2007/04/10 22:26:47  pietroni
IntersectionPlanePlane first parameter is a const

Revision 1.29  2007/04/04 23:19:40  pietroni
- Changed name of intersection function between plane and triangle from Intersection to IntersectionPlaneTriangle.
- Added Intersection_Plane_Sphere function.

Revision 1.28  2007/02/21 02:40:52  m_di_benedetto
Added const qualifier to bbox parameter in Intersection_Triangle_Box().

Revision 1.27  2006/10/25 16:04:32  pietroni
added intersection control between bounding boxes for intersection between segment and triangle function

Revision 1.26  2006/09/14 08:39:07  ganovelli
Intersection_sphere_sphere added

Revision 1.25  2006/06/06 14:35:31  zifnab1974
Changes for compilation on linux AMD64. Some remarks: Linux filenames are case-sensitive. _fileno and _filelength do not exist on linux

Revision 1.24  2006/06/01 08:38:02  pietroni
Added functions:

- Intersection_Segment_Triangle
- IntersectionPlaneBox
- Intersection_Triangle_Box

Revision 1.23  2006/03/29 07:53:36  cignoni
Missing ';' (thx Maarten)

Revision 1.22  2006/03/20 14:42:49  pietroni
IntersectionSegmentPlane and IntersectionSegmentBox functions Added

Revision 1.21  2006/01/20 16:35:51  pietroni
added IntersectionSegmentBox function

Revision 1.20  2005/10/03 16:07:50  ponchio
Changed order of functions intersection_line_box and
intersectuion_ray_box

Revision 1.19  2005/09/30 13:11:39  pietroni
corrected 1 compiling error on Ray_Box_Intersection function

Revision 1.18  2005/09/29 15:30:10  pietroni
Added function RayBoxIntersection, renamed intersection line box from "Intersection" to "Intersection_Line_Box"

Revision 1.17  2005/09/29 11:48:00  m_di_benedetto
Added functor RayTriangleIntersectionFunctor.

Revision 1.16  2005/09/28 19:40:55  m_di_benedetto
Added intersection for ray-triangle (with Ray3 type).

Revision 1.15  2005/06/29 15:28:31  callieri
changed intersection names to more specific to avoid ambiguity

Revision 1.14  2005/03/15 11:22:39  ganovelli
added intersection between tow planes (porting from old vcg lib)

Revision 1.13  2005/01/26 10:03:08  spinelli
aggiunta intersect  ray-box

Revision 1.12  2004/10/13 12:45:51  cignoni
Better Doxygen documentation

Revision 1.11  2004/09/09 14:41:32  ponchio
forgotten typename SEGMENTTYPE::...

Revision 1.10  2004/08/09 09:48:43  pietroni
correcter .dir to .Direction and .ori in .Origin()

Revision 1.9  2004/08/04 20:55:02  pietroni
added rey triangle intersections funtions

Revision 1.8  2004/07/11 22:08:04  cignoni
Added a cast to remove a warning

Revision 1.7  2004/05/14 03:14:29  ponchio
Fixed some minor bugs

Revision 1.6  2004/05/13 23:43:54  ponchio
minor bug

Revision 1.5  2004/05/05 08:21:55  cignoni
syntax errors in inersection plane line.

Revision 1.4  2004/05/04 02:37:58  ganovelli
Triangle3<T> replaced by TRIANGLE
Segment<T> replaced by EDGETYPE

Revision 1.3  2004/04/29 10:48:44  ganovelli
error in plane segment corrected

Revision 1.2  2004/04/26 12:34:50  ganovelli
plane line
plane segment
triangle triangle added

Revision 1.1  2004/04/21 14:22:27  cignoni
Initial Commit


****************************************************************************/



#ifndef __VCGLIB_INTERSECTION_3
#define __VCGLIB_INTERSECTION_3

#include <vcg/math/base.h>
#include <vcg/space/point3.h>
#include <vcg/space/line3.h>
#include <vcg/space/ray3.h>
#include <vcg/space/plane3.h>
#include <vcg/space/segment3.h>
#include <vcg/space/sphere3.h>
#include <vcg/space/triangle3.h>
#include <vcg/space/intersection/triangle_triangle3.h>




namespace vcg {
/** \addtogroup space */
/*@{*/
/** 
    Function computing the intersection between couple of geometric primitives in
    3 dimension
*/
  /// interseciton between sphere and line
  template<class T>
    inline bool IntersectionLineSphere( const Sphere3<T> & sp, const Line3<T> & li, Point3<T> & p0,Point3<T> & p1 ){

    // Per prima cosa si sposta il sistema di riferimento 
    // fino a portare il centro della sfera nell'origine
    Point3<T> neworig=li.Origin()-sp.Center();
    // poi si risolve il sistema di secondo grado (con maple...)
    T t1 = li.Direction().X()*li.Direction().X();
    T t2 = li.Direction().Y()*li.Direction().Y();
    T t3 = li.Direction().Z()*li.Direction().Z();
    T t6 = neworig.Y()*li.Direction().Y();
    T t7 = neworig.X()*li.Direction().X();
    T t8 = neworig.Z()*li.Direction().Z();
    T t15 = sp.Radius()*sp.Radius();
    T t17 = neworig.Z()*neworig.Z();
    T t19 = neworig.Y()*neworig.Y();
    T t21 = neworig.X()*neworig.X();
    T t28 = T(2.0*t7*t6+2.0*t6*t8+2.0*t7*t8+t1*t15-t1*t17-t1*t19-t2*t21+t2*t15-t2*t17-t3*t21+t3*t15-t3*t19);
    if(t28<0) return false;
    T t29 = sqrt(t28);      
    T val0 = 1/(t1+t2+t3)*(-t6-t7-t8+t29); 
    T val1 = 1/(t1+t2+t3)*(-t6-t7-t8-t29);

    p0=li.P(val0);
    p1=li.P(val1);
    return true;
  }

	/*
	* Function computing the intersection between a sphere and a segment.
	* @param[in]	sphere				the sphere
	* @param[in]	segment				the segment
	* @param[out]	intersection  the intersection point, meaningful only if the segment intersects the sphere
	* \return			(0, 1 or 2)		the number of intersections between the segment and the sphere. 
	*														t1 is a valid intersection only if the returned value is at least 1; 
	*														similarly t2 is valid iff the returned value is 2.
	*/
	template < class SCALAR_TYPE >
	inline int IntersectionSegmentSphere(const Sphere3<SCALAR_TYPE>& sphere, const Segment3<SCALAR_TYPE>& segment, Point3<SCALAR_TYPE> & t0, Point3<SCALAR_TYPE> & t1)
	{
		typedef SCALAR_TYPE													ScalarType;
		typedef typename vcg::Point3< ScalarType >	Point3t;

		Point3t s = segment.P0() - sphere.Center();
		Point3t r = segment.P1() - segment.P0();

		ScalarType rho2 = sphere.Radius()*sphere.Radius();

		ScalarType sr = s*r;
		ScalarType r_squared_norm = r.SquaredNorm(); 
		ScalarType s_squared_norm = s.SquaredNorm(); 
		ScalarType sigma = sr*sr - r_squared_norm*(s_squared_norm-rho2);

		if (sigma<ScalarType(0.0)) // the line containing the edge doesn't intersect the sphere
			return 0;

		ScalarType sqrt_sigma = ScalarType(sqrt( ScalarType(sigma) ));
		ScalarType lambda1 = (-sr - sqrt_sigma)/r_squared_norm;
		ScalarType lambda2 = (-sr + sqrt_sigma)/r_squared_norm;

		int solution_count = 0;
		if (ScalarType(0.0)<=lambda1 && lambda1<=ScalarType(1.0))
		{
			ScalarType t_enter = vcg::math::Max< ScalarType >(lambda1, ScalarType(0.0));
			t0 = segment.P0() + r*t_enter;
			solution_count++;
		}

		if (ScalarType(0.0)<=lambda2 && lambda2<=ScalarType(1.0))
		{
			Point3t *pt = (solution_count>0) ? &t1 : &t0;
			ScalarType t_exit  = vcg::math::Min< ScalarType >(lambda2, ScalarType(1.0));
			*pt = segment.P0() + r*t_exit;
			solution_count++;
		}
		return solution_count;
	}; // end of IntersectionSegmentSphere

	
	/*!
	* Compute the intersection between a sphere and a triangle.
	* \param[in]	sphere		the input sphere
	* \param[in]	triangle	the input triangle
	* \param[out]	witness		it is the point on the triangle nearest to the center of the sphere (even when there isn't intersection)
	* \param[out] res				if not null, in the first item is stored the minimum distance between the triangle and the sphere,
	*                       while in the second item is stored the penetration depth
	* \return			true			iff there is an intersection between the sphere and the triangle
	*/
	template < class SCALAR_TYPE, class TRIANGLETYPE >
	bool IntersectionSphereTriangle(const vcg::Sphere3	< SCALAR_TYPE >		& sphere  ,
																	TRIANGLETYPE														triangle, 
																	vcg::Point3					< SCALAR_TYPE >		& witness ,
																	std::pair< SCALAR_TYPE, SCALAR_TYPE > * res=NULL)
	{
		typedef SCALAR_TYPE														ScalarType;
		typedef typename vcg::Point3< ScalarType >		Point3t;
		typedef TRIANGLETYPE Triangle3t;

		bool penetration_detected = false;

		ScalarType radius = sphere.Radius();
		Point3t	center = sphere.Center();
		Point3t p0 = triangle.P(0)-center;
		Point3t p1 = triangle.P(1)-center;
		Point3t p2 = triangle.P(2)-center;

		Point3t p10 = p1-p0;
		Point3t p21 = p2-p1;
		Point3t p20 = p2-p0;

		ScalarType delta0_p01 =  p10.dot(p1);
		ScalarType delta1_p01 = -p10.dot(p0);
		ScalarType delta0_p02 =  p20.dot(p2);
		ScalarType delta2_p02 = -p20.dot(p0);
		ScalarType delta1_p12 =  p21.dot(p2);
		ScalarType delta2_p12 = -p21.dot(p1);

		// the closest point can be one of the vertices of the triangle
		if			(delta1_p01<=ScalarType(0.0) && delta2_p02<=ScalarType(0.0)) { witness = p0; }
		else if (delta0_p01<=ScalarType(0.0) && delta2_p12<=ScalarType(0.0)) { witness = p1; }
		else if (delta0_p02<=ScalarType(0.0) && delta1_p12<=ScalarType(0.0)) { witness = p2; }
		else
		{
			ScalarType temp = p10.dot(p2);
			ScalarType delta0_p012 = delta0_p01*delta1_p12 + delta2_p12*temp;
			ScalarType delta1_p012 = delta1_p01*delta0_p02 - delta2_p02*temp;
			ScalarType delta2_p012 = delta2_p02*delta0_p01 - delta1_p01*(p20.dot(p1));

			// otherwise, can be a point lying on same edge of the triangle
			if (delta0_p012<=ScalarType(0.0)) 
			{
				ScalarType denominator = delta1_p12+delta2_p12;
				ScalarType mu1 = delta1_p12/denominator;
				ScalarType mu2 = delta2_p12/denominator;
				witness = (p1*mu1 + p2*mu2);
			}
			else if (delta1_p012<=ScalarType(0.0))
			{
				ScalarType denominator = delta0_p02+delta2_p02;
				ScalarType mu0 = delta0_p02/denominator;
				ScalarType mu2 = delta2_p02/denominator;
				witness = (p0*mu0 + p2*mu2);
			}
			else if (delta2_p012<=ScalarType(0.0))
			{
				ScalarType denominator = delta0_p01+delta1_p01;
				ScalarType mu0 = delta0_p01/denominator;
				ScalarType mu1 = delta1_p01/denominator;
				witness = (p0*mu0 + p1*mu1);
			}
			else
			{
				// or else can be an point internal to the triangle
				ScalarType denominator =  delta0_p012 + delta1_p012 + delta2_p012;
				ScalarType lambda0 = delta0_p012/denominator;
				ScalarType lambda1 = delta1_p012/denominator;
				ScalarType lambda2 = delta2_p012/denominator;
				witness = p0*lambda0 + p1*lambda1 + p2*lambda2;
			}
		}

		if (res!=NULL)
		{
			ScalarType witness_norm = witness.Norm();
			res->first  = vcg::math::Max< ScalarType >( witness_norm-radius, ScalarType(0.0) );
			res->second = vcg::math::Max< ScalarType >( radius-witness_norm, ScalarType(0.0) );
		}
		penetration_detected = (witness.SquaredNorm() <= (radius*radius));
		witness += center;
		return penetration_detected;
	}; //end of IntersectionSphereTriangle


  /// intersection between line and plane
  template<class T>
    inline bool IntersectionLinePlane( const Plane3<T> & pl, const Line3<T> & li, Point3<T> & po){
    const T epsilon = T(1e-8);

    T k = pl.Direction().dot(li.Direction());						// Compute 'k' factor
    if( (k > -epsilon) && (k < epsilon))
      return false;
    T r = (pl.Offset() - pl.Direction().dot(li.Origin()))/k;	// Compute ray distance
    po = li.Origin() + li.Direction()*r;
    return true;
  }
	
   /// intersection between segment and plane
  template<class T>
    inline bool IntersectionPlaneSegment( const Plane3<T> & pl, const Segment3<T> & s, Point3<T> & p0){
		T p1_proj = s.P1()*pl.Direction()-pl.Offset();
		T p0_proj = s.P0()*pl.Direction()-pl.Offset();
		if ( (p1_proj>0)-(p0_proj<0)) return false;
		p0 =  s.P0() + (s.P1()-s.P0()) * fabs(p0_proj/(p1_proj-p0_proj));
		return true;
  }

  /// intersection between segment and plane
  template<class ScalarType>
    inline bool IntersectionPlaneSegmentEpsilon(const Plane3<ScalarType> & pl,
                                                const Segment3<ScalarType> & sg,
                                                Point3<ScalarType> & po,
                                                const ScalarType epsilon = ScalarType(1e-8)){

    typedef ScalarType T;
    T k = pl.Direction().dot((sg.P1()-sg.P0()));
    if( (k > -epsilon) && (k < epsilon))
      return false;
    T r = (pl.Offset() - pl.Direction().dot(sg.P0()))/k;	// Compute ray distance
    if( (r<0) || (r > 1.0))
      return false;
    po = sg.P0()*(1-r)+sg.P1() * r;
    return true;
  }

  /// intersection between plane and triangle 
  // not optimal: uses plane-segment intersection (and the fact the two or none edges can be intersected)
  template<typename TRIANGLETYPE> 
    inline bool IntersectionPlaneTriangle( const Plane3<typename TRIANGLETYPE::ScalarType> & pl, 
			      const  TRIANGLETYPE & tr, 
            Segment3<typename TRIANGLETYPE::ScalarType> & sg)
    {
      typedef typename TRIANGLETYPE::ScalarType T;
      if(IntersectionPlaneSegment(pl,Segment3<T>(tr.P(0),tr.P(1)),sg.P0())){
        if(IntersectionPlaneSegment(pl,Segment3<T>(tr.P(0),tr.P(2)),sg.P1()))
          return true;
        else
        {
          IntersectionPlaneSegment(pl,Segment3<T>(tr.P(1),tr.P(2)),sg.P1());
          return true;
        }
      }
      else
      {
        if(IntersectionPlaneSegment(pl,Segment3<T>(tr.P(1),tr.P(2)),sg.P0()))
        {
          IntersectionPlaneSegment(pl,Segment3<T>(tr.P(0),tr.P(2)),sg.P1());
          return true;
        }
      }
      return false;
    }

  /// intersection between two triangles
  template<typename TRIANGLETYPE> 
    inline bool IntersectionTriangleTriangle(const TRIANGLETYPE & t0,const TRIANGLETYPE & t1){
    return NoDivTriTriIsect(t0.P0(0),t0.P0(1),t0.P0(2),
			    t1.P0(0),t1.P0(1),t1.P0(2));
  }

  template<class T>
    inline bool IntersectionTriangleTriangle(Point3<T> V0,Point3<T> V1,Point3<T> V2,
			     Point3<T> U0,Point3<T> U1,Point3<T> U2){
    return NoDivTriTriIsect(V0,V1,V2,U0,U1,U2);
  }
#if 0
  template<class T>
    inline bool Intersection(Point3<T> V0,Point3<T> V1,Point3<T> V2,
			     Point3<T> U0,Point3<T> U1,Point3<T> U2,int *coplanar,
			     Point3<T> &isectpt1,Point3<T> &isectpt2){

    return tri_tri_intersect_with_isectline(V0,V1,V2,U0,U1,U2,
					    coplanar,isectpt1,isectpt2);
  }

  template<typename TRIANGLETYPE,typename SEGMENTTYPE >
    inline bool Intersection(const TRIANGLETYPE & t0,const TRIANGLETYPE & t1,bool &coplanar,
			     SEGMENTTYPE  & sg){
    Point3<typename SEGMENTTYPE::PointType> ip0,ip1; 
    return  tri_tri_intersect_with_isectline(t0.P0(0),t0.P0(1),t0.P0(2),
					     t1.P0(0),t1.P0(1),t1.P0(2),
					     coplanar,sg.P0(),sg.P1()
					     );              
  }

#endif	
	 
	/*
	* Function computing the intersection between a line and a triangle.
	* from: 
	* Tomas Moller and Ben Trumbore,  
	* ``Fast, Minimum Storage Ray-Triangle Intersection'',	
	* journal of graphics tools, vol. 2, no. 1, pp. 21-28, 1997
	* @param[in]	line				 
	* @param[in]	triangle vertices			 
	* @param[out]=(t,u,v)	the intersection point, meaningful only if the line intersects the triangle
	*            t is the line parameter and
	*            (u,v) are the baricentric coords of the intersection point
	*
	*                 Line.Orig + t * Line.Dir = (1-u-v) * Vert0 + u * Vert1 +v * Vert2 
	*
	*/

template<class T>
bool IntersectionLineTriangle( const Line3<T> & line, const Point3<T> & vert0, 
				  const Point3<T> & vert1, const Point3<T> & vert2,
				  T & t ,T & u, T & v)
{
	#define EPSIL 0.000001

	vcg::Point3<T> edge1, edge2, tvec, pvec, qvec;
	T det,inv_det;

   /* find vectors for two edges sharing vert0 */
   edge1 = vert1 - vert0;
   edge2 = vert2 - vert0;

   /* begin calculating determinant - also used to calculate U parameter */
   pvec =  line.Direction() ^ edge2;

   /* if determinant is near zero, line lies in plane of triangle */
   det =  edge1 *  pvec;

   /* calculate distance from vert0 to line origin */
   tvec = line.Origin() - vert0;
   inv_det = 1.0 / det;
   
   qvec = tvec ^ edge1;
      
   if (det > EPSIL)
   {
      u =  tvec * pvec ;
      if ( u < 0.0 ||  u > det)
	 return 0;
            
      /* calculate V parameter and test bounds */
       v =  line.Direction() * qvec;
      if ( v < 0.0 ||  u +  v > det)
	 return 0;
      
   }
   else if(det < -EPSIL)
   {
      /* calculate U parameter and test bounds */
       u =  tvec * pvec ;
      if ( u > 0.0 ||  u < det)
	 return 0;
      
      /* calculate V parameter and test bounds */
       v =  line.Direction() * qvec  ;
      if ( v > 0.0 ||  u +  v < det)
	 return 0;
   }
   else return 0;  /* line is parallell to the plane of the triangle */

    t = edge2 *  qvec  * inv_det;
   ( u) *= inv_det;
   ( v) *= inv_det;

   return 1;
}

template<class T>
bool IntersectionRayTriangle( const Ray3<T> & ray, const Point3<T> & vert0, 
				  const Point3<T> & vert1, const Point3<T> & vert2,
				  T & t ,T & u, T & v)
{
	Line3<T> line(ray.Origin(), ray.Direction());
	if (IntersectionLineTriangle(line, vert0, vert1, vert2, t, u, v))
	{
		if (t < 0) return 0;
		else return 1; 
	}else return 0;
}

// line-box
template<class T>
bool IntersectionLineBox( const Box3<T> & box, const Line3<T> & r, Point3<T> & coord )
{
	const int NUMDIM = 3;
	const int RIGHT  = 0;
	const int LEFT	 = 1;
	const int MIDDLE = 2;

	int inside = 1;
	char quadrant[NUMDIM];
    int i;
    int whichPlane;
    Point3<T> maxT,candidatePlane;
    
	// Find candidate planes; this loop can be avoided if
   	// rays cast all from the eye(assume perpsective view)
    for (i=0; i<NUMDIM; i++)
    {
        if(r.Origin()[i] < box.min[i])
		{
			quadrant[i] = LEFT;
			candidatePlane[i] = box.min[i];
			inside = 0;
		}
		else if (r.Origin()[i] > box.max[i])
		{
			quadrant[i] = RIGHT;
			candidatePlane[i] = box.max[i];
			inside = 0;
		}
		else
		{
			quadrant[i] = MIDDLE;
		}
    }

		// Ray origin inside bounding box
	if(inside){
	    coord = r.Origin();
	    return true;
	}

	// Calculate T distances to candidate planes 
    for (i = 0; i < NUMDIM; i++)
    {
		if (quadrant[i] != MIDDLE && r.Direction()[i] !=0.)
			maxT[i] = (candidatePlane[i]-r.Origin()[i]) / r.Direction()[i];
		else
			maxT[i] = -1.;
    }

	// Get largest of the maxT's for final choice of intersection
    whichPlane = 0;
    for (i = 1; i < NUMDIM; i++)
	    if (maxT[whichPlane] < maxT[i])
			whichPlane = i;

	// Check final candidate actually inside box 
    if (maxT[whichPlane] < 0.) return false;
    for (i = 0; i < NUMDIM; i++)
		if (whichPlane != i)
		{
			coord[i] = r.Origin()[i] + maxT[whichPlane] *r.Direction()[i];
			if (coord[i] < box.min[i] || coord[i] > box.max[i])
				return false;
		}
		else
		{
			coord[i] = candidatePlane[i];
		}
    return true;			// ray hits box
}	

// ray-box
template<class T>
bool IntersectionRayBox( const Box3<T> & box, const Ray3<T> & r, Point3<T> & coord )
{
	Line3<T> l;
	l.SetOrigin(r.Origin());
	l.SetDirection(r.Direction());
  return(IntersectionLineBox<T>(box,l,coord));
}	

// segment-box return fist intersection found  from p0 to p1
template<class ScalarType>
bool IntersectionSegmentBox( const Box3<ScalarType> & box,
							  const Segment3<ScalarType> & s, 
							  Point3<ScalarType> & coord )
{
	//as first perform box-box intersection
	Box3<ScalarType> test;
	test.Add(s.P0());
	test.Add(s.P1());
	if (!test.Collide(box))
		return false;
	else
	{
		Line3<ScalarType> l;
		Point3<ScalarType> dir=s.P1()-s.P0();
		dir.Normalize();
		l.SetOrigin(s.P0());
		l.SetDirection(dir);
    if(IntersectionLineBox<ScalarType>(box,l,coord))
			return (test.IsIn(coord));
		return false;
	}
}

// segment-box intersection , return number of intersections and intersection points
template<class ScalarType>
int IntersectionSegmentBox( const Box3<ScalarType> & box,
							 const Segment3<ScalarType> & s,
							 Point3<ScalarType> & coord0,
							 Point3<ScalarType> & coord1 )
{
	int num=0;
	Segment3<ScalarType> test= s;
  if (IntersectionSegmentBox(box,test,coord0 ))
	{
		num++;
		Point3<ScalarType> swap=test.P0();
		test.P0()=test.P1();
		test.P1()=swap;
    if (IntersectionSegmentBox(box,test,coord1 ))
			num++;
	}
	return num;
}	

/**
* Compute the intersection between a segment and a triangle.
* It relies on the lineTriangle Intersection
* @param[in]	segment
* @param[in]	triangle vertices
* @param[out]=(t,u,v)	the intersection point, meaningful only if the line of segment intersects the triangle
*            t     is the baricentric coord of the point on the segment
*            (u,v) are the baricentric coords of the intersection point in the segment
*
*/
template<class ScalarType>
bool IntersectionSegmentTriangle( const vcg::Segment3<ScalarType> & seg,
								   const Point3<ScalarType> & vert0, 
									const Point3<ScalarType> & vert1, const
									Point3<ScalarType> & vert2,
									ScalarType & a ,ScalarType & b, ScalarType & dist)
{
	//control intersection of bounding boxes
	vcg::Box3<ScalarType> bb0,bb1;
	bb0.Add(seg.P0());
	bb0.Add(seg.P1());
	bb1.Add(vert0);
	bb1.Add(vert1);
	bb1.Add(vert2);
	Point3<ScalarType> inter;
	if (!bb0.Collide(bb1))
		return false;
  if (!vcg::IntersectionSegmentBox(bb1,seg,inter))
		return false;

	//first set both directions of ray
  vcg::Line3<ScalarType> line;
	vcg::Point3<ScalarType> dir;
	dir=(seg.P1()-seg.P0());
	dir.Normalize();
  line.Set(seg.P0(),dir);
  if(IntersectionLineTriangle<ScalarType>(line,vert0,vert1,vert2,dist,a,b))
    return (dist>=0 && dist<=1.0);
    return false;
}
/**
* Compute the intersection between a segment and a triangle.
* Wrapper of the above function
*/
template<class TriangleType>
bool IntersectionSegmentTriangle( const vcg::Segment3<typename TriangleType::ScalarType> & seg,
                  const TriangleType &t,
                  typename TriangleType::ScalarType & a ,typename TriangleType::ScalarType & b, typename TriangleType::ScalarType & dist)
{
  return IntersectionSegmentTriangle(seg,t.P(0),t.P(1),t.P(2),a,b,dist);
}

template<class ScalarType>
bool IntersectionPlaneBox(const vcg::Plane3<ScalarType> &pl,
							vcg::Box3<ScalarType> &bbox)
{
	typedef typename vcg::Segment3<ScalarType> SegmentType;
	typedef typename vcg::Point3<ScalarType> CoordType;
	SegmentType diag[4];
	
	CoordType intersection;
	//find the 4 diagonals
	diag[0]=SegmentType(bbox.P(0),bbox.P(7));
	diag[1]=SegmentType(bbox.P(1),bbox.P(6));
	diag[2]=SegmentType(bbox.P(2),bbox.P(5));
	diag[3]=SegmentType(bbox.P(3),bbox.P(4));
	ScalarType a,b,dist;
	for (int i=0;i<3;i++)
		//call intersection of segment and plane
    if (vcg::IntersectionPlaneSegment(pl,diag[i],intersection))
			return true;
	return false;
}

///if exists return the center and ardius of circle
///that is the intersectionk between the sphere and 
//the plane
template <class ScalarType>
bool IntersectionPlaneSphere(const Plane3<ScalarType> &pl,
							   const Sphere3<ScalarType> &sphere,
							   Point3<ScalarType> &center,
							   ScalarType &radius)
{
	/* ///set the origin on the center of the sphere
	vcg::Plane3<ScalarType> pl1;
	vcg::Point3<ScalarType> p_plane=pl.Direction()*pl.Offset();
	vcg::Point3<ScalarType> p_plane=p_plane-sphere.Center();

	///set again the plane
	pl1.Set(p_plane,pl.Direction());

	///test d must be positive
	d=pl1.Offset();
	vcg::Point3<ScalarType> n=pl1.Direction();
	///invert d if is <0
	if (d<0)
	{
		n=-n;
		d=-d;
	}
	///no intersection occour
	if (d>r)
		return false;
	else
	{
		///calculate center and translate in back
		center=n*d;
		center+=sphere.Center();
		radius=math::Sqrt((r*r)-(d*d));
	}
	 */
	 assert(0);
	 return true;
}


template<class ScalarType>
bool IntersectionTriangleBox(const vcg::Box3<ScalarType>   &bbox,
							   const vcg::Point3<ScalarType> &p0,
							   const vcg::Point3<ScalarType> &p1,
							   const vcg::Point3<ScalarType> &p2)
{
	typedef typename vcg::Point3<ScalarType> CoordType;
	CoordType intersection;

	/// control bounding box collision
	vcg::Box3<ScalarType> test;
	test.SetNull();
	test.Add(p0);
	test.Add(p1);
	test.Add(p2);
	if (!test.Collide(bbox))
		return false;
	/// control if each point is inside the bouding box
	if ((bbox.IsIn(p0))||(bbox.IsIn(p1))||(bbox.IsIn(p2)))
		return true;

	/////control plane of the triangle with bbox
	//vcg::Plane3<ScalarType> plTri=vcg::Plane3<ScalarType>();
	//plTri.Init(p0,p1,p2);
  //if (!IntersectionPlaneBox<ScalarType>(plTri,bbox))
	//	return false;

	///then control intersection of segments with box
  if (IntersectionSegmentBox<ScalarType>(bbox,vcg::Segment3<ScalarType>(p0,p1),intersection)||
    IntersectionSegmentBox<ScalarType>(bbox,vcg::Segment3<ScalarType>(p1,p2),intersection)||
    IntersectionSegmentBox<ScalarType>(bbox,vcg::Segment3<ScalarType>(p2,p0),intersection))
		return true;
	///control intersection of diagonal of the cube with triangle

	Segment3<ScalarType> diag[4];

	diag[0]=Segment3<ScalarType>(bbox.P(0),bbox.P(7));
	diag[1]=Segment3<ScalarType>(bbox.P(1),bbox.P(6));
	diag[2]=Segment3<ScalarType>(bbox.P(2),bbox.P(5));
	diag[3]=Segment3<ScalarType>(bbox.P(3),bbox.P(4));
	ScalarType a,b,dist;
	for (int i=0;i<3;i++)
    if (IntersectionSegmentTriangle<ScalarType>(diag[i],p0,p1,p2,a,b,dist))
			return true;

	return false;
}

template <class SphereType>
bool IntersectionSphereSphere( const SphereType & s0,const SphereType & s1){
	return (s0.Center()-s1.Center()).SquaredNorm() < (s0.Radius()+s1.Radius())*(s0.Radius()+s1.Radius());
}

template<class T>
bool IntersectionPlanePlane (const Plane3<T> & plane0, const Plane3<T> & plane1,
                             Line3<T> & line)
{
    // If Cross(N0,N1) is zero, then either planes are parallel and separated
    // or the same plane.  In both cases, 'false' is returned.  Otherwise,
    // the intersection line is
    //
    //   L(t) = t*Cross(N0,N1) + c0*N0 + c1*N1
    //
    // for some coefficients c0 and c1 and for t any real number (the line
    // parameter).  Taking dot products with the normals,
    //
    //   d0 = Dot(N0,L) = c0*Dot(N0,N0) + c1*Dot(N0,N1)
    //   d1 = Dot(N1,L) = c0*Dot(N0,N1) + c1*Dot(N1,N1)
    //
    // which are two equations in two unknowns.  The solution is
    //
    //   c0 = (Dot(N1,N1)*d0 - Dot(N0,N1)*d1)/det
    //   c1 = (Dot(N0,N0)*d1 - Dot(N0,N1)*d0)/det
    //
    // where det = Dot(N0,N0)*Dot(N1,N1)-Dot(N0,N1)^2.

    T n00 = plane0.Direction()*plane0.Direction();
    T n01 = plane0.Direction()*plane1.Direction();
    T n11 = plane1.Direction()*plane1.Direction();
    T det = n00*n11-n01*n01;

    const T tolerance = (T)(1e-06f);
		if ( math::Abs(det) < tolerance )
        return false;

    T invDet = 1.0f/det;
    T c0 = (n11*plane0.Offset() - n01*plane1.Offset())*invDet;
    T c1 = (n00*plane1.Offset() - n01*plane0.Offset())*invDet;

    line.SetDirection(plane0.Direction()^plane1.Direction());
    line.SetOrigin(plane0.Direction()*c0+ plane1.Direction()*c1);

    return true;
}


// Ray-Triangle Functor
template <bool BACKFACETEST = true>
class RayTriangleIntersectionFunctor {
public:
	template <class TRIANGLETYPE, class SCALARTYPE>
	inline bool operator () (const TRIANGLETYPE & f, const Ray3<SCALARTYPE> & ray, SCALARTYPE & t) {
		typedef SCALARTYPE ScalarType;
		ScalarType u;
		ScalarType v;

		bool bret = IntersectionRayTriangle(ray, Point3<SCALARTYPE>::Construct(f.P(0)), Point3<SCALARTYPE>::Construct(f.P(1)), Point3<SCALARTYPE>::Construct(f.P(2)), t, u, v);
		if (BACKFACETEST) {
			if (!bret) {
				bret = IntersectionRayTriangle(ray, Point3<SCALARTYPE>::Construct(f.P(0)), Point3<SCALARTYPE>::Construct(f.P(2)), Point3<SCALARTYPE>::Construct(f.P(1)), t, u, v);
			}
		}
		return (bret);
	}
};
	

/*@}*/


} // end namespace
#endif
