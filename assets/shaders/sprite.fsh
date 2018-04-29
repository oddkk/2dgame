#version 330

layout(location=0) out vec3 out_color;
uniform sampler2D in_texture;
uniform vec2 in_screen_size;
in vec2 frag_uv;
in vec2 screen_position;

void main() {
	vec3 color = texture(in_texture, frag_uv).xyz;

	out_color = color;

	// float dist = pow(length(screen_position / in_screen_size.y), 2);

	// if (color.x < dist ) {
	// 	out_color = vec3(0);
	// } else {
	// 	out_color = vec3(1);
	// }
}
