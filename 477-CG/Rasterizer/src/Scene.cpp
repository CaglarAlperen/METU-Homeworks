#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <cmath>
#include <algorithm>

#include "Scene.h"
#include "Camera.h"
#include "Color.h"
#include "Mesh.h"
#include "Rotation.h"
#include "Scaling.h"
#include "Translation.h"
#include "Triangle.h"
#include "Vec3.h"
#include "tinyxml2.h"
#include "Helpers.h"

using namespace tinyxml2;
using namespace std;

//#define TO_RADIAN 0.017453292519944444

/*
	Transformations, clipping, culling, rasterization are done here.
	You may define helper functions.
*/

/*
=> Modelling Transformation 
=> Camera Transformation
=> Projection Transformation
=> Clipping
=> Find New Start&End for Clipped Line
=> Perspective division 
=> Viewport Transformation
=> Rasterization
FORWARD RENDERING PIPELINE ALGORITHM:
For every Mesh:
	For every Triangle of Mesh:
		Map Triangle Vertex IDs to  (x,y,z)  									=>Done
		Map A(x,y,z) => B(x,y,z,t=1,colorID) 									=>Done
		For every modelling transformation for Mesh:							=>Done
			Apply rotate														=>Done
			scale																=>Done
			transfer to all three B’s (vertices)								=>Done
		Apply Camera Tranformation for Mesh										=>Done
		If culling is enabled:													=>Done
			If we are seeing the back side : skip triangle						=>Done
		if proj_type == perspective:											=>Done
			M(Perspective(camera))*Vertex1,2,3									=>Done
		else(means orthographic):												=>Done
			M(Orthographic(camera))*Vertex1,2,3									=>Done
		if clipping needed (I don't know the case yet):
			Apply liang barsky or cohen-sutherland clipping to all three vertices
		Apply perspective divide to all three vertices							=>Done
		Apply viewport modelling
		apply triangle rasterization
*/

void translateVertex(Vec4* vertex1,Vec4* vertex2,Vec4* vertex3,Translation* t);
void scaleVertex(Vec4* vertex1,Vec4* vertex2,Vec4* vertex3,Scaling* s);
void rotateVertex(Vec4* vertex1,Vec4* vertex2,Vec4* vertex3,Rotation* r);
void applyCamTransformation(Vec4* vertex1,Vec4* vertex2, Vec4* vertex3, Camera* camera);
bool cullMeMaybe(Vec4* vertex1, Vec4* vertex2, Vec4* vertex3,Camera* camera);
void perspectiveProjection(Vec4* vertex1, Vec4* vertex2, Vec4* vertex3,Camera* camera);
void orthographicProjection(Vec4* vertex1, Vec4* vertex2, Vec4* vertex3,Camera* camera);
void perspectiveDivide(Vec4* vertex1,Vec4* vertex2, Vec4*vertex3);
void perspectiveDivide(vector<Vec4*> &line);
bool isVisible(double den, double num, double* t_e, double* t_l);
void liangBarskyLineClipping(Vec4* vertex1, Vec4* vertex2);
void viewportTransformation(vector<Vec4*> line,Camera* camera);
void viewportTransformation(Vec4* vertex1, Vec4* vertex2, Vec4* vertex3,Camera* camera);
void Rasterize(vector<vector<Vec4*>> lines, int type, vector<Color*> colorsOfVertices, vector<vector<Color>> &image);
void draw(int x, int y, Color c, vector<vector<Color>> &image);
double f01(int x, int y, int x0, int x1, int y0, int y1);
double f12(int x, int y, int x1, int x2, int y1, int y2);
double f20(int x, int y, int x2, int x0, int y2, int y0);
int my_clamp(int val, int lo, int hi);

