#include <GL/glut.h>
#include <cmath>
#include <list>
#include <vector>
#include <cstdio>

const int WINDOW_WIDTH = 1080;
const int WINDOW_HEIGHT = 720;

std::vector<std::list<std::pair<int, int>>> polyLines;
enum class PainterMode {
    IDLE, CREATE, DELETE, MOVE
} painterMode = PainterMode::IDLE;
int windowHandle;
std::pair<int, int>* selectedMovingPoints[2] = {nullptr, nullptr};

void setPainterMode(PainterMode newMode) {
    painterMode = newMode;
    switch (painterMode) {
        case PainterMode::IDLE:
            glutSetWindowTitle("Ploy Line Painter [Mode: Idle]");
            break;
        case PainterMode::CREATE:
            glutSetWindowTitle("Ploy Line Painter [Mode: Create]");
            break;
        case PainterMode::DELETE:
            glutSetWindowTitle("Ploy Line Painter [Mode: Delete]");
            break;
        case PainterMode::MOVE:
            glutSetWindowTitle("Ploy Line Painter [Mode: Move]");
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

void initializePolyLines() {
    const int x = 300;
    const int y = 50;
    polyLines = {
            {{x, y}, {x + 300, y}, {x + 300, y + 300}, {x + 150, y + 500}, {x, y + 300}, {x, y}},
            {{x + 50, y}, {x + 50, y + 100}, {x + 100, y + 100}},
            {{x + 50, y + 200}, {x + 50, y + 250}, {x + 100, y + 250}, {x + 100, y + 200}, {x + 50, y + 200}},
            {{x + 200, y + 200}, {x + 200, y + 250}, {x + 250, y + 250}, {x + 250, y + 200}, {x + 200, y + 200}},
            {{x + 200, y + 433}, {x + 200, y + 500}, {x + 250, y + 500}, {x + 250, y + 366}, {x + 200, y + 433}},
    };
}

std::pair<int, int> findPointedPoint(int cursor_x, int cursor_y) {
    double candidateDistance = 5.;
    std::pair<int, int> candidate = {-1, -1};
    int a = 0;
    for (const auto& polyLine : polyLines) {
        int b = 0;
        for (const auto& point : polyLine) {
            double distance = sqrt(pow(point.first - cursor_x, 2) + pow(point.second - cursor_y, 2));
            if (distance < candidateDistance) {
                candidateDistance = distance;
                candidate = {a, b};
            }
            b++;
        }
        a++;
    }
    return candidate;
}

void mouseCallback(int button, int state, int cursor_x, int cursor_y) {
    cursor_y = WINDOW_HEIGHT - cursor_y;
    if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
        if (painterMode == PainterMode::DELETE) {
            auto pointIndices = findPointedPoint(cursor_x, cursor_y);
            if (pointIndices.first == -1 || pointIndices.second == -1)
                return;
            auto& polyLine = polyLines[pointIndices.first];
            auto pointIndex = pointIndices.second;
            if (polyLine.size() > 1 && (pointIndex == 0 || pointIndex == polyLine.size() - 1) && *polyLine.begin() == *polyLine.rbegin()) {
                polyLine.erase(polyLine.begin());
                polyLine.erase(--polyLine.end());
                polyLine.insert(polyLine.end(), *polyLine.begin());
            } else {
                auto iterator = polyLine.begin();
                for (int i = 0; i < pointIndex; ++i, ++iterator);
                polyLine.erase(iterator);
            }
            glutSwapBuffers();
        } else if (painterMode == PainterMode::MOVE) {
            auto pointIndices = findPointedPoint(cursor_x, cursor_y);
            if (pointIndices.first == -1 || pointIndices.second == -1)
                return;
            auto& polyLine = polyLines[pointIndices.first];
            auto iterator = polyLine.begin();
            for (int i = 0; i < pointIndices.second; ++i, ++iterator);
            selectedMovingPoints[0] = &*iterator;
            if (polyLine.size() > 1 && (pointIndices.second == 0 || pointIndices.second == polyLine.size() - 1) && *polyLine.begin() == *polyLine.rbegin())
                selectedMovingPoints[1] = &*polyLine.rbegin();
        } else if (painterMode == PainterMode::CREATE) {
            auto& polyLine = polyLines.back();
            auto pointIndices = findPointedPoint(cursor_x, cursor_y);
            if (pointIndices.first == polyLines.size() - 1 && pointIndices.second == 0) {
                // Ending Poly Line:
                if (polyLine.size() > 1) {
                    polyLine.insert(polyLine.end(), *polyLine.begin());
                }
                setPainterMode(PainterMode::IDLE);
            } else {
                // Add a new point
                polyLine.insert(polyLine.end(), {cursor_x, cursor_y});
            }
        }
    } else if (state == GLUT_UP && button == GLUT_LEFT_BUTTON) {
        if (painterMode == PainterMode::MOVE) {
            for (auto & i : selectedMovingPoints) {
                if (i != nullptr) {
                    i->first = cursor_x;
                    i->second = cursor_y;
                    i = nullptr;
                }
            }
        }
    }
}

void motionCallback(int x, int y) {
    y = WINDOW_HEIGHT - y;
    if (painterMode == PainterMode::MOVE) {
        for (auto & i : selectedMovingPoints) {
            if (i != nullptr) {
                i->first = x;
                i->second = y;
            }
        }
    }
}

void keyboardCallback(unsigned char key, int cursor_x, int cursor_y) {
    cursor_y = WINDOW_HEIGHT - cursor_y;
    if (key == 'q' || key == 'Q') {
        glutDestroyWindow(windowHandle);
        exit(0);
    } else if (key == 'b' || key == 'B') {
        setPainterMode(PainterMode::CREATE);
        polyLines.emplace_back();
    } else if (key == 'd' || key == 'D') {
        setPainterMode(PainterMode::DELETE);
    } else if (key == 'm' || key == 'M') {
        setPainterMode(PainterMode::MOVE);
    } else if (key == 'r' || key == 'R') {
        setPainterMode(PainterMode::IDLE);
        initializePolyLines();
    }
}

void displayCallback() {
    glClear(GL_COLOR_BUFFER_BIT);

    for (const auto& polyLine : polyLines) {
        glBegin(GL_LINE_STRIP);
        for (const auto& point : polyLine) {
            glVertex2i(point.first, point.second);
        }
        glEnd();
        glBegin(GL_POINTS);
        for (const auto& point : polyLine) {
            glVertex2i(point.first, point.second);
        }
        glEnd();
    }

    glutSwapBuffers();
}

void timerCallback(int value) {
    glutSwapBuffers();
    glutPostRedisplay();
    glutTimerFunc(50, timerCallback, 0);
}


int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    windowHandle = glutCreateWindow("Ploy Line Painter [Mode: Idle]");
    initialize();
    initializePolyLines();
    glutMouseFunc(mouseCallback);
    glutMotionFunc(motionCallback);
    glutKeyboardFunc(keyboardCallback);
    glutDisplayFunc(displayCallback);
    glutTimerFunc(50, timerCallback, 0);
    glutMainLoop();
    return 0;
}
