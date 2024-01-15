
#include <iostream>
#include <cstdlib>
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

// GLM Libraries

#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/matrix_transform.hpp>
#include <glm/glm/gtc/type_ptr.hpp>

#include <SOIL2/SOIL2.h> 

using namespace std;

int width, height;
const double PI = 3.14159;
const float toRadians = PI / 180.0f;


// Input Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

//Declare View matrix
glm::mat4 viewMatrix;

// Initialize FOV
GLfloat fov = 45.f;

// Define Camera Attributes;
glm::vec3 cameraPosition = glm::vec3(0.f, 0.f, 3.f);
glm::vec3 target = glm::vec3(0.f, 0.f, 0.f);
glm::vec3 cameraDirection = glm::normalize(cameraPosition - target);
glm::vec3 worldUp = glm::vec3(0.f, 1.f, 0.f);
glm::vec3 cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));
glm::vec3 cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
glm::vec3 cameraFront = glm::normalize(glm::vec3(0.f, 0.f, -1.f));

//Declare target prototype
glm::vec3 getTarget();

// Camera Transformation Prototype
void TransformCamera();

// Boolean for keys and mouse buttons
bool keys[1024], mouseButtons[3];

// Boolean to Check Camera Transformations
bool isPanning = false, isOrbiting = false;

// Radius, Pitch, and Yaw
GLfloat radius = 3.f, rawYaw = 0.f, rawPitch = 0.f, degYaw, degPitch;


GLfloat deltaTime = 0.f, lastFrame = 0.f;
GLfloat lastX = 320, lastY = 240, xChange, yChange;

bool firstMouseMove = true; // Detect initial mouse movement

// Light Source Position
glm::vec3 lightPosition(0.0f, 1.0f, 0.0f);


void initCamera();


// Draw Primitive(s)
void draw()
{
	GLenum mode = GL_TRIANGLES;
	GLsizei indices = 6;
	glDrawElements(mode, indices, GL_UNSIGNED_BYTE, nullptr);


}

// Create and Compile Shaders
static GLuint CompileShader(const string& source, GLuint shaderType)
{
	// Create Shader object
	GLuint shaderID = glCreateShader(shaderType);
	const char* src = source.c_str();

	// Attach source code to Shader object
	glShaderSource(shaderID, 1, &src, nullptr);

	// Compile Shader
	glCompileShader(shaderID);

	// Return ID of Compiled shader
	return shaderID;

}

// Create Program Object
static GLuint CreateShaderProgram(const string& vertexShader, const string& fragmentShader)
{
	// Compile vertex shader
	GLuint vertexShaderComp = CompileShader(vertexShader, GL_VERTEX_SHADER);

	// Compile fragment shader
	GLuint fragmentShaderComp = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);

	// Create program object
	GLuint shaderProgram = glCreateProgram();

	// Attach vertex and fragment shaders to program object
	glAttachShader(shaderProgram, vertexShaderComp);
	glAttachShader(shaderProgram, fragmentShaderComp);

	// Link shaders to create executable
	glLinkProgram(shaderProgram);

	// Delete compiled vertex and fragment shaders
	glDeleteShader(vertexShaderComp);
	glDeleteShader(fragmentShaderComp);

	// Return Shader Program
	return shaderProgram;

}


