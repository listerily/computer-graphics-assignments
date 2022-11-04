#include <GL/glut.h>
#include <cstdio>

const int WINDOW_WIDTH = 1080;
const int WINDOW_HEIGHT = 720;
int windowHandle;
int numVertices, numFaces, numNormals;

float** vertices = nullptr;
int** faces = nullptr;
float** normals = nullptr;

const char* FILE_NAME = "../lab9.txt";

void initialize() {
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
}

void readData() {
    FILE* file = fopen(FILE_NAME, "r");
    if (file == nullptr) {
        perror("Failed to open mesh file");
        exit(0);
    }
    fscanf(file, "%d %d %d", &numVertices, &numNormals, &numFaces);
    vertices = new float*[numVertices];
    for (unsigned i = 0; i < numVertices; ++i) {
        vertices[i] = new float[3];
        fscanf(file, "%f %f %f", vertices[i] + 0, vertices[i] + 1, vertices[i] + 2);
    }
    normals = new float*[numNormals];
    for (unsigned i = 0; i < numNormals; ++i) {
        normals[i] = new float[3];
        fscanf(file, "%f %f %f", normals[i] + 0, normals[i] + 1, normals[i] + 2);
    }
    faces = new int*[numFaces];
    for (unsigned i = 0; i < numFaces; ++i) {
        int faceVertices;
        fscanf(file, "%d", &faceVertices);
        faces[i] = new int[faceVertices * 2 + 1];
        faces[i][0] = faceVertices;
        for (unsigned j = 0; j < faceVertices * 2; ++j) {
            fscanf(file, "%d", faces[i] + j + 1);
        }
    }
    fclose(file);
}

void timerCallback(int value) {
    glutSwapBuffers();
    glutPostRedisplay();
    glutTimerFunc(50, timerCallback, 0);
}

void displayCallback() {
    static float angle = 10.f;
    angle += 0.2f;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, -7.0f);
    glRotatef(angle, 1.0f, 1.0f, 1.0f);
    // draw
    glColor3f(1, .94, 0);
    for (unsigned i = 0; i < 3; ++i) {
        glBegin(GL_LINE_STRIP);
        glVertex3f(-100.f * (i == 0 ? 1.f : 0), -100.f * (i == 1 ? 1.f : 0), -100.f * (i == 2 ? 1.f : 0));
        glVertex3f(100.f * (i == 0 ? 1.f : 0), 100.f * (i == 1 ? 1.f : 0), 100.f * (i == 2 ? 1.f : 0));
        glEnd();
    }

    // draw loaded shape
    glColor3f(1.f, 1.f, 1.f);
    for (unsigned i = 0; i < numFaces; ++i) {
        int verticesCount = faces[i][0];
        glBegin(GL_LINE_STRIP);
        for (unsigned j = 0; j < verticesCount; ++j) {
            glVertex3f(vertices[faces[i][j + 1]][0], vertices[faces[i][j + 1]][1], vertices[faces[i][j + 1]][2]);
        }
        glVertex3f(vertices[faces[i][1]][0], vertices[faces[i][1]][1], vertices[faces[i][1]][2]);
        glEnd();
    }
}

void reshapeCallback(GLsizei width, GLsizei height) {
    if (height == 0)
        height = 1;
    GLfloat aspect = (GLfloat)width / (GLfloat)height;

    glViewport(0, 0, width, height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, aspect, 0.1f, 100.0f);
}

int main(int argc, char** argv) {
    readData();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    windowHandle = glutCreateWindow("Meshes Stored in Files");
    initialize();
    glutDisplayFunc(displayCallback);
    glutReshapeFunc(&reshapeCallback);
    glutTimerFunc(50, timerCallback, 0);
    glutMainLoop();
    return 0;
}