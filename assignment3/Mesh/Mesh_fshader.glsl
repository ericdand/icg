#version 330 core
out vec3 color;

in vec2 uv;

uniform sampler2D height_map;
uniform sampler2D diffuse_map;

void main() {
	// Tile the grass texture 5x5 times over the plane.
    float tiling_amount = 5;
	vec3 grass = texture(diffuse_map, uv * tiling_amount).rgb;

    float height_center = texture(height_map, uv).r;
    float dx = textureOffset(height_map, uv, ivec2(1, 0)).r -
			  textureOffset(height_map, uv, ivec2(-1, 0)).r;
	vec3 xgrad = vec3(2.0/textureSize(height_map, 0).x, dx, 0.0);
    float dz = textureOffset(height_map, uv, ivec2(0, 1)).r - 
			  textureOffset(height_map, uv, ivec2(0, -1)).r;
	vec3 zgrad = vec3(0.0, dz, 2.0/textureSize(height_map, 0).y);
    
	// Calculate normal from height differences.
    vec3 N = normalize(cross(zgrad, xgrad));
    vec3 light = normalize(vec3(1.0, 3.0, 0.0));

	// Positive values only; negative value means light is on other side.
	float diffuse = clamp(dot(N, light), 0.0, 1.0);
    float ambient = 0.2;

    color = (ambient + diffuse)*grass;
}
