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
template <typename T>
struct Vector2D {
    T x, y;

    Vector2D operator-(const Vector2D<T>& rhs) const {
        return {x - rhs.x, y - rhs.y};
    }
    Vector2D operator+(const Vector2D<T>& rhs) const {
        return {x + rhs.x, y + rhs.y};
    }
    Vector2D operator*(const T& rhs) const {
        return {x * rhs, y * rhs};
    }

    T dot(const Vector2D<T>& rhs) const {
        return x * rhs.x + y * rhs.y;
    }

    T len() const {
        return sqrt(x * x + y * y);
    }
};
std::vector<Line> lines;
enum class PainterMode {
    IDLE, LINE, POLYGON
} painterMode = PainterMode::LINE;
int windowHandle;
std::pair<int, int> tempLine[2] = {{-1, -1},
                                   {-1, -1}};
std::vector<std::pair<int, int>> polygon;

void setPainterMode(PainterMode newMode) {
    painterMode = newMode;
    switch (painterMode) {
        case PainterMode::LINE:
            glutSetWindowTitle("Cyrus Beck Clipper [Mode: Draw Lines]");
            break;
        case PainterMode::POLYGON:
            glutSetWindowTitle("Cyrus Beck Clipper [Mode: Draw View Port (Polygon)]");
            break;
        case PainterMode::IDLE:
            glutSetWindowTitle("Cyrus Beck Clipper");
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
        } else if (painterMode == PainterMode::POLYGON) {
            polygon.emplace_back(cursor_x, cursor_y);
        }
    } else if (state == GLUT_UP && button == GLUT_LEFT_BUTTON) {
        if (painterMode == PainterMode::LINE) {
            lines.push_back({tempLine[0].first, tempLine[0].second, cursor_x, cursor_y});
            tempLine[0] = {-1, -1};
            tempLine[1] = {-1, -1};
        } else if (painterMode == PainterMode::POLYGON) {
            polygon.back() = {cursor_x, cursor_y};
        }
    }
}

