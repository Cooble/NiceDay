
#shader vertex

#version 330 core
layout(location=0) in vec4 position;
layout(location=1) in vec3 normal;
layout(location=2) in vec2 uv;


uniform mat4 u_worldMat;
uniform mat4 u_viewMat;
uniform mat4 u_projMat;

out vec3 v_normal;
out vec3 v_world_pos;
out vec2 v_uv;

void main(){
	gl_Position = u_projMat * u_viewMat * u_worldMat * position;
	v_normal = (u_worldMat * vec4(normal,0)).xyz;
	v_world_pos = (u_worldMat * position).xyz;
	v_uv = uv;
}


#shader fragment
#version 330 core

in vec3 v_world_pos;
in vec3 v_normal;
in vec2 v_uv;

layout(location=0) out vec4 color;

struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shines;
	vec4 color;
};
uniform Material u_material;

struct Light {
	vec3 pos;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	//attenuation
	float constant;
	float linear;
	float quadratic;
};
uniform Light u_light;

uniform vec3 u_camera_pos;

void main(){
	vec3 diffuseColor = u_material.color.rgb;
	vec3 specularColor = vec3(1.0);
	
	if (u_material.color.a == 0.0) {
		diffuseColor = texture2D(u_material.diffuse, v_uv).rgb;
		specularColor = texture2D(u_material.specular, v_uv).rgb;
	}
	vec3 normalNormal = normalize(v_normal);
	//AMBIENT
	vec3 ambientLight = u_light.ambient * diffuseColor;

	//DIFFUSE
	vec3 toSun = normalize(u_light.pos - v_world_pos);
	vec3 diffuseLight = u_light.diffuse * max(dot(toSun, normalNormal),0.0) * diffuseColor;
	
	//SPECULAR
	vec3 toCamera = normalize(u_camera_pos - v_world_pos);
	vec3 reflection = reflect(-toCamera, normalNormal);
	vec3 reflectiveLight = vec3(0.0);
	if(u_material.shines!=0)
		reflectiveLight = u_light.specular * specularColor * pow(max(dot(reflection, toSun), 0.0), u_material.shines);

	//ATTENUATION
	float dist = length(u_light.pos - v_world_pos);
	float atten = 1.0 / (u_light.constant + dist * u_light.linear + pow(dist, 2) * u_light.quadratic);


	color = vec4((ambientLight + diffuseLight + reflectiveLight)* atten,1);
//	color = texture2D(u_texture, v_uv) - 0.3 * vec4(angle / 3.14159 / 2, angle / 3.14159 / 2, angle / 3.14159 / 2, 1);
}

