#include "engine_custom.h"
#define num_inst 3
#define RES 16

#define SCR_WIDTH 640
#define SCR_HEIGHT 360

int main() {
	jmp_buf error;
	if (setjmp(error)) {
		printf("Error Detected!\n");
		return 1;
	}

	Camera *cam = malloc(sizeof(Camera));
	camera_init(cam, (vec3){0, 0, 1}, 0, 270);

	State game = state_init(error, SCR_WIDTH, SCR_HEIGHT, "game");
	unsigned int program = program_init(error, "src/user/vertex_in.glsl", "src/user/textured.glsl");


	vec2 instanced_positions[num_inst] = {
		{0, 0}, {16, 4}, {32, 8}
	};

	vec2 instanced_spr_num[num_inst] = {
		{0, 0}, {0, 0}, {0, 0}
	};
	Sprite spr = sprite_init(error, (vec3){0, 0, 0}, 1, "assets/smiley.png", 16, 16);
	buffers_init(&spr.plane);
	instanced_buffers_init(&spr.plane, instanced_positions, instanced_spr_num, num_inst, true);


	unsigned int fb_texture;
	glGenTextures(1, &fb_texture);
	glBindTexture(GL_TEXTURE_2D, fb_texture);

	//parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


	unsigned char data[4 * 2 * 2] = {
		0, 255, 0, 1,
		0, 0, 255, 1,
		255, 0, 0, 1,
		0, 0, 255, 1
	};

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	double lastFrame = glfwGetTime();
	float pos[2];
	while (!glfwWindowShouldClose(game.window)) {
		double currentFrame = glfwGetTime();
		float dt = (float)(currentFrame - lastFrame);
		lastFrame = currentFrame;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.1, 0.1, 0.2, 1);
		glfwPollEvents();

		float speed = 50 * dt;
		key_input(game.window, cam, speed);

		if (glfwGetMouseButton(game.window, GLFW_MOUSE_BUTTON_LEFT)) {
			int width, height;
			glfwGetWindowSize(game.window, &width, &height);
			//printf("%d | %d\n", width, height);

			glBindBuffer(GL_ARRAY_BUFFER, spr.plane.instance_UV_VBO);

			double posx, posy;
			glfwGetCursorPos(game.window, &posx, &posy);

			pos[0] = posx;
			pos[1] = posy;

			pos[0] /= width;
			pos[1] /= height;

			pos[0] *= SCR_WIDTH;
			pos[1] *= SCR_HEIGHT;

			pos[1] *= -1;
			pos[1] += SCR_HEIGHT;

			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 2, pos);
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			printf("%d\n", (int)pos[1]);

		}


		glUseProgram(program);
		matrix_init(&game, program, "2D", SCR_WIDTH, SCR_HEIGHT);
		camera_rotate(cam, cam->yaw, cam->pitch, game.view_uniform.value.m4);
		uniform_send_to_gpu(&game.view_uniform, program, "view");

		spr.plane.texture = fb_texture;
		glBindTexture(GL_TEXTURE_2D, spr.plane.texture);
		
		int is_atlas_b = 0;
		Uniform is_atlas = uniform_set_data(&is_atlas_b, UNIFORM_INT1);
		uniform_send_to_gpu(&is_atlas, program, "is_atlas");

		sprite_draw(&spr, program, 1);

		glfwSwapBuffers(game.window);

	}

	glDeleteProgram(program);
	sprite_delete(&spr);
	free(cam);

	glfwTerminate();
	return 0;
}
