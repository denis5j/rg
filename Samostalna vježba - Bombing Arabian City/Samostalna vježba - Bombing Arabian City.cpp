#include <stdlib.h>
#include <glut.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <GL/gl.h>
#include <math.h>
#include <vector>
#include <string>
#include <algorithm>


float ociste[3] = { 1, -3, 10 };
float glediste[3] = { 0, 0, 0 };
float viewUp[3] = { 0, 1, 0 };

const int MAX_PARTICLES = 1000000;

template <class Type>
struct Triplet
{
	Type  x, y, z;
};

class Particle {
public:
	float position[3];
	float color[3];
	float direction[3];
	float life;

	Particle();
	Particle(float x, float y, float z);
};

Particle::Particle() {
	position[0] = 0.0;
	position[1] = 0.0;
	position[2] = 0.0;

	direction[0] = 0.0;
	direction[1] = 0.0;
	direction[2] = 0.0;

	color[0] = 1.0;
	color[1] = 1.0;
	color[2] = 1.0;

	life = 1000000.0;
}

Particle::Particle(float x, float y, float z) {

	position[0] = 0.0;
	position[1] = 0.0;
	position[2] = 0.0;

	direction[0] = x;
	direction[1] = y;
	direction[2] = z;

	color[0] = 1.0;
	color[1] = 1.0;
	color[2] = 1.0;

	life = 1000000.0;
}

class Model {
public:
	Particle* start = 0;
	Particle* last = 0;
	Particle* end = 0;

	std::vector<int> model_vertices;

	Triplet<float> center;

	bool visible = true;

	Triplet<float> acceleration;

	Model() {

		delete[] start;

		last = start = new Particle[MAX_PARTICLES];

		end = start + MAX_PARTICLES;

		acceleration.x = 1.001f;
		acceleration.y = 1.001f;
		acceleration.z = 1.001f;

	}

	void LoadModelFromFile(std::string file_name, float lambda, float x, float y, float z);
	void Explode();
	void UpdateParticles(float time);
	void InsertOne();
	void DeleteOne(Particle* p);
	void DrawParticles();
	void SetParticleDirection(float x, float y, float z);
	//void ChangePosition();
	void ColisionDetection(Model model);

};

Model Airplane;
Model ArabianCity;
Model FirstBomb;
Model SecondBomb;
//
//Triplet <float> previous_center;
////float poz_x = -2.0, poz_y = -3.0, poz_z = -12.0;
//////-2.0, -3.0, -12.0
//
//void Model::ChangePosition() {
//	std::for_each(start, end, [](Particle& p) {
//		p.position[0] += (Airplane.center.x - previous_center.x);
//		p.position[1] += (Airplane.center.y - previous_center.y);
//		p.position[2] += (Airplane.center.z - previous_center.z);
//		});
//
//	previous_center.x = Airplane.center.x;
//	previous_center.y = Airplane.center.y;
//	previous_center.z = Airplane.center.z;
//}

void Model::SetParticleDirection(float x, float y, float z) {
	std::for_each(start, end, [&x, &y, &z](Particle& p) {
		p.direction[0] = x;
		p.direction[1] = y;
		p.direction[2] = z;
		});
}

void Model::LoadModelFromFile(std::string file_name, float lambda, float x, float y, float z) {
	FILE* file_model;
	fopen_s(&file_model, file_name.c_str(), "r");
	if (file_model == NULL) {
		printf("Ne mogu otvoriti model.");
		while (1);
		return;
	}
	char input;
	Triplet<float> point;
	fscanf_s(file_model, "%c", &input, 1);

	Particle* p = start;

	while (input == 'v') {
		fscanf_s(file_model, " %f %f %f\n", &point.x, &point.y, &point.z);
		point.x *= lambda;
		point.y *= lambda;
		point.z *= lambda;
		//model_points.push_back(point);
		//std::cout << point.x << " " << point.y << " " << point.z << "\n";
		p->position[0] = point.x + x;
		p->position[1] = point.y + y;
		p->position[2] = point.z + z;
		++p;
		++last;
		fscanf_s(file_model, "%c", &input, 1);
	}
	//srediste = izracunajSrediste();
	center.x = x;
	center.y = y;
	center.z = z;
	//-2.0, -3.0, -12.0
	//printf("%f %f %fsrediste\n", srediste.x, srediste.y, srediste.z);

	Triplet<int> vertice;

	while (input == 'f') {
		fscanf_s(file_model, " %d %d %d\n", &vertice.x, &vertice.y, &vertice.z);
		vertice.x--;
		vertice.y--;
		vertice.z--;
		model_vertices.push_back(vertice.x);
		model_vertices.push_back(vertice.y);
		model_vertices.push_back(vertice.z);
		if (fscanf_s(file_model, "%c", &input, 1) != 1)return;
	}
	fclose(file_model);
	return;
}

