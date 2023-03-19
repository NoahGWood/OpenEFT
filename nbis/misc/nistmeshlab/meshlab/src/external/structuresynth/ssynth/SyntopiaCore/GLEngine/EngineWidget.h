#pragma once

#include <QVector>
#include <QGLWidget>
#include <QPoint>
#include <QList>

#include "SyntopiaCore/Math/Vector3.h"
#include "SyntopiaCore/Math/Matrix4.h"

#include "Object3D.h"

namespace SyntopiaCore {
	namespace GLEngine {	

		struct Command {
			Command() {};
			Command(QString command, QString arg) : command(command), arg(arg) {};
			QString command;
			QString arg;
		};

		
		/// Settings for the GLEngine
		class Settings {
		public:

			// Default constructor
			Settings() {
				perspectiveAngle = 22.0;
				nearClipping = 5.0;
				farClipping = 60.0;
			}

			// Projection settings
			double perspectiveAngle;
			double nearClipping;
			double farClipping;
		};


		/// Widget for the mini OpenGL engine.
		class EngineWidget : public QGLWidget {
		public:
			/// Constructor
			EngineWidget(QWidget* parent);

			/// Destructor
			~EngineWidget();

			/// Use this whenever the an redraw is required.
			/// Calling this function multiple times will still only result in one redraw
			void requireRedraw();

			void clearWorld();
			void reset();
			void addObject(Object3D* object);
			QList<Object3D*> getObjects() { return objects; };
			
			int objectCount() const { return objects.size(); }

			SyntopiaCore::Math::Vector3f getPivot() { return pivot; }
			SyntopiaCore::Math::Matrix4f getRotation() { return rotation; }
			SyntopiaCore::Math::Vector3f getTranslation() { return translation; }
			SyntopiaCore::Math::Vector3f getCameraPosition();
			SyntopiaCore::Math::Vector3f getCameraUp();
			SyntopiaCore::Math::Vector3f getCameraTarget();
			double getScale() { return scale; }

			void setPivot(SyntopiaCore::Math::Vector3f pivot) { this->pivot = pivot; }
			void setRotation(SyntopiaCore::Math::Matrix4f rotation) { this->rotation = rotation; }
			void setTranslation(SyntopiaCore::Math::Vector3f translation) { this->translation = translation; }
		    void setScale(double scale) { this->scale = scale; }

			/// RGB in [0;1]
			void setBackgroundColor(double r, double g, double b) { 
				backgroundColor = QColor((int)(r*255.0), (int)(g*255.0), (int)(b*255.0)); 
			};

			SyntopiaCore::Math::Vector3f getBackgroundColor() {
				return SyntopiaCore::Math::Vector3f(backgroundColor.red()/255.0f,backgroundColor.green()/255.0f,backgroundColor.blue()/255.0f);
			}
		
			void setContextMenu(QMenu* contextMenu) { this->contextMenu = contextMenu; }

			double getFOV();
		
			QColor getVisibleForegroundColor();

			void setDisabled(bool disabled) { this->disabled = disabled; }
			void setFastRotate(bool enabled);

			GLdouble* getModelViewCache() { return modelViewCache; };
			GLdouble* getProjectionCache() { return projectionCache; };
			GLint* getViewPortCache() { return viewPortCache; };

			void getBoundingBox(SyntopiaCore::Math::Vector3f& from, SyntopiaCore::Math::Vector3f& to) const;
			
			void setRaytracerCommands(QVector<GLEngine::Command> raytracerCommands) { this->raytracerCommands = raytracerCommands; }
			QVector<GLEngine::Command> getRaytracerCommands() { return raytracerCommands; }
			
		protected:
			void contextMenuEvent (QContextMenuEvent* ev );
			void mouseReleaseEvent ( QMouseEvent * event );
			void initializeGL();
			void timerEvent( QTimerEvent * );

			/// Actual drawing is implemented here
			void paintGL();

			/// Triggers a perspective update and a redraw
			void resizeGL(int w, int h);
			void wheelEvent(QWheelEvent* e);
			void mouseMoveEvent(QMouseEvent* e);
			void rotateWorldXY(double x, double y);
			void rotateWorldZ(double z);
			void translateWorld(double x, double y, double z);

		
		private:
			QVector<GLEngine::Command> raytracerCommands;

			// Creates the appropriate GL_PROJECTION matrix
			void updatePerspective();	
			SyntopiaCore::Math::Vector3f screenTo3D(int sx, int sy, int sz);
		

			int pendingRedraws; // the number of times we must redraw 
			// (when a redraw is requested we must draw two times, when double buffering)
			int requiredRedraws;
			Settings settings;
			double scale;
			double mouseSpeed;
			double mouseTranslationSpeed;
			double minimumScale;
			QPoint oldPos;
			QColor backgroundColor;

			SyntopiaCore::Math::Vector3f translation;
			SyntopiaCore::Math::Vector3f pivot;
			SyntopiaCore::Math::Matrix4f rotation;

			QList<Object3D*> objects;
			QString infoText;

			QMenu* contextMenu;
			bool rmbDragging;

			SyntopiaCore::Math::Vector3f cameraPosition;
			SyntopiaCore::Math::Vector3f cameraUp;
			SyntopiaCore::Math::Vector3f cameraTarget;

			GLdouble modelViewCache[16];
			GLdouble projectionCache[16];
			GLint viewPortCache[16];

			QTime textTimer;
			bool disabled;
			bool fastRotate;
			bool doingRotate;

		};
	};

};

