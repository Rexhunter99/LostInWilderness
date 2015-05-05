#version 130

in vec3 f_position;
in vec2 f_texcoord;
in vec4 f_rgba;
in vec4 f_world_position;
uniform sampler2D texture;
uniform int b_lighting;
out vec4 frag_color;

const vec4 fogcolor = vec4(0.6, 0.8, 1.0, 1.0);
const float fogdensity = .00003;

void main(void)
{
	vec4 texture_diffuse = texture2D( texture, f_texcoord );
	vec4 color = texture_diffuse * f_rgba;
	color.a = texture_diffuse.a;

	if ( f_world_position.w > 50.0f )
		color.rgb = texture_diffuse.rgb;

	// -- Perform Alpha Key
	if ( texture_diffuse.a < 0.1 )
		discard;

	// Calculate strength of fog
	//float z = gl_FragCoord.z / gl_FragCoord.w;
	//float fog = clamp(exp(-fogdensity * z * z), 0.2, 1.0);

	// Final color is a mix of the actual color and the fog color
	//frag_color = mix(fogcolor, color, fog);
	frag_color = color;
}
