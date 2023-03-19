#pragma once

#include "SyntopiaCore/Math/Vector3.h"
#include "Object3D.h"

namespace SyntopiaCore {
	namespace GLEngine {	

		class Triangle : public Object3D {
		public:
			Triangle(SyntopiaCore::Math::Vector3f p1 , 
				 SyntopiaCore::Math::Vector3f p2, 
				 SyntopiaCore::Math::Vector3f p3);

			virtual ~Triangle();

			virtual QString name() { return "Triangle"; }
			

			virtual void draw() const;

		private:
			SyntopiaCore::Math::Vector3f p1;
			SyntopiaCore::Math::Vector3f p2;
			SyntopiaCore::Math::Vector3f p3;
		};

	}
}

