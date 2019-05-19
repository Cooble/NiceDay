
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

#shader geometry////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in uint v_texture_offset[];
in uint v_corner_offset[];

out vec2 g_uv_coords;
out vec2 g_corner_uv_coords;


//2 to the n icons in atlas in row
uniform int u_texture_atlas_icon_number_bit;
uniform int u_corner_atlas_icon_number_bit;
uniform mat4 u_transform;
//todo shader chnk


void main() {
	float nibbleOfssetX = (int(gl_in[0].gl_Position.x) & 3) * 0.25f;
	float nibbleOfssetY = (int(gl_in[0].gl_Position.y) & 3) * 0.25f;

	int offset = int(v_texture_offset[0]);
	if (offset == 0)//zero means no render
		return;
	offset -= 1;
	int mask = (1 << u_texture_atlas_icon_number_bit) - 1;
	float x = float(offset & mask) + nibbleOfssetX;
	float y = float((offset >> u_texture_atlas_icon_number_bit) & mask) + nibbleOfssetY;

	float co = 1.0f / (1 << u_texture_atlas_icon_number_bit);


	int corner_offset = int(v_corner_offset[0]);

	int corner_mask = (1 << u_corner_atlas_icon_number_bit) - 1;
	int corner_x = corner_offset & corner_mask;
	int corner_y = (corner_offset >> u_corner_atlas_icon_number_bit) & corner_mask;

	float corner_co = 1.0f / float(1 << u_corner_atlas_icon_number_bit);

	g_corner_uv_coords = vec2(corner_x, corner_y) * corner_co;
	g_uv_coords = vec2(x, y) * co;
	gl_Position = u_transform * (gl_in[0].gl_Position);
	EmitVertex();

	g_corner_uv_coords = vec2(corner_x + 1, corner_y) * corner_co;
	g_uv_coords = vec2(x + 0.25f, y) * co;
	gl_Position = u_transform * (gl_in[0].gl_Position + vec2(1.0f, 0.0f));
	EmitVertex();

	g_corner_uv_coords = vec2(corner_x, corner_y + 1) * corner_co;
	g_uv_coords = vec2(x, y + 0.25f) * co;
	gl_Position = u_transform * (gl_in[0].gl_Position + vec2(0.0f, 1.0f));
	EmitVertex();

	g_corner_uv_coords = vec2(corner_x + 1, corner_y + 1) * corner_co;
	g_uv_coords = vec2(x + 0.25f, y + 0.25f) * co;
	gl_Position = u_transform * (gl_in[0].gl_Position + vec2(1.0f, 1.0f));
	EmitVertex();


	EndPrimitive();
}

#shader fragment////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#version 330 core

uniform sampler2D u_texture;
uniform sampler2D u_corners;

layout(location = 0) out vec4 color;

in vec2 g_uv_coords;
in vec2 g_corner_uv_coords;

void main() {

	vec4 corner_color = texture2D(u_corners, g_corner_uv_coords);
	if (corner_color.a == 0)
		discard;
	if (corner_color.r == 1.0f)
		color = texture2D(u_texture, g_uv_coords);
	else
		color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
}