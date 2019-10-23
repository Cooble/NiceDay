#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 uv;

uniform mat4 u_transform;

out vec2 v_uv;

void main() {
	gl_Position = u_transform * position;
	v_uv = uv;
}


#shader fragment
#version 330 core

layout(location = 0) out vec4 color;
uniform sampler2D u_texture;
in vec2 v_uv;

void main() {
	color = texture2D(u_texture, v_uv);
	if (color.a == 0)
		discard;
	if (color.a < 0.5)
		color = vec4(1, 1, 1, 1);
	//else color = vec4(0, 0, 0, 1);
}

