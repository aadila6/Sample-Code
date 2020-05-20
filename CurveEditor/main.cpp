/*
 * Adila Abudureheman
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

#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl2.h"
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
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

static bool show_demo_window = true;
static bool show_another_window = false;
static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
/****set in main()****/
//the number of pixels in the grid
char *inputFileName;
float grid_width;
float grid_height;

//the size of pixels sets the inital window height and width
//don't make the pixels too large or the screen size will be larger than
//your display size
float pixel_size;

/*Window information*/
int WIN_HEIGHT = 500;
int WIN_WIDTH = 500;
//
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
void initGui(int argc, char **argv);
void my_display_code();
void glut_display_func();

/* Screen space Coordinates */
class Coord
{
public:
    float x, y;

public:
    Coord(){};
    Coord(float x, float y)
    {
        this->x = x;
        this->y = y;
    }
};

int mainWindow;
int guiWindow;
bool bezierMode;
bool bSplineMode;
bool addMode;
bool modifyMode;
bool deleteMode;
bool selectionMode;
int gRes;
int gKvalue;

/* Curve Definition with vector of coords */
class Curve
{
public:
    std::vector<Coord> vertices;
    float n;
    // std::vector<float> uValues;
    Coord centroid;
    Curve(std::vector<Coord> vert, float u)
    {
        this->n = u;
        for (int i = 0; i < vert.size(); i++)
        {
            this->vertices.push_back(vert[i]);
        }
    }

    void updateCentroid()
    {
        float xtotal = 0, ytotal = 0;
        int count = vertices.size();
        for (int i = 0; i < count; i++)
        {
            xtotal += this->vertices[i].x;
            ytotal += this->vertices[i].y;
        }
        this->centroid.x = xtotal / (float)(count);
        this->centroid.y = ytotal / (float)(count);
    }
};

// std::vector<Coord> clicked;
Curve *clicked = nullptr;
std::vector<float> uValues;
bool started;
char lineMode;
int gloT;
std::vector<Curve> CurveList;
int activeNumber;
int findNearest(std::vector<Coord> points, Coord cur);
void resetCurves();
int selectCurve(Coord cur);
int main(int argc, char **argv)
{
    // Initialization for global variables
    pixel_size = 1;
    grid_width = 1.0f;  //500;
    grid_height = 1.0f; //500;
    gloT = 0.0;
    gRes = 10;
    gKvalue = 3;
    bezierMode = true;
    bSplineMode = false;
    addMode = false;
    modifyMode = false;
    deleteMode = false;
    activeNumber = 0;
   
    resetCurves();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    /* initialize variables, allocate memory, create buffers, etc. */
    //create window of size (win_width x win_height)
    glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
    //windown title is "glut demo"
    mainWindow = glutCreateWindow("2D Curve Editor");
    //defined glut callback functions
    glutDisplayFunc(display); //rendering calls here
    glutMouseFunc(mouse);     //mouse button events
    glutMotionFunc(motion);   //mouse movement events
    glutReshapeFunc(reshape); //update GL on window size change
    glutKeyboardFunc(key);    //Keyboard events
    glutIdleFunc(idle);       //Function called while program is sitting "idle"
    //initialize opengl variables
    init();

#ifdef __FREEGLUT_EXT_H__
    glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
#endif
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_MULTISAMPLE);
    glutInitWindowSize(500, 500);
    guiWindow = glutCreateWindow("Gui Window");
    // Setup GLUT display function
    // We will also call ImGui_ImplGLUT_InstallFuncs() to get all the other functions installed for us,
    // otherwise it is possible to install our own functions and call the imgui_impl_glut.h functions ourselves.
    glutDisplayFunc(glut_display_func);
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGLUT_Init();
    ImGui_ImplGLUT_InstallFuncs();
    ImGui_ImplOpenGL2_Init();

    //start glut event loop
    glutMainLoop();
    return 0;
}

/*initialize gl stufff*/
void init()
{
    //set clear color (Default background to white)
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glLineWidth(2.0f);
    //checks for OpenGL errors
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, grid_width, 0, grid_height, -1, 1);
    check();
}
void resetCurves()
{
    CurveList.clear();
    CurveList.push_back(Curve(std::vector<Coord>(), gloT));
    clicked = &CurveList.back();
    activeNumber = 0;
}

