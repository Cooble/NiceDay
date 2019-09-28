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

uniform sampler2D u_texture_0;
uniform sampler2D u_texture_1;
uniform vec4 u_chunkback_color;
in vec2 v_uv_coords;

layout(location = 0) out vec4 color;


void main() {
	const float maxLightValue = 16.0f;

	color = texture2D(u_texture_0, v_uv_coords).rrrr;
	color *= 255.0f / maxLightValue;

	
	vec4 totalColor = vec4(u_chunkback_color.rgb,(texture2D(u_texture_1, v_uv_coords)* 255.0f / maxLightValue).r-(1-u_chunkback_color.a));

	color = max(color, totalColor);

	//color = vec4(1, 0, 0, 1);
}

