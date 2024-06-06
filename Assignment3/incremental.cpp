#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <cmath>
#include <vector>
#include "incremental.h"
using namespace std;

// float pointSet [][3] = {{0,0,0},{0,10,0},{10,10,0},{10,0,0},{0,0,10},{0,10,10},{10,10,10},{10,0,10}};
float pointSet[][3] = {{1,1,2},{0,10,3},{3,1,1},{8,1,0},{3,4,0},{0,3,4},{1,3,1}};

struct Vertex; struct Edge; struct Triangle;

struct Vertex {
    float v[3];
    int vnum;
    Edge* duplicate;
    bool onhull;
    bool mark;
    Vertex* next, *prev;
    Vertex(){}
    Vertex(float a, float b, float c){
        v[X]=a;
        v[Y]=b;  
        v[Z]=c;
    }
};

struct Triangle {
    Vertex* vertex[3];
    Edge* edge[3];
    Triangle* next, *prev;
    bool visible;
    Triangle(){}
    Triangle(Vertex* pt1, Vertex* pt2, Vertex* pt3){
        vertex[0]=pt1;
        vertex[1]=pt2;
        vertex[2]=pt3;
    }
};

struct Edge {
    Triangle* adjFace[2];
    Vertex* endPts[2];
    Triangle* newface;  
    Edge* next, *prev;
    bool del;
    Edge(){};
};

void writeTriangleToSTL(Vertex* normal, Triangle* triangle, string fileName){
    fstream file(fileName, ios::in | ios::out);
    string lastLine="endsolid hull";
    file.seekg(-lastLine.length()-2, ios_base::end);
    file << "facet normal "<<normal->v[X]<<" "<<normal->v[Y]<<" "<<normal->v[Z]<<"\n";
    file << "outer loop\n";
    file << "vertex "<<triangle->vertex[0]->v[0]<<" "<<triangle->vertex[0]->v[1]<<" "<<triangle->vertex[0]->v[2]<<"\n";
    file << "vertex "<<triangle->vertex[1]->v[0]<<" "<<triangle->vertex[1]->v[1]<<" "<<triangle->vertex[1]->v[2]<<"\n";
    file << "vertex "<<triangle->vertex[2]->v[0]<<" "<<triangle->vertex[2]->v[1]<<" "<<triangle->vertex[2]->v[2]<<"\n";
    file << "endloop\n";
    file << "endfacet\n";
    file << lastLine <<"\n";
    file.close();
    return;
}

Vertex* vertices = NULL;
Edge* edges = NULL;
Triangle* faces = NULL;

Vertex* MakeNullVertex(){
    Vertex* v;
    v = new Vertex();
    v->duplicate = NULL;
    v->onhull = !ONHULL;
    v->mark = !PROCESSED;
    ADD(vertices, v);
    return v;
}

Edge* MakeNullEdge(){
    Edge* e = new Edge();
    e->adjFace[0] = e->adjFace[1] = e->newface = NULL;
    e->endPts[0] = e->endPts[1] = NULL;
    e->del = !REMOVED;
    ADD(edges, e);
    return e;
}

Triangle* MakeNullFace(){
    Triangle* f = new Triangle();
    for(int i = 0; i<3; i++){
        f->edge[i] = NULL;
        f->vertex[i] = NULL;
    }
    f->visible = !VISIBLE;
    ADD(faces, f);
    return f;
}

void PrintVertex(Vertex* v){
    cout<<v->vnum<<": ("<<v->v[X]<<", "<<v->v[Y]<<", "<<v->v[Z]<<")\n";
}

void PrintVertices(){
    Vertex* v = vertices;
    do{
        PrintVertex(v);
        v=v->next;
    }while(v!=vertices);
}

void ReadVertices(){
    Vertex* v; 
    int n = sizeof(pointSet)/sizeof(pointSet[0]);
    for(int i=0; i<n; i++){
        v=MakeNullVertex();
        v->v[X] = pointSet[i][X];
        v->v[Y] = pointSet[i][Y];
        v->v[Z] = pointSet[i][Z];
        v->vnum = i;
        // PrintVertex(v);
    }
}

bool Collinear(Vertex* a, Vertex* b, Vertex* c){
    return (((c->v[Y]-b->v[Y])*(b->v[Z]-a->v[Z]) - (c->v[Z]-b->v[Z])*(b->v[Y]-a->v[Y])== 0) && ((c->v[X]-b->v[X])*(b->v[Y]-a->v[Y]) - (b->v[X]-a->v[X])*(c->v[Y]-b->v[Y]) == 0) && ((c->v[X]-b->v[X])*(b->v[Z]-a->v[Z]) - (b->v[X]-a->v[X])*(c->v[Z]-b->v[Z]) == 0));
}

