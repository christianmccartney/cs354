#include <cmath>
#include <float.h>
#include <algorithm>
#include <assert.h>
#include <string.h>
#include "trimesh.h"
#include "../ui/TraceUI.h"
extern TraceUI* traceUI;

using namespace std;

Trimesh::~Trimesh()
{
	for( Materials::iterator i = materials.begin(); i != materials.end(); ++i )
		delete *i;
}

// must add vertices, normals, and materials IN ORDER
void Trimesh::addVertex( const glm::dvec3 &v )
{
    vertices.push_back( v );
}

void Trimesh::addMaterial( Material *m )
{
    materials.push_back( m );
}

void Trimesh::addNormal( const glm::dvec3 &n )
{
    normals.push_back( n );
}

// Returns false if the vertices a,b,c don't all exist
bool Trimesh::addFace( int a, int b, int c )
{
    int vcnt = vertices.size();

    if( a >= vcnt || b >= vcnt || c >= vcnt ) return false;

    TrimeshFace *newFace = new TrimeshFace( scene, new Material(*this->material), this, a, b, c );
    newFace->setTransform(this->transform);
    if (!newFace->degen) faces.push_back( newFace );


    // Don't add faces to the scene's object list so we can cull by bounding box
    // scene->add(newFace);
    return true;
}

const char* Trimesh::doubleCheck()
// Check to make sure that if we have per-vertex materials or normals
// they are the right number.
{
    if( !materials.empty() && materials.size() != vertices.size() )
        return "Bad Trimesh: Wrong number of materials.";
    if( !normals.empty() && normals.size() != vertices.size() )
        return "Bad Trimesh: Wrong number of normals.";

    return 0;
}

bool Trimesh::intersectLocal(ray& r, isect& i) const
{
	typedef Faces::const_iterator iter;
	bool have_one = false;
	for( iter j = faces.begin(); j != faces.end(); ++j )
	{
		isect cur;
		if( (*j)->intersectLocal( r, cur ) )
		{
			if( !have_one || (cur.t < i.t) )
			{
				i = cur;
				have_one = true;
			}
		}
	}
	if( !have_one ) i.setT(1000.0);
	return have_one;
} 

bool TrimeshFace::intersect(ray& r, isect& i) const {
  return intersectLocal(r, i);
}

// Intersect ray r with the triangle abc.  If it hits returns true,
// and put the parameter in t and the barycentric coordinates of the
// intersection in u (alpha) and v (beta).
bool TrimeshFace::intersectLocal(ray& r, isect& i) const
{
    const glm::dvec3& a = parent->vertices[ids[0]];
    const glm::dvec3& b = parent->vertices[ids[1]];
    const glm::dvec3& c = parent->vertices[ids[2]];

    // YOUR CODE HERE

	if (abs(area) < RAY_EPSILON)
		return false;

	double cos =  glm::dot(normal, r.d);

	if (abs(cos) < RAY_EPSILON)
		return false;

	double t = glm::dot(normal, (a - r.p)) / cos;

	if (t < RAY_EPSILON)
		return false;

	glm::dvec3 w = ((r.p + r.d * t) - a);
	double wv = glm::dot(w, v);
	double wu = glm::dot(w, u);

	double beta = ((uv * wv) - (vv * wu)) / area;
	double gamma = ((uv * wu) - (uu * wv)) / area;
	double alpha = 1.0 - (beta + gamma);

	if (alpha < 0.0 || beta < 0.0 || gamma < 0.0)
		return false;

	i.t = t;
	i.bary[0] = alpha;
	i.bary[1] = beta;
	i.bary[2] = gamma;
	i.uvCoordinates[0] = beta;
	i.uvCoordinates[1] = gamma;
	i.obj = this;

	if (parent->vertNorms) {
		const glm::dvec3& n1 = parent->normals[ids[0]];
		const glm::dvec3& n2 = parent->normals[ids[1]];
		const glm::dvec3& n3 = parent->normals[ids[2]];

		i.N = ((alpha * n1) + (beta * n2) + (gamma * n3));
		glm::normalize(i.N);
	} else {
		i.N = normal;
		glm::normalize(i.N);
	}

	if (!parent->materials.empty()) {
		Material m1(*parent->materials[ids[0]]);
		Material m2(*parent->materials[ids[1]]);
		Material m3(*parent->materials[ids[2]]);

		Material m;
		m += (alpha * m1);
		m += (beta * m2);
		m += (gamma * m3);

		i.setMaterial(m);
	} else {
		i.setMaterial(this->getMaterial());
	}

	return true;
}

void Trimesh::generateNormals()
// Once you've loaded all the verts and faces, we can generate per
// vertex normals by averaging the normals of the neighboring faces.
{
    int cnt = vertices.size();
    normals.resize( cnt );
    int *numFaces = new int[ cnt ]; // the number of faces assoc. with each vertex
    memset( numFaces, 0, sizeof(int)*cnt );
    
    for( Faces::iterator fi = faces.begin(); fi != faces.end(); ++fi )
    {
		glm::dvec3 faceNormal = (**fi).getNormal();
        
        for( int i = 0; i < 3; ++i )
        {
            normals[(**fi)[i]] += faceNormal;
            ++numFaces[(**fi)[i]];
        }
    }

    for( int i = 0; i < cnt; ++i )
    {
        if( numFaces[i] )
            normals[i]  /= numFaces[i];
    }

    delete [] numFaces;
    vertNorms = true;
}

