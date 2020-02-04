// Racunalna Grafika 2. labos.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdlib.h>
#include <glut.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <GL/gl.h>
#include <math.h>

float ociste[3] = { 1,80,60 };
float glediste[3] = { 10,20,0 };
float viewUp[3] = { 0,1,0 };

const int MAX_PARTICLES = 100000;

void NormalizeVector(float& a, float& b, float& c) {
	double norm = sqrt(a * a + b * b + c * c);
	a /= norm;
	b /= norm;
	c /= norm;
}

class Particle {
public:
	float position[3];
	float color[3];
	float direction[3];
	float life;

	Particle();
};

Particle* start = 0;

Particle* last = 0;

Particle* end = 0;

float izvor[3];
int lijevo = 0;

Particle::Particle() {
	// set position
	
	if (izvor[0] < 7 && izvor[0]> 6)lijevo = 1;
	if (izvor[0] > -7 && izvor[0] < -6)lijevo = 0;

	if (lijevo == 0) { 
		izvor[0] += 0.00001; 
	}
	else { lijevo = 1; izvor[0] -= 0.00001; }
	//
	
	
	position[0] = izvor[0];
	position[1] = izvor[1];
	position[2] = izvor[2];
	
	float fi, theta, radius;
	fi = (float(rand()) / RAND_MAX)*2*3.1415927;
	theta = acos(float(rand()) / RAND_MAX);
	radius = 0.5 + (float(rand()) / RAND_MAX) * 0.5;
	
	//std::cout << fi << "fi theta" << theta << "\n";

	direction[0] = radius * sin(theta) * cos(fi);
	direction[1] = radius * sin(theta) * sin(fi);
	direction[2] = radius * cos(theta);

	// create random direction
	/*
	direction[0] = (20000 - rand() % 40000) / 10000.0f;
	direction[1] = (20000 - rand() % 40000) / 10000.0f;
	direction[2] = (20000 - rand() % 40000) / 10000.0f;*/

	//NormalizeVector(direction[0], direction[1], direction[2]);

	//direction[0] /= 100;
	//direction[1] /= 100;
	//direction[2] /= 100;

	//position[0] = direction[0];
	//position[1] = direction[1];
	///position[2] = direction[2];
	
	color[0] = 1.0;
	color[1] = 1.0;
	color[2] = 1.0;

	//life = 1.0;

	life = rand() % 20000 / 10500.0f;
}

void DeleteOne(Particle* p) {

	if (!(start == last)) *p = *(--last);
}

void InsertOne() {

	if (!(last == end)) {
		*last = Particle();
		++last;
	}
}

void PrintParticles() {

	Particle* p = start;

	while (p != last) {
		std::cout << p->direction[0] << "\n";
		++p;
	}

}

void UpdateParticles(float time) {
	
	Particle* p = start;

	while (p != last) {
		p->life -= time;
		if (p->life > 0) {

			p->position[0] += time * p->direction[0];
			p->position[1] += time * p->direction[1];
			p->position[2] += time * p->direction[2];

			//p->direction[0] *= 1.04f;
			if (p->direction[1] > 0)p->direction[1] *= -1;
			p->direction[1] *= 1.04f;
			//p->direction[2] *= 1.04f;

			if (p->life < 0.7 && p->life > 0.3) {
				p->color[0] = 1.0;
				p->color[1] = 0.5;
				p->color[2] = 0.5;
			}

			if (p->life < 0.3) {

				p->color[0] = 1.0;
				p->color[1] = 0.0;
				p->color[2] = 0.0;
			}

			++p;
		}
		else {
			DeleteOne(p);
			
			if (p >= last) return;
		}
	}
}

//
void DrawParticles() {

	glVertexPointer(3, GL_FLOAT, sizeof(Particle), start->position);
	glColorPointer(3, GL_FLOAT, sizeof(Particle), start->color);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glDrawArrays(GL_POINTS, 0, last - start);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}


void OnDraw() {

	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);

	glLoadIdentity();

	gluLookAt(	1,-3,10,	//	eye pos
				0,0,0,	//	aim point
				0,1,0);	//	up direction

	DrawParticles();

	glutSwapBuffers();
}


void AllocateMemory(int size) {

	delete[] start;

	last = start = new Particle[size];

	end = start + size;

	izvor[0] = 0.0;
	izvor[1] = 4.0f;
	izvor[2] = 0.0;
}

void OnInit() {
	glEnable(GL_DEPTH_TEST);
	AllocateMemory(MAX_PARTICLES);
}


void OnReshape(int w, int h){
	if (h == 0) h = 1;


	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45, (float)w / h, 0.1, 100);

	glMatrixMode(GL_MODELVIEW);
}

void OnExit() {
}

int currentTime = 0; int previousTime = 0;
int count = 0;

void OnIdle() {
	currentTime = glutGet(GLUT_ELAPSED_TIME);
	int timeInterval = currentTime - previousTime;

	int val = (rand() * 10) % 40000;;

	//if (++count > 10) val = 0;

	for (int i = 0; i != val; ++i)
		InsertOne();

	std::cout << ((int)(last - start))<<" " << val << " "<< 1000.0/timeInterval <<"\n";

	UpdateParticles(timeInterval/1000.0);

	glutPostRedisplay();
	
	previousTime = currentTime;
}


void OnKeyPress(unsigned char key, int, int) {
	switch (key) {
	case 'r':
		count = 0;
		break;
	case 's':
		//PrintParticles();
		break;
	}
	glutPostRedisplay();
}

//
int main(int argc, char** argv) {

	// initialise glut
	glutInit(&argc, argv);

	// request a depth buffer, RGBA display mode, and we want double buffering
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);

	// set the initial window size
	glutInitWindowSize(1280, 760);

	// create the window
	glutCreateWindow("Stvaranje čestica");

	// set the function to use to draw our scene
	glutDisplayFunc(OnDraw);

	// set the function to handle changes in screen size
	glutReshapeFunc(OnReshape);

	//
	glutKeyboardFunc(OnKeyPress);

	// set the idle callback
	glutIdleFunc(OnIdle);

	// run our custom initialisation
	OnInit();

	// set the function to be called when we exit
	atexit(OnExit);

	// this function runs a while loop to keep the program running.
	glutMainLoop();
	return 0;
}

