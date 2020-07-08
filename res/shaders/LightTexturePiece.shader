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

layout (location = 0) out vec4 color_0;
layout (location = 1) out vec4 color_1;


void main() {
	const float maxLightValue = 16.0f;

	color_0 = texture2D(u_texture_0, v_uv_coords).rrrr;
	color_0 *= 255.0f / maxLightValue;

	
	vec4 totalColor = vec4(u_chunkback_color.rgb,(texture2D(u_texture_1, v_uv_coords)* 255.0f / maxLightValue).r-(1-u_chunkback_color.a));

	color_0 = max(color_0, totalColor);
	color_1 = color_0;
	//color_1 = color_0;
	//color = vec4(1, 0, 0, 1);
}

