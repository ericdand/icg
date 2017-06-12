#version 330 core
in vec3 vpoint;
out float c;

void main() {
	gl_Position = vec4(vpoint, 1.0);
	c = float(gl_VertexID);
}

