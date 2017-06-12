#include "icg_common.h"
#include "imshow.h"

#include <math.h>
#include <OpenGP/GL/Eigen.h>
#include <cstdio>

using namespace OpenGP;

static GLuint _pid;

static GLuint curve_vao; // VAO for curve data
static GLuint curve_vbo; // name of buffer that holds curve
static GLuint vp_attr;   // vertex attribute for curve

static GLuint cp_vao;   // VAO for control point data
static GLuint cp_vbo;   // name of buffer that holds CPs
static GLuint cp_attr;   // vertex attribute for CPs

#define VERTEX_COUNT 1000
static GLfloat vpoint[VERTEX_COUNT*2]; // points along the curve
static GLfloat cps[4*2]; // control points

/** Linear interpolation t percent [0.0, 1.0] of the way between a and b.
 * Used by calculate_curve.
 */
static inline float lerp(float a, float b, float t) {
	return a + (b-a)*t;
}

/** calculate_curve populates vpoint based on the contents of cps.
 * It uses DeCasteljau's algorithm to efficiently interpolate the points.
 */
static void calculate_curve() {
	int i;
	float a_x = cps[0], a_y = cps[1],
	      b_x = cps[2], b_y = cps[3],
		  c_x = cps[4], c_y = cps[5],
		  d_x = cps[6], d_y = cps[7],
		  ab_x, ab_y, bc_x, bc_y, cd_x, cd_y, 
		  abbc_x, abbc_y, bccd_x, bccd_y, 
		  t;
	for(i = 0; i < VERTEX_COUNT; i++) {
		t = (float)i/(VERTEX_COUNT-1);
		ab_x = lerp(a_x, b_x, t); ab_y = lerp(a_y, b_y, t);
		bc_x = lerp(b_x, c_x, t); bc_y = lerp(b_y, c_y, t);
		cd_x = lerp(c_x, d_x, t); cd_y = lerp(c_y, d_y, t);
		abbc_x = lerp(ab_x, bc_x, t); abbc_y = lerp(ab_y, bc_y, t);
		bccd_x = lerp(bc_x, cd_x, t); bccd_y = lerp(bc_y, cd_y, t);
		vpoint[i*2] = lerp(abbc_x, bccd_x, t);
		vpoint[i*2+1] = lerp(abbc_y, bccd_y, t);
	}
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(_pid);

	glBindVertexArray(curve_vao);
	if (0/* points were moved by the user */) {
		calculate_curve();
		glBindBuffer(GL_ARRAY_BUFFER, curve_vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vpoint), vpoint);
	}
	glDrawArrays(GL_LINE_STRIP, 0, VERTEX_COUNT);
	
	glBindVertexArray(cp_vao);
	if (0/* points were moved by the user */) {
		// CPs already adjusted; just copy them to the GPU.
		glBindBuffer(GL_ARRAY_BUFFER, cp_vbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(cps), cps);
	}
	glDrawArrays(GL_LINES, 0, 4);

	glBindVertexArray(0);
	glUseProgram(0);
}

void init() {
	_pid = OpenGP::load_shaders("vshader.glsl", "fshader.glsl");
	if (!_pid) exit(EXIT_FAILURE);
	glUseProgram(_pid);

	glGenVertexArrays(1, &cp_vao);
	glBindVertexArray(cp_vao);
	glGenBuffers(1, &cp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, cp_vbo);
	// Some initial values.
	cps[0] = -0.8; cps[1] = -0.7; // P0
	cps[2] = -0.1; cps[3] = 0.3;  // P1
	cps[4] = 0.3; cps[5] = -0.2;  // P2
	cps[6] = 0.8; cps[7] = 0.6;   // P3
	glBufferData(GL_ARRAY_BUFFER, sizeof(cps), cps, GL_DYNAMIC_DRAW);
	cp_attr = glGetAttribLocation(_pid, "vpoint");
	glEnableVertexAttribArray(cp_attr);
	glVertexAttribPointer(cp_attr, 2, GL_FLOAT, DONT_NORMALIZE,
			ZERO_STRIDE, ZERO_BUFFER_OFFSET);

	glGenVertexArrays(1, &curve_vao);
	glBindVertexArray(curve_vao);
	glGenBuffers(1, &curve_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, curve_vbo);
	calculate_curve();
	glBufferData(GL_ARRAY_BUFFER, sizeof(vpoint), vpoint, GL_DYNAMIC_DRAW);
	vp_attr = glGetAttribLocation(_pid, "vpoint");
	glEnableVertexAttribArray(vp_attr);
	glVertexAttribPointer(vp_attr, 2, GL_FLOAT, DONT_NORMALIZE, 
			ZERO_STRIDE, ZERO_BUFFER_OFFSET);

	glBindVertexArray(0);
	glUseProgram(0);
}

int main(int, char**) {

    OpenGP::glfwInitWindowSize(640, 480);
    OpenGP::glfwMakeWindow("Assignment 2");

    glClearColor(0.0,0.0,0.0,0.0);

	init();

    OpenGP::glfwDisplayFunc(&display);

    OpenGP::glfwMainLoop();

    return EXIT_SUCCESS;

}
#undef VERTEX_COUNT
