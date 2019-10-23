
#shader vertex////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#version 330 core
layout(location = 0) in vec4 position;
layout(location = 1) in uint texture_offset;
layout(location = 2) in uint corner_offset;

out uint v_texture_offset;
out uint v_corner_offset;

void main() {
	gl_Position = position;
	v_texture_offset = texture_offset;
	v_corner_offset = corner_offset;
}


//The geometry shader is run on every primitive(triangle, line, point) and can discard it or output more primitives than came in.
//This is similar to the tessellation shader, but much more flexible.
//However, it is not used much in today's applications
//!! because the performance is not that good on most graphics cards except for Intel's integrated GPUs.!! jep thats it

#shader geometry////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in uint v_texture_offset[];
in uint v_corner_offset[];

out vec2 g_uv_coords;
out vec2 g_corner_uv_coords;
//out vec2 g_corner_color;

uniform sampler2D u_texture;

uniform int u_texture_atlas_width;//number if block icons in a row
uniform int u_corner_atlas_width;//number if corner icons in a row
uniform mat4 u_transform;

void main() {
	
	int offset = int(v_texture_offset[0]);
	if (offset == 0)//zero means no render
		return;
	offset -= 1;
	
	float x = float(offset & ((1 << 16) - 1));//get lsb 16 bits
	float y = float((offset>>16) & ((1 << 16) - 1));//get msb 16 bits

	float co = 1.0f / u_texture_atlas_width;
	

	int corner_offset = int(v_corner_offset[0]);

	float corner_x = float(corner_offset & ((1 << 16) - 1));//get lsb 16 bits
	float corner_y = float((corner_offset >> 16) & ((1 << 16) - 1));//get msb 16 bits

	float corner_co = 1.0f / u_corner_atlas_width;

	g_corner_uv_coords = vec2(corner_x, corner_y) * corner_co;
	g_uv_coords = vec2(x, y) * co;
	gl_Position = u_transform * (gl_in[0].gl_Position);
	EmitVertex();


	g_corner_uv_coords = vec2(corner_x + 1, corner_y) * corner_co;
	g_uv_coords = vec2(x + 1, y) * co;
	gl_Position = u_transform * (gl_in[0].gl_Position + vec4(1.0f, 0.0f,0,0));
	EmitVertex();


	g_corner_uv_coords = vec2(corner_x, corner_y + 1) * corner_co;
	g_uv_coords = vec2(x, y + 1) * co;
	gl_Position = u_transform * (gl_in[0].gl_Position + vec4(0.0f, 1.0f,0,0));
	EmitVertex();

	g_corner_uv_coords = vec2(corner_x + 1, corner_y + 1) * corner_co;
	g_uv_coords = vec2(x +1, y + 1) * co;
	gl_Position = u_transform * (gl_in[0].gl_Position + vec4(1.0f, 1.0f,0,0));
	EmitVertex();


	EndPrimitive();
}

#shader fragment////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#version 330 core

uniform sampler2D u_texture;
uniform sampler2D u_corners;
uniform int u_texture_atlas_pixel_width;
uniform int u_texture_atlas_pixel_width_corner;


layout(location = 0) out vec4 color;

in vec2 g_uv_coords;
in vec2 g_corner_uv_coords;

void main() {
	vec4 corner_color = texture2D(u_corners, g_corner_uv_coords);
	if (corner_color.a == 0)
		discard;
	if (corner_color.r == 1.0f)
		color = texture2D(u_texture, g_uv_coords);
	else {
		//color = texture2D(u_texture, g_corner_color);
		ivec2 pixelCoord = 
			ivec2(
				floor(g_uv_coords.x*u_texture_atlas_pixel_width) / u_texture_atlas_pixel_width_corner,
				floor(g_uv_coords.y*u_texture_atlas_pixel_width) / u_texture_atlas_pixel_width_corner);
		color = texelFetch(u_texture, pixelCoord,0);
		//color = vec4(0,0,0,1);
	}
}