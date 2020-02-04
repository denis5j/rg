//------------------------------------------------------------
/// \file	Main.cpp
/// \author	Rob Bateman
/// \date	9-feb-2005
/// \brief	This sample demonstrates a bspline. I recommend
/// 		first looking at the bezier curve example before
/// 		this one.
/// 		The only difference between a bezier curve and
/// 		a bspline is that it uses a different set of
/// 		blending functions. This actually produces a much
/// 		shorter curve than the equivalent bezier, however
/// 		the advantage is that any curve that shares 3 control
/// 		points will remain continuos. ie, by drawing seperate
/// 		curve segments we can use as many control points
/// 		as needed.
//------------------------------------------------------------

#include <stdlib.h>
#include <glut.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>


#define PI 3.14159265

using namespace std;

template <class Type>
struct Triplet
{
	Type  x, y, z;
};

struct Matriks
{
	float x1, x2, x3;
	float y1, y2, y3;
	float z1, z2, z3;
};

vector<Triplet<float>> points;
vector<Triplet<float>> model_points;
vector<int> model_vertices;
Triplet<float> center;
Triplet<float> prev_point;
Triplet<float> tangent;
Triplet<float> second_derivation;
Triplet<float> binormal;
Triplet<float> normal;
Triplet<float> model_direction;
Triplet<float> srediste;

float ociste[3] = {1,80,60};
float glediste[3] = {10,20,0};
float viewUp[3] = {0,1,0};

Triplet<float> izracunajSrediste() {
	Triplet<float> point;
	float minx, miny, minz, maxx, maxy, maxz;
	minx = model_points[0].x;
	miny = model_points[0].y;
	minz = model_points[0].z;

	maxx = model_points[0].x;
	maxy = model_points[0].y;
	maxz = model_points[0].z;

	for (auto vrh : model_points) {
		if (vrh.x < minx)
			minx = vrh.x;
		else if (vrh.y < miny)
			miny = vrh.y;
		else if (vrh.z < minz)
			minz = vrh.z;
		else if (vrh.x > maxx)
			maxx = vrh.x;
		else if (vrh.y > maxy)
			maxy = vrh.y;
		else if (vrh.z > maxz)
			maxz = vrh.z;
	}

	point.x = (maxx + minx) / 2;
	point.y = (maxy + miny) / 2;
	point.z = (maxz + minz) / 2;

	return point;
}


void LoadModelFromFile(float lambda) {
	FILE* file_model;
	fopen_s(&file_model, "aircraft747.obj.txt", "r");
	if (file_model == NULL) {
		printf("Ne mogu otvoriti model.");
		return;
	}
	char input;
	Triplet<float> point;
	fscanf_s(file_model, "%c", &input,1);

	while (input == 'v') {
		fscanf_s(file_model, " %f %f %f\n", &point.x, &point.y, &point.z);
		point.x *= lambda;
		point.y *= lambda;
		point.z *= lambda;
		model_points.push_back(point);
		fscanf_s(file_model, "%c", &input, 1);
	}
	srediste = izracunajSrediste();
	srediste.x = 0;
	srediste.y = 0;
	srediste.z = 0;
	printf("%f %f %fsrediste\n", srediste.x, srediste.y, srediste.z);

	Triplet<int> vertice;

	while (input == 'f') {
		fscanf_s(file_model, " %d %d %d\n", &vertice.x, &vertice.y, &vertice.z);
		//printf("%d %d %d \n", vertice.x, vertice.y, vertice.z);
		vertice.x--;
		vertice.y--;
		vertice.z--;
		model_vertices.push_back(vertice.x);
		model_vertices.push_back(vertice.y);
		model_vertices.push_back(vertice.z);
		if(fscanf_s(file_model, "%c", &input, 1) != 1)return;
	}
	fclose(file_model);
	return;
}

void LoadPoints() {
	FILE* file_points;
	fopen_s(&file_points, "tocke.txt", "r");
	if (file_points == NULL) {
		printf("Ne mogu otvoriti kontrolne tocke.");
		return;
	}

	Triplet<float> point;
	points.clear();

	while (fscanf_s(file_points, "%f %f %f\n", &point.x, &point.y, &point.z) == 3) {
		points.push_back(point);
		printf("%f %f %f\n", point.x, point.y, point.z);
	}
	//cout << points[0].x << points[1].x;
	//std::array<float, 4> testArray{ {20, -3.14 / 2, 5, -3.14 / 2} };
	//std::vector<std::array<float, 4>> inputVector;
	fclose(file_points);
}

