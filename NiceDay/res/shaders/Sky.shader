
#shader vertex

#version 330 core
layout(location = 0) in vec4 position;
out float v_y;
void main() {
	gl_Position = position;
	v_y = position.y;
}


#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in float v_y;
uniform vec4 u_up_color;
uniform vec4 u_down_color;


void main() {
	
	color = mix(u_down_color, u_up_color, (v_y+1)/2);
}

