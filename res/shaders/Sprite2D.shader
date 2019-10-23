//draws texture on quad 0,0,1,1
//thats it, thats all :D
#shader vertex

#version 330 core
layout(location = 0) in vec4 position;

uniform mat4 u_model_transform;
uniform mat4 u_uv_transform;
out vec2 v_uv_coords;

void main() {
	gl_Position = u_model_transform *position;
	v_uv_coords = ((u_uv_transform * (position*2.0f-1.0f)).xy+1.0f)/2.0f;
}


#shader fragment
#version 330 core

uniform sampler2D u_texture;
in vec2 v_uv_coords;

layout(location = 0) out vec4 color;

void main() {
	color = texture2D(u_texture, v_uv_coords);
}