/*
float izracunajStranice() {
	Triplet<float> stranica;
	float maxstranica;
	float maxx, maxy, maxz, minx, miny, minz;


	stranica.x = maxx - minx;
	stranica.y = maxy - miny;
	stranica.z = maxz - minz;

	if (stranica.x >= stranica.y && stranica.x >= stranica.z)
		maxstranica = stranica.x;
	else if (stranica.y >= stranica.x && stranica.y >= stranica.z)
		maxstranica = stranica.y;
	else if (stranica.z >= stranica.x && stranica.z >= stranica.y)
		maxstranica = stranica.z;

	return maxstranica;
}*/

/// the level of detail of the curve
unsigned int LOD = 70;
//------------------------------------------------------------	OnReshape()
//
void OnReshape(int w, int h)
{
	if (h == 0)
		h = 1;

	// set the drawable region of the window
	glViewport(0, 0, w, h);

	// set up the projection matrix 
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// just use a perspective projection
	gluPerspective(45, (float)w / h, 0.1, 100);

	// go back to modelview matrix so we can move the objects about
	glMatrixMode(GL_MODELVIEW);
}

//------------------------------------------------------------	OnDraw()
//

float x, y, z;

Matriks inverse(Matriks m) {
	Matriks rez;

	float determinant = m.x1 * m.y2 * m.z3 + m.y1 * m.z2 * m.x3 + m.z1 * m.x2 * m.y3 - m.z1 * m.y2 * m.x3 - m.y1 * m.x2 * m.z3 - m.x1 * m.z2 * m.y3;

	rez.x1 = (m.y2 * m.z3 - m.z2 * m.y3) / determinant;
	rez.y1 = (m.z1 * m.y3 - m.y1 * m.z3) / determinant;
	rez.z1 = (m.y1 * m.z2 - m.z1 * m.y2) / determinant;
	rez.x2 = (m.z2 * m.x3 - m.x2 * m.z3) / determinant;
	rez.y2 = (m.x1 * m.z3 - m.x3 * m.z1) / determinant;
	rez.z2 = (m.z1 * m.x2 - m.x1 * m.z2) / determinant;
	rez.x3 = (m.x2 * m.y3 - m.y2 * m.x3) / determinant;
	rez.y3 = (m.y1 * m.x3 - m.x1 * m.y3) / determinant;
	rez.z3 = (m.x1 * m.y2 - m.y1 * m.x2) / determinant;
	//printf("%f %f %f\n%f %f %f\n%f %f %fmatricahehe\n", rez.x1, rez.y1, rez.z1, rez.x2, rez.y2, rez.z2, rez.x3, rez.y3, rez.z3);
	return rez;
}

Triplet<float> dotMatrix(Matriks m, float a, float b, float c) {
	Triplet<float> rez;
	
	rez.x = m.x1 * a + m.x2 * b + m.x3 * c;
	rez.y = m.y1 * a + m.y2 * b + m.y3 * c;
	rez.z = m.z1 * a + m.z2 * b + m.z3 * c;

	return rez;
}

void DrawModel(float pos_x, float pos_y, float pos_z, Matriks mat) {
	glTranslatef(pos_x, pos_y, pos_z);
	glColor3f(1.0f, 0.0f, 1.0f);
	mat=inverse(mat);
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < model_vertices.size(); i+=3) {
		glColor3f(0.1f, 0.2f, 0.5f);
		if (i % 3 == 0)glColor3f(0.5f, 0.2f, 0.1f);
		if (i % 3 == 1)glColor3f(0.55f, 0.25f, 0.1f);
		for (int j = 0; j < 3; j++) {
			float x, y, z;
			x = model_points[model_vertices[i + j]].x;
			y = model_points[model_vertices[i + j]].y;
			z = model_points[model_vertices[i + j]].z;

			Triplet<float> tocka;
			tocka = dotMatrix(mat, x, y, z);
			//printf("%d %d %d\n", tocka.x, tocka.y, tocka.z);
			glVertex3f(x, y, z);
		}
	}
	glEnd();
	
	glColor3f(0, 1, 0);
	glPointSize(3);
	glBegin(GL_POINTS);
	for (int i = 0; i != model_points.size(); ++i) {
		Triplet<float> tocka;
		tocka = dotMatrix(mat, model_points[i].x, model_points[i].y, model_points[i].z);
		float nesto[3];
		nesto[0] = model_points[i].x;
		nesto[1] = model_points[i].y;
		nesto[2] = model_points[i].z;
		glVertex3fv(nesto);
	}
	glEnd();
	
	glTranslatef(-pos_x, -pos_y, -pos_z);
}