void my_display_code()
{
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    // if (show_demo_window)
    //     ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        static int f = 0;
        static int f2 = 0;
        static int counter = 0;

        ImGui::Begin("2D Curve Editing Gui"); // Create a window called "Hello, world!" and append into it.

        ImGui::Text("Please select one of below to operate"); // Display some text (you can use a format strings too)
        // ImGui::Checkbox("Bezir", &bezierMode);      // Edit bools storing our window open/close state
        static int c = 0;
        ImGui::RadioButton("Bezier", &c, 0);
        ImGui::RadioButton("B-Spline", &c, 1);
        if (c == 0)
        {
            // resetCurves();
            bSplineMode = false;
            bezierMode = true;
        }
        if (c == 1)
        {
            // resetCurves();
            bezierMode = false;
            bSplineMode = true;
        }
        if (c == 0)
        {
            if (ImGui::SliderInt("n", &gloT, 0, 80)) // Edit 1 float using a slider from 0.0f to 1.0f
            {
                glutSetWindow(mainWindow);
                glutPostRedisplay();
                glutSetWindow(guiWindow);
                
            }
        }
        else if (c == 1)
        {
            //gui slider for different u values.
            if(ImGui::SliderInt("Resolution", &gRes, 0, 40)){
                glutSetWindow(mainWindow);
                glutPostRedisplay();
                glutSetWindow(guiWindow);
            }
            if(ImGui::SliderInt("K", &gKvalue, 0, 10)){
                glutSetWindow(mainWindow);
                glutPostRedisplay();
                glutSetWindow(guiWindow);
            }
            
        }

        static int e = 0;
        ImGui::RadioButton("Add", &e, 0);
        ImGui::SameLine();
        ImGui::RadioButton("Remove", &e, 1);
        ImGui::SameLine();
        ImGui::RadioButton("Modify", &e, 2);
        if (e == 0)
        {
            addMode = true;
            deleteMode = false;
            modifyMode = false;
        }
        if (e == 1)
        {
            addMode = false;
            deleteMode = true;
            modifyMode = false;
        }
        if (e == 2)
        {
            addMode = false;
            deleteMode = false;
            modifyMode = true;
        }
        if (ImGui::Button("New Curve"))
        {
            CurveList.push_back(Curve(std::vector<Coord>(), gloT));
            clicked = &CurveList.back();
            activeNumber = CurveList.size();

            glutSetWindow(mainWindow);
            glutPostRedisplay();
            glutSetWindow(guiWindow);
        }

         if (ImGui::Button("Delete"))
        {
            CurveList.erase(CurveList.begin()+activeNumber);
            clicked = &CurveList[0];
            activeNumber = 0;
            glutSetWindow(mainWindow);
            glutPostRedisplay();
            glutSetWindow(guiWindow);
        }
        
        // ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
        if (ImGui::Button("Clear All"))
        {
            // clicked.clear();
            resetCurves();
            glutSetWindow(mainWindow);
            glutPostRedisplay();
            glutSetWindow(guiWindow);
        } // Buttons return true when clicked (most widgets return true when edited/activated)

        ImGui::End();
    }
}

void glut_display_func()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplGLUT_NewFrame();

    my_display_code();
    // Rendering
    ImGui::Render();
    ImGuiIO &io = ImGui::GetIO();
    glViewport(0, 0, (GLsizei)io.DisplaySize.x, (GLsizei)io.DisplaySize.y);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound, but prefer using the GL3+ code.
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    glutSwapBuffers();
    glutPostRedisplay();
}

//called repeatedly when glut isn't doing anything else
void idle()
{
    //redraw the scene over and over again
    glutPostRedisplay();
}

//Bezier
Coord deCasteljau(std::vector<Coord> B, float u)
{
    Coord q[B.size()];
    int i, k;
    for (i = 0; i < B.size(); ++i)
    {
        q[i].x = B[i].x;
        q[i].y = B[i].y;
    }
    for (k = 1; k < B.size(); ++k)
    {
        for (i = 0; i < (B.size() - k); ++i)
        {
            q[i].x = (1.0 - u) * q[i].x + u * q[i + 1].x;
            q[i].y = (1.0 - u) * q[i].y + u * q[i + 1].y;
        }
    }
    return q[0];
}

int computeSegIndex(float ubar, std::vector<float> knotVector)
{

    for (int i = 0; i < knotVector.size(); i++)
    {
        if (ubar == knotVector[i])
        {
            return i;
        }
        else if (ubar < knotVector[i])
        {
            return i - 1;
        }
    }
}

