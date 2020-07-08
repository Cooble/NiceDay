#shader vertex

#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;

uniform mat4 u_uv_trans;
out vec2 f_uv;

void main() {
	gl_Position = vec4(position,0,1);
	f_uv = (u_uv_trans * vec4(uv, 0, 1)).xy;
}


#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 f_uv;

uniform int u_steps;
uniform int u_wrapAfter;


vec2 multip(vec2 a, vec2 b);
vec2 multip(vec2 a, vec2 b) {
	return vec2(a.x * b.x - a.y*b.y, a.x * a.y + a.y * b.x);
}

void main() {
	vec2 z = vec2(0,0);

	ivec3 colo = ivec3(0,0,0);
	ivec3 dividor = ivec3(u_wrapAfter, u_wrapAfter/2 + 123, u_wrapAfter + 256);

	for (int i = 0; i < u_steps; i++) {
		z = multip(z, z)+ f_uv;
		if (dot(z,z)> 4) {
			colo = ivec3(i) % dividor;
			break;
		}
	}
	color = vec4(vec3(colo)/ vec3(dividor),1);
}

