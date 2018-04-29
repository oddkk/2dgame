#version 330

layout(location=0) in vec2 in_vec;
layout(location=1) in vec2 in_uv;

uniform vec2 in_screen_size;
uniform vec2 in_tile_size;
uniform vec2 in_position;

out vec2 frag_uv;
out vec2 screen_position;

void main() {
	vec2 pos = vec2(0.0);

	pos += (in_vec * in_tile_size);
	pos += in_tile_size;
	pos -= in_screen_size;
	pos += in_position * 2;

	gl_Position = vec4(pos / in_screen_size, 0.0, 1.0);
	frag_uv = in_uv;
	screen_position = pos;
}
