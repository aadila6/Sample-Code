/*
 
 * Abudureheman Adila

 */

#ifdef WIN32
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "tk.h"
#endif

#if defined(__APPLE__) || defined(MACOSX)
#include <OpenGL/gl.h>
//#include <OpenGL/glu.h>
#include <GLUT/glut.h>

#else //linux
#include <GL/gl.h>
#include <GL/glut.h>
#endif

//other includes
#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
//other files

typedef int OutCode;
constexpr int INSIDE = 0; // 0000
constexpr int LEFT = 1;   // 0001
constexpr int RIGHT = 2;  // 0010
constexpr int BOTTOM = 4; // 0100
constexpr int TOP = 8;    // 1000

constexpr int X = 0;
constexpr int Y = 1;
constexpr int Z = 2;

/****set in main()****/
//the number of pixels in the grid
char *inputFileName;
//int grid_width;
//int grid_height;
float grid_width;
float grid_height;

float xMin;
float xMax;
float yMin;
float yMax;
float zMin;
float zMax;

const double pi = 3.14159265358979323846;

//the size of pixels sets the inital window height and width
//don't make the pixels too large or the screen size will be larger than
//your display size
float pixel_size;

/*Window information*/
int win_height;
int win_width;

void init();
void idle();
void display();
void draw_pix(int x, int y);
void draw();
void reshape(int width, int height);
void key(unsigned char ch, int x, int y);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void check();

/* Coordate system for 3D world space*/
class Coord
{
public:
    float x, y, z;

public:
    Coord(){};
    Coord(float x, float y, float z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }
};

/* Coordinate System respect to the user view (2D) screen space */
class ECoord
{
public:
    int x, y;

public:
    ECoord(){};
    ECoord(int x, int y)
    {
        this->x = x;
        this->y = y;
    }
};

/* Polygon class with specification required upon creation  */
class Polygon
{
    //Center of mass is 0 respect to polygon itself
    //But position vector is the centroid from the viewPort
public:
    int count;
    std::vector<float> position;
    int edgeCount;
    Coord transVec;
    float angle;
    float scale;
    std::vector<std::vector<float>> vertices;
    std::vector<std::vector<int>> edges;

public:
    void printPolygon();   // for debug purposes
    void updateCentroid(); //Updating the centroid with respect to given frame
    Polygon(){};

    Polygon(std::vector<Coord> &vert, std::vector<ECoord> &edges)
    {
        float xtotal = 0, ytotal = 0;
        this->count = vert.size();
        this->edgeCount = edges.size();
        for (int i = 0; i < vert.size(); i++)
        {
            vertices.push_back(std::vector<float>{vert[i].x, vert[i].y, vert[i].z});
        }
        for (int j = 0; j < edges.size(); j++)
        {
            this->edges.push_back(std::vector<int>{edges[j].x, edges[j].y});
        }
        this->updateCentroid();

        transVec.x = 0.0;
        transVec.y = 0.0;
        transVec.z = 0.0;
        angle = 0.0;
        scale = 1.0;
    }
};

/* For debugging purpose printing the polygon  */
void Polygon::printPolygon()
{
    std::cout << "numVertices: " << this->count << std::endl;
    for (int i = 0; i < this->count; i++)
    {
        std::cout << "x: " << (this->vertices[i])[0] << " y: " << (this->vertices[i])[1] << " z: " << (this->vertices[i])[2] << std::endl;
    }

    std::cout << std::endl;
}
/* Updating the centroid where applied shifting or rotating or scalling  */
void Polygon::updateCentroid()
{
    float xtotal = 0, ytotal = 0, ztotal = 0;
    for (int i = 0; i < this->count; i++)
    {
        xtotal += this->vertices[i][0];
        ytotal += this->vertices[i][1];
        ztotal += this->vertices[i][2];
    }
    this->position = {xtotal / (float)(this->count), ytotal / (float)(this->count), ztotal / (float)(this->count)};
}

/* Global switches/var */
bool *loadBuffer;
char lineMode;
int iD;
float translationXG, translationYG, translationZG, rotX, rotY, rotZ, ratX, ratY, ratZ, angleG, scaleG;
bool begincheck;
std::vector<Polygon> polygonList;
std::vector<Polygon> cPolygonList;