std::vector<Coord> deBoor(std::vector<Coord> points)
{
    std::vector<float> knotVector;
    float t, uL, uR, uBar, ddiff;
    int k, n, I, temps;
    k = gKvalue;
    std::vector<Coord> temp;
    Coord p1, p2, p;
    Coord dL[points.size()];
    for (int i = 0; i < points.size(); ++i)
    {
        dL[i].x = points[i].x;
        dL[i].y = points[i].y;
    }
    n = points.size() - 1;
    for (int y = 0; y <= (n + k); y++)
    {
        knotVector.push_back(float(y));
    }

    ddiff = knotVector[n + 1] - knotVector[k - 1];
    temp.clear();

    for (int res = 0; res < gRes; res++)
    {
        t = float(res) / gRes;
        uBar = (ddiff * t) + knotVector[k - 1];

        I = computeSegIndex(uBar, knotVector);
        for (int j = 1; j <= k - 1; j++)
        {
            for (int i = I - (k - 1); i <= I - j; i++)
            {
                uL = knotVector[i + j];
                uR = knotVector[i + k];
                float diff = uR - uL;
                if (diff == 0)
                {
                    std::cout << "diff is 0!!!!!!!!!!!!!!!!!!!!!!" << std::endl;
                }
                float one = float(uR - uBar) / diff;
                float two = float(uBar - uL) / diff;
                dL[i].x = one * dL[i].x + two * dL[i + 1].x;
                dL[i].y = one * dL[i].y + two * dL[i + 1].y;
                // std::cout << "Debug Value x and Y :" << dL[i].x << "  " << dL[i].y << std::endl;
            }
        }
        p = dL[I - (k - 1)]; //I-k-1=0
        temp.push_back(p);
    }
    return temp;
}

void drawLine(float x1, float y1, float x2, float y2)
{
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}
void drawLineC(float x1, float y1, float x2, float y2)
{
    glColor3f(0.38, 0.89, .90);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}
void drawLineB(float x1, float y1, float x2, float y2)
{
    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}
