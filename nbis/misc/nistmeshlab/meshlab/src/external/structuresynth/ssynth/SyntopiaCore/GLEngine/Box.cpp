#include "Box.h"

#include "SyntopiaCore/Math/Vector3.h"

using namespace SyntopiaCore::Math;

namespace SyntopiaCore {
	namespace GLEngine {


		namespace {
			void Expand(Vector3f& from, Vector3f& to, Vector3f test) {
				if (test.x()<from.x()) from.x() = test.x();
				if (test.y()<from.y()) from.y() = test.y();
				if (test.z()<from.z()) from.z() = test.z();
				if (test.x()>to.x()) to.x() = test.x();
				if (test.y()>to.y()) to.y() = test.y();
				if (test.z()>to.z()) to.z() = test.z();
			}
		}
		Box::Box(SyntopiaCore::Math::Vector3f base, 
				SyntopiaCore::Math::Vector3f dir1 , 
				 SyntopiaCore::Math::Vector3f dir2, 
				 SyntopiaCore::Math::Vector3f dir3) : base(base), v1(dir1), v2(dir2), v3(dir3) 
		{
			/// Bounding box
			from = base;
			to = base;

			Expand(from,to, base+v1);
			Expand(from,to, base+v2);
			Expand(from,to, base+v3);
			
			Expand(from,to, base+v1+v2);
			Expand(from,to, base+v2+v3);
			Expand(from,to, base+v1+v3);
			Expand(from,to, base+v1+v2+v3);
			
			/*
			Vector3f c = (from+to)/2.0;
			Vector3f c2 = (to-from)/2.0;
			from = c - c2*1.5;
			to = c + c2*1.5;
			*/

			n21 = Vector3f::cross(v2,v1).normalized();
			n32 = Vector3f::cross(v3,v2).normalized();
			n13 = Vector3f::cross(v1,v3).normalized();

			
			
			
			
			ac = base + v1*0.5 + v2*0.5 + v3*0.5;
			a[3];
			a[0] = v1.normalized();
			a[1] = v2.normalized();
			a[2] = v3.normalized();
			h[0] = v1.length()/2;
			h[1] = v2.length()/2;
			h[2] = v3.length()/2;
		};

		Box::~Box() { };

		

		void Box::draw() const {
			glPushMatrix();
			glTranslatef( base.x(), base.y(), base.z() );
			
			glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, primaryColor );
			
			glBegin( GL_QUADS );
			Vector3f O(0,0,0);
			vertex4n(O, v2,v2+v1,v1);
			vertex4rn(O+v3, v2+v3, v2+v1+v3, v1+v3);
			vertex4n(O, v3, v3+v2,v2);
			vertex4rn(O+v1, v3+v1, v3+v2+v1, v2+v1);
			vertex4n(O, v1, v3+v1, v3);
			vertex4rn(O+v2, v1+v2, v3+v2+v1, v3+v2);
			glEnd();
			
