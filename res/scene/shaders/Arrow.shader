
#shader vertex

#version 330 core
layout(location=0) in vec4 position;
layout(location=1) in vec4 colo;

uniform mat4 world;

out vec4 v_col;

void main(){
	gl_Position = world * position;
	v_col=colo;
	
}


#shader fragment
#version 330 core
layout(location=0) out vec4 coloro;

in vec4 v_col;
uniform vec4 color;

void main(){
	coloro=v_col;
}

