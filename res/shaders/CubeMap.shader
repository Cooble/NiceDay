
#shader vertex

#version 330 core
layout(location=0) in vec4 position;

uniform mat4 u_viewMat;
uniform mat4 u_projMat;

out vec3 v_texture_dir;

void main(){
	v_texture_dir = position.xyz;
	gl_Position = u_projMat * u_viewMat * position;

}


#shader fragment
#version 330 core

in vec3 v_texture_dir; // direction vector representing a 3D texture coordinate
uniform samplerCube cubemap; // cubemap texture sampler

layout(location=0) out vec4 color;

void main()
{
	color = texture(cubemap, v_texture_dir);
}

