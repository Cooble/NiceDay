#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in uint color;
layout(location = 3) in uint borderColor;

uniform mat4 u_transform;

out vec2 v_uv;
out vec4 v_color;
out vec4 v_borderColor;

void main() {
	gl_Position = u_transform * position;
	v_uv = uv;

	v_color.r = float((uint(color) & uint(0xff000000)) >> 24) / 255.0;
	v_color.g = float((uint(color) & uint(0x00ff0000)) >> 16) / 255.0;
	v_color.b = float((uint(color) & uint(0x0000ff00)) >> 8)  / 255.0;
	v_color.a = float((uint(color) & uint(0x000000ff)) >> 0)  / 255.0;

	v_borderColor.r = float((uint(borderColor) & uint(0xff000000)) >> 24) / 255.0;
	v_borderColor.g = float((uint(borderColor) & uint(0x00ff0000)) >> 16) / 255.0;
	v_borderColor.b = float((uint(borderColor) & uint(0x0000ff00)) >> 8)  / 255.0;
	v_borderColor.a = float((uint(borderColor) & uint(0x000000ff)) >> 0)  / 255.0;

	

}


#shader fragment
#version 330 core

layout(location = 0) out vec4 color;
uniform sampler2D u_texture;

//uniform vec4 u_textColor;
//uniform vec4 u_borderColor;

in vec2 v_uv;
in vec4 v_color;
in vec4 v_borderColor;

void main() {
	vec4 c = texture2D(u_texture, v_uv);
	if (c.a == 0)
		discard;
	//color = mix(u_borderColor, u_textColor, c.r);
	color = mix(v_borderColor, v_color, c.r);
	color.a = c.a;
}

