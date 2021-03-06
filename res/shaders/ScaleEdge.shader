#shader vertex

#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;

out vec2 f_uv;

void main() {
	gl_Position = vec4(position,0,1);
	f_uv = uv;
}


#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 f_uv;

uniform sampler2D u_attachment;
uniform float u_scale;


void main()
{
	
	color = texture2D(u_attachment, f_uv);
	color.a = 1-(((1-color.a) - u_scale) / (1.f - u_scale));
}
