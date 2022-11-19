#include <GL/glut.h>
#include <cstdio>
#include <vector>
#include <cmath>

const int WINDOW_WIDTH = 1080;
const int WINDOW_HEIGHT = 720;
int windowHandle;
double viewportAspect = WINDOW_HEIGHT / (double)WINDOW_WIDTH;

template <typename T> struct triple {
    T x, y, z;

    T& operator[](unsigned ind) {
        if (ind == 0) return x;
        if (ind == 1) return y;
        return z;
    }
};
struct mat4 {
    double d[16];

    mat4 operator*(mat4 const& rhs) {
        mat4 m{};
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                for (int k = 0; k < 4; ++k) {
                    m.d[i * 4 + j] += d[i * 4 + k] * rhs.d[k * 4 + j];
                }
            }
        }
        return m;
    }

    mat4 transpose() const {
        mat4 m{};
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                m.d[i * 4 + j] = d[j * 4 + i];
            }
        }
        return m;
    }

    static mat4 identity() {
        mat4 m{};
        m.d[0] = m.d[5] = m.d[10] = m.d[15] = 1;
        return m;
    }
};
mat4 operator*(mat4 const& lhs, mat4 const& rhs) {
    mat4 m{};
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            for (int k = 0; k < 4; ++k) {
                m.d[i * 4 + j] += lhs.d[i * 4 + k] * rhs.d[k * 4 + j];
            }
        }
    }
    return m;
}
std::vector<triple<double>> vertices;
std::vector<triple<unsigned>> faces;
double cameraX = 4, cameraY = 1, cameraZ = 0;
const double cameraDelta = 0.1;
mat4 cameraView;
std::vector<triple<double>> teapotPositions;
std::vector<mat4> teapotAngles;
std::vector<double> teapotScales;
unsigned selectedTeapot = 0;
const char* FILE_NAME = "../teapot.obj";


mat4 initializeCameraViewMatrix() {
    double upX = 0, upY = 1, upZ = 0;
    double dx = 1, dy = 0.0, dz = 0;
    double rx = upY * dz - upZ * dy, ry = -(upX * dz - upZ * dx) , rz = upX * dy - upY * dx;
    double ux = dy * rz - dz * ry, uy = -(dx * rz - dz * rx), uz = dx * ry - dy * rx;
    double ld = sqrt(dx * dx + dy * dy + dz * dz);
    double lr = sqrt(rx * rx + ry * ry + rz * rz);
    double lu = sqrt(ux * ux + uy * uy + uz * uz);
    mat4 matrix = {rx / lr, ry / lr, rz / lr, 0,
                   ux / lu, uy / lu, uz / lu, 0,
                   dx / ld, dy / ld, dz / ld, 0,
                   0, 0, 0, 1};
    return matrix;
}

void initialize() {
    glClearColor(0, 0, 0, 1);
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glShadeModel(GL_SMOOTH);
    cameraView = initializeCameraViewMatrix();
    teapotPositions = {{-20, 0, -20}, {+20, 0, +20}, {0, 0, 0}, {-20, 0, +20}, {+20, 0, -20}};
    for (unsigned i = 0; i < teapotPositions.size(); ++i) {
        teapotAngles.push_back(mat4::identity());
        teapotScales.push_back(1);
    }
}

void readData() {
    FILE* file = fopen(FILE_NAME, "r");
    if (file == nullptr) {
        perror("Failed to open teapot.obj");
        exit(0);
    }
    char type;
    while (!feof(file)) {
        fscanf(file, "%c", &type);
        if (type == 'v') {
            float x, y, z;
            fscanf(file, "%f %f %f", &x, &y, &z);
            vertices.push_back({x, y, z});
        } else if (type == 'f') {
            unsigned x, y, z;
            fscanf(file, "%u %u %u", &x, &y, &z);
            faces.push_back({x, y, z});
        }
    }
    fclose(file);
}

void timerCallback(int value) {
    glutSwapBuffers();
    glutPostRedisplay();
    glutTimerFunc(50, timerCallback, 0);
}

