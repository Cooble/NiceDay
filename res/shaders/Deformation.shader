
#shader vertex

#version 330 core
layout(location=0) in vec4 position;
layout(location=1) in vec2 uv_coords;



out vec2 v_uv_coords;
out vec2 v_pos_coords;

//uniform mat4 u_transform;

void main(){
	gl_Position = /*u_transform **/ position;
	v_uv_coords = uv_coords;
	v_pos_coords = vec2(position.x, position.y);
}


#shader fragment
#version 330 core

uniform sampler2D u_texture;
uniform float u_on_screen;

layout(location=0) out vec4 color;

in vec2 v_uv_coords;
in vec2 v_pos_coords;

void main(){
	//vec4 cc = texture2D(u_texture, v_uv_coords);
	//color = texture2D(u_texture, v_uv_coords);
	//float len = length(v_pos_coords);
	vec2 offseto =abs(v_pos_coords);
	offseto = offseto/30.0*sign(v_pos_coords)*length(v_pos_coords) * length(v_pos_coords);


	color = texture2D(u_texture, v_uv_coords + offseto);
	float leng = length(v_pos_coords);
	leng = leng + u_on_screen;
	color.rgb = color.rgb*(2.0-leng*leng);
	//if (length(v_pos_coords) > 1.0)
	//	color.rgb = vec3(0, 0, 0);

	//color.rgb = abs(vec3(v_pos_coords.x, 0, 0));
}

