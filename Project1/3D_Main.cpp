// 3D quad drawer application
// Kevin Wong-Hua 
// Fall quarter graphics cpsc 4700 2021
/*
Keybinds:
Left mouse button - Enter camera mode
Escape - Enter edit mode
WASD - Move camera if in camera mode
Up,Down,Left,Right - Move cursor on grid (x and z axis)
J, K - Move cursor up/down (y axis)
Space - Places a vertex at current position
C - toggles between all the colors
*/

#include <iostream>

#include <glad/glad.h>
#include <glad/glad.c>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include "shader.h"

using namespace glm;

struct Vertex {
	vec3 pos;
	vec3 color;
};

struct Ray {
	vec3 position;
	vec3 direction;

	Ray(vec3 pos, vec3 dir) {
		position = pos;
		direction = dir;
	}
};

const int grid_size = 30;
GLFWwindow* window;
int width = 1366;
int height = 768;
mat4 proj;
mat4 view;
ivec3 cursor_pos = ivec3(0);
std::vector<Vertex> vertices;
bool is_edit_mode = true;

vec3 camera_front;
float deltaX = 90;
float deltaY;
float lastX;
float lastY;

int color_index;
std::vector<vec3> colors = {
	vec3(1, 0, 0),
	vec3(0, 1, 0),
	vec3(0, 0, 1),
	vec3(0.9, 0.5, 0),
	vec3(0.2, 0.5, 0.8),
};

GLint get_uniform_location(GLuint program, const char* name) {
	GLint location = glGetUniformLocation(program, name);
	if (location == -1) {
		printf("Failed to get unifrom location of :%s: uniform!\n", name);
		return 0;
	}

	return location;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	static bool first_mouse = true;
	// dont rotate camera if we are in edit mode
	if (is_edit_mode && !first_mouse) {
		return;
	}
	first_mouse = false;

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f; // change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	deltaX += xoffset;
	deltaY += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (deltaY > 89.0f)
		deltaY = 89.0f;
	if (deltaY < -89.0f)
		deltaY = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(deltaX)) * cos(glm::radians(deltaY));
	front.y = sin(glm::radians(deltaY));
	front.z = sin(glm::radians(deltaX)) * cos(glm::radians(deltaY));
	camera_front = glm::normalize(front);
}

void error_callback(int error, const char* description)
{
	printf("Glfw Error %d: %s\n", error, description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	// If not in edit mode, exit out
	if (!is_edit_mode)
		return;

	if (key ==  GLFW_KEY_UP && action == GLFW_PRESS) {
		cursor_pos.z += 1;
	}
	else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		cursor_pos.x += 1;
	}
	else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
		cursor_pos.z -= 1;
	}
	else if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		cursor_pos.x -= 1;
	}
	else if (key == GLFW_KEY_K && action == GLFW_PRESS) {
		cursor_pos.y += 1;
	}
	else if (key == GLFW_KEY_J && action == GLFW_PRESS) {
		cursor_pos.y -= 1;
	}
	else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		// If next vertex makes a quad
		static int quad_count = 0;
		int idx = vertices.size() - 1;

		if (((idx + 2) - (quad_count * 2)) % 4 == 0) {
			vertices.push_back(vertices[idx]);
			vertices.push_back(vertices[idx - 2]);
			quad_count += 1;
		}
		vertices.push_back(Vertex{ cursor_pos, colors[color_index] });
	}
	else if (key == GLFW_KEY_C && action == GLFW_PRESS) {
		if (color_index + 1 == colors.size() - 1) {
			color_index = 0;
		}
		else {
			color_index++;
		}
	}
}

GLuint create_buffer(std::vector<Vertex> vertices) {
	GLuint vbo;

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(vec3));

	glEnableVertexAttribArray(1);

	return vbo;
}

void draw(std::vector<Vertex> vertices, GLuint draw_mode) {
	GLuint temp_vbo;
	temp_vbo = create_buffer(vertices);
	glDrawArrays(draw_mode, 0, vertices.size());
	glDeleteBuffers(1, &temp_vbo);
}

