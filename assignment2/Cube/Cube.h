#pragma once
#include "icg_common.h"

class Cube{
private:
    GLuint _vao; ///< vertex array object
    GLuint _pid; ///< GLSL shader program ID 
    GLuint _vbo_vpoint; ///< memory buffer
    GLuint _vbo_vtexcoord; ///< memory buffer
    GLuint _tex; ///< Texture ID
	GLfloat _x; // position
	GLfloat _y;
	GLfloat _z;
	GLfloat _vx; // velocity
	GLfloat _vy;
	GLfloat _vz;
	GLfloat _zfar;
public:
	void init() {
		// Compile shaders
		_pid = OpenGP::load_shaders("Cube/vshader.glsl", "Cube/fshader.glsl");
		if (!_pid) exit(EXIT_FAILURE);
		glUseProgram(_pid);

		// Vertex array
		glGenVertexArrays(1, &_vao);
		glBindVertexArray(_vao);

		// These vertices specify 12 triangles which form a cube.
		// I originally punched these in in "CCW" order, given a positive Z
		// being "forward". I later realized my mistake, but didn't want
		// to fix all these coordinates. Later on we set the front face to
		// be the CW direction, as a result.
		const GLfloat vpoint[] { 
			// front face
			-1.0f, -1.0f, -1.0f, // bottom-left
			+1.0f, -1.0f, -1.0f, // bottom-right
			-1.0f, +1.0f, -1.0f, // top-left
			-1.0f, +1.0f, -1.0f, // top-left again
			+1.0f, -1.0f, -1.0f, // bottom-right again
			+1.0f, +1.0f, -1.0f, // top-right
			// top face
			-1.0f, +1.0f, -1.0f, // near-left is front face's top-left
			+1.0f, +1.0f, -1.0f, // near-right is front face's top-right
			-1.0f, +1.0f, +1.0f, // far-left
			-1.0f, +1.0f, +1.0f, // far-left again
			+1.0f, +1.0f, -1.0f, // near-right again
			+1.0f, +1.0f, +1.0f, // far-right
			// back face; "left" and "right" match the front face, as seen from
			// "inside" of the cube. note how vertices are given in CW order.
			-1.0f, -1.0f, +1.0f, // bottom-left
			-1.0f, +1.0f, +1.0f, // top-left
			+1.0f, -1.0f, +1.0f, // bottom-right
			+1.0f, -1.0f, +1.0f, // bottom-right again
			-1.0f, +1.0f, +1.0f, // top-left again
			+1.0f, +1.0f, +1.0f, // top-right
			// bottom face
			-1.0f, -1.0f, +1.0f, // far-left
			+1.0f, -1.0f, +1.0f, // far-right
			-1.0f, -1.0f, -1.0f, // near-left
			-1.0f, -1.0f, -1.0f, // near-left again
			+1.0f, -1.0f, +1.0f, // far-right again
			+1.0f, -1.0f, -1.0f, // near-right
			// left face
			-1.0f, +1.0f, -1.0f, // near-top is front face's top-left
			-1.0f, +1.0f, +1.0f, // far-top
			-1.0f, -1.0f, -1.0f, // near-bottom is front face's bottom-left
			-1.0f, -1.0f, -1.0f, // near-bottom again
			-1.0f, +1.0f, +1.0f, // far-top
			-1.0f, -1.0f, +1.0f, // far-bottom
			// right face
			+1.0f, -1.0f, -1.0f, // near-bottom is front face's bottom-right
			+1.0f, -1.0f, +1.0f, // far-bottom
			+1.0f, +1.0f, -1.0f, // near-top is front face's top-right
			+1.0f, +1.0f, -1.0f, // near-top again
			+1.0f, -1.0f, +1.0f, // far-bottom again
			+1.0f, +1.0f, +1.0f  // far-top
		};

		// Set up backface culling.
		// CW is front, due to my booboo above.
		glFrontFace(GL_CW);
		glEnable(GL_CULL_FACE);

		// Position the cube.
		_x = 0.0;
		_y = 0.0;
		// Give it an initial velocity.
		_vx = 0.033;
		_vy = 0.017;

		// Put the cube vertices into the buffer.
		glGenBuffers(1, &_vbo_vpoint);
		glBindBuffer(GL_ARRAY_BUFFER, _vbo_vpoint);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vpoint), vpoint, GL_STATIC_DRAW);

		// Tell the GPU what it is.
		GLuint vpoint_id = glGetAttribLocation(_pid, "vpoint");
		glEnableVertexAttribArray(vpoint_id);
		glVertexAttribPointer(vpoint_id, 3, GL_FLOAT, DONT_NORMALIZE,
				ZERO_STRIDE, ZERO_BUFFER_OFFSET);

		// Provide a far clipping plane (near plane is always 1.0).
		_zfar = -100.0;
		glUniform1f(glGetUniformLocation(_pid, "zfar"), _zfar);

		// Pass the fragment shader a list of 6 colours to use.
		const GLfloat c[] = {
			1.0, 0.0, 0.0, // red
			0.0, 1.0, 0.0, // green
			0.0, 0.0, 1.0, // blue
			1.0, 1.0, 0.0, // yellow
			1.0, 0.0, 1.0, // magenta
			0.0, 1.0, 1.0, // cyan
		};
		glUniform3fv(glGetUniformLocation(_pid, "colours"), 6, c);

		glBindVertexArray(0);
		glUseProgram(0);
	}

	void draw() {
		glUseProgram(_pid);
		glBindVertexArray(_vao);

		// Pass the current time for animation reference.
		float t = glfwGetTime();
		glUniform1f(glGetUniformLocation(_pid, "time"), t);

		// Calculate a new position for the cube.
		_x += _vx;
		// exceeds left or right side of screen
		if (_x > 4.0 || _x < -4.0) { _vx = -_vx; }
		_y += _vy;
		// exceeds top or bottom of screen
		if (_y > 4.0 || _y < -4.0) { _vy = -_vy; }

		glUniform1f(glGetUniformLocation(_pid, "cubex"), _x);
		glUniform1f(glGetUniformLocation(_pid, "cubey"), _y);

		// 12 triangles to make a 6-sided cube.
		glDrawArrays(GL_TRIANGLES, 0, 12*3);

		glBindVertexArray(0);
		glUseProgram(0);
	}
};