void Model::Explode() {
	Particle* p = start;

	while (p != last) {

		float fi, theta, radius;
		fi = (float(rand()) / RAND_MAX) * 2 * 3.1415927;
		theta = acos(float(rand()) / RAND_MAX);
		radius = 0.5 + (float(rand()) / RAND_MAX) * 0.5;

		p->direction[0] = radius * sin(theta) * cos(fi);
		p->direction[1] = radius * sin(theta) * sin(fi);
		p->direction[2] = radius * cos(theta);

		p->life = rand() % 20000 / 17800.0f;
		++p;

	}
}

void NormalizeVector(float& a, float& b, float& c) {
	double norm = sqrt(a * a + b * b + c * c);
	a /= norm;
	b /= norm;
	c /= norm;
}

void Model::DeleteOne(Particle* p) {

	if (!(start == last)) *p = *(--last);
}

void Model::InsertOne() {

	if (!(last == end)) {
		*last = Particle();
		++last;
	}
}

float DistanceBetweenPoints(Triplet<float> point, float x, float y, float z) {
	return sqrt((point.x - x) * (point.x - x) + (point.y - y) * (point.y - y) + (point.z - z) * (point.z - z));
}

void Model::ColisionDetection(Model model) {

	Particle* p = start;

	while (p != last) {

		if (DistanceBetweenPoints(model.center, p->position[0], p->position[1], p->position[2]) < 0.28) {
			float fi, theta, radius;
			fi = (float(rand()) / RAND_MAX) * 2 * 3.1415927;
			theta = acos(float(rand()) / RAND_MAX);
			radius = 0.5 + (float(rand()) / RAND_MAX) * 0.5;

			p->direction[0] = radius * sin(theta) * cos(fi);
			p->direction[1] = radius * sin(theta) * sin(fi);
			p->direction[2] = radius * cos(theta);

			p->life = rand() % 20000 / 17800.0f;
		}

		++p;
	}

	if (model.center.y < -4.00) {
		model.Explode();
		model.start->direction[0] = 0.0;
		//	model.start->direction[1] = 0.05;
		model.start->direction[1] = 0.0;
		model.start->direction[2] = 0.0;
	}
}

