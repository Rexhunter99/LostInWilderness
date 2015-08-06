#version 150 core

in vec4 	v_position;
in vec2		v_texcoord;

out vec2	f_texcoord

uniform mat4 mvp;

void main()
{
	f_texcoord = v_texcoord;
	gl_Position = mvp * vec4(v_position.xyz, 1);
}
