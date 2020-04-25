#shader vertex

#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;
uniform mat4 transform;
out vec2 f_uv;

void main() {
	gl_Position = transform * vec4(position,0,1);
	f_uv = uv;
}


#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 f_uv;

uniform sampler2D u_attachment;

void main() {
	color = texture2D(u_attachment, f_uv);
}

