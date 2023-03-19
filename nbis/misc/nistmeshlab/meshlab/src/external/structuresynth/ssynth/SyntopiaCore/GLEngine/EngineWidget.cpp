#include "EngineWidget.h"
#include "../Math/Vector3.h"
#include "../Logging/Logging.h"

#include "Sphere.h"

using namespace SyntopiaCore::Math;
using namespace SyntopiaCore::Logging;

#include <QWheelEvent>

namespace SyntopiaCore {
	namespace GLEngine {


		EngineWidget::EngineWidget(QWidget* parent) : QGLWidget(parent) {

			disabled = false;
			updatePerspective();

			pendingRedraws = 0;
			requiredRedraws = 2; // for double buffering
			startTimer( 10 );
			oldPos = QPoint(0,0);

			// Engine settings
			scale = 0.4;
			minimumScale = 0.01;
			mouseSpeed = 1.0;
			mouseTranslationSpeed = 64.0;
			translation = Vector3f(0,0,-20);

			setMouseTracking(true);

			rotation = Matrix4f::Identity();
			pivot = Vector3f(0,0,0);
			backgroundColor = QColor(30,30,30);
			contextMenu = 0;

			rmbDragging = false;

			fastRotate = false;
			doingRotate = false;
		}

		void EngineWidget::setFastRotate(bool enabled) {
			fastRotate = enabled;
		}


		EngineWidget::~EngineWidget() {
		}

		void EngineWidget::mouseReleaseEvent(QMouseEvent* ev)  {
			doingRotate = false;

			if (ev->button() != Qt::RightButton) return;
			if (rmbDragging) { return; }
			if (contextMenu) contextMenu->exec(ev->globalPos());
		}


		
		void EngineWidget::contextMenuEvent(QContextMenuEvent* /*ev*/ ) {
			// Implementing this here gives trouble on Linux...
			// if (rmbDragging) { return; }
			// if (contextMenu) contextMenu->exec(ev->globalPos());	
		}
		

		void EngineWidget::reset() {
			translation = Vector3f(0,0,-20);
			rotation = Matrix4f::Identity();
			pivot = Vector3f(0,0,0);
			scale = 0.4f;
			//backgroundColor = QColor(0,0,0);
			requireRedraw();
		}

		void EngineWidget::requireRedraw() {
			pendingRedraws = requiredRedraws;
		}

		void vertexm(SyntopiaCore::Math::Vector3f v) { glVertex3f(v.x(), v.y(), v.z()); }
			
