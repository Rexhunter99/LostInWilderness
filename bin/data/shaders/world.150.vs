#version 150 core

#define MAX_LIGHTS 4

struct SLightParameters
{
	vec4 diffuse;
	vec4 ambient;
	float specular;
	vec4 position;
};

in vec4 v_position;
in vec3 v_normal;
in vec2 v_texcoord;
in vec4 v_diffuse;
float a_shininess = 0.0;

out vec2 f_texcoord;
out vec3 f_position;
out vec4 f_world_position;
out vec4 f_rgba;

uniform SLightParameters	g_SunLightSource;
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
		vec4 diffuse = g_SunLightSource.diffuse * max(dot(L, N), 0.0);
		vec4 specular = g_SunLightSource.diffuse * ( g_SunLightSource.specular * pow(max(dot(R, L), 0.0), a_shininess ) );
		f_rgba *= clamp( ambient + diffuse + specular, vec4(0.0,0.0,0.0,0.0), vec4(1.0,1.0,1.0,1.0) );
	}
	else
	{
		f_rgba = vec4( 1.0, 1.0, 1.0, 1.0 );
	}

	// -- Apply the model-view-projection matrix to the xyz components of the vertex coordinates
	gl_Position = mvp * vec4(v_position.xyz, 1);
}
