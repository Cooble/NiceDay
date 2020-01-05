
#shader vertex////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#version 330 core
layout(location = 0) in vec4 position;
layout(location = 1) in vec2 texture_offset;
layout(location = 2) in vec2 corner_offset;

out vec2 v_texture_offset;
out vec2 v_corner_offset;
uniform mat4 u_transform;


void main() {
	gl_Position = u_transform * position;
	v_texture_offset = texture_offset;
	v_corner_offset = corner_offset;
}


//The geometry shader is run on every primitive(triangle, line, point) and can discard it or output more primitives than came in.
//This is similar to the tessellation shader, but much more flexible.
//However, it is not used much in today's applications
//!! because the performance is not that good on most graphics cards except for Intel's integrated GPUs.!! so thats that: no geometry shader

#shader fragment////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#version 330 core

uniform sampler2D u_texture;
uniform sampler2D u_corners;

uniform int u_texture_atlas_pixel_width;
uniform int u_texture_atlas_pixel_width_corner;


layout(location = 0) out vec4 color;

in vec2 v_texture_offset;
in vec2 v_corner_offset;

void main() {
	vec4 corner_color = texture2D(u_corners, v_corner_offset);
	if (corner_color.a == 0)
		discard;
	if (corner_color.r == 1.0f)
		color = texture2D(u_texture, v_texture_offset);
	else {
		//color = texture2D(u_texture, g_corner_color);
		ivec2 pixelCoord = 
			ivec2(
				floor(v_texture_offset.x*u_texture_atlas_pixel_width) / u_texture_atlas_pixel_width_corner,
				floor(v_texture_offset.y*u_texture_atlas_pixel_width) / u_texture_atlas_pixel_width_corner);
		color = texelFetch(u_texture, pixelCoord,0);
		//color = vec4(1,1,1,1);
	}
}