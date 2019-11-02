#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in int textureSlot;
layout(location = 3) in int color;

uniform mat4 u_projectionMatrix;

out vec2 v_uv;
flat out int v_textureSlot;
out vec4 v_color;



void main() {
	gl_Position = u_projectionMatrix *position;
	v_uv = uv;
	v_textureSlot = textureSlot;
	v_color = vec4((
		(color >> 0) & 0xFF) / 255.f, 
		((color >> 8) & 0xFF) / 255.f, 
		((color >> 16) & 0xFF) / 255.f, 
		((color >> 24) & 0xFF) / 255.f);
}


#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

uniform sampler2D u_textures[16];

in vec2 v_uv;
in vec4 v_color;
flat in int v_textureSlot;

void main() {
	if (v_textureSlot > 10000) {
		color = v_color;
	}else
		color = texture2D(u_textures[v_textureSlot], v_uv);
}