int main(void)
{
	width = 640; height = 480;

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(width, height, "Main Window", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	// Set input Callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);



	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK)
		cout << "Error!" << endl;

	GLfloat lampVertices[] = {
		-0.5, -0.5, 0.0, // index 0
		-0.5, 0.5, 0.0, // index 1
		0.5, -0.5, 0.0,  // index 2
		0.5, 0.5, 0.0  // index 3

	};



	GLfloat vertices[] = {

		// Triangle 1
		-0.5, -0.5, 0.0, // index 0
		1.0, 0.0, 0.0, // red
		0.0, 0.0, // UV (bl) - applying texture
		0.0f, 0.0f, 1.0f, //normal positive z

		-0.5, 0.5, 0.0, // index 1
		0.0, 1.0, 0.0, // green
		0.0, 1.0, // UV (tl) - applying texture
		0.0f, 0.0f, 1.0f, //normal positive z

		0.5, -0.5, 0.0,  // index 2	
		0.0, 0.0, 1.0, // blue
		1.0, 0.0, // UV (br) - applying texture
		0.0f, 0.0f, 1.0f, //normal positive z

		// Triangle 2	
		0.5, 0.5, 0.0,  // index 3	
		1.0, 0.0, 1.0, // purple
		1.0, 1.0, // UV (tr) - applying texture
		0.0f, 0.0f, 1.0f, //normal positive z
	};

	// Define element indices
	GLubyte indices[] = {
		0, 1, 2,
		1, 2, 3
	};

	// plane positions
	glm::vec3 planePositions[] =
	{
		glm::vec3(-5.0f, -5.0f, -5.0f),  // Square plane under 3D objects from 2D picture
		glm::vec3(5.0f, -5.0f, -5.0f),
		glm::vec3(5.0f, -5.0f, 5.0f),
		glm::vec3(-5.0f, -5.0f, 5.0f)
	};

	// Plane rotations
	glm::float32 planeRotationsY[] =
	{
	0.0f, 90.0f, 0.0f, 90.0f
	};

	glm::float32 planeRotationsX[] =
	{
	45.0f, 45.0f, -45.0f, -45.0f
	};


	//Enable Depth Buffer
	glEnable(GL_DEPTH_TEST);

	// Wireframe mode
	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);



	GLuint cubeVBO, cubeEBO, cubeVAO, floorVBO, floorEBO, floorVAO, lampVBO, lampEBO, lampVAO;

	glGenBuffers(1, &cubeVBO); // Create VBO
	glGenBuffers(1, &cubeEBO); // Create EBO

	glGenBuffers(1, &floorVBO); // Create VBO
	glGenBuffers(1, &floorEBO); // Create EBO

	glGenBuffers(1, &lampVBO); // Create VBO
	glGenBuffers(1, &lampEBO); // Create EBO


	glGenVertexArrays(1, &cubeVAO); // Create VOA
	glGenVertexArrays(1, &floorVAO); // Create VOA
	glGenVertexArrays(1, &lampVAO); // Create VOA

	glBindVertexArray(cubeVAO);

	// VBO and EBO Placed in User-Defined VAO
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO); // Select EBO


	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Load indices 

	// Specify attribute location and layout to GPU
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)


	glBindVertexArray(floorVAO);

	glBindBuffer(GL_ARRAY_BUFFER, floorVBO); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO); // Select EBO

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Load indices 

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);


	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
	glEnableVertexAttribArray(3);

	glBindVertexArray(0);

	// Define Lamp VAO
	glBindVertexArray(lampVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lampVBO); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lampEBO); // Select EBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(lampVertices), lampVertices, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Load indices 
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);


	//Load Textures
	int brownTexWidth, brownTexHeight, browntexTexWidth, browntexTexHeight;
	unsigned char* brownImage = SOIL_load_image("brown.png", &brownTexWidth, &brownTexHeight, 0, SOIL_LOAD_RGB);
	unsigned char* whiteImage = SOIL_load_image("white.png", &browntexTexWidth, &browntexTexHeight, 0, SOIL_LOAD_RGB);


	// Generate Textures
	GLuint brownTexture;
	glGenTextures(1, &brownTexture);
	glBindTexture(GL_TEXTURE_2D, brownTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, brownTexWidth, brownTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, brownImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(brownImage);
	glBindTexture(GL_TEXTURE_2D, 0);


	// Generate Textures
	GLuint browntexTexture;
	glGenTextures(1, &browntexTexture);
	glBindTexture(GL_TEXTURE_2D, browntexTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, browntexTexWidth, browntexTexHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, whiteImage);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(whiteImage);
	glBindTexture(GL_TEXTURE_2D, 0);


	// Vertex shader source code
	string vertexShaderSource =
		"#version 330 core\n"
		"layout(location = 0) in vec3 vPosition;"
		"layout(location = 1) in vec3 aColor;"
		"layout(location = 2) in vec2 texCoord;"
		"layout(location = 3) in vec3 normal;"
		"out vec3 oColor;"
		"out vec2 oTexCoord;"
		"out vec3 oNormal;"
		"out vec3 FragPos;"
		"uniform mat4 model;"
		"uniform mat4 view;"
		"uniform mat4 projection;"
		"void main()\n"
		"{\n"
		"gl_Position = projection * view * model * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);"
		"oColor = aColor;"
		"oTexCoord = texCoord;"
		"oNormal = mat3(transpose(inverse(model))) * normal;"
		"FragPos = vec3(model * vec4(vPosition, 1.0f));"
		"}\n";

	// Fragment shader source code
	string fragmentShaderSource =
		"#version 330 core\n"
		"in vec3 oColor;"
		"in vec2 oTexCoord;"
		"in vec3 oNormal;"
		"in vec3 FragPos;"
		"out vec4 fragColor;"
		"uniform sampler 2D myTexture;"
		"uniform vec3 objectColor;"
		"uniform vec3 lightColor;"
		"uniform vec3 lightPos";
	"uniform vec3 viewPos;"
		"void main()\n"
		"{\n"
		"//Ambient\n"
		"float ambientStrength = 0.3f;"
		"vec3 ambient = ambientStrength * lightColor;"
		"//Diffuse\n"
		"vec 3 norm = normalize(oNormal);"
		"vec3 lightDir = normalize(lightPos - FragPos);"
		"float diff = max(dot(norm, lightDir), 0.0);"
		"vec3 diffuse = diff * lightColor;"
		"//Specularity\n"
		"float specularStrength = 3.5f;"
		"vec3 viewDir = normalize(viewPos - FragPos);"
		"vec3 reflectDir = reflect(-lightDir, norm);"
		"float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);"
		"vec3 specular = specularStrength * spec * lightColor;"
		"vec result = (ambient + diffuse + specular) * objectColor;"
		"fragColor = texture(myTexture, oTexCoord) * vec4(result, 1.0f);"
		"}\n";


	// Lamp Vertex shader source code
	string lampVertexShaderSource =
		"#version 330 core\n"
		"layout(location = 0) in vec3 vPosition;"
		"uniform mat4 model;"
		"uniform mat4 view;"
		"uniform mat4 projection;"
		"void main()\n"
		"{\n"
		"gl_Position = projection * view * model * vec4(vPosition.x, vPosition.y, vPosition.z, 1.0);"
		"}\n";


	// Lamp Fragment shader source code
	string lampFragmentShaderSource =
		"#version 330 core\n"
		"out vec4 fragColor;"
		"void main()\n"
		"{\n"
		"fragColor = vec4(1.0f);" // white light
		"}\n";

	// Creating Shader Program
	GLuint shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
	GLuint lampShaderProgram = CreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource);

	// Use Shader Program exe once
	//glUseProgram(shaderProgram);


	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		//Set delta time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;


		// Resize window and graphics simultaneously
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use Shader Program exe and select VAO before drawing 
		glUseProgram(shaderProgram); // Call Shader per-frame when updating attributes

		//Declare identity matrix

		glm::mat4 projectionMatrix;


		//Initialize Transforms

		// modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));


		viewMatrix = glm::lookAt(cameraPosition, getTarget(), worldUp);


		projectionMatrix = glm::perspective(fov, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

		// Get matrix's uniform location and set matrix
		GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
		GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
		GLint projLoc = glGetUniformLocation(shaderProgram, "projection");


		//Get light and object color and light position location
		GLint objectColorLoc = glGetUniformLocation(shaderProgram, "objectColor");
		GLint lightColorLoc = glGetUniformLocation(shaderProgram, "lightColor");
		GLint lightPosLoc = glGetUniformLocation(shaderProgram, "lightColor");
		GLint viewPosLoc = glGetUniformLocation(shaderProgram, "viewPos");

		//Assign Light and Object Colors
		glUniform3f(objectColorLoc, 0.46f, 0.36f, 0.25f);
		glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);


		// Set light Position
		glUniform3f(lightPosLoc, lightPosition.x, lightPosition.y, lightPosition.z);


		//Specify view Position
		glUniform3f(viewPosLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);


		// Pass transform to Shader

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));




		glBindVertexArray(cubeVAO); // User-defined VAO must be called before draw. 

		for (GLuint i = 0; i < 4;)
		{
			glm::mat4 modelMatrix;

			modelMatrix = glm::translate(modelMatrix, planePositions[i]);

			modelMatrix = glm::rotate(modelMatrix, planeRotationsY[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			modelMatrix = glm::rotate(modelMatrix, planeRotationsX[i] * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));

			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

			//Draw primitives
			draw();

		}






		// Unbind Shader exe and VOA after drawing per frame
		glBindVertexArray(0); //Incase different VAO wii be used after

		glUseProgram(lampShaderProgram);
		GLint lampModelLoc = glGetUniformLocation(lampShaderProgram, "model");
		GLint lampViewLoc = glGetUniformLocation(lampShaderProgram, "view");
		GLint lampProjLoc = glGetUniformLocation(lampShaderProgram, "projection");
		glUniformMatrix4fv(lampViewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(lampProjLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));


		glBindVertexArray(lampVAO); // User-defined VAO must be called before draw.

		glUseProgram(0); // Incase different shader will be used after

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();

		// Poll camera transformations
		TransformCamera();
	}

	//Clear GPU resources
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &cubeEBO);
	glDeleteVertexArrays(1, &floorVAO);
	glDeleteBuffers(1, &floorVBO);
	glDeleteBuffers(1, &floorEBO);


	glfwTerminate();
	return 0;
}