void Scene::forwardRenderingPipeline(Camera *camera)
{

	cout << "****************************************\nImage: " << camera->outputFileName << "\n********************************" << endl;
	// TODO: Implement this function.
	for(Mesh* mesh: meshes){
		
		for(int i = 0; i < mesh->numberOfTriangles; i++){ 
			
			Vec3* v1 = vertices[mesh->triangles[i].getFirstVertexId()-1];
			Vec3* v2 = vertices[mesh->triangles[i].getSecondVertexId()-1];
			Vec3* v3 = vertices[mesh->triangles[i].getThirdVertexId()-1]; 

			Vec4 *vertex1 = new Vec4(v1->x,v1->y,v1->z,1.0,v1->colorId);
			Vec4 *vertex2 = new Vec4(v2->x,v2->y,v2->z,1.0,v2->colorId);
			Vec4 *vertex3 = new Vec4(v3->x,v3->y,v3->z,1.0,v3->colorId);
			//[x,y,z,1]
			
			// 	Applying every modelling transformation listed for this mesh
			//	Combining every one these transformations into one matrix
			//	then multiplying it with our vertices, is a good optimization
			//	However we are not currently doing it.

			// cout << "First" << endl;
			// cout << "v1: " << *vertex1 << " v2: " << *vertex2 << endl;

			//MODELLING TRANSFORMATIONS
			for(int i = 0; i < mesh->numberOfTransformations; i++){ 			
				if(mesh->transformationTypes[i] == 't'){ // Translation
					Translation* t = translations[mesh->transformationIds[i]-1];
					translateVertex(vertex1,vertex2,vertex3,t);
				}
				else if(mesh->transformationTypes[i] == 's'){ // Scaling
					Scaling* s = scalings[mesh->transformationIds[i]-1];
					scaleVertex(vertex1,vertex2,vertex3,s);
				}
				else if(mesh->transformationTypes[i] == 'r'){ // Rotation
					Rotation* r = rotations[mesh->transformationIds[i]-1];
					rotateVertex(vertex1,vertex2,vertex3,r);
				}
				else{
					cout<< "SOMETHING WENT WRONG"<<endl;
				}
			}

			// cout << "Modelling Transformation" << endl;
			// cout << "v1: " << *vertex1 << " v2: " << *vertex2 << endl;
				
			//CAMERA TRANSFORMATION
			applyCamTransformation(vertex1,vertex2,vertex3,camera);

			// cout << "Camera Transformation" << endl;
			// cout << "v1: " << *vertex1 << " v2: " << *vertex2 << endl;
			
			if(camera->projectionType){// 1 for perspective, 0 for orthographic
				perspectiveProjection(vertex1,vertex2,vertex3,camera);
			}
			else{
				orthographicProjection(vertex1,vertex2,vertex3,camera);
			}

			cout << "Perspective Projection" << endl;
			cout << *vertex1 << " " << *vertex2 << " " << *vertex3 << endl;

			/*******************************CULLING TEST**************************/
			if(cullingEnabled){ 
				Vec4* vertex1Culling = new Vec4(*vertex1);
				Vec4* vertex2Culling = new Vec4(*vertex2);
				Vec4* vertex3Culling = new Vec4(*vertex3);
				//perspectiveDivide(vertex1Culling,vertex2Culling,vertex3Culling);
				if(cullMeMaybe(vertex1Culling,vertex2Culling,vertex3Culling,camera)){
					cout << "CULLED" << endl;
					continue;
				}
			}
			/*******************************CULLING TEST**************************/

			/*************************WIREFRAME*********************************/
			vector<vector<Vec4*>> lines;

			if(mesh->type == 0){
				Vec4 *clip_vertex1_1 = new Vec4(*vertex1);
				Vec4 *clip_vertex1_2 = new Vec4(*vertex1);
				Vec4 *clip_vertex2_1 = new Vec4(*vertex2);
				Vec4 *clip_vertex2_2 = new Vec4(*vertex2);
				Vec4 *clip_vertex3_1 = new Vec4(*vertex3);
				Vec4 *clip_vertex3_2 = new Vec4(*vertex3);

				vector<Vec4*> line1; //???? NEYDEN NEYE CIZGI CIZELIM KI????? 1-2 1-3 3-1 vs ?
				vector<Vec4*> line2;
				vector<Vec4*> line3;

				line1.push_back(clip_vertex1_1);
				line1.push_back(clip_vertex2_1);
				line2.push_back(clip_vertex2_2);
				line2.push_back(clip_vertex3_1);
				line3.push_back(clip_vertex3_2);
				line3.push_back(clip_vertex1_2);

				if (!line1.empty()) lines.push_back(line1);
				if (!line2.empty()) lines.push_back(line2);
				if (!line3.empty()) lines.push_back(line3);

				if(line1.empty()){
					cout << "LAN LINE 1 BOOS" << endl;
				}
				if(line2.empty()){
					cout << "LAN LINE 2 BOOS" << endl;
				}
				if(line3.empty()){
					cout << "LAN LINE 3 BOOS" << endl;
				};

				for (int i = 0; i < lines.size(); i++)
				{
					perspectiveDivide(lines[i]);
				}

				cout << "Perspective Division" << endl;
				for (vector<Vec4*> line : lines)
				{
					cout << *line[0] << " " << *line[1] << endl;
				}

				liangBarskyLineClipping(clip_vertex1_1, clip_vertex2_1);
				liangBarskyLineClipping(clip_vertex2_2, clip_vertex3_1);
				liangBarskyLineClipping(clip_vertex3_2, clip_vertex1_2);
			
				

				cout << "Liang-Barsky" << endl;
				for (vector<Vec4*> line : lines)
				{
					cout << *line[0] << " " << *line[1] << endl;
				}

				
				
				for (int i = 0; i < lines.size(); i++)
				{
					viewportTransformation(lines[i], camera);
				}

				cout << "Viewport Transformation" << endl;
				for (vector<Vec4*> line : lines)
				{
					cout << *line[0] << " " << *line[1] << endl;
				}

				for (vector<Vec4*> line : lines)
				{
					// line[0]->x = my_clamp((int)line[0]->x, 0, camera->horRes-1);
					// line[0]->y = my_clamp((int)line[0]->y, 0, camera->verRes-1);
					
					// line[1]->x = my_clamp((int)line[1]->x, 0, camera->horRes-1);
					// line[1]->y = my_clamp((int)line[1]->y, 0, camera->verRes-1);
				}	

				cout << "Values Rounded" << endl;
				for (vector<Vec4*> line : lines)
				{
					cout << *line[0] << " " << *line[1] << endl;
				}

				Rasterize(lines, mesh->type, colorsOfVertices, image);
			}
			/*************************WIREFRAME*********************************/
			
			/*************************SOLID*********************************/
			else{

				perspectiveDivide(vertex1,vertex2,vertex3);

				cout << "Perspective Division" << endl;
				cout << *vertex1 << " " << *vertex2 << " " << *vertex3 << endl;

				viewportTransformation(vertex1,vertex2,vertex3,camera);

				cout << "Viewport Tranformation" << endl;
				cout << *vertex1 << " " << *vertex2 << " " << *vertex3 << endl;


				vertex1->x = round(vertex1->x);
				vertex1->y = round(vertex1->y);

				vertex2->x = round(vertex2->x);
				vertex2->y = round(vertex2->y);

				vertex3->x = round(vertex3->x);
				vertex3->y = round(vertex3->y);

				cout << "Values Rounded" << endl;
				cout << *vertex1 << " " << *vertex2 << " " << *vertex3 << endl;

				vector<Vec4*> line1;
				line1.push_back(vertex1);
				line1.push_back(vertex2);
				vector<Vec4*> line2;
				line2.push_back(vertex2);
				line2.push_back(vertex3);
				vector<Vec4*> line3;
				line3.push_back(vertex3);
				line3.push_back(vertex1);

				lines.push_back(line1);
				lines.push_back(line2);
				lines.push_back(line3);

				Rasterize(lines, mesh->type, colorsOfVertices, image);
			}
			/*************************SOLID*********************************/

			

			/*if(camera->projectionType){  //BUNA GEREK VAR MI EMIN OLAMADIM YAPALIM GITSIN DIREKT 
				perspectiveDivide(vertex1,vertex2,vertex3);
			}*/



		}

	}		
}