mat4 getRotateMatrix(unsigned ind, double angle) {
    if (ind == 0) {
        return mat4{1, 0, 0, 0,
                    0, cos(angle), -sin(angle), 0,
                    0, sin(angle), cos(angle), 0,
                    0, 0, 0, 1};
    } else if (ind == 1) {
        return mat4{cos(angle), 0, sin(angle), 0,
                    0, 1, 0, 0,
                    -sin(angle), 0, cos(angle), 0,
                    0, 0, 0, 1};
    } else {
        return mat4{cos(angle), -sin(angle), 0, 0,
                    sin(angle), cos(angle), 0, 0,
                    0, 0, 1, 0,
                    0, 0, 0, 1};
    }
}

mat4 getPerspectiveMatrix() {
    const double near = .1f;
    const double far = 100.f;
    const double fov = M_PI_2;
    const double top = (double)tan(fov / 2.) * near;
    const double right = viewportAspect * top;
    return mat4{near / top, 0, 0, 0,
                0, near / right, 0, 0,
                0, 0, - (far + near) / (far - near), -1,
                0, 0, -2 * (far * near) / (far - near), 0}.transpose();
}
mat4 getCameraOffset() {
    mat4 matrix1 = {1, 0, 0, -cameraX,
                    0, 1, 0, -cameraY,
                    0, 0, 1, -cameraZ,
                    0, 0, 0, 1};
    return matrix1;
}
mat4 getOffsetMatrix(double x, double y, double z) {
    return mat4{1, 0, 0, x,
                0, 1, 0, y,
                0, 0, 1, z,
                0, 0, 0, 1};
}
mat4 getScaleMatrix(double x) {
    return mat4{x, 0, 0, 0,
                0, x, 0, 0,
                0, 0, x, 0,
                0, 0, 0, 1};
}

void displayCallback() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    mat4 offsetView = getCameraOffset();
    mat4 pers = getPerspectiveMatrix();
    mat4 m = pers * cameraView * offsetView;
    glLoadMatrixd(m.transpose().d);

    // draw
    for (int i = -100; i <= 100; ++i) {
        glColor3f(1, (float)(i != 0), (float)(i != 0));
        glBegin(GL_LINE_STRIP);
        glVertex3f((float)i, 0, -100);
        glVertex3f((float)i, 0, +100);
        glEnd();
        glBegin(GL_LINE_STRIP);
        glVertex3f(-100, 0, (float)i);
        glVertex3f(+100, 0, (float)i);
        glEnd();
    }

    // draw loaded shape
    for (unsigned i = 0; i < teapotPositions.size(); ++i) {
        mat4 offset = getOffsetMatrix(teapotPositions[i][0], teapotPositions[i][1], teapotPositions[i][2]);
        mat4 angle = teapotAngles[i];
        mat4 scale = getScaleMatrix(teapotScales[i]);
        glLoadMatrixd((m * offset * angle * scale).transpose().d);
        if (selectedTeapot == i + 1)
            glColor3f(1, .2f, .2f);
        else
            glColor3f(.7f, .7f, .7f);
        for (auto & face : faces) {
            glBegin(GL_LINE_STRIP);
            for (unsigned j = 0; j < 4; ++j) {
                glVertex3d(vertices[face[j % 3] - 1][0], vertices[face[j % 3] - 1][1], vertices[face[j % 3] - 1][2]);
            }
            glEnd();
        }
    }
}

void reshapeCallback(GLsizei width, GLsizei height) {
    if (height == 0)
        height = 1;
    viewportAspect = (double)height / (double)width;

    glViewport(0, 0, width, height);
}

