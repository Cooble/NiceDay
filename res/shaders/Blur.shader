#shader vertex

#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 uv;

out vec2 f_uv;

void main() {
	gl_Position = vec4(position,0,1);
	f_uv = uv;
}


#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec2 f_uv;

uniform sampler2D u_attachment;
uniform bool u_horizontal;
uniform vec2 u_pixel_size;

/*
//this version was calculated by me
const vec2 gaussFilter[5] = vec2[]
(
	vec2(-3.111111, 0.03515625),
	vec2(-1.333333, 0.328125),
	vec2(0.0,		0.2734375),
	vec2(1.333333,	0.328125),
	vec2(3.111111, 0.03515625)
	);*/

const vec2 gaussFilter[5] = vec2[]
(
	vec2(-3.2307692308, 0.03515625),
	vec2(-1.3846153846, 	0.328125),
	vec2(0.0,		0.2734375),
	vec2(1.3846153846, 	0.328125),
	vec2(3.2307692308, 0.03515625)
	);
	
void main()
{
	int size = 5;
	color = vec4(0,0,0,1);
	if (u_horizontal) {
		for (int i = 0; i < size; i++)
		{
			color += texture2D(u_attachment, vec2(f_uv.x + gaussFilter[i].x * u_pixel_size.x, f_uv.y)) * gaussFilter[i].y;
		}
	}
	else{
		for (int i = 0; i < size; i++)
		{
			color += texture2D(u_attachment, vec2(f_uv.x,f_uv.y + gaussFilter[i].x * u_pixel_size.y)) * gaussFilter[i].y;
		}
	}
}
