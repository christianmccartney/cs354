#include<iostream>
#include<sstream>
#include<string>
#include<fstream>
#include<vector>
#include<cmath>

#define INTERP_CP 17
#define TEXT 2

using namespace std;

class Array3D {
    size_t m_width, m_height;
    std::vector<float> m_data;
  public:
    Array3D(size_t x, size_t y, size_t z, float init = 0.0):
      m_width(x), m_height(y), m_data(x*y*z, init)
    {}
    float& operator()(size_t x, size_t y, size_t z) {
        return m_data.at(x + y * m_width + z * m_width * m_height);
    }
};

float m[4][4] =		{{1, 0, 0, 0},
					{-3, 3, 0, 0},
					{3, -6, 3, 0},
					{-1, 3, -3, 1}};

float mt[4][4] =	{{1, -3, 3, -1},
					{0, 3, -6, 3},
					{0, 0, 3, -3},
					{0, 0, 0, 1}};
					
float md[4][4] =	{{-1, 3, -3, 1},
					{3, -6, 3, 0},
					{-3, 3, 0, 0},
					{1, 0, 0, 0}};

float b(int i, float t) {
	switch(i) {
		case 0:
			return (float)(pow((1.0-t), 3.0));
		case 1:
			return (float)(3*pow((1.0-t), 2.0)*t);
		case 2:
			return (float)(3*(1.0-t)*pow(t, 2.0));
		case 3:
			return (float)(pow(t, 3.0));
	}
}

float tab(float t, int i, float p) {
	float a;
	switch(i) {
		case 0:
		return (-3.0 * (1.0 - t) * (1.0 - t)) * p; 						//p0
		case 1:
		return (3.0 * (1.0 - t) * (1.0 - t) - 6.0 * t * (1.0 - t)) * p;	//p1
		case 2:								
		return (6.0 * t * (1.0 - t) - 3.0 * t * t ) * p;				//p2	
		case 3:
		return (3.0 * t * t) * p;										//p3
	}
	return a;
}

float ta(float u, float v, bool c, float P[4][4]) {
	int i,j;
	float s = 0.0;
	float p = 0;
	if(c){
		for(i = 0; i < 3; i++) {
			for(j = 0; j < 3; j++) {
				p = p + (b(j, v) * P[i][j]);
			}
			s = s + tab(u, i, p);
			p = 0;
		}
	} else {
		for(j = 0; j < 3; j++) {
			for(i = 0; i < 3; i++) {
				p = p + b(i, u) * P[i][j];
			}
			s = s + tab(v, j, p);
			p = 0;
		}
	}
	return s;
}


					
float snf(float u, float v, float P[4][4]) {
	int i, j;
	float s = 0.0;
	for(i = 0; i < 3; i++) {
		for(j = 0; j < 3; j++) {
			s = s + (P[i][j] * b(i, u) * b(j, v));
		}
	}
	
	
}
					
float sf(float u, float v, float P[4][4]) {
	float um[4] = {1.0, u, pow(u, 2), pow(u, 3)};
	float vt[4] = {1.0, v, pow(v, 2), pow(v, 3)};
	//float um[4] = {pow(u, 3), pow(u, 2), u, 1.0};
	//float vt[4] = {pow(v, 3), pow(v, 2), v, 1.0};
	float nsum = 0.0;
	float nm[4];
	float n2m[4];
	float n3m[4];
	//cout << "in sf:: " << endl;
	int i, j;
	for(i = 0; i < 4; i++ ) {
		for(j = 0; j < 4; j++) {
			nsum += um[j] * m[j][i];
		}
		nm[i] = nsum;
		nsum = 0;
	}
	for(i = 0; i < 4; i++ ) {
		for(j = 0; j < 4; j++) {
			nsum += nm[j] * P[j][i];
		}
		n2m[i] = nsum;
		nsum = 0;
	}
	for(i = 0; i < 4; i++ ) {
		for(j = 0; j < 4; j++) {
			nsum += n2m[j] * mt[j][i];
		}
		n3m[i] = nsum;
		nsum = 0;
	}
	for(i = 0; i < 4; i++ ) {
		nsum += n3m[i] * vt[i];
	}
	return nsum;
}

float rf(float t, float* R , float* D, int k) {
	float a = R[k] + t*D[k];
	return a;
}

float srf(float u, float v, float t, float* R, float* D, float P[4][4], int k) {
	float a = snf(u, v, P) - rf(t, R, D, k);
	return a;
}


float fr(float u, float v, float a, float b, float e, float* R, float* D, float P[4][4], int k) {
	float c = 0.0;
	float fa = 0.0;
	float fc = 0.0;
	
	float t1 = srf(u, v, a, R, D, P, k);
	float t2 = srf(u, v, b, R, D, P, k);
	
	if(fabs(t1) < 1.0e5 ) {
		t1 = 0;
	}
	if(fabs(t2) < 1.0e5 ) {
		t2 = 0;
	}
	
	if ( t1*t2  > 0) {
		return -1;
	} else {
		while (fabs(a-b) >= e) {
			c = (a + b)/2.0;
			fa = round(srf(u, v, a, R, D, P, k)*1.0e5)/1.0e5;
			fc = round(srf(u, v, c, R, D, P, k)*1.0e5)/1.0e5;
			
			if(fc == 0 ) {
				return c;
			} else if(fa*fc < 0) {
				b = c;
			} else {
				a = c;
			}
		}
		return c;
	}
}

