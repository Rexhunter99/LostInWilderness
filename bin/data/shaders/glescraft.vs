#version 130


uniform mat4	u_view;

in vec4		v_position;
in vec3		v_normal;
in vec2		v_texcoord;
in vec4		v_diffuse;

out vec2	f_texcoord;
out vec3	f_position;
out vec4	f_world_position;
out vec4	f_rgba;

void main(void)
{
	mat4 projection = mat4(1.0);
	mat4 mvp = u_view * projection;

	// -- Attribute manipulation
	f_texcoord = v_texcoord;
	f_position = (mvp * vec4(v_position.xyz, 1.0) ).xyz;
	f_world_position = v_position;

	f_rgba = v_diffuse;

	// -- Apply the model-view-projection matrix to the xyz components of the vertex coordinates
	gl_Position = mvp * vec4(v_position.xyz, 1);
}
