// The main ray tracer.

#pragma warning (disable: 4786)

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"

#include "parser/Tokenizer.h"
#include "parser/Parser.h"

#include "ui/TraceUI.h"
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <string.h> // for memset

#include <iostream>
#include <fstream>
#include <thread>


using namespace std;
extern TraceUI* traceUI;

// Use this variable to decide if you want to print out
// debugging messages.  Gets set in the "trace single ray" mode
// in TraceGLWindow, for example.
bool debugMode = false;

// Trace a top-level ray through pixel(i,j), i.e. normalized window coordinates (x,y),
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.

glm::dvec3 RayTracer::trace(double x, double y, unsigned char *pixel, unsigned int ctr)
{
    // Clear out the ray cache in the scene for debugging purposes,
    if (TraceUI::m_debug) scene->intersectCache.clear();


    ray r(glm::dvec3(0,0,0), glm::dvec3(0,0,0), pixel, ctr, glm::dvec3(1,1,1), ray::VISIBILITY);
    scene->getCamera().rayThrough(x,y,r);
    double dummy;
    glm::dvec3 ret = traceRay(r, glm::dvec3(1.0,1.0,1.0), traceUI->getDepth() , dummy);
    ret = glm::clamp(ret, 0.0, 1.0);
    return ret;
}

glm::dvec3 RayTracer::tracePixel(int i, int j, unsigned int ctr)
{
	//cout << "in tracePixel" << endl;

	glm::dvec3 col(0,0,0);

	if( ! sceneLoaded() ) return col;

	double x = double(i)/double(buffer_width);
	double y = double(j)/double(buffer_height);

	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;
//	int aaSamples = traceUI->getAASamples();
	//cout << "i :: " << i << endl;
	//cout << "j :: " << j << endl;
	//cout << "buffer :: " << buffer << endl;
	//cout << "buffer_width :: " << buffer_width << endl;
	//cout << "in tracePixel 1" << endl;

	int aaSamples = traceUI->getSuperSamples();
	if(traceUI->aaSwitch() && aaSamples > 1) {
		//int aaSamplesSqrt = sqrt(aaSamples);

		const double xAaInc = (1.0 / ((double)buffer_width * (double)aaSamples));
		const double yAaInc = (1.0 / ((double)buffer_height * (double)aaSamples));

		for (int yaa = 0; yaa < aaSamples; yaa++) {
			double ys = y + ((double)yaa * yAaInc);
			for (int xaa = 0; xaa < aaSamples; xaa++) {
				double xs = x + ((double)xaa * xAaInc);
				col += trace(xs, ys, pixel, ctr);
			}

		}

		col /= (aaSamples * aaSamples);
	} else {
		//cout << "in tracePixel before trace" << endl;
		col = trace(x, y, pixel, ctr);
		//cout << "in tracePixel after trace" << endl;

	}
	//cout << "in tracePixel 2" << endl;
	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
	//cout << "end tracePixel" << endl;

	return col;
}


// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
glm::dvec3 RayTracer::traceRay(ray& r, const glm::dvec3& thresh, int depth, double& t )
{
	//cout << "in traceRay" << endl;

	isect i;
	glm::dvec3 colorC;
	double n_i;
	double n_t;

	if(scene->intersect(r, i)) {
		if(depth < 0)
		{
			if (traceUI->cubeMap()){
				return this->getCubeMap()->getColor(r);
			}
			return glm::dvec3(0.0, 0.0, 0.0);
		}
		double c_t = i.t;
		glm::dvec3 n2 = glm::normalize(i.N);
		const Material& m = i.getMaterial();

		glm::dvec3 q = r.at(c_t);
		colorC = m.shade(scene, r, i);
		//cout << "1" << endl;

		if(m.kr(i)[0] > 0 || m.kr(i)[1] > 0 || m.kr(i)[2] > 0 ) {
			ray reflectRay = ray (q, glm::normalize(glm::reflect(r.getDirection(), n2)),
						0, 0, glm::dvec3(1,1,1), ray::REFLECTION);
			colorC = colorC + m.kr(i) * traceRay(reflectRay, thresh, depth-1, t);
		}

		if(!(m.kt(i)[0] > 0 || m.kt(i)[1] > 0 || m.kt(i)[2] > 0)) {
			return colorC;
		}
		//cout << "2" << endl;

		double enterobj = glm::dot(glm::normalize(r.getDirection()), glm::normalize(n2));
		if(enterobj < 0) {
			n_i = 1.0;
			n_t = m.index(i);
		} else if (enterobj > 0) {
			n2 = -n2;
			n_i = m.index(i);
			n_t = 1.0;
		} else {
			return glm::dvec3(0.0, 0.0, 0.0);
		}
		//cout << "3" << endl;

		double j = 1 - (glm::pow(n_i/n_t,2) * (1-glm::pow(glm::dot(n2,r.getDirection()),2)));
		if((m.kt(i)[0] > 0 || m.kt(i)[1] > 0 || m.kt(i)[2] > 0) && !((j < 0) && (n_t < n_i))) {
			ray refractRay = ray (q, glm::normalize(glm::refract(r.getDirection(), n2, n_i/n_t)),
						 0, 0, glm::dvec3(0,0,0), ray::REFRACTION);
			colorC = colorC + m.kt(i) * traceRay(refractRay, thresh, depth-1, t);
		}
	} else {
		if (traceUI->cubeMap()) {
			return this->getCubeMap()->getColor(r);
		}
		colorC = glm::dvec3(0.0, 0.0, 0.0);
	}
	//cout << "end traceRay" << endl;
	return colorC;
}

