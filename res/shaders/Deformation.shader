
#shader vertex

#version 330 core
layout(location=0) in vec4 position;
layout(location=1) in vec2 uv_coords;



out vec2 v_uv_coords;
out vec2 v_pos_coords;

//uniform mat4 u_transform;

void main(){
	gl_Position = position;
	v_uv_coords = uv_coords;
	v_pos_coords = vec2(position.x, position.y);
}


#shader fragment
#version 330 core

uniform sampler2D u_texture;
uniform float u_on_screen;
uniform float u_line;

layout(location=0) out vec4 color;

in vec2 v_uv_coords;
in vec2 v_pos_coords;

void main(){
	//vec4 cc = texture2D(u_texture, v_uv_coords);
	//color = texture2D(u_texture, v_uv_coords);
	float len = length(v_pos_coords);
	vec2 offseto =abs(v_pos_coords);
	offseto = offseto/(15.0*clamp(0.5+sin(u_line*15)*sin(u_line * 31),0.5,1))*sign(v_pos_coords)*len*len;

	//offseto.x+=u_line*0.05-0.05;//slight shift in x direction for a good taste
	offseto.x+= u_line*0.05+sin(u_line * 17)*sin(u_line*8)*0.01;//slight shift in x direction for a good taste
	const float line_thiccness = 0.06;
	vec2 cor = v_uv_coords + offseto;
	float dist = 1.- clamp(abs(cor.y-u_line),0.0,line_thiccness)/line_thiccness;

	dist = smoothstep(0.0,1.0,dist);
	cor.y-=(cor.y-u_line)*dist;
	color = texture2D(u_texture,cor);
	cor.x -= 0.01*mod(u_line*3,1)-0.005;
	color += texture2D(u_texture,cor)*0.2;
	len += u_on_screen;
	
	float f = mod(cor.y*2,0.1); 
	f = f>0.05?0.0:1.0;
	f*=0.025;
	f+=dist*0.2;
	color.r+=f + dist*color.r/2;
	color.g+=f + dist*color.g/2;
	color.b+=f + dist*color.b/2;
	
	//fall in the slight dark
	f = mod(cor.y*4*clamp(u_line,0.0,1.0),0.1);
	f = f>0.05?0.0:1.0;
	f*=0.002;
	color.r-=f;
	color.g-=f;
	color.b-=f;

	color.rgb = color.rgb*(2.0-len*len);

	//if (length(v_pos_coords) > 0.9)
	//	color.rgb = vec3(0, 0, 0);

//	color.rgb = abs(vec3(v_pos_coords.x, 0, 0));
}

