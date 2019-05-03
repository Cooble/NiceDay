
#shader vertex////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#version 330 core
layout(location = 0) in vec4 position;
layout(location = 1) in int texture_offset;

out int v_texture_offset;

void main() {
	gl_Position = position;
	v_texture_offset = texture_offset;
}

#shader geometry////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in int v_texture_offset[];

out vec2 f_uv_coords;

//2 to the n icons in atlas in row
uniform int u_atlas_icon_number_bit;
uniform mat4 u_transform;
//todo shader chnk


void main() {
	int offset = v_texture_offset[0];
	int mask = (1 << u_atlas_icon_number_bit) - 1;
	int x = offset & mask;
	int y = (offset >> u_atlas_icon_number_bit) & mask;

	float co = 1.0f / (1 << u_atlas_icon_number_bit);
	

	f_uv_coords = vec2(x, y) * co;
	gl_Position = u_transform * (gl_in[0].gl_Position);
	EmitVertex();

	f_uv_coords = vec2(x + 1, y) * co;
	gl_Position = u_transform * (gl_in[0].gl_Position + vec2(1, 0));
	EmitVertex();

	f_uv_coords = vec2(x, y + 1) * co;
	gl_Position = u_transform * (gl_in[0].gl_Position + vec2(0, 1));
	EmitVertex();

	f_uv_coords = vec2(x + 1, y + 1) * co;
	gl_Position = u_transform * (gl_in[0].gl_Position + vec2(1, 1));
	EmitVertex();


	EndPrimitive();
}

#shader fragment////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#version 330 core

uniform sampler2D u_texture;

layout(location = 0) out vec4 color;

in vec2 f_uv_coords;

void main() {
	color = texture2D(u_texture, f_uv_coords);
	//color = vec4(0, 0, 1, 1);
}