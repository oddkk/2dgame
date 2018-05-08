#version 330

layout(location=0) out vec3 out_color;
uniform sampler2D in_texture;
uniform vec2 in_screen_size;
uniform uint in_texture_id;
uniform vec2 in_texmap_sprites;
in vec2 frag_uv;
in vec2 screen_position;

void main() {
	vec2 uv = vec2(float(in_texture_id % uint(in_texmap_sprites.x)),
				   float(in_texture_id / uint(in_texmap_sprites.y)));
	uv /= in_texmap_sprites;
	uv += frag_uv / in_texmap_sprites;
	vec3 color = texture(in_texture, uv).xyz;

	if (color == vec3(1.0, 0.0, 1.0)) {
		discard;
	} else {
		out_color = color;
	}
}