void translateVertex(Vec4* vertex1,Vec4* vertex2,Vec4* vertex3,Translation* t){
	double translationMatrix[4][4] = {{1, 0, 0, t->tx},
                                      {0, 1, 0, t->ty},
                                      {0, 0, 1, t->tz},
                                      {0, 0, 0, 1}};
	*vertex1 = multiplyMatrixWithVec4(Matrix4(translationMatrix),*vertex1);
	*vertex2 = multiplyMatrixWithVec4(Matrix4(translationMatrix),*vertex2);
	*vertex3 = multiplyMatrixWithVec4(Matrix4(translationMatrix),*vertex3);
}

void scaleVertex(Vec4* vertex1,Vec4* vertex2,Vec4* vertex3,Scaling* s){
	double scalingMatrix[4][4] = {{s->sx, 0, 0, 0},
                                  {0, s->sy, 0, 0},
                                  {0, 0, s->sz, 0},
                                  {0, 0, 0, 1}};
	*vertex1 = multiplyMatrixWithVec4(Matrix4(scalingMatrix),*vertex1);
	*vertex2 = multiplyMatrixWithVec4(Matrix4(scalingMatrix),*vertex2);
	*vertex3 = multiplyMatrixWithVec4(Matrix4(scalingMatrix),*vertex3);		  
}

void rotateVertex(Vec4* vertex1,Vec4* vertex2,Vec4* vertex3, Rotation* r){
	//Using the orthonormal axis method
	double angle = r->angle * 0.017453292519944444;				// Angle converted to radians
	Vec3 u = Vec3(r->ux,r->uy,r->uz,0); 						// last argument is color ID but it is unnecessary for us now
	Vec3 absU = Vec3(abs(r->ux),abs(r->uy),abs(r->uz),0); 		// we compare absolute values
	Vec3 v; 													// we construct v by 0ing smallest element of u, and swapping remaining two
	
	if(absU.x <= absU.y && absU.x <= absU.z){ 					// x is the absolutely smallest element
		v.x = 0;
		v.y = -u.z;
		v.z = u.y;
	}
	else if (absU.y <= absU.x && absU.y <= absU.z){ 			// y is the absolutely smallest element
		v.x = -u.z; 
		v.y = 0;
		v.z = u.x;
	}
	else if (absU.z <= absU.x && absU.z <= absU.y){				// z is the absolutely smallest element
		v.x = -u.y;
		v.y = u.x;
		v.z = 0;
	}
	else{
		cout << "SOMETHING IS WRONG I CAN FEEL IT" << endl;
	}

	Vec3 w = crossProductVec3(u,v);								// w is cross product of u and v
	v = normalizeVec3(v);										// normalizing v
	w = normalizeVec3(w);										// normalizing w
	
	double orthonormalMatrix[4][4] = {{u.x,u.y,u.z,0},
									  {v.x,v.y,v.z,0},
									  {w.x,w.y,w.z,0},
									  {0  ,0  ,0  ,1}};
	double inverseOrthonormalMatrix[4][4] = {{u.x,v.x,w.x,0},
											 {u.y,v.y,w.y,0},
											 {u.z,v.z,w.z,0},
											 {0  ,0  ,0  ,1}};
	double rotationMatrix[4][4] = {	{1,0,0,0},
									{0,cos(angle),-sin(angle),0},
									{0,sin(angle),cos(angle),0},
									{0,0,0,1}};
	
	*vertex1 = multiplyMatrixWithVec4(Matrix4(orthonormalMatrix),*vertex1);   			// Multiply with orthonormal matrix
	*vertex2 = multiplyMatrixWithVec4(Matrix4(orthonormalMatrix),*vertex2);
	*vertex3 = multiplyMatrixWithVec4(Matrix4(orthonormalMatrix),*vertex3);

	*vertex1 = multiplyMatrixWithVec4(Matrix4(rotationMatrix),*vertex1);				// Multiply with rotation wrt x matrix
	*vertex2 = multiplyMatrixWithVec4(Matrix4(rotationMatrix),*vertex2);
	*vertex3 = multiplyMatrixWithVec4(Matrix4(rotationMatrix),*vertex3);

	*vertex1 = multiplyMatrixWithVec4(Matrix4(inverseOrthonormalMatrix),*vertex1);		// Multiply with inverse orthonormal matrix
	*vertex2 = multiplyMatrixWithVec4(Matrix4(inverseOrthonormalMatrix),*vertex2);
	*vertex3 = multiplyMatrixWithVec4(Matrix4(inverseOrthonormalMatrix),*vertex3);
}

void applyCamTransformation(Vec4* vertex1,Vec4* vertex2,Vec4* vertex3,Camera* camera){
	Vec3 u = Vec3(camera->u.x,camera->u.y,camera->u.z,0);
	Vec3 v = Vec3(camera->v.x,camera->v.y,camera->v.z,0);
	Vec3 w = Vec3(camera->w.x,camera->w.y,camera->w.z,0);
	Vec3 e = Vec3(camera->pos.x,camera->pos.y,camera->pos.z,0);
	double rc0 = -(u.x*e.x+u.y*e.y+u.z*e.z);
	double rc1 = -(v.x*e.x+v.y*e.y+v.z*e.z);
	double rc2 = -(w.x*e.x+w.y*e.y+w.z*e.z);

	double cameraTransformationMatrix[4][4] = {{u.x,u.y,u.z,rc0},
											   {v.x,v.y,v.z,rc1},
											   {w.x,w.y,w.z,rc2},
											   {0,0,0,1}};
	
	*vertex1 = multiplyMatrixWithVec4(Matrix4(cameraTransformationMatrix),*vertex1);   			// Multiply with orthonormal matrix
	*vertex2 = multiplyMatrixWithVec4(Matrix4(cameraTransformationMatrix),*vertex2);
	*vertex3 = multiplyMatrixWithVec4(Matrix4(cameraTransformationMatrix),*vertex3);
}