void motionCallback(int x, int y) {
    y = WINDOW_HEIGHT - y;
    if (painterMode == PainterMode::LINE) {
        tempLine[1] = {x, y};
    } else if (painterMode == PainterMode::POLYGON) {
        polygon.back() = {x, y};
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
    } else if (key == 'p' || key == 'P') {
        polygon.clear();
        setPainterMode(PainterMode::POLYGON);
    } else if (key == 'r' || key == 'R') {
        lines.clear();
        std::default_random_engine rand(std::chrono::system_clock::now().time_since_epoch().count());
        for (int i = 0; i < 50; ++i) {
            lines.push_back({static_cast<int>(rand() % WINDOW_WIDTH),
                             static_cast<int>(rand() % WINDOW_HEIGHT),
                             static_cast<int>(rand() % WINDOW_WIDTH),
                             static_cast<int>(rand() % WINDOW_HEIGHT)});
        }
        setPainterMode(PainterMode::IDLE);
    } else if (key == 'c' || key == 'C') {
        lines.clear();
        setPainterMode(PainterMode::IDLE);
    } else if (key == 'i' || key == 'I') {
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

    // Validate Polygon
    bool validity = polygon.size() > 2;
    if (validity) {
        // Check if the Polygon is Convex
        bool sign;
        for (int i = 0; i < polygon.size(); ++i) {
            const auto& p0 = polygon[(i - 1 + polygon.size()) % polygon.size()];
            const auto& p1 = polygon[i];
            const auto& p2 = polygon[(i + 1) % polygon.size()];
            int cp = (p1.first - p0.first) * (p2.second - p1.second) - (p1.second - p0.second) * (p2.first - p1.first);
            if (i == 0)
                sign = cp > 0;
            else if (cp > 0 != sign) {
                validity = false;
                break;
            }
        }
    }

    if (validity) {
        // Find perpendicular vectors for each line
        Vector2D<float> n_arr[polygon.size()];
        for (int i = 0; i < polygon.size(); ++i) {
            const auto& p0 = polygon[i];
            const auto& p1 = polygon[(i + 1) % polygon.size()];
            const auto& p2 = polygon[(i + 2) % polygon.size()];
            Vector2D<float> this_line = Vector2D<float>{(float)p1.first, (float)p1.second} - Vector2D<float>{(float)p0.first, (float)p0.second};
            Vector2D<float> that_line = Vector2D<float>{(float)p2.first, (float)p2.second} - Vector2D<float>{(float)p1.first, (float)p1.second};
            float dot = this_line.dot(that_line);
            float cos_value = dot / (this_line.len() * that_line.len());
            Vector2D<float> v1 = this_line * (1 / this_line.len()) * cos_value * that_line.len();
            n_arr[i] = that_line - v1;
        }

        // Render lines after clipping
        if (polygon.size() >= 3) {
            glColor3f(0, 1, 0);
            glLineWidth(3.0);
            for (const auto& line : lines) {
                float upper_t = 1.0f, lower_t = 0.0f;
                Vector2D<float> line_vector = Vector2D<float>{float(line.dx - line.sx), float(line.dy - line.sy)};
                for (int i = 0; i < polygon.size(); ++i) {
                    const auto& p0 = polygon[i];
                    const auto& p1 = polygon[(i + 1) % polygon.size()];
                    // Intersect
                    auto x1 = (float) line.sx, x2 = (float) line.dx, y1 = (float) line.sy, y2 = (float) line.dy;
                    auto x3 = (float) p0.first, x4 = (float) p1.first, y3 = (float) p0.second, y4 = (float) p1.second;
                    float intersect_x = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4)) / ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4));
                    float intersect_y = ((x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)) / ((x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4));
                    auto t = (float)((intersect_x - x1) / (x2 - x1));
                    auto dot_product = line_vector.dot(n_arr[i]);
                    if (dot_product > 0) {
                        lower_t = lower_t < t ? t : lower_t;
                    } else {
                        upper_t = upper_t > t ? t : upper_t;
                    }
                }
                if (lower_t > upper_t || upper_t < 0 || lower_t > 1)
                    continue;
                float sx = float(line.dx - line.sx) * lower_t + float(line.sx);
                float sy = float(line.dy - line.sy) * lower_t + float(line.sy);
                float dx = float(line.dx - line.sx) * upper_t + float(line.sx);
                float dy = float(line.dy - line.sy) * upper_t + float(line.sy);
                glColor3f(0, 1, 0);
                glBegin(GL_LINE_STRIP);
                glLineWidth(2.0);
                glVertex2f(sx, sy);
                glVertex2f(dx, dy);
                glEnd();
            }
        }
    }

    // Draw temp line
    if (tempLine[0].first != -1 && tempLine[1].first != -1) {
        glBegin(GL_LINE_STRIP);
        glLineWidth(2.0);
        glColor3f(0, 0, 1);
        glVertex2i(tempLine[0].first, tempLine[0].second);
        glVertex2i(tempLine[1].first, tempLine[1].second);
        glEnd();
    }

    glColor3f(validity ? 0 : 1, 0, 0);
    glLineWidth(2.0);
    // Draw polygon
    if (polygon.size() == 1) {
        glBegin(GL_POINTS);
        glVertex2i(polygon[0].first, polygon[0].second);
        glEnd();
    } else if (!polygon.empty()) {
        glBegin(GL_LINE_STRIP);
        for (const auto & point: polygon) {
            glVertex2i(point.first, point.second);
        }
        glEnd();
        if (polygon.size() > 2) {
            // If drawing isn't finished, draw a green line to connect the first and last point
            glBegin(GL_LINE_STRIP);
            if (painterMode == PainterMode::POLYGON) glColor3f(0, 1, 0);
            glVertex2i(polygon[0].first, polygon[0].second);
            glVertex2i(polygon.back().first, polygon.back().second);
            glEnd();
        }
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
    windowHandle = glutCreateWindow("Cyrus Beck Clipper");
    initialize();
    glutMouseFunc(mouseCallback);
    glutMotionFunc(motionCallback);
    glutKeyboardFunc(keyboardCallback);
    glutDisplayFunc(displayCallback);
    glutTimerFunc(50, timerCallback, 0);
    glutMainLoop();
    return 0;
}