		void EngineWidget::paintGL() {
			
			
			static int count = 0;
			count++;

			if (pendingRedraws > 0) pendingRedraws--;

			if (disabled) {
				qglClearColor(backgroundColor);
				glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
				return;
			}
			

			qglClearColor(backgroundColor);
			glMatrixMode(GL_MODELVIEW);

			glLoadIdentity();

			// Calc camera position
			glTranslatef( translation.x(), translation.y(), translation.z() );
			glScalef( scale, scale, scale );
			Vector3f v(1,2,3);
			Vector3f v2 = rotation*v;
			glMultMatrixf(rotation.getArray());
			glTranslatef( -pivot.x(), -pivot.y(), -pivot.z() );
		
			glGetDoublev(GL_MODELVIEW_MATRIX, modelViewCache );
			glGetDoublev(GL_PROJECTION_MATRIX, projectionCache );
			glGetIntegerv(GL_VIEWPORT, viewPortCache);


			cameraPosition = screenTo3D(width()/2, height()/2, 0);
			cameraTarget = screenTo3D(width()/2, height()/2, 1);


			/*
			
			RayInfo ri;
			ri.startPoint = cameraPosition;
			ri.lineDirection = cameraTarget - cameraPosition;
			
			static float f = 0;
			f = f + 0.1;
			if (f>1) f = 0;
			for (int i = 0; i < objects.count(); i++) {
				if (objects[i]->intersectsRay(&ri)) {
					INFO(QString("Index: %0, Pos: %1, reflection: %2").arg(i).arg( objects[i]->name() ).arg(objects[i]->getPrimitiveClass()->reflection));;
				}

			}
			*/

			cameraUp = screenTo3D(width()/2, height()/2-height()/4, 0)-cameraPosition;
			cameraUp.normalize();

			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
			qglColor(getVisibleForegroundColor());

			renderText(10, 20, infoText);
			
			if (QApplication::keyboardModifiers() == Qt::AltModifier || (doingRotate && fastRotate && ( objects.size()>1000))) {
				// Fast-draw

				int objs =  objects.size();
				int step = objs/5000;
				if (step < 1) step = 1;
				if (count >= step) count = 0;

				//glDisable (GL_LIGHTING);
				//glLineWidth( 1.0 );
				qglColor(getVisibleForegroundColor());
				for (int i = count; i < objects.size(); i+=step) {
						objects[i]->draw();
					/*
					glColor3f(
						objects[i]->getColor()[0],
						objects[i]->getColor()[1],
						objects[i]->getColor()[2]
					);
					Vector3f pos1;
					Vector3f pos2;
					objects[i]->getBoundingBox(pos1,pos2);

					glPushMatrix();
					glTranslatef( pos1.x(), pos1.y(), pos1.z() );
					Vector3f v1(0,(pos2-pos1).y(),0);
					Vector3f v2((pos2-pos1).x(),0,0);
					Vector3f v3(0,0,(pos2-pos1).z());

					glBegin( GL_LINE_LOOP  );
					Vector3f O(0,0,0);
					vertexm(O);
					vertexm(v2);
					vertexm(v2+v1);
					vertexm(v1);
					glEnd();

					glBegin( GL_LINE_LOOP  );
					vertexm(v3);
					vertexm(v2+v3);
					vertexm(v2+v1+v3);
					vertexm(v1+v3);
					glEnd();

					glBegin( GL_LINES  );
					vertexm( v3 );   vertexm( O );
					vertexm( v2 );   vertexm( v2+v3 );
					vertexm( v1+v2 );   vertexm( v1+v2+v3 );
					vertexm( v1 );   vertexm( v1+v3 );
					glEnd();

					glPopMatrix();			

					*/
				}

				glEnable (GL_LIGHTING);

				//if (doingRotate && fastRotate) doingRotate = false;
				requireRedraw();

			} else {


				glPolygonMode(GL_FRONT, GL_FILL);
				glPolygonMode(GL_BACK, GL_FILL);

				glEnable (GL_LIGHTING);
				glEnable(GL_CULL_FACE); // TODO: do we need this?

				glEnable (GL_BLEND);
				glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glMateriali( GL_FRONT, GL_SPECULAR, 30 );
				glMateriali(GL_FRONT, GL_SHININESS, 127);


				for (int i = 0; i < objects.size(); i++) {
					objects[i]->draw();
				}
			}

		};

		void EngineWidget::resizeGL( int /* width */, int /* height */) {
			// When resizing the perspective must be recalculated
			requireRedraw();
			updatePerspective();
		};

		void EngineWidget::updatePerspective() {
			if (height() == 0) return;

			GLfloat w = (float) width() / (float) height();
			infoText = QString("[%1x%2] Aspect=%3").arg(width()).arg(height()).arg((float)width()/height());
			textTimer = QTime::currentTime();
			//GLfloat h = 1.0;

			glViewport( 0, 0, width(), height() );
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();

			//settings.perspectiveAngle = 90;
			gluPerspective(settings.perspectiveAngle, w,  (float)settings.nearClipping, (float)settings.farClipping);
			//glOrtho( -w, w, -h, h, (float)0, (float) 60 );
		}