float kocka_x = -23.0, kocka_y = 10.0, kocka_z = 0.0;

int kocka_i=1, kocka_j=0;

int inicijal = 1;

float angle = 0;

float dx=0, dy=0, dz=0;

Triplet<float> cross(Triplet<float> vect1, Triplet<float> vect2) {
	Triplet<float> rez;
	rez.x = vect1.y * vect2.z - vect1.z * vect2.y;
	rez.y = vect1.z * vect2.x - vect1.x * vect2.z;
	rez.z = vect1.x * vect2.y - vect1.y * vect2.x;
	return rez;
}

Triplet<float> normalize(Triplet<float> vect) {
	float x = sqrt(vect.x * vect.x + vect.y * vect.y + vect.z + vect.z);
	if (x == 0)return vect;
	vect.x /= x;
	vect.y /= x;
	vect.z /= x;
	return vect;
}

void pokreniKocku() {

	if (inicijal == 1) { prev_point.x = 0; prev_point.y = 0; prev_point.z = 0; inicijal = 0; }

	if (kocka_i >= points.size() - 2) {
		return;
	}

	float t = (float)kocka_j / (LOD - 1);

	// calculate blending functions
	float b0 = (-1 * t * t * t + 3 * t * t - 3 * t + 1) / 6.0f;
	float b1 = (3 * t * t * t - 6 * t * t + 4) / 6.0f;
	float b2 = (-3 * t * t * t + 3 * t * t + 3 * t + 1) / 6.0f;
	float b3 = t * t * t / 6.0f;

	// sum the control points mulitplied by their respective blending functions

	float x = b0 * points[kocka_i - 1].x +
		b1 * points[kocka_i].x +
		b2 * points[kocka_i + 1].x +
		b3 * points[kocka_i + 2].x;

	float y = b0 * points[kocka_i - 1].y +
		b1 * points[kocka_i].y +
		b2 * points[kocka_i + 1].y +
		b3 * points[kocka_i + 2].y;

	float z = b0 * points[kocka_i - 1].z +
		b1 * points[kocka_i].z +
		b2 * points[kocka_i + 1].z +
		b3 * points[kocka_i + 2].z;

	float t0 = (-3 * t * t + 6 * t - 3);
	float t1 = (9 * t * t - 12 * t);
	float t2 = (-9 * t * t + 6 * t + 3);
	float t3 = 3 * t * t;

	float x1 = t0 * points[kocka_i - 1].x +
		t1 * points[kocka_i].x +
		t2 * points[kocka_i + 1].x +
		t3 * points[kocka_i + 2].x;

	float y1 = t0 * points[kocka_i - 1].y +
		t1 * points[kocka_i].y +
		t2 * points[kocka_i + 1].y +
		t3 * points[kocka_i + 2].y;

	float z1 = t0 * points[kocka_i - 1].z +
		t1 * points[kocka_i].z +
		t2 * points[kocka_i + 1].z +
		t3 * points[kocka_i + 2].z;

	tangent.x = x1;
	tangent.y = y1;
	tangent.z = z1;
	tangent = normalize(tangent);

	float pt0 = (-6 * t + 6);
	float pt1 = (18 * t - 12);
	float pt2 = (-18 * t + 6);
	float pt3 = 6 * t;

	second_derivation.x = pt0 * points[kocka_i - 1].x +
		pt1 * points[kocka_i].x +
		pt2 * points[kocka_i + 1].x +
		pt3 * points[kocka_i + 2].x;

	second_derivation.y = pt0 * points[kocka_i - 1].y +
		pt1 * points[kocka_i].y +
		pt2 * points[kocka_i + 1].y +
		pt3 * points[kocka_i + 2].y;

	second_derivation.z = pt0 * points[kocka_i - 1].z +
		pt1 * points[kocka_i].z +
		pt2 * points[kocka_i + 1].z +
		pt3 * points[kocka_i + 2].z;

	//prev_tangent.x = tangent.x - dx;
	//prev_tangent.y = tangent.x - dy;
	//prev_tangent.z = tangent.x - dz;

	second_derivation = normalize(second_derivation);

	float nesto[3];

	Triplet<float> rt;
	rt.x = x-prev_point.x;
	rt.y = y-prev_point.y;
	rt.z = z-prev_point.z;

	rt = normalize(rt);

	printf("%f %f %f tangenta racunska\n", rt.x,rt.y,rt.z);


	printf("%f %f %f tangenta\n", tangent.x, tangent.y, tangent.z);

	printf("%f %f %f druga derivacija\n", second_derivation.x, second_derivation.y, second_derivation.z);

	Triplet<float> drt;
	drt.x = tangent.x - dx;
	drt.y = tangent.y - dy;
	drt.z = tangent.z - dz;

	/*
	prev_tangent.x = drt.x;
	prev_tangent.y = drt.y;
	prev_tangent.z = drt.z;*/

	drt = normalize(drt);

	printf("%f %f %f druga derivacija racunska\n", drt.x, drt.y,drt.z);
	//tangent.x = x - prev_point.x;
	//tangent.y = y - prev_point.y;
	//tangent.z = z - prev_point.z;


	//prev_tangent.x = tangent.x;
	//prev_tangent.y = tangent.y;
	//prev_tangent.z = tangent.z;


	//if (prev_tangent.x < 0 && prev_tangent.y < 0) { prev_tangent.x *= -1; prev_tangent.y *= -1; }
	//if (prev_tangent.x > 0 && prev_tangent.y < 0) { prev_tangent.x *= -1; prev_tangent.y *= -1; }

	normal = cross(tangent, second_derivation);

	//binormal.x = tangent.y * second_derivation.z - second_derivation.z * second_derivation.y;
	//binormal.y = tangent.z * second_derivation.x - second_derivation.x * second_derivation.z;
	//binormal.z = tangent.x * second_derivation.y - second_derivation.y * second_derivation.x;

	printf("%f %f %f 1. skalar 2.\n", second_derivation.x, second_derivation.y, second_derivation.z);

	//prev_tangent = normalize(prev_tangent);

	//if (prev_tangent.z < 0) { prev_tangent.z *= -1;}

	binormal = cross(tangent, normal);

	//m1 = tangent.y * prev_tangent.z - tangent.z * prev_tangent.y;
	//m2 = tangent.z * prev_tangent.x - tangent.x * prev_tangent.z;
	//m3 = tangent.x * prev_tangent.y - tangent.y * prev_tangent.x;

	glColor3f(0, 0, 1);
	glBegin(GL_LINE_STRIP);
	nesto[3];
	nesto[0] = x;
	nesto[1] = y;
	nesto[2] = z;
	glVertex3fv(nesto);
	nesto[0] = x + normal.x * 100;
	nesto[1] = y + normal.y * 100;
	nesto[2] = z + normal.z * 100;
	glVertex3fv(nesto);
	glEnd();

	//printf("%f %f %f onaj v\n", m1,m2,m3);

	/*
	if (0 <m2) {
		m1 = prev_tangent.y * tangent.z - prev_tangent.z * tangent.y;
		m2 = prev_tangent.z * tangent.x - prev_tangent.x * tangent.z;
		m3 = prev_tangent.x * tangent.y - prev_tangent.y * tangent.x;
	}*/

	Matriks matrica;
	matrica.x1 = tangent.x;
	matrica.x2 = tangent.y;
	matrica.x3 = tangent.z;

	matrica.y1 = second_derivation.x;
	matrica.y2 = second_derivation.y;
	matrica.y3 = second_derivation.z;

	matrica.z1 = binormal.x;
	matrica.z2 = binormal.x;
	matrica.z3 = binormal.x;

	//printf("%f %f %f \n %f %f %f\n %f %f %f\nmatrica", tangent.x,tangent.y,tangent.z, prev_tangent.x, prev_tangent.y, prev_tangent.z,m1,m2,m3 );

	

	/*prev_tangent.x = m1;
	prev_tangent.y = m2;
	prev_tangent.z = m3;*/
	
	model_direction.x = 0.0;
	model_direction.y = 0.0;
	model_direction.z = 1.0;
	//if (tangent.x > 0 && tangent.y > 0)model_direction.z = -1.0;

	//tangent.x = prev_tangent.y * tangent.z - prev_tangent.z * tangent.y;
	//tangent.y = prev_tangent.z * tangent.x - prev_tangent.x * tangent.z;
	//tangent.z = prev_tangent.x * tangent.y - prev_tangent.y * tangent.x;

	//tangent.z = 0;

	Triplet<float> rotating_vector;

	rotating_vector = cross(model_direction, tangent);
	//rotating_vector.x = model_direction.y * tangent. z - model_direction.z* tangent.y;
	//rotating_vector.y = model_direction.z * tangent.x - model_direction.x * tangent.z;
	//rotating_vector.z = model_direction.x * tangent.y - model_direction.y * tangent.x;

	//printf("%f %f %f\n", tangent.x, tangent.y, tangent.z);
	//printf("%f %f %f, %f %f %f ",tangent.x, tangent.y, tangent.z, (tangent.x - prev_point.x), (tangent.y - prev_point.y), (tangent.z - prev_point.z));
	
	if((sqrt(tangent.x * tangent.x + tangent.y * tangent.y + tangent.z * tangent.z) * sqrt(model_direction.x * model_direction.x + model_direction.y * model_direction.y + model_direction.z * model_direction.z)) !=0)angle = acos((tangent.x * model_direction.x + tangent.y * model_direction.y + tangent.z * model_direction.z) / (sqrt(tangent.x * tangent.x + tangent.y * tangent.y + tangent.z * tangent.z) * sqrt(model_direction.x * model_direction.x + model_direction.y * model_direction.y + model_direction.z * model_direction.z)));

	//if (0.0 > rotating_vector.x || 0.0 > rotating_vector.y || 0.0 > rotating_vector.z)angle = 180.0 - angle;

	//glTranslatef(-23.0, 10.0, 0.0);
	//glRotatef(180, 1.0, 0.0, 0.0);
	
	
	glColor3f(0, 1, 1);
	glBegin(GL_LINE_STRIP);
	nesto[0] = x;
	nesto[1] = y;
	nesto[2] = z;
	glVertex3fv(nesto);
	nesto[0] = x + x1;
	nesto[1] = y + y1;
	nesto[2] = z + z1;
	glVertex3fv(nesto);
	glEnd();

	

	//glRotatef(-180, 1.0, 0.0, 0.0);
	//glTranslatef(23.0, -10.0, 0.0);
	/*
	glColor3f(0, 1, 1);
	glBegin(GL_LINE_STRIP);
	float nesto[3];
	nesto[0] = prev_point.x;
	nesto[1] = prev_point.y;
	nesto[2] = prev_point.z;
	glVertex3fv(nesto);
	nesto[0] += prev_point.x;
	nesto[1] += prev_point.y;
	nesto[2] += prev_point.z;
	glVertex3fv(nesto);
	glEnd();*/

	prev_point.x = x; prev_point.y = y; prev_point.z = z;
	dx = tangent.x; dy = tangent.y; dz = tangent.z;


	glTranslatef(x - srediste.x, y - srediste.y, z - srediste.z);

	//printf("%f\n", angle * 180.0 / PI);

	//printf("%f\n", angle*180/PI);

	//glRotatef(90, 0.0, 0.0, 1.0);

	glRotatef(angle*180.0/PI ,rotating_vector.x, rotating_vector.y, rotating_vector.z);

	

	//if (tangent.x > 0 && tangent.y > 0)glRotatef(180.0, tangent.x, tangent.y, tangent.z);

	DrawModel(0.0, 0.0, 0.0, matrica);

	/*
	model_direction.x = tangent.x;
	model_direction.y = tangent.y;
	model_direction.z = tangent.z;*/

	glRotatef(-angle * 180.0 / PI, rotating_vector.x, rotating_vector.y, rotating_vector.z);

	//if (tangent.x > 0 && tangent.y > 0)glRotatef(180.0, tangent.x, tangent.y, tangent.z);

	//glRotatef(-angle *180.0 / PI, rotating_vector.x, rotating_vector.y, rotating_vector.z);

	//glRotatef(-90, 0.0, 0.0, 1.0);

	glTranslatef(-x + srediste.x, -y + srediste.y, -z + srediste.z);

	++kocka_j;
	if (kocka_j == LOD) { kocka_j = 0; kocka_i++; }

}

