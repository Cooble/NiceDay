
#shader vertex

#version 330 core
layout(location=0) in vec4 position;
layout(location=1) in vec4 color;



out vec4 v_color;

uniform mat4 u_transformationMatrix;
uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;

void main(){
	gl_Position = u_projectionMatrix * u_viewMatrix * u_transformationMatrix * position;
	v_color = color;
}


#shader fragment
#version 330 core

layout(location=0) out vec4 color;

in vec4 v_color;

void main(){
	color = v_color;
}