bool cullMeMaybe(Vec4* A, Vec4* B, Vec4* C,Camera* camera){
	/*
	Vec3f getNormal(Vec3f A, Vec3f B, Vec3f C)
	{
		Vec3f AB = add(B, multS(A, -1)); // B - A
		Vec3f AC = add(C, multS(A, -1)); // C - A
		Vec3f normal = cross(AB, AC);
		return normalize(normal);}
	*/
	cout << "Culling" << endl;
	cout << *A << " " << *B << " " << *C << endl; 

	Vec3 AB = subtractVec3(Vec3(B->x,B->y,B->z,0),Vec3(A->x,A->y,A->z,0));
	Vec3 AC = subtractVec3(Vec3(C->x,C->y,C->z,0),Vec3(A->x,A->y,A->z,0));
	Vec3 normal = crossProductVec3(AB,AC);
	normal = normalizeVec3(normal);

	Vec3 fromTheEyeToAnyPointOnThePolygon = subtractVec3(Vec3(A->x,A->y,A->z,0),Vec3(0,0,0,0));

	cout << "Normal: " << normal << " v: " << fromTheEyeToAnyPointOnThePolygon << endl;

	if(dotProductVec3(normal,fromTheEyeToAnyPointOnThePolygon) < 0){
		return true; //BACK FACED TRIANGLE
	}
	return false; //FRONT FACED TRIANGLE
}
void perspectiveProjection(Vec4* vertex1, Vec4* vertex2, Vec4* vertex3,Camera* camera){
	double l = camera->left, r = camera-> right;
	double t = camera->top, b = camera-> bottom;
	double n = camera->near, f = camera-> far;
	double perspectiveProjectionMatrix[4][4] = {{	2*n/(r-l),	0		 ,	(r+l)/(r-l)	,		0		 },
												{	0		 ,	2*n/(t-b), 	(t+b)/(t-b)	, 		0		 },
												{	0		 ,	0		 ,	-(f+n)/(f-n),	-2*f*n/(f-n) },
												{	0		 ,	0		 ,	-1			,		0		 }};
	*vertex1 = multiplyMatrixWithVec4(Matrix4(perspectiveProjectionMatrix),*vertex1);
	*vertex2 = multiplyMatrixWithVec4(Matrix4(perspectiveProjectionMatrix),*vertex2);
	*vertex3 = multiplyMatrixWithVec4(Matrix4(perspectiveProjectionMatrix),*vertex3);
													
}
void orthographicProjection(Vec4* vertex1, Vec4* vertex2, Vec4* vertex3,Camera* camera){
	double l = camera->left, r = camera-> right;
	double t = camera->top, b = camera-> bottom;
	double n = camera->near, f = camera-> far;
	double orthographicProjectionMatrix[4][4] = {{2/(r-l),	0		,	0		,	-(r+l)/(r-l)	},
                                    			{0		 ,	2/(t-b) ,	0		,	-(t+b)/(t-b)	},
                                    			{0		 ,	0		,	-2/(f-n),	-(f+n)/(f-n)	},
                                    			{0		 ,	0		,	0		,	 1				}};

	*vertex1 = multiplyMatrixWithVec4(Matrix4(orthographicProjectionMatrix),*vertex1);
	*vertex2 = multiplyMatrixWithVec4(Matrix4(orthographicProjectionMatrix),*vertex2);
	*vertex3 = multiplyMatrixWithVec4(Matrix4(orthographicProjectionMatrix),*vertex3);
}

void perspectiveDivide(Vec4* vertex1,Vec4* vertex2, Vec4*vertex3){ // I AM NOT SURE IF THIS WORKS!!!!
	vertex1->x = vertex1->x/vertex1->t;
	vertex1->y = vertex1->y/vertex1->t;
	vertex1->z = vertex1->z/vertex1->t;
	vertex1->t = vertex1->t/vertex1->t;


	vertex2->x = vertex2->x/vertex2->t;
	vertex2->y = vertex2->y/vertex2->t;
	vertex2->z = vertex2->z/vertex2->t;
	vertex2->t = vertex2->t/vertex2->t;


	vertex3->x = vertex3->x/vertex3->t;
	vertex3->y = vertex3->y/vertex3->t;
	vertex3->z = vertex3->z/vertex3->t;
	vertex3->t = vertex3->t/vertex3->t;

}

void perspectiveDivide(vector<Vec4*> &line){
	
	line[0]->x = line[0]->x / line[0]->t;
	line[0]->y = line[0]->y / line[0]->t;
	line[0]->z = line[0]->z / line[0]->t;
	line[0]->t = line[0]->t / line[0]->t;

	line[1]->x = line[1]->x / line[1]->t;
	line[1]->y = line[1]->y / line[1]->t;
	line[1]->z = line[1]->z / line[1]->t;
	line[1]->t = line[1]->t / line[1]->t;

}

bool isVisible(double den, double num, double* t_e, double* t_l){
	double t;
	if(den > 0){
		t = num/den;
		if(t > *t_l){
			return false;
		}
		if(t > *t_e){
			*t_e = t;
		}
	}
	else if( den < 0){
		t = num/den;
		if(t < *t_e){
			return false;
		}
		if(t < *t_l){
			*t_l = t;
		}
	}
	else if(num > 0){
		return false;
	}
	return true;
}