		void EngineWidget::timerEvent(QTimerEvent*) {
			static bool firstTime = true;
			if (firstTime) {
				firstTime = false;
				updatePerspective(); 
				requireRedraw();
				infoText = "";
			}

			// Check if we are displaying a message.
			if (infoText != "" && abs((int)(textTimer.msecsTo(QTime::currentTime()))>1000)) {
				infoText = "";
				requireRedraw();
			}

			if (pendingRedraws) updateGL();
		}


		void EngineWidget::initializeGL()
		{
			requireRedraw();

			GLfloat pos[4] = {-55.0f, -25.0f, 20.0f, 1.0f };
			//GLfloat pos2[4] = {52.0f, 25.0f, 50.0f, 1.0f };
			GLfloat agreen[4] = {0.0f, 0.8f, 0.2f, 1.0f };

			float ambient = 0.4f;
			float diffuse = 0.6f;
			float specular = 0.1f;
			Vector3f color = Vector3f(1,1,1);
			GLfloat ambientLight[] = { ambient * color.x(),   ambient * color.y(),  ambient * color.z(), 1.0f };
			GLfloat diffuseLight[] = { diffuse * color.x(),   diffuse * color.y(),  diffuse * color.z(), 1.0f };
			GLfloat specularLight[] = {specular * color.x(),  specular * color.y(), specular* color.z(), 1.0f };
			glLightfv(0, GL_AMBIENT, ambientLight);
			glLightfv(0, GL_DIFFUSE, diffuseLight);
			glLightfv(0, GL_SPECULAR, specularLight);
			glLightfv(0, GL_POSITION, pos );
			glEnable( GL_LIGHT0 ); 



			glEnable( GL_CULL_FACE );
			glEnable( GL_LIGHTING );
			glEnable( GL_DEPTH_TEST );
			glEnable( GL_NORMALIZE );
			glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, agreen );

			glEnable(GL_LINE_SMOOTH);
			glEnable(GL_POINT_SMOOTH);
			glEnable(GL_POLYGON_SMOOTH);
			glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
		}


		void EngineWidget::wheelEvent(QWheelEvent* e) {
			e->accept();
			
			double interval = (double)e->delta() / 800.0;
			
			if ( scale <= mouseSpeed*interval ) return;
			scale -= mouseSpeed*interval;
			requireRedraw();
		}

		void EngineWidget::mouseMoveEvent( QMouseEvent *e ) {
			e->accept();

			// store old position

			if (oldPos.x() == 0 && oldPos.y() == 0) {
				// first time
				oldPos = e->pos(); 
				return;
			}
			double dx = e->x() - oldPos.x();
			double dy = e->y() - oldPos.y();

			// normalize wrt screen size
			double rx = dx / width();
			double ry = dy / height();

			oldPos = e->pos();

			if ( (e->buttons() == Qt::LeftButton && e->modifiers() == Qt::ShiftModifier ) 
				|| e->buttons() == (Qt::LeftButton | Qt::RightButton )) 
			{
				doingRotate = true;

				// 1) dragging with left mouse button + shift down, or
				// 2) dragging with left and right mouse button down
				//
				// This results in zooming for vertical movement, and Z axis rotation for horizontal movement.

				scale += (ry * 4.0f);

				if (scale < minimumScale) scale = minimumScale;

				rotateWorldZ( rx );
				requireRedraw();

				if (e->buttons() == (Qt::LeftButton | Qt::RightButton ))  {
					rmbDragging = true;
				}
			} else if ( ( e->buttons() == Qt::RightButton ) 
				|| (e->buttons() == Qt::LeftButton && e->modifiers() == Qt::ControlModifier ) 
				|| (e->buttons() == Qt::LeftButton && e->modifiers() == Qt::MetaModifier ) ) 
			{ 
				doingRotate = true;

				// 1) dragging with right mouse button, 
				// 2) dragging with left button + control button,
				// 3) dragging with left button + meta button (Mac)
				//
				// results in translation

				if (rx != 0 || ry != 0) {
					translateWorld( mouseSpeed*mouseTranslationSpeed* rx, - mouseSpeed*mouseTranslationSpeed*ry, 0 );
					requireRedraw();
					rmbDragging = true;
				} 
			} else if ( e->buttons() == Qt::LeftButton ) {
				doingRotate = true;

				// Dragging with left mouse button.
				// Results in rotation.

				rotateWorldXY( rx, ry );
				requireRedraw();

				rmbDragging = false;
			} else {
				rmbDragging = false;
			}
		}


