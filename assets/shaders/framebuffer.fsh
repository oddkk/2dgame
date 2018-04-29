#version 330

uniform sampler2D in_framebuffer;
uniform sampler2D in_dither;

uniform vec2 in_framebuffer_size;
uniform vec2 in_tile_size;

in vec2 frag_uv;

layout(location=0) out vec3 out_color;

void main() {
	// float ratio = in_framebuffer_size.x / in_framebuffer_size.y;
	// vec2 screen_pos = ((frag_uv * 2) - vec2(1)) * vec2(ratio, 1);
	// float power = clamp(1.0 - pow(length(screen_pos), 5), 0.0, 1.0);
	// vec3 dither = texture(in_dither, frag_uv * in_framebuffer_size / 16.0).xyz;
	// if (power <= dither.x) discard;

	out_color = texture(in_framebuffer, vec2(frag_uv.x, 1 - frag_uv.y)).xyz;
}
