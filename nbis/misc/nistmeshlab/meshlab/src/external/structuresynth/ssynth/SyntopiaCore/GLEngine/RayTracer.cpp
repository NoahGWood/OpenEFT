#include "RayTracer.h"

#include "SyntopiaCore/Math/Vector3.h"
#include "SyntopiaCore/Logging/Logging.h"
#include "SyntopiaCore/Misc/MiniParser.h"

using namespace SyntopiaCore::Math;
using namespace SyntopiaCore::Misc;

namespace SyntopiaCore {
	namespace GLEngine {


		using namespace SyntopiaCore::Logging;



		/// See here for details about this approach:
		/// http://www.devmaster.net/articles/raytracing_series/part4.php
		class VoxelStepper {
		public:
			VoxelStepper(Vector3f minPos, Vector3f maxPos, int steps) 
				:  steps(steps), minPos(minPos), maxPos(maxPos), size(( maxPos - minPos)/(double)steps) {
				currentT = 0;
				grid = new QList<Object3D*>[steps*steps*steps];
				for (int i = 0; i < steps*steps*steps; i++) grid[i] = QList<Object3D*>();
			};

			~VoxelStepper() { delete[] grid; };

			void registerObject(Object3D* obj) {
				// Simple method - check all cells intersecting the objects bounding boxes.

				obj->prepareForRaytracing();
				Vector3f from;
				Vector3f to;
				obj->getBoundingBox(from,to);
				from = from - minPos;
				to = to - minPos;

				int xStart = floor(from.x()/size.x());
				int xEnd = ceil(to.x()/size.x());
				int yStart = floor(from.y()/size.y());
				int yEnd = ceil(to.y()/size.y());
				int zStart = floor(from.z()/size.z());
				int zEnd = ceil(to.z()/size.z());
				if (xStart < 0) xStart = 0;
				if (yStart < 0) yStart = 0;
				if (zStart < 0) zStart = 0;
				if (xEnd > (int)steps) xEnd = steps;
				if (yEnd > (int)steps) yEnd = steps;
				if (zEnd > (int)steps) zEnd = steps;

				/*
				xStart = 0; 
				yStart = 0; 
				zStart = 0; 
				xEnd = steps;
				yEnd = steps;
				zEnd = steps;
				*/

				for (unsigned int x = xStart; x < (unsigned int)xEnd; x++) {
					for (unsigned int y = yStart; y < (unsigned int)yEnd; y++) {
						for (unsigned int z = zStart; z < (unsigned int)zEnd; z++) {
							if (obj->intersectsAABB(minPos + Vector3f(size.x()*x,size.y()*y,size.z()*z),minPos + Vector3f(size.x()*(x+1),size.y()*(y+1),size.z()*(z+1)) )) {
								grid[x+y*steps+z*steps*steps].append(obj);
							}
						}
					}
				}

			}

