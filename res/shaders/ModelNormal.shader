
#shader vertex

#version 330 core
layout(location=0) in vec4 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;
layout(location=3) in vec3 tangent;


struct GLO {

	mat4 view;
	mat4 proj;

	vec3 sunPos;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 camera_pos;
	//attenuation
	float constant;
	float linear;
	float quadratic;
};
uniform GLO glo;

uniform mat4 world;




out vec3 v_normal;
out vec3 v_world_pos;
out vec2 v_uv;

void main(){
	gl_Position = glo.proj * glo.view * world * position;
	v_normal = (world * vec4(normal,0)).xyz;
	v_world_pos = (world * position).xyz;
	v_uv = uv;
}


#shader fragment
#version 330 core

in vec3 v_normal;
in vec3 v_world_pos;
in vec2 v_uv;

layout(location=0) out vec4 color;

struct MAT {
	vec4 color;
	sampler2D diffuse;
	sampler2D specular;
	float shines;
};
uniform MAT mat;

struct GLO {
	
	mat4 view;
	mat4 proj;

	vec3 sunPos;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 camera_pos;
	//attenuation
	float constant;
	float linear;
	float quadratic;
};
uniform GLO glo;

void main(){
	//color = vec4(normalize(v_normal), 1);
    // return;
	vec3 diffuseColor = mat.color.rgb;
	vec3 specularColor = vec3(1.0);
	
	if (mat.color.a == 0.0) {
		diffuseColor = texture2D(mat.diffuse, v_uv).rgb;
		specularColor = texture2D(mat.specular, v_uv).rgb;
	}
   
	vec3 normalNormal = normalize(v_normal);
	//AMBIENT
	vec3 ambientLight = glo.ambient * diffuseColor;

	//DIFFUSE
	vec3 toSun = normalize(glo.sunPos - v_world_pos);
	vec3 diffuseLight = glo.diffuse * max(dot(toSun, normalNormal),0.0) * diffuseColor;
	
	//SPECULAR
	vec3 toCamera = normalize(glo.camera_pos - v_world_pos);
	vec3 reflection = reflect(-toCamera, normalNormal);
	vec3 reflectiveLight = vec3(0.0);
	if(mat.shines!=0)
		reflectiveLight = glo.specular * specularColor * pow(max(dot(reflection, toSun), 0.0), mat.shines);

	//ATTENUATION
	float dist = length(glo.sunPos - v_world_pos);
	float atten = 1.0 / (glo.constant + dist * glo.linear + pow(dist, 2) * glo.quadratic);


	color = vec4((ambientLight + diffuseLight + reflectiveLight)* atten,1);
//	color = texture2D(u_texture, v_uv) - 0.3 * vec4(angle / 3.14159 / 2, angle / 3.14159 / 2, angle / 3.14159 / 2, 1);
}

