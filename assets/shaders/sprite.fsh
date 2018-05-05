#version 330

layout(location=0) out vec3 out_color;
uniform sampler2D in_texture;
uniform vec2 in_screen_size;
in vec2 frag_uv;
in vec2 screen_position;

void main() {
	vec3 color = texture(in_texture, frag_uv).xyz;

	if (color == vec3(1.0, 0.0, 1.0)) {
		discard;
	} else {
		out_color = color;
	}
}