int pokreni = 1;

void OnDraw() {
	for (int l = 0; l != pokreni; l++) {
	// clear the screen & depth buffer
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		// clear the previous transform
		glLoadIdentity();

		// set the camera position
		gluLookAt(ociste[0], ociste[1], ociste[2],	//	eye pos
			glediste[0], glediste[1], glediste[2],	//	aim point
			viewUp[0], viewUp[1], viewUp[2]);	//	up direction	
	
		pokreniKocku();
		
		//glRotatef(180, 1.0, 0.0, 0.0);
		glColor3f(1, 0, 0);
		glBegin(GL_LINE_STRIP);

		for (int i = 0; i < points.size() - 3; i++) {
			for (int j = 0; j != LOD; ++j) {

				float t = (float)j / (LOD - 1);

				float b0 = (-1 * t * t * t + 3 * t * t - 3 * t + 1) / 6.0f;
				float b1 = (3 * t * t * t - 6 * t * t + 4) / 6.0f;
				float b2 = (-3 * t * t * t + 3 * t * t + 3 * t + 1) / 6.0f;
				float b3 = t * t * t / 6.0f;

				float x = b0 * points[i].x +
					b1 * points[i + 1].x +
					b2 * points[i + 2].x +
					b3 * points[i + 3].x;

				float y = b0 * points[i].y +
					b1 * points[i + 1].y +
					b2 * points[i + 2].y +
					b3 * points[i + 3].y;

				float z = b0 * points[i].z +
					b1 * points[i + 1].z +
					b2 * points[i + 2].z +
					b3 * points[i + 3].z;
				
				glVertex3f(x, y, z);
				
			}
		}
		glEnd();
		
		// draw the control points
		glColor3f(0, 1, 0);
		glPointSize(3);
		glBegin(GL_POINTS);
		for (int i = 0; i != points.size(); ++i) {
			float nesto[3];
			nesto[0] = points[i].x;
			nesto[1] = points[i].y;
			nesto[2] = points[i].z;
			glVertex3fv(nesto);
		}
		glEnd();

		// draw the hull of the curve
		glColor3f(0, 1, 1);
		glBegin(GL_LINE_STRIP);
		for (int i = 0; i != points.size(); ++i) {
			float nesto[3];
			nesto[0] = points[i].x;
			nesto[1] = points[i].y;
			nesto[2] = points[i].z;
			glVertex3fv(nesto);
		}
		glEnd();

		// currently we've been drawing to the back buffer, we need
		// to swap the back buffer with the front one to make the image visible
		glutSwapBuffers();
	}
}