void drawLineOrange(float x1, float y1, float x2, float y2)
{
    glColor3f(1.0, 0.5, 0.5);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

void drawLineActive(float x1, float y1, float x2, float y2)
{
    glColor3f(1.0, 0.41, 0.74);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}
void printOut(std::vector<Coord> B)
{
    for (int i = 0; i < B.size(); i++)
    {
        std::cout << "Debug printing: " << B[i].x << "  " << B[i].y << std::endl;
    }
}


//this is where we render the screen
void display()
{
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    std::cout<<"Total Number Of Curve: " <<CurveList.size()<<std::endl;;

    if (bezierMode)
    {
        std::vector<Coord> B;
        float step, iq;
        step = 1.0f / gloT;
        
        float id;
        if (CurveList.size() > 0)
        {
            for (int j = 0; j < CurveList.size(); j++)
            {
                //if (CurveList[j].vertices.size() > 0)
                {
                    id = CurveList[j].n;
                    // std::cout << "id : " << id << std::endl;
                    float steps = 1.0f / id;
                    std::vector<Coord> D;
                    for (iq = 0.0f; iq <= 1.0; iq += steps)
                    {
                        D.push_back(deCasteljau(CurveList[j].vertices, iq));
                    }
                    // std::cout<<"D size : "<<D.size()<<std::end;
                    for (int i = 1; i < CurveList[j].vertices.size(); i++)
                    {
                        Coord vA = CurveList[j].vertices[i - 1];
                        Coord vB = CurveList[j].vertices[i];
                        drawLine(vA.x / grid_width, vA.y / grid_width, vB.x / grid_width, vB.y / grid_width);
                    }
                    if (D.size() > 0)
                    {
                        for (int i = 1; i < D.size(); i++)
                        {
                            Coord cA = D[i - 1];
                            Coord cB = D[i];
                            drawLineC(cA.x / grid_width, cA.y / grid_width, cB.x / grid_width, cB.y / grid_width);
                        }
                    }
                    D.clear();
                }
            }
        }
        if (clicked->vertices.size() > 1)
        {
            for (iq = 0.0f; iq <= 1.0; iq += step)
            {
                B.push_back(deCasteljau(clicked->vertices, iq));
            }
            for (int i = 1; i < clicked->vertices.size(); i++)
            {
                Coord vA = clicked->vertices[i - 1];
                Coord vB = clicked->vertices[i];
                drawLineActive(vA.x / grid_width, vA.y / grid_width, vB.x / grid_width, vB.y / grid_width);
            }
        }
        if (B.size() > 1)
        {
            for (int i = 1; i < B.size(); i++)
            {
                Coord vA = B[i - 1];
                Coord vB = B[i];
                drawLineC(vA.x / grid_width, vA.y / grid_width, vB.x / grid_width, vB.y / grid_width);
            }
        }
        B.clear();
    }

    if (bSplineMode)
    {
        
        float id;
        if (CurveList.size() > 0)
        {
            std::vector<Coord> bspline;
            for (int j = 0; j < CurveList.size(); j++)
            {
                bspline = deBoor(CurveList[j].vertices);
                for (int i = 1; i < CurveList[j].vertices.size(); i++)
                {
                    Coord vA = CurveList[j].vertices[i - 1];
                    Coord vB = CurveList[j].vertices[i];
                    drawLine(vA.x / grid_width, vA.y / grid_width, vB.x / grid_width, vB.y / grid_width);
                }
                if (bspline.size() > 1)
                {
                    for (int i = 1; i < bspline.size(); i++)
                    {
                        Coord vA = bspline[i - 1];
                        Coord vB = bspline[i];
                        drawLineOrange(vA.x / grid_width, vA.y / grid_width, vB.x / grid_width, vB.y / grid_width);
                    }
                }
                bspline.clear();
            }

        }
        std::vector<Coord> spline;
        
        spline = deBoor(clicked->vertices);
        if (clicked->vertices.size() > 1)
        {
            for (int i = 1; i < clicked->vertices.size(); i++)
            {
                Coord vA = clicked->vertices[i - 1];
                Coord vB = clicked->vertices[i];
                drawLineActive(vA.x / grid_width, vA.y / grid_width, vB.x / grid_width, vB.y / grid_width);
            }
            if (spline.size() > 1)
            {
                for (int i = 1; i < spline.size(); i++)
                {
                    Coord vA = spline[i - 1];
                    Coord vB = spline[i];
                    drawLineC(vA.x / grid_width, vA.y / grid_width, vB.x / grid_width, vB.y / grid_width);
                }
            }
        }
        spline.clear();
    }
    glutSwapBuffers();
    check();
}

//Draws a single "pixel" given the current grid size

void draw_pix(int x, int y)
{
    glBegin(GL_POINTS);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(x + .5, y + .5, 0);
    glEnd();
}

//Gets called when a key is pressed on the keyboard
void key(unsigned char ch, int x, int y)
{
    switch (ch)
    {
    case ' ':
        if (activeNumber < CurveList.size()-1){
            activeNumber++;
            clicked = &CurveList[activeNumber];}
        else{
            activeNumber = 0;
            clicked = &CurveList[activeNumber];
        }
        // *ptr = CurveList[activeNumber];
        std::cout << "Changed ptr to curve number " << activeNumber << std::endl;
        glutPostRedisplay();
        break;
    default:
        //prints out which key the user hit
        printf("User hit the \"%c\" key\n", ch);
        break;
    }
    //redraw the scene after keyboard input
    glutPostRedisplay();
}

void reshape(int width, int height)
{
    /*set up projection matrix to define the view port*/
    //update the ne window width and height
    WIN_WIDTH = width;
    WIN_HEIGHT = height;

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

void mouse(int button, int state, int x, int y)
{
   
    float normx = (float)x / WIN_WIDTH;
    float normy = (WIN_HEIGHT - (float)y) / WIN_HEIGHT;
    int number = 0;
    Coord newPoint(normx, normy);
    if (addMode)
    {
        clicked->n = gloT;
        if (clicked->vertices.size() == 0)
        {
            clicked->vertices.push_back(newPoint);
        }
        else if (clicked->vertices.back().x != newPoint.x && clicked->vertices.back().y != newPoint.y)
        {
            clicked->vertices.push_back(newPoint);
        }
    }
    if (deleteMode)
    {
        clicked->n = gloT;
        //find the line & delete
        if(clicked->vertices.size()>0){
            int index = findNearest(clicked->vertices, newPoint);
            std::cout << "Deleted " << clicked->vertices[index].x << " " << clicked->vertices[index].y << std::endl;
            clicked->vertices.erase(clicked->vertices.begin() + index);
        }else{
            std::cout<<"No more to delete, press space to change other curves."<<std::endl;
        }
        
    }
    if (modifyMode)
    {
        clicked->n = gloT;
        //Function finds the nearest vertex
        int index = findNearest(clicked->vertices, newPoint);
        
            clicked->vertices[index].x = newPoint.x;
            clicked->vertices[index].y = newPoint.y;
        
    }

    switch (button)
    {
    case GLUT_LEFT_BUTTON: //left button
        printf("LEFT ");
        break;
    case GLUT_RIGHT_BUTTON: //right button
        printf("RIGHT ");
        //find the curve and return int number +store that to current curve and modify.
        break;
    default:
        printf("UNKNOWN "); //any other mouse button
        break;
    }
    if (state != GLUT_DOWN) //button released
        printf("BUTTON UP\n");
    else
        printf("BUTTON DOWN\n"); //button clicked

    glutPostRedisplay();
}
int findNearest(std::vector<Coord> points, Coord cur)
{
    float distance = 999;
    int min = 0;
    float tempdis;
    for (int i = 0; i < points.size(); i++)
    {
        tempdis = sqrt(pow((points[i].x - cur.x), 2) + pow((points[i].y - cur.y), 2));
        if (tempdis < distance)
        {
            distance = tempdis;
            min = i;
        }
    }
    return min;
}

//gets called when the curser moves accross the scene
void motion(int x, int y)
{
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
