
#shader vertex

#version 330 core
layout(location=0) in vec4 position;
layout(location=1) in vec2 uv_coords;



out vec2 v_uv_coords;

uniform mat4 u_transformationMatrix;
uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;

void main(){
	gl_Position = u_projectionMatrix * u_viewMatrix * u_transformationMatrix * position;
	v_uv_coords = uv_coords;
}


#shader fragment
#version 330 core

uniform sampler2D u_texture;

layout(location=0) out vec4 color;

in vec2 v_uv_coords;

void main(){
	vec4 cc = texture2D(u_texture, v_uv_coords);
	//color = texture2D(u_texture, v_uv_coords+vec2(-0.05*cc.g, 0.05*cc.b)).rgba;
	color = cc;
}

