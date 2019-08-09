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
uniform vec2 u_pixel_size;

const vec2 gaussFilter[7] =
{
	vec2(-3.0,	0.015625),
	vec2(-2.0,	0.09375),
	vec2(-1.0,	0.234375),
	vec2(0.0,	0.3125),
	vec2(1.0,	0.234375),
	vec2(2.0,	0.09375),
	vec2(3.0,	0.015625)
};

void main()
{
	vec4 colorE = vec4(0,0,0,0);
	for (int i = 0; i < 7; i++)
	{
		colorE += texture2D(u_attachment, vec2(f_uv.x + gaussFilter[i].x*u_pixel_size.x, f_uv.y + gaussFilter[i].x*u_pixel_size.y))*gaussFilter[i].y;
	}

	color = colorE;
}
