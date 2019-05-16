//draws full lightmap onto world
#shader vertex

#version 330 core
layout(location=0) in vec4 position;

uniform mat4 u_transform;

out vec2 v_uv_coords;

void main(){
	gl_Position = u_transform*position;
	v_uv_coords = position.xy;
}


#shader fragment
#version 330 core

uniform sampler2D u_texture;
in vec2 v_uv_coords;

layout(location=0) out vec4 color;


void main(){
	color = vec4(0,0,0,texture2D(u_texture, v_uv_coords).a);
	//if (color.r == 0)
	//discard;

}