// Define Input Callback Functions
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// Display ASCII Keycode
	// cout << "ASCI: " << key << endl;
	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	/*
	// Display scroll offset
	if (yoffset > 0)
		cout << "Scroll up: ";
	if (yoffset < 0)
		cout << "Scroll down: ";

	cout << yoffset << endl;
	*/

	// Clamp FOV
	if (fov >= 1.f && fov <= 45.f)
		fov -= yoffset * 0.01f;

	// Default FOV
	if (fov < 1.f)
		fov = 1.f;

	if (fov > 45.f)
		fov = 45.f;



}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	/*
	// Detect mouse button clicks
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		cout << "LMB clicked!" << endl;
	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
		cout << "MMB clicked!" << endl;
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		cout << "RMB clicked!" << endl;

	*/

	if (action == GLFW_PRESS)
		mouseButtons[button] = true;
	else if (action == GLFW_RELEASE)
		mouseButtons[button] = false;
}
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	// Display mouse x and y coordinates.
	// cout << "Mouse x: " << xpos << endl;
	// cout << "Mouse y: " << ypos << endl;

	if (firstMouseMove)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouseMove = false;
	}

	// Calculate cursor offset
	xChange = xpos - lastX;
	yChange = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	// Pan Camera

	if (isPanning)
	{

		if (cameraPosition.z < 0.f)
			cameraFront.z = 1.f;
		else
			cameraFront.z = -1.f;



		GLfloat cameraSpeed = xChange * deltaTime;
		cameraPosition += cameraSpeed * cameraRight;

		cameraSpeed = yChange * deltaTime;
		cameraPosition += cameraSpeed * cameraUp;
	}

	// Orbit Camera
	if (isOrbiting)
	{
		rawYaw += xChange;
		rawPitch += yChange;

		//Convert Taw and Pitch to degrees
		degYaw = glm::radians(rawYaw);
		//degPitch = glm::radians(rawPitch);
		degPitch = glm::clamp(glm::radians(rawPitch), -glm::pi<float>() / 2.f + .1f, glm::pi<float>() / 2.f - .1f);

		// Azimuth Altitude formula
		cameraPosition.x = target.x + radius * cosf(degPitch) * sin(degYaw);
		cameraPosition.y = target.y + radius * sinf(degPitch);
		cameraPosition.z = target.z + radius * cosf(degPitch) * cosf(degYaw);



	}


}

//Define getTarget function
glm::vec3 getTarget()
{
	if (isPanning)
		target = cameraPosition + cameraFront;

	return target;
}

// Define TransformManmera function
void TransformCamera()
{
	// Pan camera
	if (keys[GLFW_KEY_LEFT_ALT] && mouseButtons[GLFW_MOUSE_BUTTON_MIDDLE])
		isPanning = true;
	else
		isPanning = false;

	// Orbit Camera
	if (keys[GLFW_KEY_LEFT_ALT] && mouseButtons[GLFW_MOUSE_BUTTON_LEFT])
		isOrbiting = true;
	else
		isOrbiting - false;

	// Reset Camera
	if (keys[GLFW_KEY_P])
		initCamera();

}

void initCamera()
{
	cameraPosition = glm::vec3(0.f, 0.f, 3.f);
	target = glm::vec3(0.f, 0.f, 0.f);
	cameraDirection = glm::normalize(cameraPosition - target);
	worldUp = glm::vec3(0.f, 1.f, 0.f);
	cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));
	cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
	cameraFront = glm::normalize(glm::vec3(0.f, 0.f, -1.f));

}

