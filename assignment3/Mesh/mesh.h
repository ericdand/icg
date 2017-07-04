#pragma once
#include <vector>
#include "icg_common.h"
#include "OpenGP/Eigen/Image.h"

class Mesh{
protected:
    GLuint _vao; ///< vertex array object
    GLuint _pid; ///< GLSL shader program ID
    GLuint _height_tex;
    GLuint _diffuse_tex;

    OpenGP::SurfaceMesh mesh;
    GLuint _vpoint;    ///< memory buffer
    GLuint _vnormal;   ///< memory buffer

	float *noise_data;
	int noise_width = 512;
	int noise_height = 512;

	inline float mix(float x, float y, float alpha) {
		return y * alpha + x * (1.0f - alpha);
	}

	inline float f(float t) {
		float t_3 = t * t * t;
		return 6 * t * t * t_3 - 15 * t * t_3 + 10 * t_3;
	}

	inline float rand01() {
		return ((float) std::rand())/((float) RAND_MAX);
	}

public:
    GLuint getProgramID(){ return _pid; }

	float height_at(float x, float z) {
		if (noise_data == nullptr) return 1.0;
		if (0.0 > x || x > 1.0 || 0.0 > z || z > 1.0) return 1.0;
		return noise_data[(int)(noise_width*x) + ((int)(noise_height*z))*noise_width];
		// TODO: For bonus points, smooth this out to lerp between the
		// four nearest height points. Kinda jerky as-is.
	}

	inline GLuint generate_perlin_noise() {
		float *gradients = new float[2 * noise_width * noise_height];
		auto sample_gradient = [&](int i, int j) {
			float x = gradients[2*i + 2*j*noise_height];
			float y = gradients[2*i + 2*j*noise_height];
			return vec2(x,y);
		};
		noise_data = new float[noise_width * noise_height];

		for (int i = 0; i < noise_width; ++ i) {
			for (int j = 0; j < noise_height; ++ j) {

				float angle = rand01();

				gradients[2*i + 2*j*noise_height]     = cos(2 * angle * M_PI);
				gradients[1 + 2*i + 2*j*noise_height] = sin(2 * angle * M_PI);

			}
		}

		for (int i = 0; i < noise_width; ++ i) {
			for (int j = 0; j < noise_height; ++ j) {
				noise_data[i + j * noise_height] = 0;
			}
		}

		int period = 512;
		for (int octave = 0; octave < 4; octave++) {
			float frequency = 1.0f / period;
			float scale = (octave+1)/(float)16;
			for (int i = 0; i < noise_width; ++ i) {
				for (int j = 0; j < noise_height; ++ j) {
					int left = (i / period) * period;
					int right = (left + period) % noise_width;
					float dx = (i - left) * frequency;

					int top = (j / period) * period;
					int bottom = (top + period) % noise_height;
					float dy = (j - top) * frequency;

					vec2 a(dx, -dy); //		 (+, -)
					vec2 b(dx - 1, -dy); //	 (-, -)
					vec2 c(dx - 1, 1 - dy);//(-, +)
					vec2 d(dx, 1 - dy); //	 (+, +)

					vec2 topleft = sample_gradient(left, top);
					vec2 topright = sample_gradient(right, top);
					vec2 bottomleft = sample_gradient(left, bottom);
					vec2 bottomright = sample_gradient(right, bottom);

					float tldot = a.dot(topleft);
					float trdot = b.dot(topright);
					float bldot = d.dot(bottomleft);
					float brdot = c.dot(bottomright);

					float leftside = mix(tldot, bldot, f(dy));
					float rightside = mix(trdot, brdot, f(dy));
					float noise = scale*mix(leftside, rightside, f(dx));

					noise_data[i + j * noise_height] += noise;
				}
			}
			period /= 2;
		}

		GLuint _tex;

		glGenTextures(1, &_tex);
		glBindTexture(GL_TEXTURE_2D, _tex);

		check_error_gl();
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		check_error_gl();
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F,
					 noise_width, noise_height, 0,
					 GL_RED, GL_FLOAT, noise_data);
		check_error_gl();

		delete gradients;