RayTracer::RayTracer()
	: scene(0), buffer(0), thresh(0), buffer_width(256), buffer_height(256), m_bBufferReady(false), cubemap (0)
{
}

RayTracer::~RayTracer()
{
	delete scene;
	delete [] buffer;
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer;
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return sceneLoaded() ? scene->getCamera().getAspectRatio() : 1;
}

bool RayTracer::loadScene( char* fn ) {
	ifstream ifs( fn );
	if( !ifs ) {
		string msg( "Error: couldn't read scene file " );
		msg.append( fn );
		traceUI->alert( msg );
		return false;
	}

	// Strip off filename, leaving only the path:
	string path( fn );
	if( path.find_last_of( "\\/" ) == string::npos ) path = ".";
	else path = path.substr(0, path.find_last_of( "\\/" ));

	// Call this with 'true' for debug output from the tokenizer
	Tokenizer tokenizer( ifs, false );
	Parser parser( tokenizer, path );
	try {
		delete scene;
		scene = 0;
		scene = parser.parseScene();
	} 
	catch( SyntaxErrorException& pe ) {
		traceUI->alert( pe.formattedMessage() );
		return false;
	}
	catch( ParserException& pe ) {
		string msg( "Parser: fatal exception " );
		msg.append( pe.message() );
		traceUI->alert( msg );
		return false;
	}
	catch( TextureMapException e ) {
		string msg( "Texture mapping exception: " );
		msg.append( e.message() );
		traceUI->alert( msg );
		return false;
	}

	if( !sceneLoaded() ) return false;
	//scene->setupKd();
	return true;
}

void RayTracer::traceSetup(int w, int h)
{
	if (buffer_width != w || buffer_height != h)
	{
		buffer_width = w;
		buffer_height = h;
		bufferSize = buffer_width * buffer_height * 3;
		delete[] buffer;
		buffer = new unsigned char[bufferSize];
	}
	memset(buffer, 0, w*h*3);
	m_bBufferReady = true;
}

void RayTracer::traceImage(int w, int h, int bs, double thresh)
{
	//cout << "in traceImage" << endl;
	#ifdef USE_THREADS
//		std::thread thread(&RayTracer::traceAux, this, 512, 192, 320);
		int threadcount = traceUI->getThreads();
		cout << "threadcount :: " << threadcount << endl;
		int interval = h/threadcount;
		std::thread threads[threadcount];
		int c = 0;
		for( int i = 0; i < threadcount; i++) {
			if( (c + interval) < h ) {
				threads[i] = std::thread(&RayTracer::traceAux, this, w, c, c+interval);
				c += interval;
			} else {
				threads[i] = std::thread(&RayTracer::traceAux, this, w, c, h);
			}
		}
		for( int i = 0; i < threadcount; i++) {
			threads[i].join();
		}
	#else
		traceAux(w,0,h);
	#endif
}

void RayTracer::traceAux(int w, int h1, int h2) {
	//cout << "tracing width :: " << w << ", and from height :: " << h1 << " to " << h2 << endl;
	//cout << "in traceAux" << endl;

	for( int j = h1; j < h2; ++j )
		for( int i = 0; i < w; ++i )
			tracePixel(i,j,0);
}

int RayTracer::aaImage(int samples, double aaThresh)
{
}

bool RayTracer::checkRender()
{
	// YOUR CODE HERE
	// FIXME: Return true if tracing is done.
	return true;
}

glm::dvec3 RayTracer::getPixel(int i, int j)
{
	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;
	return glm::dvec3((double)pixel[0]/255.0, (double)pixel[1]/255.0, (double)pixel[2]/255.0);
}

void RayTracer::setPixel(int i, int j, glm::dvec3 color)
{
	unsigned char *pixel = buffer + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * color[0]);
	pixel[1] = (int)( 255.0 * color[1]);
	pixel[2] = (int)( 255.0 * color[2]);
}


