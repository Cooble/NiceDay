#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 uv_0;
layout(location = 2) in vec2 uv_1;
layout(location = 3) in int textureSlot;
layout(location = 4) in float mixCoeficient;

uniform mat4 u_projectionMatrix;

out vec2 v_uv_0;
out vec2 v_uv_1;

flat out float v_mix_coeficient;
flat out int v_textureSlot;



void main() {
	gl_Position = u_projectionMatrix *position;
	v_uv_0 = uv_0;
	v_uv_1 = uv_1;
	v_textureSlot = textureSlot;
	v_mix_coeficient = mixCoeficient;
}


#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

uniform sampler2D u_textures[16];

in vec2 v_uv_0;
in vec2 v_uv_1;
flat in float v_mix_coeficient;
flat in int v_textureSlot;

void main() {
	color = mix(texture2D(u_textures[v_textureSlot], v_uv_0), texture2D(u_textures[v_textureSlot], v_uv_1), pow(v_mix_coeficient,1.0f/0.5f));
}

