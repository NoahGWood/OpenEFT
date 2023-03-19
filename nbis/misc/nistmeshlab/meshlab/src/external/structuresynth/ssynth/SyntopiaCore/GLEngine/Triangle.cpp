#include "Triangle.h"

#include "SyntopiaCore/Math/Vector3.h"

using namespace SyntopiaCore::Math;

namespace SyntopiaCore {
	namespace GLEngine {


		Triangle::Triangle(SyntopiaCore::Math::Vector3f p1 , 
				 SyntopiaCore::Math::Vector3f p2, 
				 SyntopiaCore::Math::Vector3f p3) : p1(p1), p2(p2), p3(p3) 
		{
			/// Bounding box (very approximate :-) )
			from = p1;
			to = p3;
		};

		Triangle::~Triangle() { };

		void Triangle::draw() const {
			glPushMatrix();
			
			glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, primaryColor );
			glPolygonMode(GL_FRONT, GL_FILL);
				
			glEnable(GL_CULL_FACE);
			glEnable (GL_LIGHTING);

			glEnable (GL_BLEND);
			glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			
			glMateriali(GL_FRONT, GL_SPECULAR, 30);
			glMateriali(GL_FRONT, GL_SHININESS, 127);
			
			glBegin(GL_TRIANGLES);
			vertex3n(p1,p2,p3);
			glEnd();	
			
			glPopMatrix();			
		};

	}
}