int VolumeSign(Triangle* f, Vertex* p){
    double ax, ay, az, bx, by, bz, cx, cy, cz;
    ax = f->vertex[0]->v[X]-p->v[X];
    ay = f->vertex[0]->v[Y]-p->v[Y];
    az = f->vertex[0]->v[Z]-p->v[Z];
    bx = f->vertex[1]->v[X]-p->v[X];
    by = f->vertex[1]->v[Y]-p->v[Y];
    bz = f->vertex[1]->v[Z]-p->v[Z];
    cx = f->vertex[2]->v[X]-p->v[X];
    cy = f->vertex[2]->v[Y]-p->v[Y];
    cz = f->vertex[2]->v[Z]-p->v[Z];
    double vol = ax*(by*cz-bz*cy)+ay*(bz*cx-bx*cz)+(az*(bx*cy-by*cx));
    // PrintVertex(f->vertex[0]); PrintVertex(f->vertex[1]); PrintVertex(f->vertex[2]);
    // PrintVertex(p);
    // cout<<vol<<"\n";
    if (vol>0.5) return 1;
    else if (vol<-0.5) return -1;
    else return 0;
}

Triangle* MakeFace(Vertex* v0, Vertex* v1, Vertex* v2, Triangle* fold){
    Triangle* f; Edge *e0, *e1, *e2;
    if(!fold){
        e0=MakeNullEdge();
        e1=MakeNullEdge();
        e2=MakeNullEdge();
    }
    else{
        e0=fold->edge[2];
        e1=fold->edge[1];
        e2=fold->edge[0];
    }
    e0->endPts[0]=v0; e0->endPts[1]=v1;
    e1->endPts[0]=v1; e1->endPts[1]=v2;
    e2->endPts[0]=v2; e2->endPts[1]=v0;
    f=MakeNullFace();
    f->edge[0]=e0; f->edge[1]=e1; f->edge[2]=e2;
    f->vertex[0]=v0; f->vertex[1]=v1; f->vertex[2]=v2;

    e0->adjFace[0] = e1->adjFace[0] = e2->adjFace[0] = f;

    return f;
}

void DoubleTriangle(){
    Vertex* v0, *v1, *v2, *v3, *t;
    Triangle* f0, *f1 = NULL;
    Edge* e0, e1, e2, s;
    v0 = vertices;
    // cout<<Collinear(v0,v0->next,v0->next->next);
    while(Collinear(v0, v0->next, v0->next->next))
    {
        if((v0=v0->next)==vertices){
            printf("Can't solve, all points collinear\n");
            return;
        }
    }
    v1=v0->next; v2=v1->next;
    v0->mark=PROCESSED; v1->mark=PROCESSED; v2->mark=PROCESSED;
    f0=MakeFace(v0,v1,v2,f1);
    f1=MakeFace(v2,v1,v0,f0);

    f0->edge[0]->adjFace[1] = f1;
    f0->edge[1]->adjFace[1] = f1;
    f0->edge[2]->adjFace[1] = f1;
    f1->edge[0]->adjFace[1] = f0;
    f1->edge[1]->adjFace[1] = f0;
    f1->edge[2]->adjFace[1] = f0;

    v3=v2->next;
    int vol=VolumeSign(f0, v3);
    while(!vol){
        if((v3=v3->next)==v0){
            printf("Can't solve, all points coplanar\n");
            return;
        }
        vol=VolumeSign(f0,v3);
    }
    // PrintVertex(v3);
    vertices = v3;
}

void MakeCcw(Triangle* f, Edge* e, Vertex* p){
    Triangle* fv; Edge* s; int i;
    if(e->adjFace[0]->visible)
        fv=e->adjFace[0];
    else 
        fv=e->adjFace[1];

    for(i=0; fv->vertex[i]!=e->endPts[0]; i++); 
    //i gives the index of vertex in fv that is equal to endPt[0] of edge e
    if(fv->vertex[(i+1)%3]!=e->endPts[1]){
        f->vertex[0]=e->endPts[1];
        f->vertex[1]=e->endPts[0];   
    }
    else{
        f->vertex[0]=e->endPts[0];
        f->vertex[1]=e->endPts[1];
        s=f->edge[1];
        f->edge[1]=f->edge[2];
        f->edge[2]=s;
    }
        f->vertex[2]=p;
}

Triangle* MakeConeFace(Edge* e, Vertex* p){
    Edge* new_edge[2];
    Triangle* new_face;
    for(int i=0; i<2; i++){
        if(e->endPts[i]->duplicate){
            new_edge[i]= e->endPts[i]->duplicate;
        }
        else{
            new_edge[i] = MakeNullEdge();
            new_edge[i]->endPts[0]=e->endPts[i];
            new_edge[i]->endPts[1]=p;
            e->endPts[i]->duplicate=new_edge[i];
        }
    }
    new_face=MakeNullFace();
    new_face->edge[0]=e;
    new_face->edge[1]=new_edge[0];
    new_face->edge[2]=new_edge[1];
    MakeCcw(new_face, e, p);
    for(int i=0; i<2; i++){
        for(int j=0; j<2; j++){
            if(!new_edge[i]->adjFace[j]){
                new_edge[i]->adjFace[j]=new_face;
                break;
            }
        }
    }

    return new_face;
}

