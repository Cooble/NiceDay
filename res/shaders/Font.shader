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

uniform vec4 u_textColor;
uniform vec4 u_borderColor;

in vec2 v_uv;

void main() {
	vec4 c = texture2D(u_texture, v_uv);
	if (c.a == 0)
		discard;
	color = mix(u_borderColor, u_textColor, c.r);
	color.a = c.a;
}

