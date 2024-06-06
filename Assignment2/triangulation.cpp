#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <chrono>

using namespace std;

struct Point {
    float x, y;
    Point(){}
    Point(float a, float b){
        x=a;
        y=b;
    }
};

struct Vertex {
    bool ear;
    struct Point v;
    Vertex *previous, *next;
    Vertex(){}
    Vertex(float a, float b){
        v.x = a;
        v.y = b; 
    }
};

// void printPoint(Vertex* v1){
//     cout << "(" << v1->v.x << ", "<< v1->v.y <<")\n";
//     return;
// }

void savePointsToFile(vector<Vertex*> points, string fileName){
    fstream PointsFile;
    PointsFile.open(fileName, ios::app);
    for(Vertex* vertex : points){
        PointsFile << vertex->v.x << ", "<< vertex->v.y << "\n";
    }
    PointsFile.close();
    return;
}

void addVertex(Vertex **head,float x,float y){
    Vertex* newPt;
    newPt = new Vertex();
    newPt->v.x = x;
    newPt->v.y = y;
    if(*head==NULL){
        newPt->next = newPt;
        newPt->previous = newPt;
        *head = newPt;
        return;
    }
    Vertex* last = (*head)->previous;
    last->next = newPt;
    newPt->previous = last;
    newPt->next = *head;
    (*head)->previous = newPt;
    return;
}

//cross product of vectors (P1-O) and (P2-O)
float cross(struct Point o, struct Point p1, struct Point p2){
    return ((p1.x-o.x)*(p2.y-o.y)-(p2.x-o.x)*(p1.y-o.y));
}

//is P on the left of line AB
bool left_on(struct Point p, struct Point a, struct Point b){
    return (cross(a,b,p)>=0);
}

// bool collinear(Point p, Point a, Point b){
//     return (cross(a,b,p)==0);
// }

bool pointsEqual(Point a, Point b){
    return ((a.x==b.x)&&(a.y==b.y));
}

//is V in the triangle ABC
bool in_triangle(struct Point v, struct Point a, struct Point b, struct Point c){
    return(left_on(v,a,b)&&left_on(v,b,c)&&left_on(v,c,a));
}

//check if diagonal ab in ear apb intersects any polygon edge
bool edge_intersect(Vertex **head, Point p, Point a, Point b){
    Vertex *currPt = *head;
    do{
        if(!(pointsEqual(currPt->v,p) || pointsEqual(currPt->v,a) || pointsEqual(currPt->v,b)) && in_triangle(currPt->v, a, b, p))
            return false;
        currPt = currPt->next;
    }while(currPt!=*head);
    return true;
}

bool isEar(Vertex *currPt, Vertex **head){
    Vertex *p = currPt->previous; Vertex *n = currPt->next;
    return(left_on(currPt->v, p->v, n->v) && edge_intersect(head, currPt->v, p->v, n->v));
}

void initialiseEars(Vertex **head){
    Vertex *currPt=*head; 
    do{
        currPt->ear = isEar(currPt, head);
        currPt = currPt->next;
    }
    while(currPt!=*head);
    return;
}

void triangulate(Vertex **head, int numOfVertices){
// take three consecutive vertices
// check if they form a diagonal (i.e, is it an ear) : it's a diagonal if it lies on the left (if traversing cw)
// and no vertex lies in the triangle formed by the three vertices
// if ear, mark as ear

//iterate through the points, check if ear
//if ear, clip it off (i.e, update the linked list and add diagonal to file)
//note: for checking if diagonal intersects w any of the edges, use the linked list (set of points) as that's what we're updating
//if not ear, go to next 
    Vertex *v1, *v2, *v3;
    int n = numOfVertices;
    initialiseEars(head);
    while(n>3){
        v2=*head;
        do{
            if(v2->ear){
                v3 = v2->next;
                v1 = v2->previous;
                savePointsToFile({v1, v3}, "diagonals.csv");

                //clipping off ear:
                v1->next = v3;
                v3->previous = v1;
                *head = v3;
                n--;

                //update ear status of diagonal endpoints
                v1->ear = isEar(v1, head);
                v3->ear = isEar(v3, head);
            
                break;
            }            
            v2=v2->next;
        }while(v2!=*head);
    }
    return;
}

int main()
{
    fstream ClearFile;
    ClearFile.open("points.csv", ios::out);
    ClearFile.close();
    ClearFile.open("diagonals.csv", ios::out);
    ClearFile.close();
    float vertices[][2] = {{1,2},{3.5,1},{4,4},{5,6},{6,3},{11,4},{8,0},{7.5,-2},{6,-4},{3,-2},{2.5,-7},{1.5,-3}};
    int n = sizeof(vertices)/sizeof(vertices[0]);
    Vertex *start = NULL;
    vector<Vertex*> vertices_vector;
    for(int i = 0; i<n; i++){
        addVertex(&start, vertices[i][0], vertices[i][1]);
        vertices_vector.push_back(new Vertex(vertices[i][0], vertices[i][1]));
    }
    savePointsToFile(vertices_vector, "points.csv");
    //uncomment the below lines to measure time
    // std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    triangulate(&start, n);
    // std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    // float duration = std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
    // savePointsToFile({new Vertex(float(n),duration)}, "time.csv");
    system("py plotter.py");
    //for this to plot, python should be in the environment variables
}
