#include "material.h"
#include "ray.h"
#include "light.h"
#include "../ui/TraceUI.h"
extern TraceUI* traceUI;

#include "../fileio/bitmap.h"
#include "../fileio/pngimage.h"
#include <iostream>

using namespace std;
extern bool debugMode;

Material::~Material()
{
}

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
glm::dvec3 Material::shade(Scene *scene, const ray& r, const isect& i) const
{
	glm::dvec3 p = r.getPosition() + r.getDirection() * i.t;
        glm::dvec3 color = ke(i) + ka(i)*scene->ambient();

        for (vector<Light*>::const_iterator it = scene->beginLights(); it != scene->endLights(); it++) {
                Light* light = *it;
                glm::dvec3 direction = light->getDirection(p);
                double nl = glm::dot(i.N, direction);

                if (nl < 0) continue;

                glm::dvec3 ref = glm::normalize(2.0 * i.N * glm::dot(i.N, direction) - direction);
                ray lightRay(p, direction, r.getPixel(), r.ctr, r.getAtten(), ray::SHADOW);
                glm::dvec3 attenuation = light->distanceAttenuation(p) * light->shadowAttenuation(lightRay, p);
                glm::dvec3 c = light->getColor();
                double l = max(0.0, nl);

                color += c * l * kd(i) * attenuation + c * ks(i) *
                        max(0.0, pow(glm::dot(ref, -glm::normalize(r.d)), shininess(i))) * attenuation;
        }

        return color;
}

TextureMap::TextureMap( string filename ) {

	int start = (int) filename.find_last_of('.');
	int end = (int) filename.size() - 1;
	if (start >= 0 && start < end) {
		string ext = filename.substr(start, end);
		if (!ext.compare(".png")) {
			png_cleanup(1);
			if (!png_init(filename.c_str(), width, height)) {
				double gamma = 2.2;
				int channels, rowBytes;
				unsigned char* indata = png_get_image(gamma, channels, rowBytes);
				int bufsize = rowBytes * height;
				data = new unsigned char[bufsize];
				for (int j = 0; j < height; j++)
					for (int i = 0; i < rowBytes; i += channels)
						for (int k = 0; k < channels; k++)
							*(data + k + i + j * rowBytes) = *(indata + k + i + (height - j - 1) * rowBytes);
				png_cleanup(1);
			}
		}
		else
			if (!ext.compare(".bmp")) data = readBMP(filename.c_str(), width, height);
			else data = NULL;
	} else data = NULL;
	if (data == NULL) {
		width = 0;
		height = 0;
		string error("Unable to load texture map '");
		error.append(filename);
		error.append("'.");
		throw TextureMapException(error);
	}
}

glm::dvec3 TextureMap::getMappedValue( const glm::dvec2& coord ) const
{
	double a = coord[0]*getWidth();
	double b = coord[1]*getHeight();
	int i = (int)(a - fmod(a,1.0));
	int j = (int)(b - fmod(b,1.0));
	double dx = a - (a - fmod(a,1.0));
	double dy = b - (b - fmod(b,1.0));
	glm::dvec3 tcolor = (1 - dx)*(1 - dy)*getPixelAt(i,j) + dx*(1 - dy)*getPixelAt(i+1,j) +
						(1 - dx)*dy*getPixelAt(i,j+1) + dx*dy*getPixelAt(i+1,j+1);

	return tcolor;
}


glm::dvec3 TextureMap::getPixelAt( int x, int y ) const
{
    // This keeps it from crashing if it can't load
    // the texture, but the person tries to render anyway.
    if (0 == data)
      return glm::dvec3(1.0, 1.0, 1.0);

    if( x >= width )
       x = width - 1;
    if( y >= height )
       y = height - 1;

    // Find the position in the big data array...
    int pos = (y * width + x) * 3;
    return glm::dvec3(double(data[pos]) / 255.0, 
       double(data[pos+1]) / 255.0,
       double(data[pos+2]) / 255.0);
}

glm::dvec3 MaterialParameter::value( const isect& is ) const
{
    if( 0 != _textureMap )
        return _textureMap->getMappedValue( is.uvCoordinates );
    else
        return _value;
}

double MaterialParameter::intensityValue( const isect& is ) const
{
    if( 0 != _textureMap )
    {
        glm::dvec3 value( _textureMap->getMappedValue( is.uvCoordinates ) );
        return (0.299 * value[0]) + (0.587 * value[1]) + (0.114 * value[2]);
    }
    else
        return (0.299 * _value[0]) + (0.587 * _value[1]) + (0.114 * _value[2]);
}