void liangBarskyLineClipping(Vec4* vertex1, Vec4* vertex2){
	//THIS IMPLEMENTATION MODIFIES VERTICES, actually like every other one until now :D
	//CLIPPING BEFORE PDIVISION!!!!!!!!!!!!
	cout << "Liang-Barsky" << endl;
	//cout << "v1: " << *vertex1 << " v2: " << *vertex2 << endl;
	//cout << "Vertex1_w: " << vertex1->t << " Vertex2_w: " << vertex2->t << endl;

	double t_e = 0,t_l = 1;bool visible = false;	
	double dx = vertex2->x - vertex1->x;  
	double dy = vertex2->y - vertex1->y;
	double dz = vertex2->z - vertex1->z;
	// double w;
	// if(abs(vertex1->t) >= abs(vertex2->t)){
	// 	w = vertex1->t;
	// }
	// else{
	// 	w = vertex2->t;
	// }
	 double xyzMax = 1;
	 double xyzMin = -1;

	//Visible is a reserved keyword, who would have thought -_-
	//Or it reported an error due to flag's name :/
	if(isVisible(dx, xyzMin - vertex1->x, &t_e, &t_l)){
		cout << "ASAMA1" << endl;
		if(isVisible(-dx, vertex1->x - xyzMax, &t_e, &t_l)){
			cout << "ASAMA2" << endl;
			if(isVisible(dy, xyzMin - vertex1->y, &t_e, &t_l)){
				cout << "ASAMA3" << endl;
				if(isVisible(-dy, vertex1->y - xyzMax, &t_e, &t_l)){
					cout << "ASAMA4" << endl;
					if(isVisible(dz, xyzMin - vertex1->z, &t_e, &t_l)){
						cout << "ASAMA5" << endl;
						if(isVisible(-dz, vertex1->z - xyzMax, &t_e, &t_l)){
							cout << "AŞAMA6" << endl;
							visible = true;
							if(t_l < 1){
								cout << "***********UPDATE1**************" << endl;
								vertex2->x = vertex1->x + dx * t_l;
								vertex2->y = vertex1->y + dy * t_l;
								vertex2->z = vertex1->z + dz * t_l;
							}
							if(t_e > 0){
								cout << "***********UPDATE2**************" << endl;
								vertex1->x = vertex1->x + dx * t_e;
								vertex1->y = vertex1->y + dy * t_e;
								vertex1->z = vertex1->z + dz * t_e;
							}
						}
					}
				}
			}
		}
	}
}

void viewportTransformation(vector<Vec4*> line,Camera* camera){
	if(line.size() == 0){
		cout << "AGA BU LINE BOS BUNU NIYE BURAYA KADAR GETIRDIN KI?" << endl;
	}
	double n_x = camera->horRes;
	double n_y = camera->verRes;

	cout << "*************************VIEWPORT*********************************" << endl;
	cout << "n_x: " << n_x << " n_y: " << n_y << endl;
	cout << *line[0] << " " << *line[1] << endl;

	//last row is placeholder for matrix multiplication;
	double viewportTransformationMatrix[4][4] = {{n_x/2, 0    , 0  , (n_x-1)/2},
											     {0    , n_y/2, 0  , (n_y-1)/2},
											     {0    , 0    , 0.5, 0.5      },
											     {0    , 0    , 0  , 0        }};

	if(line.size() == 2){
		*line[0] = multiplyMatrixWithVec4(Matrix4(viewportTransformationMatrix),*line[0]);
		*line[1] = multiplyMatrixWithVec4(Matrix4(viewportTransformationMatrix),*line[1]);
		//result.push_back(new Vec3(line[0]->x,line[0]->y,line[0]->z,line[0]->colorId));
		//result.push_back(new Vec3(line[0]->x,line[0]->y,line[0]->z,line[0]->colorId));

		cout << *line[0] << " " << *line[1] << endl;
	}
}

void viewportTransformation(Vec4* vertex1, Vec4* vertex2, Vec4* vertex3,Camera* camera){
	double n_x = camera->horRes;
	double n_y = camera->verRes;
	//last row is placeholder for matrix multiplication;
	double viewportTransformationMatrix[4][4] = {{n_x/2, 0    , 0  , (n_x-1)/2},
											     {0    , n_y/2, 0  , (n_y-1)/2},
											     {0    , 0    , 0.5, 0.5      },
											     {0    , 0    , 0  , 0        }};
	*vertex1 = multiplyMatrixWithVec4(Matrix4(viewportTransformationMatrix),*vertex1);
	*vertex2 = multiplyMatrixWithVec4(Matrix4(viewportTransformationMatrix),*vertex2);
	*vertex3 = multiplyMatrixWithVec4(Matrix4(viewportTransformationMatrix),*vertex3);
}

void draw(int x, int y, Color c, vector<vector<Color>> &image)
{
	//cout << "Drawing..." << endl;
	if (x >= 0 && x < image.size() && y >= 0 && y < image[0].size())
		image[x][y] = c;
	//cout << "Drawing finished" << endl;
}

