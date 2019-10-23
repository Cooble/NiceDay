#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in int textureSlot;

uniform mat4 u_projectionMatrix;

out vec2 v_uv;
flat out int v_textureSlot;



void main() {
	gl_Position = u_projectionMatrix *position;
	v_uv = uv;
	v_textureSlot = textureSlot;
}


#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

uniform sampler2D u_textures[16];

in vec2 v_uv;
flat in int v_textureSlot;

void main() {
	color = texture2D(u_textures[v_textureSlot], v_uv);
}