void readinput(char *filename, std::vector<Polygon> &polygons);
void writeFile(char *filename, std::vector<Polygon> &polygons);
void drawLineDDA(std::vector<float> start, std::vector<float> end);
void drawLineBresenham(Coord start, Coord end);
void rasterization(Polygon &p);
void translation(Coord transl, Polygon &poly);
void rotation(float angle, Polygon &poly);
void scaling(float scal, Polygon &poly);
bool sortVert(const std::vector<float> &a, const std::vector<float> &b);
void polyClip(Polygon &poly);
void polyClipLeft(Polygon &poly);
void polyClipRight(Polygon &poly);
void polyClipBottom(Polygon &poly);
void polyClipTop(Polygon &poly);
void exchangeV(std::vector<float> &vA, std::vector<float> &vB);
void copyList(std::vector<std::vector<float>> &s, std::vector<std::vector<float>> &t, int n);
void copyVertex(std::vector<float> &s, std::vector<float> &t);

int main(int argc, char **argv)
{
    inputFileName = "bunny_scene.txt";
    pixel_size = 1;

    grid_width = 1.0f;  //500;
    grid_height = 1.0f; //500;

    xMin = 0;
    xMax = grid_width;
    yMin = 0;
    yMax = grid_height;

    lineMode = 't';
    iD = -1;
    translationXG = 0;
    translationYG = 0;
    translationZG = 0;
    rotX = 0;
    rotY = 0;
    rotZ = 0;
    ratX = 0;
    ratY = 0;
    ratZ = 0;
    angleG = 0;
    scaleG = 1;
    begincheck = true;
    win_height = 500; //grid_height * pixel_size;
    win_width = 500;  //grid_width * pixel_size;

    readinput(inputFileName, polygonList);

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    /*initialize variables, allocate memory, create buffers, etc. */
    //create window of size (win_width x win_height
    glutInitWindowSize(win_width, win_height);
    //windown title is "glut demo"
    glutCreateWindow("Three Dimentional Drawing");

    /*defined glut callback functions*/
    glutDisplayFunc(display); //rendering calls here
    glutReshapeFunc(reshape); //update GL on window size change
    glutMouseFunc(mouse);     //mouse button events
    glutMotionFunc(motion);   //mouse movement events
    glutKeyboardFunc(key);    //Keyboard events
    glutIdleFunc(idle);       //Function called while program is sitting "idle"

    //initialize opengl variables
    init();
    //start glut event loop
    glutMainLoop();
    return 0;
}

/*initialize gl stufff*/
void init()
{
    //set clear color (Default background to white)
    glClearColor(1.0, 1.0, 1.0, 1.0);
    //glLineWidth(1.0f);
    //checks for OpenGL errors
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, grid_width, 0, grid_height, -1, 1);
    check();
}

/*Reseting the constriains when needed*/
void resetConstrains()
{
    iD = -1;
    translationXG = 0;
    translationYG = 0;
    translationZG = 0;
    rotX = 0;
    rotY = 0;
    rotZ = 0;
    ratX = 0;
    ratY = 0;
    ratZ = 0;
    angleG = 0;
    scaleG = 1;
}

//called repeatedly when glut isn't doing anything else
void idle()
{
    //redraw the scene over and over again
    glutPostRedisplay();
    resetConstrains();
    int choice;
    lineMode = 't';
    std::cout << "1. Rotation \n";
    std::cout << "2. Translation\n";
    std::cout << "3. Scalling \n";
    std::cout << "4. Exit \n";
    std::cout << "Please select one of options above for your operation: ";
    std::cin >> choice;
    iD = 0;
    switch (choice)
    {
    case 1:
        std::cout << "Please enter rotation angle: ";
        std::cin >> angleG;
        std::cout << "Please enter rotation Axis (EX. 1 1 1 0 0 0): ";
        std::cin >> rotX >> rotY >> rotZ >> ratX >> ratY >> ratZ;
        // std::cout << "Please enter Polygon ID like 0 to n for specific ones: ";
        // std::cin >> iD;
        lineMode = 'r';
        break;
    case 2:
        std::cout << "Please enter translation vector seperated by a space (EX. 2 2 2): ";
        std::cin >> translationXG >> translationYG >> translationZG;
        // std::cout << "Please enter Polygon ID like 0,1,2,3.. for specific ones: ";
        // std::cin >> iD;
        lineMode = 't';
        break;
    case 3:
        std::cout << "Please enter scalling factor: ";
        std::cin >> scaleG;
        // std::cout << "Please enter Polygon ID like 0,1,2,3.. for specific ones: ";
        // std::cin >> iD;
        lineMode = 's';
        break;
    case 4:
        writeFile(inputFileName, polygonList);
        exit(0);
        break;
    default:
        break;
    }
}

