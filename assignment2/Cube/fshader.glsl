#version 330 core
in float face;
out vec3 colour;
uniform vec3 colours[6];

void main() {
	colour = colours[int(face)];
}
