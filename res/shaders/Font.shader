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
uniform vec2 u_colorWidths;

in vec2 v_uv;

void main() {
	//color = vec4(1, 1, 1, 1);
	//return;
	color = texture2D(u_texture, v_uv);
	//color.a = color.a * 2;
	if (color.a > 1- u_colorWidths.x)
		color = u_textColor;
	else if (color.a > 1- u_colorWidths.x- u_colorWidths.y)
		color = u_borderColor;
	else discard;
}

