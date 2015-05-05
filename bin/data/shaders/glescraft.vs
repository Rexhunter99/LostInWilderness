#version 130

#define MAX_LIGHTS 4

struct SLightParameters
{
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 position;
};

uniform SLightParameters	g_LightSource[MAX_LIGHTS];
uniform mat4 mvp;

in vec4 v_position;
in vec3 v_normal;
in vec2 v_texcoord;
in vec4 v_diffuse;

out vec2 f_texcoord;
out vec3 f_position;
out vec4 f_world_position;
out vec4 f_rgba;

void main(void)
{
	// -- Attribute manipulation
	f_texcoord = v_texcoord;
	f_position = (mvp * vec4(v_position.xyz, 1.0) ).xyz;
	f_world_position = v_position;

	// -- Lighting calculations
	vec3 N = normalize(v_normal);
	vec3 V = normalize(f_position);
	vec3 R = reflect(V, N);
	vec3 L = g_LightSource[0].position.xyz;

	vec4 ambient = g_LightSource[0].ambient;
	vec4 diffuse = g_LightSource[0].diffuse * max(dot(L, N), 0.0);
	vec4 specular = g_LightSource[0].specular * pow(max(dot(R, L), 0.0), gl_FrontMaterial.shininess );
	f_rgba *= clamp( ambient + diffuse + specular, vec4(0.0,0.0,0.0,0.0), vec4(1.0,1.0,1.0,1.0) );

	// -- Apply the model-view-projection matrix to the xyz components of the vertex coordinates
	gl_Position = mvp * vec4(v_position.xyz, 1);
}