void readinput(char *filename, std::vector<Polygon> &polygons)
{
    std::ifstream inputFile;
    inputFile.open(filename);
    std::string line;
    int count;
    int edgeCount;
    inputFile >> count;
    getline(inputFile, line); //line 1
    getline(inputFile, line); //point count
    for (int i = 0; i < count; i++)
    {
        int num;
        std::vector<Coord> vertices;
        std::vector<ECoord> edges;
        inputFile >> num;
        getline(inputFile, line);
        for (int j = 0; j < num; j++)
        {
            float x, y, z;
            std::string inputX, inputY, inputZ;
            getline(inputFile, line);
            std ::istringstream record(line);
            getline(record, inputX, ' ');
            getline(record, inputY, ' ');
            getline(record, inputZ);
            x = std::stof(inputX);
            y = std::stof(inputY);
            z = std::stof(inputZ);
            Coord point;
            point.x = x;
            point.y = y;
            point.z = z;
            vertices.push_back(point);
        }
        std ::string input;
        getline(inputFile, line);
        edgeCount = std::stoi(line);
        for (int j = 0; j < edgeCount; j++)
        {
            int x, y;
            std ::string inputX, inputY;
            getline(inputFile, line);
            std ::istringstream record(line);
            getline(record, inputX, ' ');
            getline(record, inputY);
            x = std::stoi(inputX);
            y = std::stoi(inputY);
            ECoord point;
            point.x = x;
            point.y = y;
            edges.push_back(point);
        }
        Polygon polygon(vertices, edges);
        polygons.push_back(polygon);
        getline(inputFile, line);
    }
    inputFile.close();
}


/* Writing to the resulted coordinates into a file 
   When user reopen the application, the previous information is kept
*/
void writeFile(char *filename, std::vector<Polygon> &polygons)
{
    std::ofstream outputFile(filename);
    outputFile << polygons.size() << "\n\n";
    for (int i = 0; i < polygons.size(); i++)
    {
        outputFile << polygons[i].count << std::endl;
        for (int j = 0; j < polygons[i].count; j++)
        {
            outputFile << polygons[i].vertices[j][0] << ' ' << polygons[i].vertices[j][1] << ' ' << polygons[i].vertices[j][2] << std::endl;
        }
        outputFile << polygons[i].edgeCount << std::endl;
        for (int k = 0; k < polygons[i].edgeCount; k++)
        {
            outputFile << polygons[i].edges[k][0] << ' ' << polygons[i].edges[k][1] << std::endl;
        }
        outputFile << std::endl;
    }
}

/*  */
bool sortVert(const std::vector<float> &a, const std::vector<float> &b)
{
    return (a[0] < b[0]);
}

typedef float Matrix4x4[4][4];
Matrix4x4 matComposite;
void matrix4x4SetIdentity(Matrix4x4 matIdent4x4)
{
    int row, col;
    for (row = 0; row < 4; row++)
        for (col = 0; col < 4; col++)
            matIdent4x4[row][col] = (row == col);
}

/* Premultiply matrix m1 by matrix m2, store result in m2. */
void matrix4x4PreMultiply(Matrix4x4 m1, Matrix4x4 m2)
{
    int row, col;
    Matrix4x4 matTemp;
    for (row = 0; row < 4; row++)
        for (col = 0; col < 4; col++)
            matTemp[row][col] = m1[row][0] * m2[0][col] + m1[row][1] * m2[1][col] + m1[row][2] * m2[2][col] +
                                m1[row][3] * m2[3][col];
    for (row = 0; row < 4; row++)
        for (col = 0; col < 4; col++)
            m2[row][col] = matTemp[row][col];
}
/*  Procedure for generating 3-D translation matrix.  */
void translate3D(float tx, float ty, float tz)
{
    Matrix4x4 matTransl3D;
    /*  Initialize translation matrix to identity.  */
    matrix4x4SetIdentity(matTransl3D);
    matTransl3D[0][3] = tx;
    matTransl3D[1][3] = ty;
    matTransl3D[2][3] = tz;
    /*  Concatenate matTransl3D with composite matrix.  */
    matrix4x4PreMultiply(matTransl3D, matComposite);
}