void draw_grid_lines(GLuint shaderID, float cell_size) {

	mat4 model = translate(mat4(1), -(vec3(grid_size, 0.0f, grid_size) / 2.0f));
	GLuint location = get_uniform_location(shaderID, "model");
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(model));

	std::vector<Vertex> points;
	for (int i = 0; i <= grid_size; ++i) {
		points.push_back({
				vec3(i * cell_size, 0, 0),
				vec3(0.3),
			});
		points.push_back({
				vec3(i * cell_size, 0, grid_size * cell_size),
				vec3(0.3),
			});
		points.push_back({
				vec3(0, 0, i * cell_size),
				vec3(0.3),
			});
		points.push_back({
				vec3(grid_size * cell_size, 0, i * cell_size),
				vec3(0.3),
			});
	}
	draw(points, GL_LINES);
}

void draw_all_quads(GLuint shaderID, std::vector<Vertex> vertices) {

	mat4 model = mat4(1);
	GLuint location = get_uniform_location(shaderID, "model");
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(model));

	draw(vertices, GL_TRIANGLES);
}


void draw_quad(GLuint shaderID, vec3 pos, float size, vec3 color, float xrot = 0) {
	std::vector<Vertex> quad = {
		{
			vec3(-0.5, -0.5, 0),
			color,
		},
		{
			vec3(0.5, -0.5, 0),
			color,
		},
		{
			vec3(0.5, 0.5, 0),
			color,
		},
		{
			vec3(-0.5, -0.5, 0),
			color,
		},
		{
			vec3(0.5, 0.5, 0),
			color,
		},
		{
			vec3(-0.5, 0.5, 0),
			color,
		},
	};

	mat4 model = translate(mat4(1), pos) * rotate(mat4(1), xrot, vec3(1, 0, 0)) * scale(mat4(1), vec3(size));
	GLuint location = get_uniform_location(shaderID, "model");
	glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(model));

	draw(quad, GL_TRIANGLES);
}

int main() {

	// Initialise GLFW
	if (!glfwInit())
	{
		printf("Failed to initialize GLFW\n");
		return -1;
	}

	glfwSetErrorCallback(error_callback);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(width, height, "3D demo", NULL, NULL);
	if (window == NULL) {
		printf("Failed to open GLFW window.");
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		printf("Failed to initialize GLAD\n");
		return -1;
	}

	glfwSwapInterval(1); // Enable vsync
	//specify the size of the rendering window
	glViewport(0, 0, width, height);
	//glEnable(GL_DEPTH_TEST);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

    GLuint shaderID = load_shaders("vert.glsl", "frag.glsl");
	vec3 cam_pos = vec3(5, 20, -20);

	float last_time = 0;

	while (!glfwWindowShouldClose(window)) 
    {
		float time = glfwGetTime();
		float delta_time = time - last_time;
		last_time = time;

		if (!is_edit_mode) {
			float cam_speed = 10 * delta_time;
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
				cam_pos += cam_speed * camera_front;
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
				cam_pos -= cam_speed * camera_front;
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
				cam_pos -= glm::normalize(glm::cross(camera_front, vec3(0, 1, 0))) * cam_speed;
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
				cam_pos += glm::normalize(glm::cross(camera_front, vec3(0, 1, 0))) * cam_speed;
		}

        proj = infinitePerspective(radians(60.0f), (float)width/(float)height, 0.1f);
		view = lookAt(cam_pos, cam_pos + camera_front, vec3(0, 1, 0));
		mat4 view_proj = proj * view;

		glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderID);

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {
			is_edit_mode = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		if (glfwGetKey(window, GLFW_KEY_ESCAPE)) {
			is_edit_mode = true;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}


		GLuint location = get_uniform_location(shaderID, "view_proj");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(view_proj));

		draw_grid_lines(shaderID, 1);

		draw_all_quads(shaderID, vertices);

		draw_quad(shaderID, cursor_pos, 1, vec3(1), radians(90.0f));

		// Set view and proj to orthographic for drawing the current color quad
		view_proj = ortho(0.0f, (float)width, (float)height, 0.0f, -500.0f, 500.0f);

		location = get_uniform_location(shaderID, "view_proj");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(view_proj));

		draw_quad(shaderID, vec3(100), 100, colors[color_index]);

		glfwPollEvents();
		glfwSwapBuffers(window);
	}
}