void Rasterize(vector<vector<Vec4*>> lines, int type, vector<Color*> colorsOfVertices, vector<vector<Color>> &image)
{
	cout << "Rasterize: " << endl;
	// Line rasterization
	if (type == 0) // Wireframe
	{
		cout << "Wireframe" << endl;
		// The Midpoint Algorithm
		float d; // Slope according to midpoint
		float m; // Slope of the line
		int x0,x1,y0,y1; // Viewport coordinates of the line
		int x,y; // Variables of midpoint algorithm
		double c0r,c0g,c0b,c1r,c1g,c1b; // color values of endpoints 
		double cr, cg, cb; // color variable of midpoint algorithm
		double dcr, dcg, dcb; // color change rate

		for (vector<Vec4*> line : lines)
		{
			cout << "Line: " << *line[0] << " " << *line[1] << endl;
			Vec4* left =  (line[0]->x <= line[1]->x) ? line[0] : line[1];
			Vec4* right = (line[0]->x > line[1]->x) ? line[0] : line[1];
			x0 = left->x;
			x1 = right->x;
			y0 = left->y;
			y1 = right->y;
			c0r = colorsOfVertices[left->colorId-1]->r;
			c0g = colorsOfVertices[left->colorId-1]->g;
			c0b = colorsOfVertices[left->colorId-1]->b;
			c1r = colorsOfVertices[right->colorId-1]->r;
			c1g = colorsOfVertices[right->colorId-1]->g;
			c1b = colorsOfVertices[right->colorId-1]->b;

			m = (y1 - y0) / (float)(x1 - x0);

			cout << "m: " << m << endl;

			if (m >= 0 && m <= 1) // Standard algorithm
			{
				cout << "m (0,1]" << endl;
				y = y0;
				d = 2*(y0 - y1) + (x1 - x0);
				cr = c0r;
				cg = c0g;
				cb = c0b;
				dcr = (c1r - c0r) / (double)(x1 - x0);
				dcg = (c1g - c0g) / (double)(x1 - x0);
				dcb = (c1b - c0b) / (double)(x1 - x0);
				for (x = x0; x <= x1; x++)
				{
					Color *c = new Color(cr,cg,cb);
					draw(x,y,*c, image);
					if (d < 0) // Over the midpoint
					{
						y++;
						d += 2*(y0 - y1 + x1 - x0);
					}
					else // Below the midpoint
					{
						d += 2*(y0-y1);
					}
					cr += dcr;
					cg += dcg;
					cb += dcb;
				}
			} 
			else if (m > 1) // Swap the roles of x and y
			{
				cout << "m > 1" << endl;
				x = x0;
				d = 2*(x0 - x1) + (y1 - y0);
				cr = c0r;
				cg = c0g;
				cb = c0b;
				dcr = (c1r - c0r) / (double)(y1 - y0);
				dcg = (c1g - c0g) / (double)(y1 - y0);
				dcb = (c1b - c0b) / (double)(y1 - y0);

				for (y = y0; y <= y1; y++)
				{
					Color *c = new Color(cr,cg,cb);
					draw(x,y,*c, image);
					if (d < 0) // Over the midpoint
					{
						x++;
						d += 2*(x0 - x1 + y1 - y0);
					}
					else // Below the midpoint
					{
						d += 2*(x0-x1);
					}
					cr += dcr;
					cg += dcg;
					cb += dcb;
				}
			}
			else if (m < 0 && m >= -1) // from v1 to v0 (reversed standard)
			{
				cout << "m [-1,0)" << endl;
				y = y1;
				d = 2*(y1 - y0) + (x1 - x0);
				cr = c1r;
				cg = c1g;
				cb = c1b;
				dcr = (c0r - c1r) / (double)(x1 - x0);
				dcg = (c0g - c1g) / (double)(x1 - x0);
				dcb = (c0b - c1b) / (double)(x1 - x0);

				for (x = x1; x >= x0; x--)
				{
					Color *c = new Color(cr,cg,cb);
					draw(x,y,*c, image);
					if (d < 0) // Over the midpoint
					{
						y++;
						d += 2*(y1 - y0 + x1 - x0);
					}
					else // Below the midpoint
					{
						d += 2*(y1-y0);
					}
					cr += dcr;
					cg += dcg;
					cb += dcb;
				}
			}
			else if (m < -1) // Swap x and y and negate d and y control
			{
				cout << "m < -1" << endl;
				x = x1;
				d = 2*(x0 - x1) + (y0 - y1);
				cr = c1r;
				cg = c1g;
				cb = c1b;
				dcr = (c0r - c1r) / (double)(y0 - y1);
				dcg = (c0g - c1g) / (double)(y0 - y1);
				dcb = (c0b - c1b) / (double)(y0 - y1);
				for (y = y1; y <= y0; y++)
				{
					Color *c = new Color(cr,cg,cb);
					draw(x,y,*c, image);
					if (d < 0) // Over the midpoint
					{
						x--;
						d += 2*(x0 - x1 + y0 - y1);
					}
					else // Below the midpoint
					{
						d += 2*(x0 - x1);
					}
					cr += dcr;
					cg += dcg;
					cb += dcb;
				}
			}
		}
	}
	else if (type == 1) // Solid
	{
		cout << "Solid" << endl;
		int x0,y0,x1,y1,x2,y2; // Coordinates of vertices
		int xmin,xmax,ymin,ymax; // Bounding box
		int nx, ny; // Viewport limits
		int x,y; // Algorithm variables
		double alpha,beta,gamma; // Barycentric coefficients
		double c0r,c0g,c0b,c1r,c1g,c1b,c2r,c2g,c2b; // color values of endpoints
		double cr,cg,cb;

		cout << "Here3, len_lines: " << lines.size() << endl;
		cout << "Line1: " << *lines[0][0] << " " << *lines[0][1] << endl;
		cout << "Line2: " << *lines[1][0] << " " << *lines[1][1] << endl;
		cout << "Line3: " << *lines[2][0] << " " << *lines[2][1] << endl;


		x0 = lines[0][0]->x;
		y0 = lines[0][0]->y;
		c0r = colorsOfVertices[lines[0][0]->colorId-1]->r;
		c0g = colorsOfVertices[lines[0][0]->colorId-1]->g;
		c0b = colorsOfVertices[lines[0][0]->colorId-1]->b;
		x1 = lines[1][0]->x;
		y1 = lines[1][0]->y;
		c1r = colorsOfVertices[lines[1][0]->colorId-1]->r;
		c1g = colorsOfVertices[lines[1][0]->colorId-1]->g;
		c1b = colorsOfVertices[lines[1][0]->colorId-1]->b;
		x2 = lines[2][0]->x;
		y2 = lines[2][0]->y;
		c2r = colorsOfVertices[lines[2][0]->colorId-1]->r;
		c2g = colorsOfVertices[lines[2][0]->colorId-1]->g;
		c2b = colorsOfVertices[lines[2][0]->colorId-1]->b;

		cout << "Here1" << endl;

		xmin = min(x0,min(x1,x2));
		xmax = max(x0,max(x1,x2));
		ymin = min(y0,min(y1,y2));
		ymax = max(y0,max(y1,y2));

		cout << "Here2" << endl;

		nx = image.size(); // hor_res
		ny = image[0].size(); // ver_res

		cout << "Values initiated, nx: " << nx << " ny: " << ny << endl;

		for (y = ymin; y < ymax; y++)
		{
			if (y < 0 || y >= ny) continue;
			for (x = xmin; x < xmax; x++)
			{
				if (x < 0 || x >= nx) continue;

				alpha = f12(x,y,x1,x2,y1,y2) / f12(x0,y0,x1,x2,y1,y2);
				beta = f20(x,y,x2,x0,y2,y0) / f20(x1,y1,x2,x0,y2,y0);
				gamma = f01(x,y,x0,x1,y0,y1) / f01(x2,y2,x0,x1,y0,y1);

				if (alpha >= 0 && beta >= 0 && gamma >= 0)
				{
					cr = alpha*c0r + beta*c1r + gamma*c2r;
					cg = alpha*c0g + beta*c1g + gamma*c2g;
					cb = alpha*c0b + beta*c1b + gamma*c2b;
					Color *c = new Color(cr,cg,cb);
					draw(x, y, *c, image);
				}
			}
		}
	}

}