			QList<Object3D*>* setupRay(Vector3f pos, Vector3f dir, double& maxT) {
				this->pos = pos; 
				this->dir = dir;

				currentT = 0;

				const Vector3f ro = pos - minPos;
				cx = floor(ro.x() / size.x());
				cy = floor(ro.y() / size.y());
				cz = floor(ro.z() / size.z());


				if ((cx < 0 || cx >= steps) ||
					(cy < 0 || cy >= steps) || 
					(cz < 0 || cz >= steps)) {						
						// we are outside grid.
						// advance ray to inside grid.

						bool found = false;
						double p;
						if (dir.x() > 0) {
							p = (minPos.x()-pos.x())/dir.x();
							cx = 0;
						} else {
							p = (maxPos.x()-pos.x())/dir.x();
							cx = steps-1;								
						}
						Vector3f v = pos + dir*p - minPos;								
						cy = floor(v.y() / size.y());
						cz = floor(v.z() / size.z());
						if ((cy >= 0 && cy < steps) && (cz >= 0 && cz < steps)) {
							found = true;
							pos = v+minPos;
						}

						if (!found) {
							if (dir.y() > 0) {
								p = (minPos.y()-pos.y())/dir.y();
								cy = 0;
							} else {
								p = (maxPos.y()-pos.y())/dir.y();
								cy = steps-1;
							}
							Vector3f v = pos + dir*p - minPos;									
							cx = floor(v.x() / size.x());
							cz = floor(v.z() / size.z());
							if ((cx >= 0 && cx < steps) && (cz >= 0 && cz < steps)) {
								pos = v+minPos;
								found = true;
							}
						}

						if (!found) {
							if (dir.z() > 0) {
								p = (minPos.z()-pos.z())/dir.z();
								cz = 0;
							} else {
								p = (maxPos.z()-pos.z())/dir.z();
								cz = steps-1;
							}
							Vector3f v = pos + dir*p - minPos;									
							cx = floor(v.x() / size.x());
							cy = floor(v.y() / size.y());
							if ((cy >= 0 && cy < steps) && (cx >= 0 && cx < steps)) {
								pos = v+minPos;
								found = true;
							}
						}

						currentT = p;

						// We do not intersect grid.
						if (!found) return false;
				}

				stepX = (dir.x() > 0) ? 1 : -1;
				stepY = (dir.y() > 0) ? 1 : -1;
				stepZ = (dir.z() > 0) ? 1 : -1;

				tDeltaX = stepX*size.x() / dir.x();
				tDeltaY = stepY*size.y() / dir.y();
				tDeltaZ = stepZ*size.z() / dir.z();

				Vector3f orv = pos- (minPos + Vector3f(size.x()*cx, size.y()*cy, size.z()*cz));
				tMaxX = stepX*orv.x()/dir.x();
				if (stepX>0) tMaxX = tDeltaX - tMaxX;
				tMaxY = stepY*orv.y()/dir.y();
				if (stepY>0) tMaxY = tDeltaY - tMaxY;
				tMaxZ = stepZ*orv.z()/dir.z();
				if (stepZ>0) tMaxZ = tDeltaZ - tMaxZ;

				// Now pos is advanced properly.
				// cx,cy,cz contains current cell.
				QList<Object3D*>* list = &grid[cx+cy*steps+cz*steps*steps];

				if (list && (list->count() == 0)) {
					list = advance(maxT);
				} else {
					maxT = currentT + minn(tMaxX, tMaxY, tMaxZ);					
				}

				return list;
			}

			inline double minn(double a, double b, double c) {
				if (a<b) return (a<c ? a : c);
				return (b<c ? b : c);
			}

			QList<Object3D*>* advance(double& maxT) {
				QList<Object3D*>* list = 0;
				do {
					if(tMaxX < tMaxY) {
						if(tMaxX < tMaxZ) {
							cx += stepX;
							if (cx >= steps || cx < 0) return 0;
							tMaxX = tMaxX + tDeltaX;
						} else {
							cz += stepZ;
							if (cz >= steps || cz < 0) return 0;
							tMaxZ = tMaxZ + tDeltaZ;
						} 
					} else {
						if(tMaxY < tMaxZ) {
							cy += stepY;
							if (cy >= steps || cy < 0) return 0;
							tMaxY = tMaxY + tDeltaY;
						} else {
							cz += stepZ;
							if (cz >= steps || cz < 0) return 0;
							tMaxZ = tMaxZ + tDeltaZ;
						}
					}
					list = &grid[cx+cy*steps+cz*steps*steps];

					if (list && (list->count() == 0)) list = 0; // Continue until we find an non-empty list.
				} while(list == 0);

				maxT = currentT + minn(tMaxX, tMaxY, tMaxZ);
				return(list);
			}

		private:
			double currentT;
			double tDeltaX;
			double tDeltaY;
			double tDeltaZ;
			double tMaxX;
			double tMaxY;
			double tMaxZ;
			int stepX;
			int stepY;
			int stepZ;
			int cx;
			int cy;
			int cz;

			const Vector3f size;
			Vector3f pos;
			Vector3f dir;
			int steps;
			Vector3f minPos;
			Vector3f maxPos;		
			QList<Object3D*>* grid;
		};


