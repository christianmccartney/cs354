#include "TrimeshSubdivide.h"


void subdivide(char* rayfile) {

	int swtch = 1;
	int pointcount = 0;
	int facecount = 0;
	int isreading = 0;
	int i,j = 0;
	int count = 0;
	size_t foundpoints = 0;
	size_t foundfaces = 0;
	size_t isfound = 0;
	vector<string> buffer;
	
	unsigned int o1, o2, o3;

        string s;
        smatch m;
        regex e ("([0-9]+([.]([0-9]+))?)");

		
	ifstream testinput;
	testinput.open(rayfile);
	string testline;

	if (testinput.is_open()) {
		while( getline(testinput, testline)) {
			foundpoints = testline.find("points");
			foundfaces = testline.find("faces");
			if( foundpoints < string::npos ) {
				swtch = 0;
				while( testline.find(";") == string::npos) {
					pointcount += 1;
					getline(testinput, testline);
				}
			}
			if( foundfaces < string::npos ) {
                                while( testline.find(";") == string::npos) {
                                        facecount += 1;
					getline(testinput, testline);
                                }
                        }
			if(swtch) {
				buffer.push_back(testline);
			}
		}

	}
	testinput.close();
	
	
	cout << "1" << endl;

	int face1[facecount];
	int face2[facecount];
	int face3[facecount];

	vertex vertices[pointcount];
	//halfedge halfedges[facecount*3];
	face faces[facecount];

        ifstream input;
        input.open(rayfile);
        string line;

	//cout << "in subdivide" << endl;
        if (input.is_open()) {
                while( getline(input, line)) {
                        foundpoints = line.find("points");
                        foundfaces = line.find("faces");
                        if(foundpoints < string::npos) {
                                isreading = 1;
								count = 0;
                                while( isreading ) {

										getline(input, line);

                                        isfound = line.find(";");
                                        if( isfound < string::npos)
                                                isreading = 0;

                                        s = line;
										i = 0;
                                        while( regex_search(s, m, e)) {
												if(i == 0) vertices[count].x = stod(m.str(0));
                                                if(i == 1) vertices[count].y = stod(m.str(0));
                                                if(i == 2) vertices[count].z = stod(m.str(0));
												vertices[count].e = NULL;
												i+=1;
                                                s = m.suffix().str();
                                        }
									count += 1;
                                }
                        }
						if(foundfaces < string::npos) {
                                isreading = 1;
                                count = 0;
                                while( isreading ) {

                                        getline(input, line);

                                        isfound = line.find(";");
                                        if( isfound < string::npos)
                                                isreading = 0;

                                        s = line;
                                        i = 0;
                                        while( regex_search(s, m, e)) {
                                                if(i == 0) face1[count] = stoul(m.str(0));
                                                if(i == 1) face2[count] = stoul(m.str(0));
                                                if(i == 2) face3[count] = stoul(m.str(0));
												i+=1;
                                                s = m.suffix().str();
                                        }
                                        count += 1;
                                }
                        }

                }
        }

	input.close();
	

	map< pair<unsigned int, unsigned int>, halfedge* > edges;


        for( i = 0; i < facecount; i++) {

		if( i%2 == 0 ) {
        	        faces[i].ei[0] = make_pair(face1[i], face2[i]);
	                faces[i].ei[1] = make_pair(face2[i], face3[i]);
                	faces[i].ei[2] = make_pair(face3[i], face1[i]);
		} else {
					faces[i].ei[0] = make_pair(face2[i], face1[i]);
					faces[i].ei[1] = make_pair(face3[i], face2[i]);
					faces[i].ei[2] = make_pair(face1[i], face3[i]);
		}

                //cout << "faces:: " << faces[i].ei[0].first << ", " << faces[i].ei[0].second << endl;
                //cout << "faces:: " << faces[i].ei[1].first << ", " << faces[i].ei[1].second << endl;
                //cout << "faces:: " << faces[i].ei[2].first << ", " << faces[i].ei[2].second << endl;
        }

	unsigned int edgecount = 0;
	for( i = 0; i < facecount; i++) {
		j = 0;
		for( pair<unsigned int, unsigned int> p : faces[i].ei) {
			//cout << "test:: " << p.first << ", " << p.second << endl;
			edges[p] = new halfedge();
			edges[p]->f = &faces[i];
			edges[p]->v = &vertices[p.first];
			edges[p]->isnew = false;
			edgecount+=1;
			if( vertices[p.first].e == NULL) {
				vertices[p.first].e = edges[p];
			}
		}

		for( pair<unsigned int, unsigned int> p : faces[i].ei) {

			switch(j) {
				case 0: edges[p]->next = edges[faces[i].ei[1]];
						edges[p]->prev = edges[faces[i].ei[2]];
					break;
				case 1: edges[p]->next = edges[faces[i].ei[2]];
						edges[p]->prev = edges[faces[i].ei[0]];
					break;
				case 2: edges[p]->next = edges[faces[i].ei[0]];
						edges[p]->prev = edges[faces[i].ei[1]];
					break;
			}
			j+=1;
		}
	}

	for( i = 0; i < facecount; i++) {
		for( pair<unsigned int, unsigned int> p : faces[i].ei) {
			pair<unsigned int, unsigned int> pr (p.second, p.first);
			if( edges.find(pr) != edges.end()) {
				//cout << "in if:: " << p.first << ", " << p.second << " : " << pr.first << ", " << pr.second << endl;
				edges[p]->twin = edges[pr];
				edges[pr]->twin = edges[p];
			}
		}
	}
	
	
	//------------------------------------------------------------------------- build new mesh
	
	for( i = 0; i < pointcount; i++) {
		vertices[i].isnew = false;
	}

	double a, b, c, d, r = 0.0;

	j = 0;
	unsigned int newvertcount = pointcount+(edgecount/2);
	vertex *newvertices[newvertcount];
	
	unsigned int newedgecount = 4*edgecount;
	unsigned int newfacecount = 4*facecount;
	
	halfedge **newedges = (halfedge**)malloc(sizeof(halfedge)*newedgecount);
	face **newfaces = (face**)malloc(sizeof(face)*newfacecount);
	
	double beta = 3.0/16.0;
	a, b, c = 0.0;
	
	
	for(auto const& e : edges) {
		if(!e.second->v->isnew) {
			e.second->v->newv = new vertex();
			
			a = e.second->next->v->x;
			b = e.second->next->next->v->x;
			c = e.second->twin->next->next->v->x;
			
			e.second->v->newv->x = e.second->v->x * (1.0 - 3.0*beta) + (a+b+c) * beta;
			
			a = e.second->next->v->y;
			b = e.second->next->next->v->y;
			c = e.second->twin->next->next->v->y;
			
			e.second->v->newv->y = e.second->v->y * (1.0 - 3.0*beta) + (a+b+c) * beta;
			
			a = e.second->next->v->z;
			b = e.second->next->next->v->z;
			c = e.second->twin->next->next->v->z;
			
			e.second->v->newv->z = e.second->v->z * (1.0 - 3.0*beta) + (a+b+c) * beta;
			
			newvertices[j] = e.second->v->newv;
			newvertices[j]->e = e.second;
			j+=1;
			e.second->v->isnew = 1;
		}
		
		if(!e.second->isnew) {
			e.second->newv = new vertex();
			e.second->twin->newv = new vertex();
			
			a = e.second->v->x;
			b = e.second->twin->v->x;
			c = e.second->next->next->v->x;
			d = e.second->twin->next->next->v->x;
			r = (3.0/8.0)*(a+b) + (1.0/8.0)*(c+d);
			e.second->newv->x = r;
			
			a = e.second->v->y;
			b = e.second->twin->v->y;
			c = e.second->next->next->v->y;
			d = e.second->twin->next->next->v->y;
			r = (3.0/8.0)*(a+b) + (1.0/8.0)*(c+d);
			e.second->newv->y = r;

			a = e.second->v->z;
			b = e.second->twin->v->z;
			c = e.second->next->next->v->z;
			d = e.second->twin->next->next->v->z;
			r = (3.0/8.0)*(a+b) + (1.0/8.0)*(c+d);
			e.second->newv->z = r;
			
			e.second->twin->newv = e.second->newv;

			e.second->isnew = 1;
			e.second->twin->isnew = 1;
			
			e.second->isorig = 1;
			e.second->twin->isorig = 1;
			
			
			
			newvertices[j] = e.second->newv;
			newvertices[j]->e = e.second;
			j+=1;
		}
	}
	
	i=0;
	for(auto e : edges) {
		newedges[i] = e.second;
		//cout << i << " :: " << e.second->v->newv << " : " << e.second->v->newv->x << ", " << e.second->v->newv->y << ", " << e.second->v->newv->z << endl;
		//cout << i+1 << " :: " << e.second->next->v->newv << " : " << e.second->next->v->newv->x << ", " << e.second->next->v->newv->y << ", " << e.second->next->v->newv->z << endl;
		//cout << i+2 << " :: " << e.second->next->next->v->newv << " : " << e.second->next->next->v->newv->x << ", " << e.second->next->next->v->newv->y << ", " << e.second->next->next->v->newv->z << endl;
		newedges[i+1] = new halfedge();
		newedges[i+2] = new halfedge();
		newedges[i+3] = new halfedge();
		
		i+=4;
	}

	for(i = 0; i < edgecount*4; i+=4) {
		if(newedges[i]->isnew) {
			
			//cout << i << endl;
			
			halfedge *temp1 = newedges[i]->next;
			halfedge *temp2 = newedges[i]->next->next;
			
			//cout << "edge:: " << newedges[i]->v->newv->x << ", " << newedges[i]->v->newv->y << ", " << newedges[i]->v->newv->z << endl;
			//cout << "next:: " << temp1->v->x << ", " << temp1->v->y << ", " << temp1->v->z << endl;
			//cout << "prev:: " << temp2->v->x << ", " << temp2->v->y << ", " << temp2->v->z << endl;
			
			newedges[i]->next = newedges[i+1];
			newedges[i+1]->v = newedges[i]->newv;
			newedges[i]->isnew = 0;
			newedges[i]->v = newedges[i]->v->newv;
			newedges[i]->v->isnew = 0;
			
			newedges[i+1]->next = temp2;
			newedges[i+1]->prev = newedges[i];
			newedges[i+1]->twin = newedges[i+3];
			newedges[i+2]->v = newedges[i]->newv;
			
			newedges[i+2]->next = temp1;
			newedges[i+2]->prev = newedges[i+3];
			if(temp2->v->isnew) {
				newedges[i+3]->v = temp2->v->newv;
			} else {
				newedges[i+3]->v = temp2->v;
			}
			newedges[i+3]->next = newedges[i+2];
			newedges[i+3]->prev = temp1;
			newedges[i+3]->twin = newedges[i+1];
			
			
			temp1->next = newedges[i+3];
			temp1->prev = newedges[i+2];
			temp2->next = newedges[i];
			temp2->prev = newedges[i+1];
			
			
			face *f1 = new face();
			face *f2 = new face();
			
			newedges[i]->f = f1;
			newedges[i+1]->f = f1;
			newedges[i]->prev->f = f1;
			
			newedges[i+2]->f = f2;
			newedges[i+3]->f = f2;
			newedges[i+2]->next->f = f2;
			
			newedges[i]->f->e = newedges[i];
			newedges[i+2]->f->e = newedges[i+2];
			
			
			if(newedges[i]->v->isnew) {
				newedges[i]->v = newedges[i]->v->newv;
				newedges[i]->v->isnew = 0;
			}
			/*
			cout << "split:: " << newedges[i]->v->x << ", " << newedges[i]->v->y << ", " << newedges[i]->v->z; 
			cout << " : " << newedges[i]->newv->x << ", " << newedges[i]->newv->y << ", " << newedges[i]->newv->z;// << " : " << vertexindex.find(newedges[i]->v)->second << " : " << newedges[i+1]->v << endl;
			cout << " into:: " <<  newedges[i+1]->v->x << ", " << newedges[i+1]->v->y << ", " << newedges[i+1]->v->z;// << " : " << vertexindex.find(newedges[i+1]->v)->second << " : " << newedges[i+1]->v << endl;
			cout << " into:: " << newedges[i+2]->v->x << ", " << newedges[i+2]->v->y << ", " << newedges[i+2]->v->z;// << " : " << vertexindex.find(newedges[i+2]->v)->second << " : " << newedges[i+1]->v << endl;
			cout << " into:: " << newedges[i+3]->v->x << ", " << newedges[i+3]->v->y << ", " << newedges[i+3]->v->z;// << " : " << vertexindex.find(newedges[i+3]->v)->second  << " : " << newedges[i+1]->v << endl;
			cout << endl << endl;
			*/
			j+=2;
		}
	}
	
	
	for(i = 0; i < edgecount*4; i++) {
		for(j = 0; j < pointcount; j++){
			if(newedges[i]->v == &vertices[j] && !newedges[i]->isorig) {
				halfedge *temp1 = newedges[i]->next;			//11
				halfedge *temp2 = newedges[i]->twin->next;		//21
				halfedge *temp3 = newedges[i]->prev;			//12
				halfedge *temp4= newedges[i]->twin->prev;		//22
				
				/*
				face *tf1 = newedges[i]->f;
				face *tf2 = newedges[i]->twin->f;
				*/
				face *tf1 = new face();
				face *tf2 = new face();
				vertex *v1 = newedges[i]->v;
				vertex *v2 = newedges[i]->twin->v;
				
				newedges[i]->next = temp2->next;
				newedges[i]->v = temp1->next->v;
				newedges[i]->twin->next = temp1->next;
				newedges[i]->twin->v = temp2->next->v;
				
				temp1->next = newedges[i];
				temp2->next = newedges[i]->twin;
				temp1->f = tf1;
				temp2->f = tf2;
				temp3->next = temp2;
				temp3->f = tf2;
				temp4->next = temp1;
				temp4->f = tf1;
				
				if(tf2->e == temp4)
					tf2->e = temp3;
				if(tf1->e == temp3)
					tf1->e = temp4;
				if(v1->e == newedges[i])
					v1->e = temp2;
				if(v2->e == newedges[i]->twin)
					v2->e = temp1;
			}
		}
	}
	
	ofstream output;
	output.open("trimeshdiv.ray");
	
	for(string i : buffer) {
		output << i << "\n";
	}
	
	output << "\tpoints = (\n";
	
	for(i = 0; i < newvertcount; i++) {
		if(i < newvertcount-1)
			output << "\t\t(" << newvertices[i]->x << ", " << newvertices[i]->y << ", " << newvertices[i]->z << "),\n";
		else
			output << "\t\t(" << newvertices[i]->x << ", " << newvertices[i]->y << ", " << newvertices[i]->z << "));\n";
	}
	
	
	for(i = 0; i < newedgecount; i++){
		//newedges[i]->f = new face();
	}
	
	map< vertex*, int > vertexindex;
	
	j = 0;
	
	output << "\tfaces = (\n";
	cout << endl;
	
	for(i = 0; i < newedgecount; i++) {
		if(!newedges[i]->isadded) {
			newfaces[j] = newedges[i]->f;
			newfaces[j]->e = newedges[i];
			newedges[i]->isadded = 1;
			newedges[i]->next->isadded = 1;
			newedges[i]->next->next->isadded = 1;
			j+=1;
		}
	}
	
	
	for(i = 0; i < newfacecount; i++) {
		
		
		o1 = vertexindex.find(newfaces[i]->e->v)->second;
		o2 = vertexindex.find(newfaces[i]->e->next->v)->second;
		o3 = vertexindex.find(newfaces[i]->e->next->next->v)->second;
		
		//unsigned int out[] = {0, 0, 1, 2, 3, -1, 16};
		
		vector<unsigned int> outv = {o1,o2,o3};
		
		//vector<unsigned int> outv (out, out+7); 
		//sort(outv.begin(), outv.begin()+3);
		
		if(i%2==0){
		
			if( i < newfacecount - 1){
				output << "\t\t(" << outv[0] << ", " << outv[1] << ", " << outv[2] << "),\n";
			} else {
				output << "\t\t(" << outv[0] << ", " << outv[1] << ", " << outv[2] << "));\n";
			}
		} else {
			if( i < newfacecount - 1){
				output << "\t\t(" << outv[2] << ", " << outv[1] << ", " << outv[0] << "),\n";
			} else {
				output << "\t\t(" << outv[2] << ", " << outv[1] << ", " << outv[0] << "));\n";
			}
		}
		
		//cout << "edges " << i << " :: " << o1 << ", " << o2 << ", " << o3 << endl;
		//cout << "edges " << i << " :: " << newfaces[i]->e->v->x << ", " << newfaces[i]->e->v->y << ", " << newfaces[i]->e->v->z << " : " << newfaces[i]->e->v << endl;
		//cout << " : " << newfaces[i]->e->next->v->x << ", " << newfaces[i]->e->next->v->y << ", " << newfaces[i]->e->next->v->z << " : " << newfaces[i]->e->next->v << endl;
		//cout << " : " << newfaces[i]->e->next->next->v->x << ", " << newfaces[i]->e->next->next->v->y << ", " << newfaces[i]->e->next->next->v->z << " : " << newfaces[i]->e->next->next->v << endl << endl;
		
	}
	
	output << "\t}";
}

bool exists(const string& name) {
	ifstream f(name.c_str());
	return f.good();
}
