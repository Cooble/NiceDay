//draws light texture onto lightmap
#shader vertex

#version 330 core
layout(location = 0) in vec4 position;

out vec2 v_uv_coords;

void main() {
	gl_Position = position;
	v_uv_coords = (position.xy+1)/2;
}


#shader fragment
#version 330 core

uniform sampler2D u_texture_first;
//uniform sampler2D u_texture_second;
//uniform vec4 u_chunkback_color;
in vec2 v_uv_coords;

layout(location = 0) out vec4 color;


void main() {
	const float maxLightValue = 16.0f;
	color = texture2D(u_texture_first, v_uv_coords).rrrr;
	color *= 255.0f / maxLightValue;
	//color = max(color, u_chunkback_color*(texture2D(u_texture_second, v_uv_coords).rrrr * 255.0f / maxLightValue));

	//color = vec4(1, 0, 0, 1);
}