		RayTracer::RayTracer(EngineWidget* engine) {
			for (int i = 0; i < 16; i++) modelView[i] = engine->getModelViewCache()[i];
			for (int i = 0; i < 16; i++) projection[i] = engine->getProjectionCache()[i];
			for (int i = 0; i < 16; i++) viewPort[i] = engine->getViewPortCache()[i];

			userCancelled = false;
			Vector3f from;
			Vector3f to;
			engine->getBoundingBox(from,to);


			accelerator = new VoxelStepper(from,to, 35);
			backgroundColor = Vector3f(0,0,0);
			windowHeight = engine->width();
			windowWidth = engine->height();
			objects = engine->getObjects();
			for (int i = 0; i < objects.count(); i++) accelerator->registerObject(objects[i]);

			ambMinRays = 10;
			ambMaxRays = 100;
			ambExponent = 1;
			ambPrecision = 0.9;
			aaSamples = 2;
			ambSmooth = 0;
			useShadows = true;
			globalAmbient = 0.3;
			globalDiffuse = 0.5;
			globalSpecular = 0.8;
			
			foreach (Command c, engine->getRaytracerCommands()) {
				QString arg = c.arg;
				arg = arg.remove("[");
				arg = arg.remove("]");
				setParameter(c.command, arg);
			}
			setBackgroundColor(engine->getBackgroundColor());
			
		
		}

		Vector3f RayTracer::rayCastPixel(float x, float y) {
			
			Vector3f startPoint = frontStart + frontX*x + frontY*y;
			Vector3f endPoint  =   backStart  + backX*x  + backY*y;
			Vector3f direction = endPoint - startPoint;
				
			return rayCast(startPoint, direction, 0);
		}

