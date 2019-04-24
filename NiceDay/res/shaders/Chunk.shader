
#shader vertex////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#version 330 core
layout(location = 0) in vec4 position;
layout(location = 1) in int texture_offset;



out int v_texture_offset;

uniform mat4 u_transformationMatrix;
uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;

void main() {
	gl_Position = u_projectionMatrix * u_viewMatrix * u_transformationMatrix * position;
	v_texture_offset = texture_offset;
}

#shader geometry////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#version 330 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in int v_texture_offset[];

uniform int u_atlas_size;
uniform int u_atlas_icon_number;

out vec2 f_uv_coords;

void main() {
	int offset = v_texture_offset[0];
	
	int x = offset % u_atlas_icon_number;
	int y = offset / u_atlas_icon_number;
	float co = 1 / u_atlas_icon_number;
	
	f_uv_offset = vec2((float)x *co, (float)y *co);
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();

	f_uv_offset = vec2((float)(x+1) *co, (float)y *co);
	gl_Position = gl_in[0].gl_Position+vec2(0.1,0);
	EmitVertex();

	f_uv_offset = vec2((float)(x + 1) *co, (float)(y+1) *co);
	gl_Position = gl_in[0].gl_Position + vec2(0.1, 0.1);

	f_uv_offset = vec2((float)x *co, (float)(y + 1) *co);
	gl_Position = gl_in[0].gl_Position + vec2(0, 0.1);
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
}