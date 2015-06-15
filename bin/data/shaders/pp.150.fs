#version 150 core

#define FLAG_FXAA					1
#define FLAG_SMAA					2
#define FLAG_SSAO					4
#define FLAG_BLOOM					8


in vec2		vf_TexCoord;

out vec4	o_FragColor;

uniform int			u_Flags = 0;
uniform sampler2D	u_FrameBuffer;
uniform vec2		u_Resolution = vec2( 960, 640 );

#include "fxaa.150.fs"
//#include "smaa.150.fs"
//#include "ssao.150.fs"

void main( )
{
	vec4 diffuse = texture2D( u_FrameBuffer, vf_TexCoord );

	// -- Check if FXAA is enabled
	if ( u_Flags & FLAG_FXAA )
	{
		diffuse = getFXAA( u_FrameBuffer, vf_TexCoord, u_Resolution );
	}

	o_FragColor = diffuse;
}