		Vector3f RayTracer::rayCast(Vector3f startPoint, Vector3f direction, Object3D* excludeThis, int level) {
			if (level>15) return Vector3f(backgroundColor.x(),backgroundColor.y(),backgroundColor.z());
			static int rayID = 1;
			rayID++;
			pixels++;
			
			double lengthToClosest = -1;
			Vector3f foundNormal;
			GLfloat foundColor[4];

			Object3D* bestObj = 0;
			double maxT = 0;
			RayInfo ri;
			QList<Object3D*>* list = accelerator->setupRay(startPoint, direction, maxT);
			ri.startPoint = startPoint;
			ri.lineDirection = direction;			

			while (list != 0) { 
				// check objects
				for (int i = 0; i < list->count(); i++) {
					checks++;

					if (list->at(i) == excludeThis) continue;
					// Check if we already tested this...
					if (list->at(i)->getLastRayID() == rayID) continue;

					bool found = list->at(i)->intersectsRay(&ri);
					list->at(i)->setLastRayID(rayID);
					if (!found) continue;
					if ((ri.intersection<1E-7)) continue;
					
					if ((ri.intersection>0) && ((ri.intersection <= lengthToClosest) || (lengthToClosest == -1))) {
						// We hit something and it was closer to us than the object before...
						foundNormal = ri.normal;
						for (int ix = 0; ix < 4; ix++) foundColor[ix] = ri.color[ix];							
						lengthToClosest = ri.intersection;
						bestObj = list->at(i);
					}
				}

				if (bestObj != 0 && lengthToClosest < maxT) break;
				list = accelerator->advance(maxT); 
			}

			// Now we can calculate the lightning.
			if (lengthToClosest>0) {	
				// iPoint is the intersection point in 3D.
				Vector3f iPoint = startPoint + direction*lengthToClosest;
				Vector3f lightDirection = (lightPos-iPoint);

				double light = 0;

				// This is a Phong lightning model, see e.g. (http://ai.autonomy.net.au/wiki/Graphics/Illumination)
				// -- Diffuse light 
				double diffuse = globalDiffuse*(Vector3f::dot(foundNormal, (lightDirection).normalized()));				
				if (diffuse<0) diffuse = 0;
				light += diffuse;

				// -- Specular light
				double spec = 0;
				double dot = Vector3f::dot(foundNormal, lightDirection);
				if (dot<0.1) {
				} else {
					Vector3f reflected = foundNormal*dot*2 - lightDirection;
					reflected.normalize();
					spec = -(Vector3f::dot(reflected, (direction).normalized()));		
					if (spec< 0.1) {
						spec = 0;
					} else {
						spec = globalSpecular*pow(spec,50);
						if (spec<0) spec = 0;
					}
				}
				light += spec;

				// -- Ambient light
				double ambient = globalAmbient;
				light += ambient; 

				// -- calculate shadow...
				// TODO: Calculate shadow in transperant media
				bool inShadow = false;
				if (useShadows) {
					double maxT = 0;
					QList<Object3D*>* list = accelerator->setupRay(iPoint,(lightPos-iPoint), maxT);
					ri.startPoint = iPoint;
					ri.lineDirection = lightPos-iPoint;

					while (list != 0 && !inShadow) { 
						// check objects
						for (int i = 0; i < list->size(); i++) {
							if (list->at(i) == bestObj) continue; // self-shadow? (probably not neccesary, since the specular light will be negative)							
							inShadow = list->at(i)->intersectsRay(&ri);
							if (ri.intersection < 1E-5 || ri.intersection > 1) inShadow = false;
							if (ri.color[3]<1) inShadow=false;
							if (inShadow) break;								
						}

						if (!inShadow) list = accelerator->advance(maxT); 
					}
				}

				// Clamp light values.
				if (useShadows && inShadow) light=ambient; // drop-shadow strength (only ambient light...)
				if (light > 1) light = 1;
				if (light < 0) light = 0;
				
				if (foundColor[3] < 1) {
					Vector3f color = rayCast(iPoint, direction, bestObj, level+5); 
					foundColor[0] = light*foundColor[0]*(foundColor[3]) + color.x()*(1-foundColor[3]);
					foundColor[1] = light*foundColor[1]*(foundColor[3]) + color.y()*(1-foundColor[3]);
					foundColor[2] = light*foundColor[2]*(foundColor[3]) + color.z()*(1-foundColor[3]);
				}

				
				double reflection = bestObj->getPrimitiveClass()->reflection;
					
				if (reflection > 0) {
					Vector3f nDir = foundNormal*(-2)*Vector3f::dot(foundNormal, direction)/foundNormal.sqrLength() + direction;
					Vector3f color = rayCast(iPoint, nDir, bestObj, level+1);
					Vector3f thisColor = Vector3f(light*foundColor[0],light*foundColor[1],light*foundColor[2]) *(1-reflection)
					+ color*(reflection);
					foundColor[0] = light*foundColor[0]*(1-reflection) + color.x()*reflection;
					foundColor[1] = light*foundColor[1]*(1-reflection) + color.y()*reflection;
					foundColor[2] = light*foundColor[2]*(1-reflection) + color.z()*reflection;
					if (foundColor[0]>1) foundColor[0] = 1;
					if (foundColor[1]>1) foundColor[1] = 1;
					if (foundColor[2]>1) foundColor[2] = 1;
				}

				if (level==-1 && ambMaxRays>0) {
					int hits = 0;
					int tests = 0;
					totalAOCasts++;
					for (int ix = 0; ix < ambMaxRays; ix++) {
						tests++;
						// Use monte carlo sampling to obtain random vector.
						Vector3f random ;
						do {
							random = Vector3f(rg.getDouble(-1,1),rg.getDouble(-1,1),rg.getDouble(-1,1));
						} while (random.sqrLength() > 1);
						random.normalize();
						if (Vector3f::dot(random, foundNormal)<0) random = random*-1.0; // Only check away from surface.

						double maxT = 0;
						QList<Object3D*>* list = accelerator->setupRay(iPoint,random, maxT);
						ri.startPoint = iPoint;
						ri.lineDirection = random;
						bool occluded = false;
						while (list != 0 && !occluded) { 
							// check objects
							for (int i = 0; i < list->size(); i++) {
								if (list->at(i) == bestObj) continue; // self-shadow? 							
								occluded = list->at(i)->intersectsRay(&ri);
								if (ri.intersection < 1E-5) occluded = false;
								if (occluded) break;								
							}

							if (!occluded) list = accelerator->advance(maxT); 
						}
						
						if (ix>=ambMinRays) {
							double oldPercentage = hits/(double)(tests-1);
							if (occluded) hits++;
							double newPercentage = hits/(double)tests;
							if (fabs(oldPercentage-newPercentage)/oldPercentage < (1-ambPrecision)) {
								break;
							}
						} else {
							if (occluded) hits++;
						}
					}
					double occ = (1-hits/(double)tests);
					/*
					if (occ < 0.4) occ = 0;
					else if (occ > 1) occ = 1;
					else occ = (occ-0.4)/(1-0.4);
					occ = occ*occ;

					*/
					light = light*occ;
				}
				

				normal = foundNormal;
				depth = lengthToClosest;
				color = Vector3f(light*foundColor[0],light*foundColor[1],light*foundColor[2]);
				intersection = iPoint;
				hitObject = bestObj;
				return color;
			} else {
				depth = 10; // Real depth is [0;1]
				normal = Vector3f(0,0,0);
				color = Vector3f(backgroundColor.x(),backgroundColor.y(),backgroundColor.z());
				intersection =  Vector3f(0,0,0);
				hitObject = 0;
				return color;
			}
		}


