#version 330

layout(location=0) in vec2 in_vec;
layout(location=1) in vec2 in_uv;

uniform vec2 in_screen_size;
uniform vec2 in_framebuffer_size;
uniform vec2 in_framebuffer_screen_size;
out vec2 frag_uv;

void main() {
	gl_Position = vec4(in_vec * in_framebuffer_screen_size, 0.0, 1.0);
	frag_uv = in_uv;
}
