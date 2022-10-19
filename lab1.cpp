#include <GL/glut.h>
#include <cmath>

const int WINDOW_WIDTH = 1920;
const int WINDOW_HEIGHT = 360;

void initialize() {
    glClearColor(1, 1, 1, 1);
    glColor3f(.0f, .0f, .0f);
    glPointSize(2.0);
    glLineWidth(1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
}

void displayCallback() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Render Coordinates
    glBegin(GL_LINE_STRIP);
    glColor3f(.9f, .9f, .9f);
    glVertex2i(0, WINDOW_HEIGHT / 2);
    glVertex2i(WINDOW_WIDTH, WINDOW_HEIGHT / 2);
    glEnd();
    glBegin(GL_LINE_STRIP);
    glVertex2i(WINDOW_WIDTH / 2, 0);
    glVertex2i(WINDOW_WIDTH / 2, WINDOW_HEIGHT);
    glEnd();

    // Plotting
    glBegin(GL_LINE_STRIP);
    glColor3f(.0f, .0f, .0f);
    double scale_x  = 0.05;
    double scale_y  = 120;
    for (int i = 0; i < WINDOW_WIDTH; ++i) {
        double x = (i - WINDOW_WIDTH / 2.) * scale_x;
        double y = sin(x) / x;
        if (x < .1 && x > -.1) y = 1.;
        glVertex2i((int)i, (int)(WINDOW_HEIGHT / 2. + y * scale_y));
    }
    glEnd();

    glFlush();
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("[Plot] sin(x) / x");
    initialize();
    glutDisplayFunc(displayCallback);
    glutMainLoop();
    return 0;
}
