
#shader vertex

#version 330 core
layout(location=0) in vec4 position;

struct GLO {
	mat4 view;
	mat4 proj;
};
uniform GLO glo;
out vec3 v_texture_dir;

void main(){
	v_texture_dir = position.xyz;
	gl_Position = glo.proj * glo.view * position;
}


#shader fragment
#version 330 core


struct MAT {
	samplerCube cubemap; // cubemap texture sampler
	//float upper;
};
uniform MAT mat;

in vec3 v_texture_dir; // direction vector representing a 3D texture coordinate
layout(location=0) out vec4 color;

void main()
{

	color = texture(mat.cubemap, normalize(v_texture_dir+vec3(0,2,0)));
	//color = texture(mat.cubemap, normalize(v_texture_dir+vec3(0,mat.upper,0)));

}