			glPopMatrix();			
		};

		
		bool Box::intersectsRay(RayInfo* ri) {
			// Following the Real-Time Rendering book p. 574
			
			float tmin = -1.0E37f;
			float tmax = 1.0E37f;
			float temp = 0;
			Vector3f p = ac - ri->startPoint;
			int minhit = 0;
			int maxhit = 0;
			for (int i = 0; i < 3; i++) {
				bool reverse = false;
			
				float e = Vector3f::dot(a[i], p);
				float f = Vector3f::dot(a[i], ri->lineDirection);
				if (fabs(f)>1E-17) {
					float t1 = (e+h[i])/f;
					float t2 = (e-h[i])/f;
					if (t1 > t2) { temp = t1; t1 = t2; t2 = temp; reverse = true; }
					if (t1 > tmin) { 
						tmin = t1;
						minhit = reverse ? (i+1) : -i;
					}
					if (t2 < tmax) { 
						tmax = t2;
						maxhit = reverse ? (i+1) : -i;
					}
					if (tmin > tmax) return false;
					if (tmax < 0) return false;
					
				} else {
					if ( (-e-h[i] > 0) || (-e+h[i]<0)) return false;
				}
			}
			if (tmin>0) {
				ri->intersection = tmin;
				for (int i = 0; i < 4; i++) ri->color[i] = primaryColor[i];
				// TODO: Find a better solution or at least make a switch...
				if (minhit == 1) { ri->normal = n32;   }
				else if (minhit == 2)  { ri->normal = n13;  }
				else if (minhit == 3) { ri->normal = n21;  }
				else if (minhit == 0) { ri->normal = -n32;   }
				else if (minhit == -1)  { ri->normal = -n13; }
				else if (minhit == -2) { ri->normal = -n21;  }
				
				return true;
			} else {
				ri->intersection = tmax;
				for (int i = 0; i < 4; i++) ri->color[i] = primaryColor[i];
				if (maxhit == 0) {ri->normal = n32; ri->color[2] = 1; }
				else if (maxhit == 1){ ri->normal = n13; ri->color[2] = 1; }
				else { ri->normal = n21; ri->color[2] = 1; }
				 ri->color[0] = 0; ri->color[1] = 1; ri->color[2] = 1;
				return true;
			}

		}

		
/*
		bool Box::intersectsRay(RayInfo* ri) {

			if (Vector3f::dot(n21, ri->lineDirection) < 0) {
				double is  = Vector3f::dot(n21, base-ri->startPoint)/Vector3f::dot(n21, ri->lineDirection);
				if (is > 0  && is < 1) {
					Vector3f ip = ri->startPoint + ri->lineDirection * is;
					float t1 = Vector3f::dot(ip-base, v1);
					if (t1 > 0 && t1 < v1.sqrLength()) {
						float t2 = Vector3f::dot(ip-base, v2);
						if ( t2 > 0 && t2 < v2.sqrLength()) {
							for (int i = 0; i < 4; i++) ri->color[i] = primaryColor[i];
							//ri->color[0] = 0;
							ri->intersection = is;
							ri->normal = n21;
							return true;
						}
					}
				}
			} else {
				double is  = Vector3f::dot(n21, (base+v3)-ri->startPoint)/Vector3f::dot(n21, ri->lineDirection);
				if (is > 0  && is < 1) {
					Vector3f ip = ri->startPoint + ri->lineDirection * is;
					float t1 = Vector3f::dot(ip-(base+v3), v1);
					if (t1 > 0 && t1 < v1.sqrLength()) {
						float t2 = Vector3f::dot(ip-(base+v3), v2);
						if ( t2 > 0 && t2 < v2.sqrLength()) {
							for (int i = 0; i < 4; i++) ri->color[i] = primaryColor[i];
							//ri->color[0] = 1;
							
							ri->intersection = is;
							ri->normal = -n21;
							return true;
						}
					}
				}
			}

			if (Vector3f::dot(n32, ri->lineDirection) < 0) {
				double is  = Vector3f::dot(n32, base-ri->startPoint)/Vector3f::dot(n32, ri->lineDirection);
				if (is > 0 && is < 1) {
					Vector3f ip = ri->startPoint + ri->lineDirection * is;
					float t1 = Vector3f::dot(ip-base, v2);
					if (t1 > 0 && t1 < v2.sqrLength()) {
						float t2 = Vector3f::dot(ip-base, v3);
						if ( t2 > 0 && t2 < v3.sqrLength()) {
							for (int i = 0; i < 4; i++) ri->color[i] = primaryColor[i];
							//ri->color[1] = 0;
							
							ri->intersection = is;
							ri->normal = n32;
							return true;
						}
					}
				}
			} else {
				double is  = Vector3f::dot(n32, (base+v1)-ri->startPoint)/Vector3f::dot(n32, ri->lineDirection);
				if (is > 0  && is < 1) {
					Vector3f ip = ri->startPoint + ri->lineDirection * is;
					float t1 = Vector3f::dot(ip-(base+v1), v2);
					if (t1 > 0 && t1 < v2.sqrLength()) {
						float t2 = Vector3f::dot(ip-(base+v1), v3);
						if ( t2 > 0 && t2 < v3.sqrLength()) {
							for (int i = 0; i < 4; i++) ri->color[i] = primaryColor[i];
							//ri->color[1] = 1;
							
							ri->intersection = is;
							ri->normal = -n32;
							return true;
						}
					}
				}
			}


			if (Vector3f::dot(n13, ri->lineDirection) < 0) {
				double is  = Vector3f::dot(n13, base-ri->startPoint)/Vector3f::dot(n13, ri->lineDirection);
				if (is > 0  && is < 1) {
					Vector3f ip = ri->startPoint + ri->lineDirection * is;
					float t1 = Vector3f::dot(ip-base, v3);
					if (t1 > 0 && t1 < v3.sqrLength()) {
						float t2 = Vector3f::dot(ip-base, v1);
						if ( t2 > 0 && t2 < v1.sqrLength()) {
							for (int i = 0; i < 4; i++) ri->color[i] = primaryColor[i];
							//ri->color[2] = 0;
							
							ri->intersection = is;
							ri->normal = n13;
							return true;
						}
					}
				}
			} else {
				double is  = Vector3f::dot(n13, (base+v2)-ri->startPoint)/Vector3f::dot(n13, ri->lineDirection);
				if (is > 0  && is < 1) {
					Vector3f ip = ri->startPoint + ri->lineDirection * is;
					float t1 = Vector3f::dot(ip-(base+v2), v3);
					if (t1 > 0 && t1 < v3.sqrLength()) {
						float t2 = Vector3f::dot(ip-(base+v2), v1);
						if ( t2 > 0 && t2 < v1.sqrLength()) {
							for (int i = 0; i < 4; i++) ri->color[i] = primaryColor[i];
							//ri->color[2] = 1;
							
							ri->intersection = is;
							ri->normal = -n13;
							return true;
						}
					}
				}
			}


			return false;
			
		}
		
*/

		bool Box::intersectsAABB(SyntopiaCore::Math::Vector3f from2, SyntopiaCore::Math::Vector3f to2) {
			return
					    (from.x() < to2.x()) && (to.x() > from2.x()) &&
						(from.y() < to2.y()) && (to.y() > from2.y()) &&
						(from.z() < to2.z()) && (to.z() > from2.z());
		}
			
	}
}

