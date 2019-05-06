
#shader vertex

#version 330 core
layout(location = 0) in vec4 position;
layout(location = 1) in float color;

out vec4 v_color;

void main() {
	gl_Position = position;

	float colorr = color/3.0f;
	
	
	v_color = vec4(colorr, colorr, colorr, 1.0f);

}


#shader fragment
#version 330 core

in vec4 v_color;
layout(location = 0) out vec4 color;


void main() {
	//color = vec4(0, 1, 0, 1);
	color = v_color;
}