void Model::UpdateParticles(float time) {

	Particle* p = start;

	//update center of model
	center.x += time * p->direction[0];
	center.y += time * p->direction[1];
	center.z += time * p->direction[2];

	while (p != last) {
		p->life -= time;
		if (p->life > 0.0) {

			p->position[0] += time * p->direction[0];
			p->position[1] += time * p->direction[1];
			p->position[2] += time * p->direction[2];

			if (p->position[1] < -5.0)p->position[1] = -5.0;

			p->direction[0] *= acceleration.x;
			p->direction[1] *= acceleration.y;
			p->direction[2] *= acceleration.z;

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


void Model::DrawParticles() {

	glVertexPointer(3, GL_FLOAT, sizeof(Particle), start->position);
	glColorPointer(3, GL_FLOAT, sizeof(Particle), start->color);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glDrawArrays(GL_POINTS, 0, last - start);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}


void OnDraw() {

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glLoadIdentity();

	gluLookAt(ociste[0], ociste[1], ociste[2],	//	eye pos
		glediste[0], glediste[1], glediste[2],	//	aim point
		viewUp[0], viewUp[1], viewUp[2]);	//	up direction

	Airplane.DrawParticles();
	ArabianCity.DrawParticles();
	if (FirstBomb.visible) FirstBomb.DrawParticles();
	if (SecondBomb.visible) SecondBomb.DrawParticles();

	glutSwapBuffers();
}


void OnInit() {
	/*previous_center.x = -1.0;
	previous_center.y = -2.0;
	previous_center.z = -12.0;*/

	glEnable(GL_DEPTH_TEST);

	Airplane.LoadModelFromFile("Aircraft747.obj", 0.8, -1.0, -2.0, -12.0);
	Airplane.SetParticleDirection(0.0, 0.0, 0.9);
	ArabianCity.LoadModelFromFile("ArabianCity.obj", 2.5, -1.0, -4.0, 8.5);
	ArabianCity.SetParticleDirection(0.0, 0.0, 0.0);
	FirstBomb.LoadModelFromFile("Projectile-hd.obj", 0.05, -0.7, -2.0, -12.0);
	FirstBomb.SetParticleDirection(0.0, 0.0, 0.9);
	FirstBomb.visible = false;
	SecondBomb.LoadModelFromFile("Projectile-hd.obj", 0.05, -1.3, -2.0, -12.0);
	SecondBomb.SetParticleDirection(0.0, 0.0, 0.9);
	SecondBomb.visible = false;

}


void OnReshape(int w, int h) {
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

	//std::cout << ((int)(Airplane.last - Airplane.start + ArabianCity.last - ArabianCity.start)) << " " << 1000.0 / timeInterval << "\n";

	//std::cout << timeInterval << "\n";

	//ArabianCity.ChangePosition(Bomb.)

	//std::cout << Bomb.center.x << " " << Bomb.center.y << " " << Bomb.center.z << "\n";

	ArabianCity.ColisionDetection(FirstBomb);
	ArabianCity.ColisionDetection(SecondBomb);

	Airplane.UpdateParticles(timeInterval / 1000.0);
	ArabianCity.UpdateParticles(timeInterval / 1000.0);
	FirstBomb.UpdateParticles(timeInterval / 1000.0);
	SecondBomb.UpdateParticles(timeInterval / 1000.0);



	glutPostRedisplay();

	previousTime = currentTime;
}


void OnKeyPress(unsigned char key, int, int) {
	Particle* p = Airplane.start + 2;

	switch (key) {
	case 'r':
		Airplane.Explode();
		break;
	case 's':
		ArabianCity.Explode();
		break;
	case 'p':
		FirstBomb.visible = true;
		FirstBomb.acceleration.x = 0.992f;
		FirstBomb.acceleration.y = 1.001f;
		FirstBomb.acceleration.z = 0.992f;

		FirstBomb.SetParticleDirection(p->direction[0], -1.0, p->direction[2]);
		break;
	case'o':
		SecondBomb.visible = true;
		SecondBomb.acceleration.x = 0.992f;
		SecondBomb.acceleration.y = 1.001f;
		SecondBomb.acceleration.z = 0.992f;

		SecondBomb.SetParticleDirection(p->direction[0], -1.0, p->direction[2]);
		break;
		/*case '1':
			ociste[0] += 0.1;
			break;
		case '2':
			ociste[1] += 0.1;
			break;
		case '3':
			ociste[2] += 0.1;
			break;
		case '4':
			glediste[0] += 0.1;
			break;
		case '5':
			glediste[1] += 0.1;
			break;
		case 't':
			glediste[0] -= 0.1;
			break;
		case 'z':
			glediste[1] -= 0.1;
			break;
		case '6':
			glediste[2] += 0.1;
			break;
		case '7':
			viewUp[0] += 0.1;
			break;
		case '8':
			viewUp[1] += 0.1;
			break;
		case '9':
			viewUp[2] += 0.1;
			break;*/
	}
	glutPostRedisplay();
}

int last_x = 1280 / 2, last_y = 768 / 2;

void OnMouse(int x, int y) {

	glediste[0] -= ((long float)last_x - (long float)x) / 10.0;
	glediste[1] += ((float)last_y - (float)y) / 10.0;

	last_y = y;
	last_x = x;
}

//
int main(int argc, char** argv) {

	// initialise glut
	glutInit(&argc, argv);

	// request a depth buffer, RGBA display mode, and we want double buffering
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);

	// set the initial window size
	glutInitWindowSize(1980, 1080);

	// create the window
	glutCreateWindow("Bombing Arabian City");

	// set the function to use to draw our scene
	glutDisplayFunc(OnDraw);

	// set the function to handle changes in screen size
	glutReshapeFunc(OnReshape);

	//
	glutKeyboardFunc(OnKeyPress);

	//
	glutPassiveMotionFunc(OnMouse);

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

