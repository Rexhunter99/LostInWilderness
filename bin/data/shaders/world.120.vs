#version 120

#define MAX_LIGHTS 4

struct SLightParameters
{
	vec4	diffuse;
	vec4	ambient;
	float	specular;
	vec4	position;
};

attribute vec4 v_position;
attribute vec3 v_normal;
attribute vec2 v_texcoord;
attribute vec4 v_diffuse;

varying vec2 f_texcoord;
varying vec3 f_position;
varying vec4 f_world_position;
varying vec4 f_rgba;

uniform SLightParameters	g_SunLightSource;// = SLightParameters( vec4(1.0,1.0,1.0,1.0), vec4(0.0,0.0,0.0,0.0), 1.0, vec4(0.0,0.0,0.0,0.0) );
uniform mat4 mvp;
uniform int b_lighting;


void main(void)
{
	// -- Attribute manipulation
	f_texcoord = v_texcoord;
	f_position = (mvp * vec4(v_position.xyz, 1.0) ).xyz;
	f_world_position = v_position;
	f_rgba = v_diffuse;

	// -- Lighting calculations
	vec3 N = normalize(v_normal);
	vec3 V = normalize(f_position);
	vec3 R = reflect(V, N);
	vec3 L = g_SunLightSource.position.xyz;

	if ( b_lighting == 1 )
	{
		vec4 ambient = g_SunLightSource.ambient;
		vec4 diffuse = ( v_diffuse * g_SunLightSource.diffuse ) * max(dot(L, N), 0.0);
		vec4 specular = v_position.w * g_SunLightSource.diffuse * ( pow(max(dot(R, L), 0.0), g_SunLightSource.specular ) );
		f_rgba = clamp( ambient + diffuse + specular, vec4(0.0), vec4(1.0) );
	}
	else
	{
		f_rgba = vec4( 1.0 );
	}

	// -- Apply the model-view-projection matrix to the xyz components of the vertex coordinates
	gl_Position = mvp * vec4(v_position.xyz, 1);
}