/*  Procedure for generating a quaternion rotation matrix.  */
void rotate3D(Coord p1, Coord p2, float angle)
{
    Matrix4x4 matQuatRot;
    float radianAngle = (angle / 180.0) * 3.14;
    float axisVectLength = sqrt((p2.x - p1.x) * (p2.x - p1.x) +
                                (p2.y - p1.y) * (p2.y - p1.y) +
                                (p2.z - p1.z) * (p2.z - p1.z));
    float cosA = cosf(radianAngle);
    float oneC = 1 - cosA;
    float sinA = sinf(radianAngle);
    float ux = (p2.x - p1.x) / axisVectLength;
    float uy = (p2.y - p1.y) / axisVectLength;
    float uz = (p2.z - p1.z) / axisVectLength;
    /*  Set up translation matrix for moving p1 to origin,
     *  and concatenate translation matrix with matComposite.
     */
    translate3D(-p1.x, -p1.y, -p1.z);
    /*  Initialize matQuatRot to identity matrix.  */
    matrix4x4SetIdentity(matQuatRot);
    matQuatRot[0][0] = ux * ux * oneC + cosA;
    matQuatRot[0][1] = ux * uy * oneC - uz * sinA;
    matQuatRot[0][2] = ux * uz * oneC + uy * sinA;
    matQuatRot[1][0] = uy * ux * oneC + uz * sinA;
    matQuatRot[1][1] = uy * uy * oneC + cosA;
    matQuatRot[1][2] = uy * uz * oneC - ux * sinA;
    matQuatRot[2][0] = uz * ux * oneC - uy * sinA;
    matQuatRot[2][1] = uz * uy * oneC + ux * sinA;
    matQuatRot[2][2] = uz * uz * oneC + cosA;
    /*  Concatenate matQuatRot with composite matrix.  */
    matrix4x4PreMultiply(matQuatRot, matComposite);
    /*  Construct inverse translation matrix for p1 and
     *  concatenate with composite matrix.
     */
    translate3D(p1.x, p1.y, p1.z);
}

