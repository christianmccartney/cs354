#include<iostream>
#include<sstream>
#include<string>
#include<fstream>
#include<vector>
#include<cmath>
#include<regex>
#include<utility>

using namespace std;


struct halfedge;
struct vertex;


struct face {
	pair<unsigned int, unsigned int> ei[3];
	halfedge *e;
	bool isadded = 0;
};

struct halfedge {
	halfedge *next;
	halfedge *prev;
	halfedge *twin;
	vertex *v;
	face *f;
	vertex *newv;
	bool isnew;
	bool isorig = 0;
	bool isadded = 0;
	//double newx;
	//double newy;
	//double newz;
};

struct vertex {
	double x;
	double y;
	double z;
	halfedge *e;
	vertex *newv;
	bool isnew;
	bool isorig;
	//double newx;
	//double newy;
	//double newz;
};

void subdivide(char* rayfile);

bool exists(const string& name);
