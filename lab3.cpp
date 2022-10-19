#include <GL/glut.h>
#include <cmath>
#include <list>
#include <vector>
#include <random>
#include <chrono>

const int WINDOW_WIDTH = 1080;
const int WINDOW_HEIGHT = 720;

struct Line {
    int sx, sy, dx, dy;
};
std::vector<Line> lines;
enum class PainterMode {
    IDLE, LINE, RECT
} painterMode = PainterMode::LINE;
int windowHandle;
std::pair<int, int> tempLine[2] = {{-1, -1},
                                   {-1, -1}};
std::pair<int, int> rectangle[2] = {{-1, -1},
                                    {-1, -1}};

void setPainterMode(PainterMode newMode) {
    painterMode = newMode;
    switch (painterMode) {
        case PainterMode::LINE:
            glutSetWindowTitle("Ploy Line Painter [Mode: Draw Lines]");
            break;
        case PainterMode::RECT:
            glutSetWindowTitle("Ploy Line Painter [Mode: Draw View Port]");
            break;
        case PainterMode::IDLE:
            glutSetWindowTitle("Ploy Line Painter");
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
        if (painterMode == PainterMode::LINE) {
            tempLine[0] = {cursor_x, cursor_y};
        } else if (painterMode == PainterMode::RECT) {
            rectangle[0] = {cursor_x, cursor_y};
            rectangle[1] = {-1, -1};
        }
    } else if (state == GLUT_UP && button == GLUT_LEFT_BUTTON) {
        if (painterMode == PainterMode::LINE) {
            lines.push_back({tempLine[0].first, tempLine[0].second, cursor_x, cursor_y});
            tempLine[0] = {-1, -1};
            tempLine[1] = {-1, -1};
        } else if (painterMode == PainterMode::RECT) {
            rectangle[1] = {cursor_x, cursor_y};
        }
    }
}

void motionCallback(int x, int y) {
    y = WINDOW_HEIGHT - y;
    if (painterMode == PainterMode::LINE) {
        tempLine[1] = {x, y};
    } else if (painterMode == PainterMode::RECT) {
        rectangle[1] = {x, y};
    }
}

void keyboardCallback(unsigned char key, int cursor_x, int cursor_y) {
    cursor_y = WINDOW_HEIGHT - cursor_y;
    if (key == 'q' || key == 'Q') {
        glutDestroyWindow(windowHandle);
        exit(0);
    } else if (key == 'l' || key == 'L') {
        setPainterMode(PainterMode::LINE);
        tempLine[0] = {-1, -1};
        tempLine[1] = {-1, -1};
    } else if (key == 'v' || key == 'V') {
        setPainterMode(PainterMode::RECT);
    } else if (key == 'r' || key == 'R') {
        lines.clear();
        std::default_random_engine rand(std::chrono::system_clock::now().time_since_epoch().count());
        for (int i = 0; i < 100; ++i) {
            lines.push_back({static_cast<int>(rand() % WINDOW_WIDTH),
                             static_cast<int>(rand() % WINDOW_HEIGHT),
                             static_cast<int>(rand() % WINDOW_WIDTH),
                             static_cast<int>(rand() % WINDOW_HEIGHT)});
        }
        setPainterMode(PainterMode::IDLE);
    } else if (key == 'c' || key == 'C') {
        lines.clear();
        setPainterMode(PainterMode::IDLE);
    }
}

void displayCallback() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Render all lines
    for (const auto &line: lines) {
        glBegin(GL_LINE_STRIP);
        glLineWidth(2.0);
        glColor3f(1, 0, 0);
        glVertex2i(line.sx, line.sy);
        glVertex2i(line.dx, line.dy);
        glEnd();
    }

    // Render lines after clipping
    if (rectangle[0].first != -1 && rectangle[1].first != -1) {
        int l = std::min(rectangle[0].first, rectangle[1].first);
        int r = std::max(rectangle[0].first, rectangle[1].first);
        int t = std::max(rectangle[0].second, rectangle[1].second);
        int b = std::min(rectangle[0].second, rectangle[1].second);
        glColor3f(0, 1, 0);
        glLineWidth(3.0);
        for (const auto &line: lines) {
            int mask1 = 0, mask2 = 0;
            mask1 = mask1 | ((line.sx < l ? 1 : 0) << 3);
            mask1 = mask1 | ((line.sy > t ? 1 : 0) << 2);
            mask1 = mask1 | ((line.sx > r ? 1 : 0) << 1);
            mask1 = mask1 | ((line.sy < b ? 1 : 0) << 0);
            mask2 = mask2 | ((line.dx < l ? 1 : 0) << 3);
            mask2 = mask2 | ((line.dy > t ? 1 : 0) << 2);
            mask2 = mask2 | ((line.dx > r ? 1 : 0) << 1);
            mask2 = mask2 | ((line.dy < b ? 1 : 0) << 0);

            if (mask1 == 0 && mask2 == 0) {
                // Trivial Accept
                glBegin(GL_LINE_STRIP);
                glVertex2i(line.sx, line.sy);
                glVertex2i(line.dx, line.dy);
                glEnd();
            } else if (mask1 & mask2) {
                // Trivial Reject
                continue;
            } else {
                int sx = line.sx, sy = line.sy, dx = line.dx, dy = line.dy;
                int delta_y = dy - sy;
                int delta_x = dx - sx;
                if (mask1) {
                    if (sx < l) { sy += static_cast<int>(round((l - sx) * delta_y / (1.0 * delta_x))); sx = l; }
                    if (sy > t) { sx += static_cast<int>(round((t - sy) * delta_x / (1.0 * delta_y))); sy = t; }
                    if (sx > r) { sy += static_cast<int>(round((r - sx) * delta_y / (1.0 * delta_x))); sx = r; }
                    if (sy < b) { sx += static_cast<int>(round((b - sy) * delta_x / (1.0 * delta_y))); sy = b; }
                }
                if (mask2) {
                    if (dx < l) { dy += static_cast<int>(round((l - dx) * delta_y / (1.0 * delta_x))); dx = l; }
                    if (dy > t) { dx += static_cast<int>(round((t - dy) * delta_x / (1.0 * delta_y))); dy = t; }
                    if (dx > r) { dy += static_cast<int>(round((r - dx) * delta_y / (1.0 * delta_x))); dx = r; }
                    if (dy < b) { dx += static_cast<int>(round((b - dy) * delta_x / (1.0 * delta_y))); dy = b; }
                }
                glBegin(GL_LINE_STRIP);
                glVertex2i(sx, sy);
                glVertex2i(dx, dy);
                glEnd();
            }
        }
    }

    if (tempLine[0].first != -1 && tempLine[1].first != -1) {
        glBegin(GL_LINE_STRIP);
        glLineWidth(2.0);
        glColor3f(0, 0, 1);
        glVertex2i(tempLine[0].first, tempLine[0].second);
        glVertex2i(tempLine[1].first, tempLine[1].second);
        glEnd();
    }

    if (rectangle[0].first != -1 && rectangle[1].first != -1) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.0);
        glColor3f(0, 0, 0);
        glRecti(rectangle[0].first, rectangle[0].second, rectangle[1].first, rectangle[1].second);
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
    glutMouseFunc(mouseCallback);
    glutMotionFunc(motionCallback);
    glutKeyboardFunc(keyboardCallback);
    glutDisplayFunc(displayCallback);
    glutTimerFunc(50, timerCallback, 0);
    glutMainLoop();
    return 0;
}