		return _tex;
	}

	inline GLuint generate_noise() {
		return generate_perlin_noise();
	}

    void init(){
        ///--- Compile the shaders
        _pid = OpenGP::load_shaders("Mesh/Mesh_vshader.glsl", "Mesh/Mesh_fshader.glsl");
        if(!_pid) exit(EXIT_FAILURE);
        check_error_gl();

		// Generate the mesh. 128x128.
		// Vertices in x-major order: (-1, 0, -1), (-1, 0, -0.9), (-1, 0, -0.8)
		for(int i = 0; i < 128; i++) {
			for(int j = 0; j < 128; j++) {
				mesh.add_vertex(vec3(2.0*(i/127.0)-1.0, 0.0, 2.0*(j/127.0)-1.0));
			}
		}
		// Faces.
		// A face is made up of 4 vertices. There are 127x127 faces.
		// Vertices must be given in CCW order, in a Z pattern. 
		// (e.g. bottom, top, left, right).
		std::vector<OpenGP::SurfaceMesh::Vertex> face_vertices;
		for(int i = 0; i < 127; i++) {
			for(int j = 0; j < 127; j++) {
				face_vertices.clear();
				face_vertices.push_back(OpenGP::SurfaceMesh::Vertex(i*128+j+1));
				face_vertices.push_back(OpenGP::SurfaceMesh::Vertex(i*128+j));
				face_vertices.push_back(OpenGP::SurfaceMesh::Vertex((i+1)*128+j));
				face_vertices.push_back(OpenGP::SurfaceMesh::Vertex((i+1)*128+j+1));
				mesh.add_face(face_vertices);
			}
		}
        mesh.triangulate();
        mesh.update_vertex_normals();
        printf("Generated mesh (#V=%d, #F=%d)\n", mesh.n_vertices(), mesh.n_faces());

        ///--- Vertex one vertex Array
        glGenVertexArrays(1, &_vao);
        glBindVertexArray(_vao);
        check_error_gl();

        ///--- Vertex Buffer
        OpenGP::SurfaceMesh::Vertex_property<OpenGP::Point> vpoints = mesh.get_vertex_property<OpenGP::Point>("v:point");
        glGenBuffers(ONE, &_vpoint);
        glBindBuffer(GL_ARRAY_BUFFER, _vpoint);
        glBufferData(GL_ARRAY_BUFFER, mesh.n_vertices() * sizeof(vec3), vpoints.data(), GL_STATIC_DRAW);
        check_error_gl();

        ///--- Normal Buffer
        OpenGP::SurfaceMesh::Vertex_property<OpenGP::Normal> vnormals = mesh.get_vertex_property<OpenGP::Normal>("v:normal");
        glGenBuffers(ONE, &_vnormal);
        glBindBuffer(GL_ARRAY_BUFFER, _vnormal);
        glBufferData(GL_ARRAY_BUFFER, mesh.n_vertices() * sizeof(vec3), vnormals.data(), GL_STATIC_DRAW);
        check_error_gl();

        ///--- Index Buffer
        std::vector<unsigned int> indices;
        for(OpenGP::SurfaceMesh::Face_iterator fit = mesh.faces_begin(); fit != mesh.faces_end(); ++fit) {
            unsigned int n = mesh.valence(*fit);
            OpenGP::SurfaceMesh::Vertex_around_face_circulator vit = mesh.vertices(*fit);
            for(unsigned int v = 0; v < n; ++v) {
                indices.push_back((*vit).idx());
                ++vit;
            }
        }

        GLuint _vbo_indices;
        glGenBuffers(ONE, &_vbo_indices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_indices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        check_error_gl();

        glUseProgram(_pid);

        _height_tex = generate_noise();

        GLuint tex_id = glGetUniformLocation(_pid, "height_map");
        check_error_gl();
        glUniform1i(tex_id, 0);
        check_error_gl();

        OpenGP::EigenImage<vec3> image;
        OpenGP::imread("grass.tga", image);

        glGenTextures(1, &_diffuse_tex);
        glBindTexture(GL_TEXTURE_2D, _diffuse_tex);

        check_error_gl();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        check_error_gl();
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F,
                     image.cols(), image.rows(), 0,
                     GL_RGB, GL_FLOAT, image.data());
        check_error_gl();

        tex_id = glGetUniformLocation(_pid, "diffuse_map");
        check_error_gl();
        glUniform1i(tex_id, 1);
        check_error_gl();

        glUseProgram(0);

    }

    void draw(mat4 Model, mat4 View, mat4 Projection){
        glUseProgram(_pid);
        glBindVertexArray(_vao);
        check_error_gl();

        ///--- Vertex Attribute ID for Positions
        GLint vpoint_id = glGetAttribLocation(_pid, "vpoint");
        glEnableVertexAttribArray(vpoint_id);

        glBindBuffer(GL_ARRAY_BUFFER, _vpoint);
        glVertexAttribPointer(vpoint_id, 3 /*vec3*/, GL_FLOAT, DONT_NORMALIZE, ZERO_STRIDE, ZERO_BUFFER_OFFSET);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _height_tex);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, _diffuse_tex);

        ///--- Set the MVP to vshader
        glUniformMatrix4fv(glGetUniformLocation(_pid, "MODEL"), 1, GL_FALSE, Model.data());
        glUniformMatrix4fv(glGetUniformLocation(_pid, "VIEW"), 1, GL_FALSE, View.data());
        glUniformMatrix4fv(glGetUniformLocation(_pid, "PROJ"), 1, GL_FALSE, Projection.data());

        check_error_gl();
        ///--- Draw
        glDrawElements(GL_TRIANGLES,
                    /*#vertices*/ 3*mesh.n_faces(),
                    GL_UNSIGNED_INT,
                    ZERO_BUFFER_OFFSET);
        check_error_gl();

        ///--- Clean up
        glBindVertexArray(0);
        glUseProgram(0);
    }
};