/*  Procedure for generating a 3-D scaling matrix.  */
void scale3D(float sx, float sy, float sz, std::vector<float> fixedPt)
{
    Matrix4x4 matScale3D;
    /*  Initialize scaling matrix to identity.  */
    matrix4x4SetIdentity(matScale3D);
    matScale3D[0][0] = sx;
    matScale3D[0][3] = (1 - sx) * fixedPt[0] * grid_width;
    matScale3D[1][1] = sy;
    matScale3D[1][3] = (1 - sy) * fixedPt[1] * grid_width;
    matScale3D[2][2] = sz;
    matScale3D[2][3] = (1 - sz) * fixedPt[2] * grid_width;
    /*  Concatenate matScale3D with composite matrix.  */
    matrix4x4PreMultiply(matScale3D, matComposite);
}
void drawLine(float x1, float y1, float x2, float y2)
{
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

void drawSplitLines()
{
    glColor3f(0.41, 0.4, 0.4);
    glBegin(GL_LINES);
    glVertex2f(0.5f, 0.0f);
    glVertex2f(0.5f, 1.0f);
    glVertex2f(0.0f, 0.5f);
    glVertex2f(1.0f, 0.5f);
    glEnd();
}

//this is where we render the screen
void display()
{
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    //find xMax mins.
    drawSplitLines();
    cPolygonList.clear();
    for (int u = 0; u < polygonList.size(); u++)
    {
        cPolygonList.push_back(polygonList[u]);
    }
    xMax = 0;
    xMin = 99999;
    yMax = 0;
    yMin = 99999;
    zMax = 0;
    zMin = 99999;
    float deltaX = 0;
    float deltaY = 0;
    float deltaZ = 0;
    float delta = 0;
    Coord axisA = Coord(0, 0, 0);
    Coord axisB = Coord(0, 0, 0);
    for (int s = 0; s < cPolygonList.size(); s++)
    {
        matrix4x4SetIdentity(matComposite);

        if (s == iD)
        {
            if (lineMode == 't')
            {
                translate3D(translationXG, translationYG, translationZG);
            }
            else if (lineMode == 'r')
            {
                axisA = Coord(rotX, rotY, rotZ);
                axisB = Coord(ratX, ratY, ratZ);
                rotate3D(Coord(rotX, rotY, rotZ), Coord(ratX, ratY, ratZ), angleG);
            }
            else if (lineMode == 's')
            {
                scale3D(scaleG, scaleG, scaleG, cPolygonList[iD].position);
            }
        }
        for (int i = 0; i < polygonList[s].count; i++)
        {
            // v' = M * v
            cPolygonList[s].vertices[i][0] = matComposite[0][0] * polygonList[s].vertices[i][0] * grid_width + matComposite[0][1] * polygonList[s].vertices[i][1] * grid_width + matComposite[0][2] * polygonList[s].vertices[i][2] * grid_width + matComposite[0][3] * 1;
            cPolygonList[s].vertices[i][1] = matComposite[1][0] * polygonList[s].vertices[i][0] * grid_width + matComposite[1][1] * polygonList[s].vertices[i][1] * grid_width + matComposite[1][2] * polygonList[s].vertices[i][2] * grid_width + matComposite[1][3] * 1;
            cPolygonList[s].vertices[i][2] = matComposite[2][0] * polygonList[s].vertices[i][0] * grid_width + matComposite[2][1] * polygonList[s].vertices[i][1] * grid_width + matComposite[2][2] * polygonList[s].vertices[i][2] * grid_width + matComposite[2][3] * 1;

            if (cPolygonList[s].vertices[i][0] > xMax)
            {
                xMax = cPolygonList[s].vertices[i][0];
            }
            if (cPolygonList[s].vertices[i][0] < xMin)
            {
                xMin = cPolygonList[s].vertices[i][0];
            }

            if (cPolygonList[s].vertices[i][1] > yMax)
            {
                yMax = cPolygonList[s].vertices[i][1];
            }
            if (cPolygonList[s].vertices[i][1] < yMin)
            {
                yMin = cPolygonList[s].vertices[i][1];
            }

            if (cPolygonList[s].vertices[i][2] > zMax)
            {
                zMax = cPolygonList[s].vertices[i][2];
            }
            if (cPolygonList[s].vertices[i][2] < zMin)
            {
                zMin = cPolygonList[s].vertices[i][2];
            }
        }
    }

    xMin -= .1f;
    xMax += .1f;
    yMin -= .1f;
    yMax += .1f;
    zMin -= .1f;
    zMax += .1f;
    deltaX = xMax - xMin;
    deltaY = yMax - yMin;
    deltaZ = zMax - zMin;
    delta = fmax(fmax(deltaX, deltaY), deltaZ) + 0.1f;

    /*Bounding BOX where if the scene goes out of the scene, we can reset it back.*/
    if (xMin < 0 || xMax > 1 || yMin < 0 || yMax > 1 || zMin < 0 || zMax > 1)
    {
        for (int s = 0; s < cPolygonList.size(); s++)
        {
            for (int i = 0; i < cPolygonList[s].count; i++)
            {
                cPolygonList[s].vertices[i][0] = (cPolygonList[s].vertices[i][0] - xMin) / delta;
                cPolygonList[s].vertices[i][1] = (cPolygonList[s].vertices[i][1] - yMin) / delta;
                cPolygonList[s].vertices[i][2] = (cPolygonList[s].vertices[i][2] - zMin) / delta;
            }
        }
    }
    /* Drawing the axis with bounding box */
    if (lineMode == 'r')
    {
        axisA.x = (axisA.x - xMin) / delta;
        axisA.y = (axisA.y - yMin) / delta;
        axisA.z = (axisA.z - zMin) / delta;
        axisB.x = (axisB.x - xMin) / delta;
        axisB.y = (axisB.y - yMin) / delta;
        axisA.z = (axisA.z - zMin) / delta;
        drawLine(axisA.x * (.5), axisA.y * (.5), axisB.x * (.5), axisB.y * (.5));
        drawLine(axisA.x * (.5), axisA.z * (.5) + (.5), axisB.x * (.5), axisB.z * (.5) + (.5));
        drawLine(axisA.y * (.5) + (.5), axisA.z * (.5) + (.5), axisB.y * (.5) + (.5), axisB.z * (.5) + (.5));
    }

    for (int s = 0; s < cPolygonList.size(); s++)
    {
        for (int k = 0; k < cPolygonList[s].edgeCount; k++)
        {
            int a = cPolygonList[s].edges[k][0] - 1;
            int b = cPolygonList[s].edges[k][1] - 1;
            if ((cPolygonList[s].vertices[a][0] < grid_width) && (cPolygonList[s].vertices[b][0] < grid_width) &&
                (cPolygonList[s].vertices[b][1] < grid_height) && (cPolygonList[s].vertices[a][1] < grid_height))
            {
                drawLine(cPolygonList[s].vertices[a][0] * (.5),
                         cPolygonList[s].vertices[a][1] * (.5),
                         cPolygonList[s].vertices[b][0] * (.5),
                         cPolygonList[s].vertices[b][1] * (.5));
            }
            if (((cPolygonList[s].vertices[a][0]) < grid_width) && ((cPolygonList[s].vertices[b][0]) < grid_width) && ((cPolygonList[s].vertices[b][2]) < grid_height) && ((cPolygonList[s].vertices[a][2]) < grid_height))
            {
                drawLine(cPolygonList[s].vertices[a][0] * (.5),
                         cPolygonList[s].vertices[a][2] * (.5) + .5 * grid_width,
                         cPolygonList[s].vertices[b][0] * (.5),
                         cPolygonList[s].vertices[b][2] * (.5) + .5 * grid_width);
            }
            if ((cPolygonList[s].vertices[a][1] < grid_width) && (cPolygonList[s].vertices[b][1] < grid_width) &&
                ((cPolygonList[s].vertices[b][2]) < grid_height) && ((cPolygonList[s].vertices[a][2]) < grid_height))
            {
                drawLine(cPolygonList[s].vertices[a][1] * (.5) + .5 * grid_width,
                         cPolygonList[s].vertices[a][2] * (.5) + .5 * grid_width,
                         cPolygonList[s].vertices[b][1] * (.5) + .5 * grid_width,
                         cPolygonList[s].vertices[b][2] * (.5) + .5 * grid_width);
            }
        }
    }
    glutSwapBuffers();
    //checks for opengl errors
    check();
    polygonList.clear();
    for (int u = 0; u < cPolygonList.size(); u++)
    {
        cPolygonList[u].updateCentroid();
        polygonList.push_back(cPolygonList[u]);
    }
    writeFile(inputFileName, polygonList);
}

//Draws a single "pixel" given the current grid size
//don't change anything in this for project 1
void draw_pix(int x, int y)
{
    glBegin(GL_POINTS);
    glColor3f(0.41, 0.4, 0.4);
    glVertex3f(x + .5, y + .5, 0);
    glEnd();
}

/*Gets called when display size changes, including initial craetion of the display*/
void reshape(int width, int height)
{
    /*set up projection matrix to define the view port*/
    //update the ne window width and height
    win_width = width;
    win_height = height;

    //creates a rendering area across the window
    glViewport(0, 0, width, height);
    // up an orthogonal projection matrix so that
    // the pixel space is mapped to the grid space
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, grid_width, 0, grid_height, -10, 10);

    //clear the modelview matrix
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //set pixel size based on width, if the aspect ratio
    //changes this hack won't work as well
    pixel_size = width / (float)grid_width;

    //set pixel size relative to the grid cell size
    glPointSize(pixel_size);
    //check for opengl errors
    check();
}

