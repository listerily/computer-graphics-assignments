#include <GL/glut.h>
#include <cmath>
#include <list>
#include <vector>
#include <random>
#include <chrono>

const int WINDOW_WIDTH = 1080;
const int WINDOW_HEIGHT = 720;

std::vector<std::pair<int, int>> polygon1;
std::vector<std::pair<int, int>> polygon2;
enum class PainterMode {
    PLAYING, PAUSED, POLY1, POLY2
} painterMode = PainterMode::POLY1;
int windowHandle;

void setPainterMode(PainterMode newMode) {
    painterMode = newMode;
    switch (painterMode) {
        case PainterMode::POLY1:
            glutSetWindowTitle("Ploy Line Painter [Mode: Draw Polygon 1]");
            break;
        case PainterMode::POLY2:
            glutSetWindowTitle("Ploy Line Painter [Mode: Draw Polygon 2]");
            break;
        case PainterMode::PLAYING:
            glutSetWindowTitle("Ploy Line Painter [Playing]");
            break;
        case PainterMode::PAUSED:
            glutSetWindowTitle("Ploy Line Painter [Paused]");
            break;
    }
}

void initialize() {
    glClearColor(1, 1, 1, 1);
    glColor3f(.0f, .0f, .0f);
    glPointSize(5.0);
    glLineWidth(2.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT);
}


void mouseCallback(int button, int state, int cursor_x, int cursor_y) {
    cursor_y = WINDOW_HEIGHT - cursor_y;
    if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
        if (painterMode == PainterMode::POLY1) {
            polygon1.emplace_back(cursor_x, cursor_y);
        } else if (painterMode == PainterMode::POLY2) {
            polygon2.emplace_back(cursor_x, cursor_y);
            if (polygon2.size() >= polygon1.size()) {
                setPainterMode(PainterMode::PLAYING);
            }
        }
    }
}

void keyboardCallback(unsigned char key, int cursor_x, int cursor_y) {
    cursor_y = WINDOW_HEIGHT - cursor_y;
    if (key == 'q' || key == 'Q') {
        glutDestroyWindow(windowHandle);
        exit(0);
    } else if (key == ' ') {
        if (painterMode != PainterMode::PLAYING) {
            setPainterMode(PainterMode::PLAYING);
        } else {
            setPainterMode(PainterMode::PAUSED);
        }
    } else if (key == '1') {
        setPainterMode(PainterMode::POLY1);
        polygon1.clear();
        polygon2.clear();
    } else if (key == '2') {
        setPainterMode(PainterMode::POLY2);
        polygon2.clear();
    } else if (key == 'r' || key == 'R') {
        polygon1.clear();
        polygon2.clear();
        setPainterMode(PainterMode::POLY1);
    }
}

void displayCallback() {
    static int accumulator = 0;
    static int flag = 0;
    static const int period = 100;
    static const int step = 1;
    if (painterMode == PainterMode::PLAYING) {
        accumulator = (accumulator + step) % period;
        if (accumulator == 0) flag = 1 - flag;
    }

    glClear(GL_COLOR_BUFFER_BIT);
    // Render animation
    if (polygon1.size() == polygon2.size() && (painterMode == PainterMode::PLAYING || painterMode == PainterMode::PAUSED) && !polygon1.empty()) {
        size_t size = polygon1.size();
        double progression = 1.0 * accumulator / period;
        if (flag) progression = 1 - progression;

        glBegin(GL_LINE_STRIP);
        glColor3f(1, 0.5, 0.5);
        int start_x, start_y;
        for (int i = 0; i < size; ++i) {
            int sx = polygon1[i].first, dx = polygon2[i].first;
            int sy = polygon1[i].second, dy = polygon2[i].second;
            int x = sx + static_cast<int>((dx - sx) * progression);
            int y = sy + static_cast<int>((dy - sy) * progression);
            glVertex2i(x, y);
            if (i == 0) {
                start_x = x, start_y = y;
            } else if (i == size - 1 && size > 2) {
                glVertex2i(start_x, start_y);
            }
        }
        glEnd();
    }

    // Render 2 polygons
    for (const auto& polygon : {polygon1, polygon2}) {
        if (polygon.size() == 1) {
            glBegin(GL_POINTS);
            glVertex2i(polygon[0].first, polygon[0].second);
            glEnd();
        } else if (!polygon.empty()) {
            glBegin(GL_LINE_STRIP);
            glColor3f(0, 0, 0);
            for (const auto & point: polygon) {
                glVertex2i(point.first, point.second);
            }
            glEnd();
        }
    }

    if (polygon1.size() > 2) {
        glBegin(GL_LINE_STRIP);
        glColor3f(0, painterMode == PainterMode::POLY1 ? 1 : 0, 0);
        glVertex2i(polygon1[0].first, polygon1[0].second);
        glVertex2i(polygon1[polygon1.size() - 1].first, polygon1[polygon1.size() - 1].second);
        glEnd();
    }
    if (polygon2.size() > 2) {
        glBegin(GL_LINE_STRIP);
        glColor3f(0, painterMode == PainterMode::POLY2 ? 1 : 0, 0);
        glVertex2i(polygon2[0].first, polygon2[0].second);
        glVertex2i(polygon2[polygon2.size() - 1].first, polygon2[polygon2.size() - 1].second);
        glEnd();
    }

    glutSwapBuffers();
}

void timerCallback(int value) {
    glutSwapBuffers();
    glutPostRedisplay();
    glutTimerFunc(50, timerCallback, 0);
}


int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    windowHandle = glutCreateWindow("Cohen Sutherland Clipping");
    initialize();
    setPainterMode(PainterMode::POLY1);
    glutMouseFunc(mouseCallback);
    glutKeyboardFunc(keyboardCallback);
    glutDisplayFunc(displayCallback);
    glutTimerFunc(50, timerCallback, 0);
    glutMainLoop();
    return 0;
}
