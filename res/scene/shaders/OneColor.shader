
#shader vertex

#version 330 core
layout(location=0) in vec4 position;

uniform mat4 world;

void main(){
	gl_Position = world * position;
}


#shader fragment
#version 330 core

layout(location=0) out vec4 coloro;
uniform vec4 color;

void main(){
	coloro = color;
}