		namespace {
			int sqr(int a) { return a*a; } 

			int cdist(QRgb c1, QRgb c2) {
				return sqr(qGreen(c1)-qGreen(c2))+sqr(qRed(c1)-qRed(c2))+sqr(qBlue(c1)-qBlue(c2));
			}
		}

		QImage RayTracer::calculateImage(int w, int h) {

			float oh = (float)h; 
			float ow = (float)w; 
			float vh = (float)h; // windowHeight; 
			//float vw = (float)w; // windowWidth; 
			windowHeight = h;
			windowWidth = w;

			GLdouble ox1, oy1, oz1;				


			gluUnProject(0, windowHeight, 0.0f, modelView, projection, viewPort, &ox1, &oy1 ,&oz1);
			frontStart = Vector3f(ox1,oy1,oz1);
			gluUnProject(windowWidth, windowHeight, 0.0f, modelView, projection, viewPort, &ox1, &oy1 ,&oz1);
			frontX = Vector3f(ox1,oy1,oz1)-frontStart;
			gluUnProject(0, 0, 0.0f, modelView, projection, viewPort, &ox1, &oy1 ,&oz1);
			frontY = Vector3f(ox1,oy1,oz1)-frontStart;

			gluUnProject(0, windowHeight, 1.0f, modelView, projection, viewPort, &ox1, &oy1 ,&oz1);
			backStart = Vector3f(ox1,oy1,oz1);
			gluUnProject(windowWidth, windowHeight, 1.0f, modelView, projection, viewPort, &ox1, &oy1 ,&oz1);
			backX = Vector3f(ox1,oy1,oz1)-backStart;
			gluUnProject(0, 0, 1.0f, modelView, projection, viewPort, &ox1, &oy1 ,&oz1);
			backY = Vector3f(ox1,oy1,oz1)-backStart;

			// Setup progress dialog.
			QProgressDialog progress("progress", "Cancel", 0, 100);
			progress.setLabelText("Ray tracing");
			progress.setMinimumDuration(1000);
			progress.setWindowModality(Qt::WindowModal);

			TIME("Rendering...");
			QImage im(w,h, QImage::Format_RGB32);

			
			double* depths = new double[w*h];
			for (int i = 0; i<w*h; i++) depths[i] = -1;
			Vector3f* normals = new Vector3f[w*h];
			Vector3f* colors = new Vector3f[w*h];
			Vector3f* intersections = new Vector3f[w*h];
			double* aoMap = new double[w*h];
			Object3D** objs = new Object3D*[w*h];
			for (int i = 0; i<w*h; i++) aoMap[i] = -1;
			

			// Find a suitable light position. TODO: CHANGE!
			GLdouble sx1, sy1, sz1;				
			gluUnProject((float)-200, vh, 0.0f, modelView, projection, viewPort, &sx1, &sy1 ,&sz1);				
			lightPos = Vector3f((GLfloat)sx1, (GLfloat)sy1, (GLfloat)sz1);
			light1Ambient = 0.2f;
			light1Diffuse = 0.6f;
			light1Specular = 0.6f;

			pixels = 0;
			checks = 0;
			aaPixels = 0;
			for (int x = 0; x < w; x++) {
				float fx = x/(float)w;
				if (x % 7 == 0) {
					progress.setValue((x*100)/w);
					//qApp->processEvents();
					if	(progress.wasCanceled()) {
						userCancelled = true;
						break;
					}
				}
				for (int y = 0; y < h; y++) {	
					float fy = y/(float)h;
					Vector3f colorn = rayCastPixel(fx,fy);
					im.setPixel(x,y,qRgb(colorn.x()*255,colorn.y()*255,colorn.z()*255));
					colors[x+y*w] = colorn;
					depths[x+y*w] = depth;
					normals[x+y*w] = normal;
					intersections[x+y*w] = intersection;
					objs[x+y*w] = hitObject;
				}
			}
			TIME();


			TIME("Ambient Occlusion");
			progress.setLabelText("Ambient Occlusion (Step 2/3)...");



			int step = ambMinRays;
			if ( ambMaxRays>0) {
				double tr = 0.99;

				QVector<Vector3f> dirs;
				for (int i = 0; i < ambMaxRays; i++) {
					Vector3f random ;
					do {
						random = Vector3f(rg.getDouble(-1,1),rg.getDouble(-1,1),rg.getDouble(-1,1));
					} while (random.sqrLength() > 1);
					random.normalize();
					dirs.append(random);
				}
							

				for (int x = 0; x < w; x=x+1) {
					
					//float fx = x/(float)w;
					if (x % 7 == 0) {
						progress.setValue((x*100)/w);
						//qApp->processEvents();
						if	(progress.wasCanceled()) {
							userCancelled = true;
							break;
						}
					}
					for (int y = 0; y < h; y=y+1) {	

						bool sample = false;
						if ((y % step == 0) && (x % step == 0)) { sample = true; } 	
						else if ((x > 0) && 
							(objs[x+y*w]!=objs[x+y*w-1] || Vector3f::dot(normals[x+y*w],normals[x+y*w-1])<tr)) { sample = true; }
						else if ((y > 0) && 
							(objs[x+y*w]!=objs[x+y*w-w] || Vector3f::dot(normals[x+y*w],normals[x+y*w-w])<tr)) { sample = true; }
						else if ((y < h-1) && 
							(objs[x+y*w]!=objs[x+y*w+w] || Vector3f::dot(normals[x+y*w],normals[x+y*w+w])<tr)) { sample = true; }
						else if ((x < w-1) && 
							(objs[x+y*w]!=objs[x+y*w+1] || Vector3f::dot(normals[x+y*w],normals[x+y*w+1])<tr)) { sample = true; }
						
						if (!sample) continue;


						/*
					    Vector3f c = colors[x+y*w];
						c.y() =1;
						colors[x+y*w] = Vector3f(0,1,0);*/

						//float fy = y/(float)h;
						if (depths[x+y*w] > 1) {
							aoMap[x+y*w] = 1;
							continue;
						}
						int hits = 0;
						int tests = 0;
						totalAOCasts++;
						for (int ix = 0; ix < ambMaxRays; ix++) {
							tests++;
							// Use monte carlo sampling to obtain random vector.
							
							Vector3f random ;
							do {
								random = Vector3f(rg.getDouble(-1,1),rg.getDouble(-1,1),rg.getDouble(-1,1));
							} while (random.sqrLength() > 1);
							random.normalize();
							if (Vector3f::dot(random, normals[x+y*w])<0) random = random*-1.0; // Only check away from surface.
							
							/*
							Vector3f random = dirs[ix];
							if (Vector3f::dot(random, normals[x+y*w])<0) random = random*-1.0; // Only check away from surface.
							*/
							

							double maxT = 0;
							QList<Object3D*>* list = accelerator->setupRay(intersections[x+y*w],random, maxT);
							RayInfo ri;
							ri.startPoint = intersections[x+y*w];
							ri.lineDirection = random;
							bool occluded = false;
							while (list != 0 && !occluded) { 
								// check objects
								for (int i = 0; i < list->size(); i++) {
									if (list->at(i) == objs[x+y*w]) continue; // self-shadow? 							
									occluded = list->at(i)->intersectsRay(&ri);
									if (ri.intersection < 1E-5 || ri.intersection> ambPrecision) occluded = false;
									if (occluded) break;								
								}

								if (maxT>ambPrecision) break;
								if (!occluded) list = accelerator->advance(maxT); 

							}

							/*
							if (ix>=ambMinRays) {
							double oldPercentage = hits/(double)(tests-1);
							if (occluded) hits++;
							double newPercentage = hits/(double)tests;
							if (fabs(oldPercentage-newPercentage)/oldPercentage < (1-ambPrecision)) {
							break;
							}
							} else {
							*/
							if (occluded) hits++;
						}
						double occ = 1-pow(hits/(double)tests,ambExponent);
						aoMap[x+y*w] = occ;
					}
				}


				for (int i = 0; i<w*h; i++) if (depths[i]>1) aoMap[i] = 1;

				double* aoMap2 = new double[w*h];
				for (int i = 0; i<w*h; i++) aoMap2[i] = aoMap[i];

				// Fill with simple
				for (int y = 0; y < h; y=y+step) {
					for (int x = 0; x < w; x=x+step) {
						double d = aoMap[x+y*w];;
						for (int yy = 0; yy < step; yy++) {
							for (int xx = 0; xx < step; xx++) {
								if ((x+xx>=w) || (y+yy>=h)) continue;
								if ((xx == 0 && yy == 0) || aoMap[xx+x+(y+yy)*w]>=0) 
									d = aoMap[xx+x+(y+yy)*w];
								aoMap2[xx+x+(y+yy)*w] =  d;

							}
						}	
					}
				}


				//for (unsigned int i = 0; i<w*h; i++) if (aoMap[i]>=0) if (aoMap2[i] != aoMap[i]) { WARNING("Lousy0"); break; }

				/*
				int iterations = step*10;
				for (unsigned int i = 0; i< iterations; i++) {
					for (int y = 1; y < h-1; y++) {
						for (int x = 1; x < w-1; x++) {
							if (aoMap[x+y*w]<0) {
								aoMap2[x+y*w] += aoMap2[x+y*w-1]+aoMap2[x+y*w+1]+aoMap2[x+y*w-w]+aoMap2[x+y*w+w];
								aoMap2[x+y*w]/=5;
							}
						}
					}
				}
				*/

				
				//iterations = 10;
				tr = 0.9;
				const int f = 10;
				for (int i = 0; i< ambSmooth; i++) {
					for (int y = 1 ; y < h-1; y++) {
						for (int x = 1 ; x < w-1; x++) {
							int c = aoMap[x+y*w+1]<0 ? 1 : f;
							double d = c*aoMap2[x+y*w];
							if (objs[x+y*w]==objs[x+y*w+1] && Vector3f::dot(normals[x+y*w],normals[x+y*w+1])>tr) { int n = aoMap[x+y*w+1]<0 ? 1 : f ; c=c+n; d+=n*aoMap2[x+y*w+1]; };
							if (objs[x+y*w]==objs[x+y*w+w] && Vector3f::dot(normals[x+y*w],normals[x+y*w+w])>tr) { int n = aoMap[x+y*w+w]<0 ? 1 : f ; c=c+n; d+=n*aoMap2[x+y*w+w]; };
							if (objs[x+y*w]==objs[x+y*w-w] && Vector3f::dot(normals[x+y*w],normals[x+y*w-w])>tr) { int n = aoMap[x+y*w-w]<0 ? 1 : f ; c=c+n; d+=n*aoMap2[x+y*w-w]; };
							if (objs[x+y*w]==objs[x+y*w-1] && Vector3f::dot(normals[x+y*w],normals[x+y*w-1])>tr) { int n = aoMap[x+y*w-1]<0 ? 1 : f ; c=c+n; d+=n*aoMap2[x+y*w-1]; };
							d = d/c;
							aoMap2[x+y*w] = d;	
						}
					}

					
				}
				

				for (int i = 0; i<w*h; i++) aoMap[i] = aoMap2[i];
				delete[] aoMap2;

			} else {
				for (int i = 0; i<w*h; i++) aoMap[i] = 1;
			
			}


			for (int x = 0; x < w; x++) {
				for (int y = 0; y < h; y++) {
					Vector3f c = colors[x+y*w]*aoMap[x+y*w];
					//c = Vector3f(1,1,1)*aoMap[x+y*w];
					im.setPixel(x,y,qRgb(c.x()*255, c.y()*255, c.z()*255));
				}
			}
				

			TIME();

			progress.setLabelText("Raytracing Anti-Alias (Step 3/3)...");

			long aaPixels = 0;

			
			if (aaSamples>0) {
				TIME("Anti-Alias");
				float xs = (1.0/ow);
				float ys = (1.0/oh);
				for (int x = 1; x+1 < w; x++) {
					float fx = x/(float)w;
				
					if (x % 7 == 0) {
						progress.setValue((x*100)/w);
						//qApp->processEvents();
						if (progress.wasCanceled()) {
							userCancelled = true;
							break;
						}
					}

					for (int y = 1; y+1 < h; y++) {
						float fy = y/(float)h;

						QRgb c1 = im.pixel(x,y);
						int tres = 3*50*50;

						if ( cdist(c1,im.pixel(x+1,y)) > tres ||
							cdist(c1,im.pixel(x,y+1)) > tres ||
							cdist(c1,im.pixel(x-1,y)) > tres ||
							cdist(c1,im.pixel(x,y-1)) > tres)

						{
							aaPixels++;
							Vector3f color(0,0,0);

							unsigned int steps = aaSamples;
							double xstepsize = xs/steps;
							double ystepsize = ys/steps;
							for (unsigned int xo = 0; xo < steps; xo++) {
								for (unsigned int yo = 0; yo < steps; yo++) {	
									color = color + rayCastPixel(fx-xs/2.0 +xo*xstepsize+xstepsize/2.0,
										(fy-ys/2.0 +yo*ystepsize+ystepsize/2.0));
								}
							}
							color = color / (steps*steps);
							im.setPixel(x,y,qRgb(color.x()*255,color.y()*255,color.z()*255));

							Vector3f c = color*aoMap[x+y*w];
							//c = Vector3f(1,1,1)*aoMap[x+y*w];
							im.setPixel(x,y,qRgb(c.x()*255, c.y()*255, c.z()*255));
						}

					}
				}
				TIME();
			}

			delete[] aoMap;
			delete[] colors;
			delete[] normals;
			delete[] depths;
			delete[] objs;
			delete[] intersections;

			


			INFO(QString("Enabled objects: %1, inside viewport: %2").arg(objects.size()).arg("n.a."));
			INFO(QString("Pixels: %1, Object checks: %2, Objects checked per pixel: %3").
				arg(pixels).arg(checks).arg(checks/(float)pixels));
			INFO(QString("Pixels: %1, Needing AA: %2, Percentage: %3").
				arg(pixels).arg(aaPixels).arg(aaPixels/(float)pixels));

			INFO(QString("Total tests: %1M").arg(totalAOCasts/1000000));
		
			progress.close();

			
			return im;
		}


	
		void RayTracer::setParameter(QString param, QString value) {
			param=param.toLower();

			/*
			int ambMinRays;
			int ambMaxRays;
			float ambPrecision;
			int aaSamples;
			int width;
			int height;
			bool useShadows;
			float globalAmbient;
			float globalDiffuse;
			float globalSpecular;
			float reflections;
			*/

			if (param == "ambient-occlusion") {
				// Min rays, Max rays, Precision...		
				MiniParser(value, ',').getInt(ambMinRays).getInt(ambMaxRays).getDouble(ambPrecision).getInt(ambSmooth).getDouble(ambExponent);
				INFO(QString("Min: %1, Max: %2, Prec: %3, ").arg(ambMinRays).arg(ambMaxRays).arg(ambPrecision));
		
			} else if (param == "anti-alias") {
				// Min rays, Max rays, Precision...		
				MiniParser(value, ',').getInt(aaSamples);
				
			} else if (param == "phong") {
				MiniParser(value, ',').getDouble(globalAmbient).getDouble(globalDiffuse).getDouble(globalSpecular);
				INFO(QString("Ambient: %1, Diffuse: %2, Specular: %3").arg(globalAmbient).arg(globalDiffuse).arg(globalSpecular));
			} else if (param == "shadows") {
				MiniParser(value, ',').getBool(useShadows);
				INFO(QString("Shadows: %3").arg(useShadows ? "true" : "false"));
		
			} else if (param == "reflection") {
				/*
				MiniParser(param,value, ',').getDouble(reflection);
				INFO(QString("Reflection: %3").arg(reflection));
		*/
			} else {
				WARNING("Unknown parameter: " + param);
			}
		}



		/*

		*/



	}
}

