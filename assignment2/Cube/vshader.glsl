#version 330 core
in vec3 vpoint;
out float face;
uniform float time;
uniform float zfar;
uniform float cubex;
uniform float cubey;

mat4 scale(float p) {
	return mat4(mat3(p));
}

mat4 translate(float x, float y, float z) {
	mat4 T = mat4(mat3(1));
	T[3][0] = x;
	T[3][1] = y;
	T[3][2] = z;
	return T;
}

mat4 roll(float rad) {
	mat3 R = mat3(1);
	R[0][0] = cos(rad);
	R[0][1] = sin(rad);
	R[1][0] = -sin(rad);
	R[1][1] = cos(rad);
	return mat4(R);
}

mat4 pitch(float rad) {
	mat3 R = mat3(1);
	R[1][1] = cos(rad);
	R[1][2] = sin(rad);
	R[2][1] = -sin(rad);
	R[2][2] = cos(rad);
	return mat4(R);
}

mat4 yaw(float rad) {
	mat3 R = mat3(1);
	R[2][2] = cos(rad);
	R[2][0] = sin(rad);
	R[0][2] = -sin(rad);
	R[0][0] = cos(rad);
	return mat4(R);
}

void main() {
	float znear = 1.0;
	float aspect_ratio = 640.0/480.0;
	mat4 scale_by_ar = mat4(1.0);
	scale_by_ar[1][1] = aspect_ratio;
	mat4 proj = mat4( // remember: column-major order.
			vec4(znear, 0.0, 0.0, 0.0),
			vec4(0.0, znear, 0.0, 0.0),
			vec4(0.0, 0.0, -(zfar + znear)/(zfar-znear), -1.0),
			vec4(0.0, 0.0, -2*zfar*znear/(zfar-znear), 0.0)
			);

	gl_Position = proj * scale_by_ar * // projection
		translate(cubex, cubey, -5.0) *  roll(time/2) * 
			pitch(3.1415926/6) * yaw(time/5) * // view
		scale(0.5) * vec4(vpoint, 1.0); // model

	// Tell the fragment shader which face this is.
	face = float(gl_VertexID/6);
}

