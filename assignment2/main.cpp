#include "icg_common.h"
#include "imshow.h"

#include <math.h>
#include <OpenGP/GL/Eigen.h>

// #include "Triangle/Triangle.h"
// #include "Quad/Quad.h"
#include "Cube/Cube.h"


using namespace OpenGP;

// Triangle triangle;
// Quad quad;
Cube cube;

void display() {

    glClear(GL_COLOR_BUFFER_BIT);

    // triangle.draw();
    // quad.draw();
	cube.draw();

}

int main(int, char**) {

    OpenGP::glfwInitWindowSize(640, 480);
    OpenGP::glfwMakeWindow("Assignment 2");

    glClearColor(0.0,0.0,0.0,0.0);

    // triangle.init();
    // quad.init();
	cube.init();

    OpenGP::glfwDisplayFunc(&display);

    OpenGP::glfwMainLoop();

    return EXIT_SUCCESS;

}