int main() {

	ifstream source;
	if(TEXT == 1) {
		source.open("text.txt");
	} else if(TEXT == 2) {
		source.open("text2.txt");
	}
	
	float R[3];
	float D[3];
	int i,j,k = 0;
	float Px[4][4];
	float Py[4][4];
	float Pz[4][4];
	
	float x, y, z;
	for( i = 0; i < 4; i++ ) {
		for( j = 0; j < 4; j++) {
			source >> x >> y >> z;
			Px[i][j] = x;
			Py[i][j] = y;
			Pz[i][j] = z;
		}
	}
	source >> x >> y >> z;
	R[0] = x;
	R[1] = y;
	R[2] = z;
	
	
	source >> x >> y >> z;
	D[0] = x;
	D[1] = y;
	D[2] = z;

	source.close();

	cout << "P:: " << endl;
	for( i = 0; i < 4; i++ ) {
		for( j = 0; j < 4; j++ ) {
			cout << Px[i][j] << ", " << Py[i][j] << ", " << Pz[i][j] << endl;
		}
	}
	cout <<  "R:: " << R[0] << ", " << R[1] << ", " << R[2] << endl;
	cout <<  "D:: " << D[0] << ", " << D[1] << ", " << D[2] << endl;


	float e = 0.00001;
	float a = 0.0;
	float b = 10.0;
	
	float xt = -1;
	float yt = -1;
	float zt = -1;
	
	float xi = -1;
	float yi = -1;
	float zi = -1;
	
	float uf = -1;
	float vf = -1;
	
	float ri = -1;
	float rp = 10;
	
	float newres;
	float res = 10;
	float rt;
	float rft = 10;

	int ni = INTERP_CP-1;
	float u[ni];
	float v[ni];
	float t = 1.0/(ni-1);
	
	for(i = 0; i < ni; i++) {
		u[i] = i*t;
		v[i] = i*t;
	}
	
	for(i = 0; i < ni; i++) {
		for(j = 0; j < ni; j++) {
			if(D[0]) {
				
				xt = fr(u[i], v[j], a, b, e, R, D, Px, 0);
				yt = fr(u[i], v[j], a, b, e, R, D, Py, 1);
				zt = fr(u[i], v[j], a, b, e, R, D, Pz, 2);
				
				rt = fr(u[i], v[j], a, b, e, R, D, Px, 0);
				
				newres = fabs(floor(srf(u[i], v[j], rt, R, D, Px, 0)*1.0e5)/1.0e5);
			} else if(D[1]) {
				
				xt = fr(u[i], v[j], a, b, e, R, D, Px, 0);
				yt = fr(u[i], v[j], a, b, e, R, D, Py, 1);
				zt = fr(u[i], v[j], a, b, e, R, D, Pz, 2);
				
				rt = fr(u[i], v[j], a, b, e, R, D, Py, 1);
				
				newres = fabs(floor(srf(u[i], v[j], rt, R, D, Py, 1)*1.0e5)/1.0e5);
			} else if(D[2]) {
				
				xt = fr(u[i], v[j], a, b, e, R, D, Px, 0);
				yt = fr(u[i], v[j], a, b, e, R, D, Py, 1);
				zt = fr(u[i], v[j], a, b, e, R, D, Pz, 2);
				
				rt = fr(u[i], v[j], a, b, e, R, D, Pz, 2);
				
				newres = fabs(floor(srf(u[i], v[j], rt, R, D, Pz, 2)*1.0e5)/1.0e5);
			} else {
				cout << "error: no direction" << endl;
				return 0;
			}
			
			xi = fabs(floor(srf(u[i], v[j], rt, R, D, Px, 0)*1.0e5)/1.0e5);
			yi = fabs(floor(srf(u[i], v[j], rt, R, D, Py, 1)*1.0e5)/1.0e5);
			zi = fabs(floor(srf(u[i], v[j], rt, R, D, Pz, 2)*1.0e5)/1.0e5);
			
			ri = xi + yi + zi;
			
			if(fabs(newres) <= 1.0e5) 
				newres = 0;
			
			if(newres <= res){
				if(xt >= 0 && yt >= 0 && zt >= 0) {
					if(xi >= newres && yi >= newres && zi >= newres) {
						if(xi < 1 && yi < 1 && zi < 1 && ri < rp) {
							res = newres;
							rft = rt;
							rp = ri;
							uf = u[i];
							vf = v[j];
						}
					}
				}
			}
		}
	}
	
	cout << endl;
	cout << "u, v, t:: " << uf << ", " << vf << ", " << rft;
	
	
	float xtan[3] = {ta(uf, vf, true, Px), ta(uf, vf, true, Py), ta(uf, vf, true, Pz)};
	float ytan[3] = {ta(uf, vf, false, Px), ta(uf, vf, false, Py), ta(uf, vf, false, Pz)};
	float normal[3] =  {xtan[1]*ytan[2]-xtan[2]*ytan[1],
						xtan[2]*ytan[0]-xtan[0]*ytan[2],
						xtan[0]*ytan[1]-xtan[1]*ytan[0]};
	
	cout << endl;
	cout << "xtan:: " << endl;
	cout << "(" << xtan[0];
	cout << "," << xtan[1];
	cout << "," << xtan[2] << ")";
	cout << endl;
	cout << "ytan:: " << endl;
	cout << "(" << ytan[0];
	cout << "," << ytan[1];
	cout << "," << ytan[2] << ")";
	cout << endl;
	cout << endl;
	cout << "normal:: " << endl;
	cout << "(" << normal[0];
	cout << "," << normal[1];
	cout << "," << normal[2] << ")";
	cout << endl;
	
	return 0;
}