bool AddOne(Vertex* p){
    Triangle* f;
    Edge* e, *temp;
    bool vis=false;
    f=faces;
    do{
        if(VolumeSign(f,p)<0){
            // cout<<"adding ";
            // PrintVertex(p);
            f->visible=VISIBLE;
            vis=true;
        }
        f=f->next;
    }while(f!=faces);

    if(!vis){
        p->onhull = !ONHULL;
        return false;
    }

    e=edges;
    do{
        temp=e->next;
        if(e->adjFace[0]->visible && e->adjFace[1]->visible)
            e->del = REMOVED;
        else if(e->adjFace[0]->visible||e->adjFace[1]->visible)
            e->newface=MakeConeFace(e,p);
        e=temp;
    }while(e!=edges);
    return true;
}

void CleanFaces(){
    Triangle* f, *t;
    while(faces && faces->visible){
        f=faces;
        DELETE(faces, f);
    }
    f=faces->next;
    do{
        if(f->visible){
            t=f;
            f=f->next;
            DELETE(faces, t);
        }
        else f=f->next;
    }while(f!=faces);
}

void CleanEdges(){
    Edge* e, *t;
    e=edges;
    do{
        if(e->newface){
            if(e->adjFace[0]->visible){
                e->adjFace[0]=e->newface;
            }
            else{
                e->adjFace[1]=e->newface;
            }
            e->newface=NULL;
        }
        e=e->next;
    }while(e!=edges);
    while(edges && edges->del){
        e=edges;
        DELETE(edges, e);
    }
    e=edges->next;
    do{
        if(e->del){
            t=e;
            e=e->next;
            DELETE(edges, t);
        }
        else e=e->next;
    }while(e!=edges);
}

void CleanVertices(){
    Edge* e; Vertex* v, *t;
    e=edges;
    do{
        e->endPts[0]->onhull=e->endPts[1]->onhull=ONHULL;
        e=e->next;
    }while(e!=edges);
    while(vertices&&vertices->mark&&!vertices->onhull){
        v=vertices;
        DELETE(vertices, v);
    }
    v=vertices->next;
    do{
        if(v->mark && !v->onhull){
            t=v;
            v=v->next;
            DELETE(vertices,t);
        }
        else v=v->next;
    }while(v!=vertices);
    v=vertices;
    do{
        v->duplicate=NULL;
        v->onhull=!ONHULL;
        v=v->next;
    }while(v!=vertices);
}

void CleanUp(){
    CleanEdges();
    CleanFaces();
    CleanVertices();
}

void PrintFace(Triangle* f){
    cout<<"Face: \n";
    PrintVertex(f->vertex[0]);
    PrintVertex(f->vertex[1]);
    PrintVertex(f->vertex[2]);
}

void PrintFaces(){
    Triangle *f = faces;
    do{
        PrintFace(f);
        f=f->next;
    }while(f!=faces);
}

void ConstructHull(){
    Vertex* v, *vnext;
    v=vertices;
    do{
        vnext=v->next;
        if(!v->mark){
            v->mark=PROCESSED;
            bool added = AddOne(v);
            // cout<<added<<" ";
            CleanUp();
        }
        v=vnext;
    }while(v!=vertices);
    // PrintFaces();
    // PrintVertices();
}

Vertex* Displacement(Vertex* a, Vertex* b){
    Vertex* disp = new Vertex();
    disp->v[X]=b->v[X]-a->v[X];
    disp->v[Y]=b->v[Y]-a->v[Y];
    disp->v[Z]=b->v[Z]-a->v[Z];
    return disp;
}

Vertex* FindNormal(Triangle* f){
    Vertex *unit_norm = new Vertex();
    Vertex* A = Displacement(f->vertex[0], f->vertex[1]);
    Vertex* B = Displacement(f->vertex[1], f->vertex[2]);
    unit_norm->v[X]=A->v[Y]*B->v[Z]-A->v[Z]*B->v[Y];
    unit_norm->v[Y]=-(A->v[X]*B->v[Z]-A->v[Z]*B->v[X]);
    unit_norm->v[Z]=A->v[X]*B->v[Y]-A->v[Y]*B->v[X];
    float magn = sqrt(pow(unit_norm->v[X],2)+pow(unit_norm->v[Y],2)+pow(unit_norm->v[Z],2));
    unit_norm->v[X]/=magn;
    unit_norm->v[Y]/=magn;
    unit_norm->v[Z]/=magn;
    return unit_norm;
}

void Print(){
    Triangle* f = faces;
    Vertex* normal;
    do{
        PrintFace(f);
        normal = FindNormal(f);
        // PrintVertex(normal);
        writeTriangleToSTL(normal, f, "hull.stl");
        f=f->next;
    }while(f!=faces);
}

int main(){
    ReadVertices();
    DoubleTriangle();
    ConstructHull();
    Print();
}