//gets called when a key is pressed on the keyboard
void key(unsigned char ch, int x, int y)
{
    switch (ch)
    {
    case 'b':
        lineMode = 'b';
        break;

    case 'd':
        lineMode = 'd';
        break;

    default:
        //prints out which key the user hit
        printf("User hit the \"%c\" key\n", ch);
        break;
    }
    //redraw the scene after keyboard input
    glutPostRedisplay();
}

//gets called when a mouse button is pressed
void mouse(int button, int state, int x, int y)
{
    //print the pixel location, and the grid location
    printf("MOUSE AT PIXEL: %d %d, GRID: %d %d\n", x, y, (int)(x / pixel_size), (int)((win_height - y) / pixel_size));
    switch (button)
    {
    case GLUT_LEFT_BUTTON: //left button
        printf("LEFT ");
        break;
    case GLUT_RIGHT_BUTTON: //right button
        printf("RIGHT ");
    default:
        printf("UNKNOWN "); //any other mouse button
        break;
    }
    if (state != GLUT_DOWN) //button released
        printf("BUTTON UP\n");
    else
        printf("BUTTON DOWN\n"); //button clicked

    //redraw the scene after mouse click
    glutPostRedisplay();
}

//gets called when the curser moves accross the scene
void motion(int x, int y)
{
    //redraw the scene after mouse movement
    glutPostRedisplay();
}

//checks for any opengl errors in the previous calls and
//outputs if they are present
void check()
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        printf("GLERROR: There was an error %s\n", "error");
        exit(1);
    }
}