void keyboardCallback(unsigned char key, int cursor_x, int cursor_y) {
    if (key >= '0' && key <= '9') {
        unsigned id = key - '0';
        if (id <= teapotPositions.size()) {
            selectedTeapot = id;
        } else {
            selectedTeapot = 0;
        }
    } else if (selectedTeapot == 0) {
        if (key == 'w' || key == 'W') {
            cameraX -= cameraView.d[8] * cameraDelta;
            cameraY -= cameraView.d[9] * cameraDelta;
            cameraZ -= cameraView.d[10] * cameraDelta;
        } else if (key == 's' || key == 'S') {
            cameraX += cameraView.d[8] * cameraDelta;
            cameraY += cameraView.d[9] * cameraDelta;
            cameraZ += cameraView.d[10] * cameraDelta;
        } else if (key == 'a' || key == 'A') {
            cameraX -= cameraView.d[0] * cameraDelta;
            cameraY -= cameraView.d[1] * cameraDelta;
            cameraZ -= cameraView.d[2] * cameraDelta;
        } else if (key == 'd' || key == 'D') {
            cameraX += cameraView.d[0] * cameraDelta;
            cameraY += cameraView.d[1] * cameraDelta;
            cameraZ += cameraView.d[2] * cameraDelta;
        } else if (key == ' ') {
            cameraX += cameraView.d[4] * cameraDelta;
            cameraY += cameraView.d[5] * cameraDelta;
            cameraZ += cameraView.d[6] * cameraDelta;
        } else if (key == '.') {
            cameraX -= cameraView.d[4] * cameraDelta;
            cameraY -= cameraView.d[5] * cameraDelta;
            cameraZ -= cameraView.d[6] * cameraDelta;
        } else if (key == 'Q' || key == 'q') {
            cameraView = getRotateMatrix(2, -0.05) * cameraView;
        } else if (key == 'E' || key == 'e') {
            cameraView = getRotateMatrix(2, +0.05) * cameraView;
        }
    } else {
        if (key == 'w' || key == 'W') {
            teapotPositions[selectedTeapot - 1][0] -= .25;
        } else if (key == 's' || key == 'S') {
            teapotPositions[selectedTeapot - 1][0] += .25;
        } else if (key == 'a' || key == 'A') {
            teapotPositions[selectedTeapot - 1][2] += .25;
        } else if (key == 'd' || key == 'D') {
            teapotPositions[selectedTeapot - 1][2] -= .25;
        } else if (key == ' ') {
            teapotPositions[selectedTeapot - 1][1] += .25;
        } else if (key == '.') {
            teapotPositions[selectedTeapot - 1][1] -= .25;
        } else if (key == 'Q' || key == 'q') {
            teapotAngles[selectedTeapot - 1] = getRotateMatrix(2, -0.05) * teapotAngles[selectedTeapot - 1];
        } else if (key == 'E' || key == 'e') {
            teapotAngles[selectedTeapot - 1] = getRotateMatrix(2, +0.05) * teapotAngles[selectedTeapot - 1];
        } else if (key == '+') {
            teapotScales[selectedTeapot - 1] *= 1.05;
        } else if (key == '-') {
            teapotScales[selectedTeapot - 1] *= 0.95;
        }
    }
}

void specialCallback(int key, int cursor_x, int cursor_y) {
    if (selectedTeapot == 0) {
        if (key == GLUT_KEY_UP) {
            cameraView = getRotateMatrix(0, -0.05) * cameraView;
        } else if (key == GLUT_KEY_DOWN) {
            cameraView = getRotateMatrix(0, +0.05) * cameraView;
        } else if (key == GLUT_KEY_LEFT) {
            cameraView = getRotateMatrix(1, -0.05) * cameraView;
        } else if (key == GLUT_KEY_RIGHT) {
            cameraView = getRotateMatrix(1, +0.05) * cameraView;
        }
    } else {
        if (key == GLUT_KEY_UP) {
            teapotAngles[selectedTeapot - 1] = getRotateMatrix(0, -0.05) * teapotAngles[selectedTeapot - 1];
        } else if (key == GLUT_KEY_DOWN) {
            teapotAngles[selectedTeapot - 1] = getRotateMatrix(0, +0.05) * teapotAngles[selectedTeapot - 1];
        } else if (key == GLUT_KEY_LEFT) {
            teapotAngles[selectedTeapot - 1] = getRotateMatrix(1, -0.05) * teapotAngles[selectedTeapot - 1];
        } else if (key == GLUT_KEY_RIGHT) {
            teapotAngles[selectedTeapot - 1] = getRotateMatrix(1, +0.05) * teapotAngles[selectedTeapot - 1];
        }
    }
}

int main(int argc, char** argv) {
    readData();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowPosition(0, 0);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    windowHandle = glutCreateWindow("Flying a camera through a scene");
    initialize();
    glutDisplayFunc(displayCallback);
    glutReshapeFunc(&reshapeCallback);
    glutTimerFunc(50, &timerCallback, 0);
    glutKeyboardFunc(&keyboardCallback);
    glutSpecialFunc(&specialCallback);
    glutMainLoop();
    return 0;
}