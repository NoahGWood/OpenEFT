#pragma once

#include "SyntopiaCore/Math/Vector3.h"
#include <QGLWidget>
#include <QtOpenGL>

namespace SyntopiaCore {
	namespace GLEngine {	

		/// Every Primitive can be assigned a class.
		struct PrimitiveClass {
			PrimitiveClass() : reflection(0), hasShadows(true), castShadows(true), 
				ambientOcclusionMin(0), ambientOcclusionMax(0), ambientOcclusionPrecision(0.95), ambientOcclusionMaxLength(0) {};
			QString name;
			double reflection;
			bool hasShadows;
			bool castShadows;
			int ambientOcclusionMin;
			int ambientOcclusionMax;
			double ambientOcclusionPrecision;
			double ambientOcclusionMaxLength;
			double ambient;
			double specular;
			double diffuse;
		};

		/// Used by the raytracer, when tracing rays.
		struct RayInfo {
			SyntopiaCore::Math::Vector3f startPoint;
			SyntopiaCore::Math::Vector3f lineDirection;

			// 'Return' variables - if a hit is found these will be overwritten.
			float intersection;
			SyntopiaCore::Math::Vector3f normal;
			GLfloat color[4];
		};

		class Object3D {
		public:
			Object3D() : lastRayID(-1) {};
			virtual ~Object3D() {};

			virtual QString name() { return "Object3D base"; }
			virtual void draw() const = 0;

			void setColor(SyntopiaCore::Math::Vector3f rgb, float alpha);
			const GLfloat (&getColor() const)[4] { return primaryColor; }

			void getBoundingBox(SyntopiaCore::Math::Vector3f& from, SyntopiaCore::Math::Vector3f& to) const;
			void expandBoundingBox(SyntopiaCore::Math::Vector3f& from, SyntopiaCore::Math::Vector3f& to) const;

			/// These must be implemented for an Object3D to support raytracing.
			virtual bool intersectsRay(RayInfo* /*rayInfo*/) { return false; };
			virtual bool intersectsAABB(SyntopiaCore::Math::Vector3f /*from*/, SyntopiaCore::Math::Vector3f /*to*/) { return false; };
			virtual void prepareForRaytracing() { }; // Implement for additional preprocessing
			
			int getLastRayID() { return lastRayID; }
			void setLastRayID(int id) { lastRayID = id; }

			PrimitiveClass* getPrimitiveClass() { return primitiveClass; }
			void setPrimitiveClass(PrimitiveClass* value) { primitiveClass = value; }
			
		protected:

			void vertex(SyntopiaCore::Math::Vector3f v) const { glVertex3f(v.x(), v.y(), v.z()); }
			void normal(SyntopiaCore::Math::Vector3f v) const { glNormal3f(v.x(), v.y(), v.z()); }
			void vertex4(SyntopiaCore::Math::Vector3f v1,SyntopiaCore::Math::Vector3f v2,SyntopiaCore::Math::Vector3f v3,SyntopiaCore::Math::Vector3f v4) const { vertex(v1); vertex(v2); vertex(v3); vertex(v4); }
			void vertex3n(SyntopiaCore::Math::Vector3f v1,SyntopiaCore::Math::Vector3f v2,SyntopiaCore::Math::Vector3f v3) const;
			void vertex4r(SyntopiaCore::Math::Vector3f v1,SyntopiaCore::Math::Vector3f v2,SyntopiaCore::Math::Vector3f v3,SyntopiaCore::Math::Vector3f v4) const { vertex(v4); vertex(v3); vertex(v2); vertex(v1); }
			
			
			void vertex4(const GLfloat* col1, SyntopiaCore::Math::Vector3f c1,SyntopiaCore::Math::Vector3f v1,SyntopiaCore::Math::Vector3f v2,const GLfloat* col2, SyntopiaCore::Math::Vector3f c2, SyntopiaCore::Math::Vector3f v3,SyntopiaCore::Math::Vector3f v4, bool reverse) const;

			void vertex4n(SyntopiaCore::Math::Vector3f v1,SyntopiaCore::Math::Vector3f v2,SyntopiaCore::Math::Vector3f v3,SyntopiaCore::Math::Vector3f v4) const;
			void vertex4rn(SyntopiaCore::Math::Vector3f v1,SyntopiaCore::Math::Vector3f v2,SyntopiaCore::Math::Vector3f v3,SyntopiaCore::Math::Vector3f v4) const ;
			void vertex4nc(SyntopiaCore::Math::Vector3f v1,SyntopiaCore::Math::Vector3f v2,SyntopiaCore::Math::Vector3f v3,SyntopiaCore::Math::Vector3f v4,SyntopiaCore::Math::Vector3f center) const ;
			void vertex4rnc(SyntopiaCore::Math::Vector3f v1,SyntopiaCore::Math::Vector3f v2,SyntopiaCore::Math::Vector3f v3,SyntopiaCore::Math::Vector3f v4,SyntopiaCore::Math::Vector3f center) const;

			GLfloat primaryColor[4];

			// Bounding box
			SyntopiaCore::Math::Vector3f from;
			SyntopiaCore::Math::Vector3f to;
			
			// Used by Voxel Stepper when raytracing.
			int lastRayID;

			PrimitiveClass* primitiveClass;

		};

	}
}

