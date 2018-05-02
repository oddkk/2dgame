#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include "glad/glad_glx.h"
#include "int.h"
#include "opengl.h"
#include "utils.h"
#include "world.h"
#include "render.h"
#include "texture_map.h"
#include "editor.h"
#include "chunk_loader.h"

bool should_exit = false;

static int context_error_handler(Display *display, XErrorEvent *ev) {
	print_error("glx", "Something went wrong while creating OpenGL context.");
	return 0;
}

int main(int argc, char **argv) {
	(void)argc;
	(void)argv;

	Display *display;
	Window root_window, window, glx_window;
	int screen;
	GLXFBConfig *fb_configs;
	GLXFBConfig fb_config;
	XVisualInfo *visual;
	XSetWindowAttributes attributes = {0};
	int attributes_mask;
	Colormap colormap;
	GLXContext gl_context = NULL;
	Atom WM_DELETE_WINDOW;
	int window_width = 800, window_height = 600;

	display = XOpenDisplay(0);

	if (!display) {
		panic("Could not connect to X server.");
	}

	screen = XDefaultScreen(display);
	root_window = XRootWindow(display, screen);

	if (!gladLoadGLX(display, screen)) {
		panic("Failed to initialize glx!");
	}

	const int fb_attributes[] = {
		GLX_RENDER_TYPE,   GLX_RGBA_BIT,
		GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
		GLX_DOUBLEBUFFER, True,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_DEPTH_SIZE, 16,
		None
	};

	int fb_config_count;
	fb_configs = glXChooseFBConfig(
		display, screen, fb_attributes, &fb_config_count
	);

	if (!fb_configs) {
		panic("Could not choose frame buffer config.");
	}

	fb_config = fb_configs[0];

	visual = glXGetVisualFromFBConfig(display, fb_config);

	if (!fb_configs) {
		panic("Could not get visual from frame buffer configuration.");
	}

	colormap = XCreateColormap(
		display, root_window, visual->visual, AllocNone
	);

	attributes.colormap = colormap;
	attributes.background_pixmap = None;
	attributes.border_pixmap = None;
	attributes.border_pixel = 0;
	attributes.event_mask =
		  StructureNotifyMask
		| EnterWindowMask
		| LeaveWindowMask
		| ExposureMask
		| ButtonPressMask
		| ButtonReleaseMask
		| OwnerGrabButtonMask
		| KeyPressMask
		| KeyReleaseMask
		| FocusChangeMask;

	attributes_mask =
		  CWColormap
		| CWBorderPixel
		| CWEventMask;

	window = XCreateWindow(
		display,
		root_window,
		0, 0, window_width, window_height, // x, y, width, height
		0, // border
		visual->depth,
		InputOutput, // class
		visual->visual,
		attributes_mask,
		&attributes
	);

	if (!window) {
		panic("Could not create X window.");
	}

	int glx_attributes[] = {None};

	glx_window = glXCreateWindow(
		display, fb_config, window, glx_attributes
	);

	if (!glx_window) {
		panic("Could not create GLX window.");
	}

	WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", False);
	Atom protocols[] = {WM_DELETE_WINDOW};
	XSetWMProtocols(display, window, protocols,
					sizeof(protocols) / sizeof(Atom));


	XMapWindow(display, window);
	XStoreName(display, window, "Game");

	if (GLX_ARB_create_context) {
		int context_attributes[] = {
			GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
			GLX_CONTEXT_MINOR_VERSION_ARB, 0,
			None
		};
		int (*old_error_handler)(Display*, XErrorEvent*);
		old_error_handler = XSetErrorHandler(&context_error_handler);
		gl_context = glXCreateContextAttribsARB(
				display, fb_config, 0, True, context_attributes
			);
		XSync(display, False);
		XSetErrorHandler(old_error_handler);
	}

	if (!gl_context) {
		print_error("glx", "Falling back to glXCreateContext.");
		gl_context = glXCreateContext(
			display, visual, NULL, GL_TRUE
		);
	}

	if (!gl_context) {
		panic("Could not create OpenGL context.");
	}

	bool context_ok = glXMakeCurrent(display, window, gl_context);

	if (!context_ok) {
		panic("Could not activate the OpenGL context.");
	}

	if (!gladLoadGL()) {
		panic("Failed to initialize gl!");
	}

	const GLubyte *version = glGetString(GL_VERSION);
	print_debug("OpenGL version: %s", version);

	setup_gl_debug_output();

	glClearColor(0.0, 0.0, 0.0, 1.0);

	bool btn_north = false;
	bool btn_south = false;
	bool btn_east  = false;
	bool btn_west  = false;

	(void)btn_north;
	(void)btn_south;
	(void)btn_east;
	(void)btn_west;

	struct world world = {};
	struct chunk test_chunk = {};
	struct chunk test_chunk2 = {};
	struct texture_map texture_map = {};
	struct render_context render_ctx = {};

	world.chunks[1][0] = &test_chunk;
	world.chunks[0][0] = &test_chunk2;

	load_chunk(&test_chunk,  &texture_map, 0,-1, 0);
	load_chunk(&test_chunk2, &texture_map, 0, 0, 0);

	render_ctx.texture_map = &texture_map;

	render_context_init(&render_ctx);
	render_size_change(&render_ctx, window_width, window_height);

	tex_id arrow;

	arrow  = load_texture_from_file(&texture_map, STR("assets/tiles/arrow.tile"));


	struct entity *test_entity;

	test_entity = allocate_entity(&world);

	test_entity->x = 0;
	test_entity->y = 0;

	test_entity->width = 16;
	test_entity->height = 16;

	test_entity->num_tiles_x = 1;
	test_entity->num_tiles_y = 1;
	test_entity->tex_id = arrow;

	glDisable(GL_DEPTH_TEST);

	while (!should_exit) {
		XEvent event;

		while (XPending(display) > 0) {
			bool key_down = false;
			XNextEvent(display, &event);
			switch (event.type) {
			case ConfigureNotify:
				if (event.xconfigure.width != window_width ||
					event.xconfigure.height != window_height) {
					window_width = event.xconfigure.width;
					window_height = event.xconfigure.height;

					render_size_change(&render_ctx, window_width, window_height);
				}
				break;

			case ClientMessage:
				if (event.xclient.data.l[0] == (long)WM_DELETE_WINDOW) {
					should_exit = true;
				}
				break;

			case KeyPress:
				key_down = true;
			case KeyRelease: {
				KeySym keysym;
				unsigned int mods;

				XkbLookupKeySym(display, event.xkey.keycode, 0, &mods, &keysym);
				switch (keysym) {
				case 'w': btn_north = key_down; break;
				case 's': btn_south = key_down; break;
				case 'd': btn_east  = key_down; break;
				case 'a': btn_west  = key_down; break;
				}
			} break;

			case FocusOut:
				btn_north = false;
				btn_south = false;
				btn_east  = false;
				btn_west  = false;
				break;
			}
		}

		int diff_x = 0;
		int diff_y = 0;

		if (btn_north) diff_y -= 1;
		if (btn_south) diff_y += 1;

		if (btn_west)  diff_x -= 1;
		if (btn_east)  diff_x += 1;

		test_entity->x += diff_x;
		test_entity->y += diff_y;

		world.camera_x = test_entity->x + (test_entity->num_tiles_x * TILE_SIZE) / 2;
		world.camera_y = test_entity->y + (test_entity->num_tiles_y * TILE_SIZE) / 2;

		render(&render_ctx, &world);

		glXSwapBuffers(display, window);
	}
}