		Vector3f EngineWidget::screenTo3D(int sx, int sy, int sz) {
			// 2D -> 3D conversion
			/*
			GLdouble modelView[16];
			GLdouble projection[16];
			GLint viewPort[16];

			glGetDoublev( GL_MODELVIEW_MATRIX, modelView );
			glGetDoublev( GL_PROJECTION_MATRIX, projection );
			glGetIntegerv( GL_VIEWPORT, viewPort);
			*/

			GLdouble x, y, z;
			float h = (float)height(); 
			gluUnProject(sx, h-sy, sz, modelViewCache, projectionCache, viewPortCache, &x, &y ,&z);
			return Vector3f(x,y,z);
		}

		void EngineWidget::rotateWorldXY(double x, double y) {
			double rotateSpeed = 5.0;

			Vector3f startPoint = screenTo3D(oldPos.x(), oldPos.y(),1);
			Vector3f xDir =       (screenTo3D(oldPos.x()+10, oldPos.y(),1) - startPoint).normalized() ;
			Vector3f yDir =       (screenTo3D(oldPos.x(), oldPos.y()+10,1) - startPoint).normalized() ;

			Matrix4f mx = Matrix4f::Rotation(xDir, y*rotateSpeed);
			Matrix4f my = Matrix4f::Rotation(yDir, -x*rotateSpeed);
			rotation = rotation*my * mx   ; 
			requireRedraw();
		}

		void EngineWidget::rotateWorldZ(double z) {
			double rotateSpeed = 5.0;

			Vector3f startPoint = screenTo3D(oldPos.x(), oldPos.y(), 0);
			Vector3f endPoint =   screenTo3D(oldPos.x(), oldPos.y(), 1);

			Matrix4f mz = Matrix4f::Rotation(startPoint-endPoint, z*rotateSpeed);
			rotation = rotation*mz  ; 
			requireRedraw();
		}

		void EngineWidget::translateWorld(double x, double y, double z) {
			translation = Vector3f(translation.x() + x, translation.y() + y, translation.z() + z);
		}


		void EngineWidget::clearWorld() {
			for (int i = 0; i < objects.size(); i++) delete(objects[i]);
			objects.clear();
		}

		void EngineWidget::addObject(Object3D* object) {
			objects.append(object);
		}

		SyntopiaCore::Math::Vector3f EngineWidget::getCameraPosition() {
			return cameraPosition;
		};

		SyntopiaCore::Math::Vector3f EngineWidget::getCameraUp() {
			return cameraUp;
		};

		SyntopiaCore::Math::Vector3f EngineWidget::getCameraTarget() {
			return cameraTarget;
		};

		double EngineWidget::getFOV() {
			double ar = width()/(double)height();
			return 29.0*ar; // Hack - this is not entirely accurate for large AR's.
		}
		
		QColor EngineWidget::getVisibleForegroundColor() {
			int r = backgroundColor.red() < 127 ? 255 : 0;
			int g = backgroundColor.green() < 127 ? 255 : 0;
			int b = backgroundColor.blue() < 127 ? 255 : 0;
			return QColor(r,g,b);
		}

		void EngineWidget::getBoundingBox(SyntopiaCore::Math::Vector3f& from, SyntopiaCore::Math::Vector3f& to) const {
			from = Vector3f(0,0,0);
			to = Vector3f(0,0,0);
			for (int i = 0; i < objects.count(); i++) {
				if (i == 0) { objects[i]->getBoundingBox(from,to); }
				else { objects[i]->expandBoundingBox(from,to);  }
			}
		}
			
	}
}