double f01(int x, int y, int x0, int x1, int y0, int y1)
{
	return x*(y0 - y1) + y*(x1 - x0) + x0*y1 - y0*x1;
}

double f12(int x, int y, int x1, int x2, int y1, int y2)
{
	return x*(y1 - y2) + y*(x2 - x1) + x1*y2 - y1*x2;
}

double f20(int x, int y, int x2, int x0, int y2, int y0)
{
	return x*(y2 - y0) + y*(x0 - x2) + x2*y0 - y2*x0;
}

int my_clamp(int val, int lo, int hi)
{
	if (val < lo)
		return lo;
	if (val > hi)
		return hi;
	return val;
}


/*
	Parses XML file
*/
Scene::Scene(const char *xmlPath)
{
	const char *str;
	XMLDocument xmlDoc;
	XMLElement *pElement;

	xmlDoc.LoadFile(xmlPath);

	XMLNode *pRoot = xmlDoc.FirstChild();

	// read background color
	pElement = pRoot->FirstChildElement("BackgroundColor");
	str = pElement->GetText();
	sscanf(str, "%lf %lf %lf", &backgroundColor.r, &backgroundColor.g, &backgroundColor.b);

	// read culling
	pElement = pRoot->FirstChildElement("Culling");
	if (pElement != NULL) {
		str = pElement->GetText();
		
		if (strcmp(str, "enabled") == 0) {
			cullingEnabled = true;
		}
		else {
			cullingEnabled = false;
		}
	}

	// read cameras
	pElement = pRoot->FirstChildElement("Cameras");
	XMLElement *pCamera = pElement->FirstChildElement("Camera");
	XMLElement *camElement;
	while (pCamera != NULL)
	{
		Camera *cam = new Camera();

		pCamera->QueryIntAttribute("id", &cam->cameraId);

		// read projection type
		str = pCamera->Attribute("type");

		if (strcmp(str, "orthographic") == 0) {
			cam->projectionType = 0;
		}
		else {
			cam->projectionType = 1;
		}

		camElement = pCamera->FirstChildElement("Position");
		str = camElement->GetText();
		sscanf(str, "%lf %lf %lf", &cam->pos.x, &cam->pos.y, &cam->pos.z);

		camElement = pCamera->FirstChildElement("Gaze");
		str = camElement->GetText();
		sscanf(str, "%lf %lf %lf", &cam->gaze.x, &cam->gaze.y, &cam->gaze.z);

		camElement = pCamera->FirstChildElement("Up");
		str = camElement->GetText();
		sscanf(str, "%lf %lf %lf", &cam->v.x, &cam->v.y, &cam->v.z);

		cam->gaze = normalizeVec3(cam->gaze);
		cam->u = crossProductVec3(cam->gaze, cam->v);
		cam->u = normalizeVec3(cam->u);

		cam->w = inverseVec3(cam->gaze);
		cam->v = crossProductVec3(cam->u, cam->gaze);
		cam->v = normalizeVec3(cam->v);

		camElement = pCamera->FirstChildElement("ImagePlane");
		str = camElement->GetText();
		sscanf(str, "%lf %lf %lf %lf %lf %lf %d %d",
			   &cam->left, &cam->right, &cam->bottom, &cam->top,
			   &cam->near, &cam->far, &cam->horRes, &cam->verRes);

		camElement = pCamera->FirstChildElement("OutputName");
		str = camElement->GetText();
		cam->outputFileName = string(str);

		cameras.push_back(cam);

		pCamera = pCamera->NextSiblingElement("Camera");
	}

	// read vertices
	pElement = pRoot->FirstChildElement("Vertices");
	XMLElement *pVertex = pElement->FirstChildElement("Vertex");
	int vertexId = 1;

	while (pVertex != NULL)
	{
		Vec3 *vertex = new Vec3();
		Color *color = new Color();

		vertex->colorId = vertexId;

		str = pVertex->Attribute("position");
		sscanf(str, "%lf %lf %lf", &vertex->x, &vertex->y, &vertex->z);

		str = pVertex->Attribute("color");
		sscanf(str, "%lf %lf %lf", &color->r, &color->g, &color->b);

		vertices.push_back(vertex);
		colorsOfVertices.push_back(color);

		pVertex = pVertex->NextSiblingElement("Vertex");

		vertexId++;
	}

	// read translations
	pElement = pRoot->FirstChildElement("Translations");
	XMLElement *pTranslation = pElement->FirstChildElement("Translation");
	while (pTranslation != NULL)
	{
		Translation *translation = new Translation();

		pTranslation->QueryIntAttribute("id", &translation->translationId);

		str = pTranslation->Attribute("value");
		sscanf(str, "%lf %lf %lf", &translation->tx, &translation->ty, &translation->tz);

		translations.push_back(translation);

		pTranslation = pTranslation->NextSiblingElement("Translation");
	}

	// read scalings
	pElement = pRoot->FirstChildElement("Scalings");
	XMLElement *pScaling = pElement->FirstChildElement("Scaling");
	while (pScaling != NULL)
	{
		Scaling *scaling = new Scaling();

		pScaling->QueryIntAttribute("id", &scaling->scalingId);
		str = pScaling->Attribute("value");
		sscanf(str, "%lf %lf %lf", &scaling->sx, &scaling->sy, &scaling->sz);

		scalings.push_back(scaling);

		pScaling = pScaling->NextSiblingElement("Scaling");
	}

	// read rotations
	pElement = pRoot->FirstChildElement("Rotations");
	XMLElement *pRotation = pElement->FirstChildElement("Rotation");
	while (pRotation != NULL)
	{
		Rotation *rotation = new Rotation();

		pRotation->QueryIntAttribute("id", &rotation->rotationId);
		str = pRotation->Attribute("value");
		sscanf(str, "%lf %lf %lf %lf", &rotation->angle, &rotation->ux, &rotation->uy, &rotation->uz);

		rotations.push_back(rotation);

		pRotation = pRotation->NextSiblingElement("Rotation");
	}

	// read meshes
	pElement = pRoot->FirstChildElement("Meshes");

	XMLElement *pMesh = pElement->FirstChildElement("Mesh");
	XMLElement *meshElement;
	while (pMesh != NULL)
	{
		Mesh *mesh = new Mesh();

		pMesh->QueryIntAttribute("id", &mesh->meshId);

		// read projection type
		str = pMesh->Attribute("type");

		if (strcmp(str, "wireframe") == 0) {
			mesh->type = 0;
		}
		else {
			mesh->type = 1;
		}

		// read mesh transformations
		XMLElement *pTransformations = pMesh->FirstChildElement("Transformations");
		XMLElement *pTransformation = pTransformations->FirstChildElement("Transformation");

		while (pTransformation != NULL)
		{
			char transformationType;
			int transformationId;

			str = pTransformation->GetText();
			sscanf(str, "%c %d", &transformationType, &transformationId);

			mesh->transformationTypes.push_back(transformationType);
			mesh->transformationIds.push_back(transformationId);

			pTransformation = pTransformation->NextSiblingElement("Transformation");
		}

		mesh->numberOfTransformations = mesh->transformationIds.size();

		// read mesh faces
		char *row;
		char *clone_str;
		int v1, v2, v3;
		XMLElement *pFaces = pMesh->FirstChildElement("Faces");
        str = pFaces->GetText();
		clone_str = strdup(str);

		row = strtok(clone_str, "\n");
		while (row != NULL)
		{
			int result = sscanf(row, "%d %d %d", &v1, &v2, &v3);
			
			if (result != EOF) {
				mesh->triangles.push_back(Triangle(v1, v2, v3));
			}
			row = strtok(NULL, "\n");
		}
		mesh->numberOfTriangles = mesh->triangles.size();
		meshes.push_back(mesh);

		pMesh = pMesh->NextSiblingElement("Mesh");
	}
}