//------------------------------------------------------------	OnInit()
//
void OnInit() {
	// enable depth testing
	glEnable(GL_DEPTH_TEST);
}

//------------------------------------------------------------	OnExit()
//
void OnExit() {
}


//------------------------------------------------------------	OnKeyPress()
//
void OnKeyPress(unsigned char key, int, int) {
	switch (key) {
	case 'r':
		kocka_i = 1, kocka_j = 0;
		break;
	case 'p':
		pokreni = (points.size() - 3) * LOD;
		break;
	case 's':
		pokreni = 1;
		break;
	case '1':
		glediste[0]++;
		break;
	case '2':
		glediste[1]++;
		break;
	case '3':
		glediste[2]++;
		break;
	case '8':
		glediste[0]--;
		break;
	case '9':
		glediste[1]--;
		break;
	case '0':
		glediste[2]--;
		break;
	case 'y':
		ociste[0]++;
		break;
	case 'x':
		ociste[1]++;
		break;
	case 'c':
		ociste[2]++;
		break;
	case 'v':
		ociste[0]--;
		break;
	case 'b':
		ociste[1]--;
		break;
	case 'n':
		ociste[2]--;
		break;
		
	default:
		break;
	}

	// ask glut to redraw the screen for us... 
	glutPostRedisplay();
}

//------------------------------------------------------------	main()
//
int main(int argc, char** argv){

	LoadPoints();
	LoadModelFromFile(15.0);
	// initialise glut
	glutInit(&argc, argv);

	// request a depth buffer, RGBA display mode, and we want double buffering
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);

	// set the initial window size
	glutInitWindowSize(1920, 1080);

	// create the window
	glutCreateWindow("Bezier Curve: +/- to Change Level of Detail");

	// set the function to use to draw our scene
	glutDisplayFunc(OnDraw);

	// set the function to handle changes in screen size
	glutReshapeFunc(OnReshape);

	// set the function for the key presses
	glutKeyboardFunc(OnKeyPress);

	// run our custom initialisation
	OnInit();

	// set the function to be called when we exit
	atexit(OnExit);

	// this function runs a while loop to keep the program running.
	glutMainLoop();
	return 0;
}