/*
	Initializes image with background color
*/
void Scene::initializeImage(Camera *camera)
{
	if (this->image.empty())
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			vector<Color> rowOfColors;

			for (int j = 0; j < camera->verRes; j++)
			{
				rowOfColors.push_back(this->backgroundColor);
			}

			this->image.push_back(rowOfColors);
		}
	}
	else
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			for (int j = 0; j < camera->verRes; j++)
			{
				this->image[i][j].r = this->backgroundColor.r;
				this->image[i][j].g = this->backgroundColor.g;
				this->image[i][j].b = this->backgroundColor.b;
			}
		}
	}
}

/*
	If given value is less than 0, converts value to 0.
	If given value is more than 255, converts value to 255.
	Otherwise returns value itself.
*/
int Scene::makeBetweenZeroAnd255(double value)
{
	if (value >= 255.0)
		return 255;
	if (value <= 0.0)
		return 0;
	return (int)(value);
}

/*
	Writes contents of image (Color**) into a PPM file.
*/
void Scene::writeImageToPPMFile(Camera *camera)
{
	ofstream fout;

	fout.open(camera->outputFileName.c_str());

	fout << "P3" << endl;
	fout << "# " << camera->outputFileName << endl;
	fout << camera->horRes << " " << camera->verRes << endl;
	fout << "255" << endl;

	for (int j = camera->verRes - 1; j >= 0; j--)
	{
		for (int i = 0; i < camera->horRes; i++)
		{
			fout << makeBetweenZeroAnd255(this->image[i][j].r) << " "
				 << makeBetweenZeroAnd255(this->image[i][j].g) << " "
				 << makeBetweenZeroAnd255(this->image[i][j].b) << " ";
		}
		fout << endl;
	}
	fout.close();
}

/*
	Converts PPM image in given path to PNG file, by calling ImageMagick's 'convert' command.
	os_type == 1 		-> Ubuntu
	os_type == 2 		-> Windows
	os_type == other	-> No conversion
*/
void Scene::convertPPMToPNG(string ppmFileName, int osType)
{
	string command;

	// call command on Ubuntu
	if (osType == 1)
	{
		command = "convert " + ppmFileName + " " + ppmFileName + ".png";
		system(command.c_str());
	}

	// call command on Windows
	else if (osType == 2)
	{
		command = "magick convert " + ppmFileName + " " + ppmFileName + ".png";
		system(command.c_str());
	}

	// default action - don't do conversion
	else
	{
	}
}