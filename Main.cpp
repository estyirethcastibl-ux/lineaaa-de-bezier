
#include<filesystem>
namespace fs = std::filesystem;
//------------------------------

#include"Model.h"


const unsigned int width = 800;
const unsigned int height = 800;


unsigned int samples = 8;


float gamma = 2.2f;


float rectangleVertices[] =
{

	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f,

	 1.0f,  1.0f,  1.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f
};


static float cameraFOV = 45.0f;


static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	cameraFOV -= (float)yoffset * 2.0f; // scale scroll sensitivity
	cameraFOV = glm::clamp(cameraFOV, 15.0f, 90.0f);
}


std::vector<Vertex> vertices =
{
	Vertex{glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)},
	Vertex{glm::vec3(-1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 1.0f)},
	Vertex{glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(1.0f, 1.0f)},
	Vertex{glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(1.0f, 0.0f)}
};


std::vector<GLuint> indices =
{
	0, 1, 2,
	0, 2, 3
};

int main()
{
	// Initialize GLFW
	glfwInit();


	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(width, height, "YoutubeOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetScrollCallback(window, scroll_callback);

	gladLoadGL();

	glViewport(0, 0, width, height);





	// Generates shaders
	Shader shaderProgram("default.vert", "default.frag", "default.geom");
	Shader framebufferProgram("framebuffer.vert", "framebuffer.frag");

	// Take care of all the light related things
	glm::vec4 lightColor = glm::vec4(100.0f, 100.0f, 100.0f, 1.0f);
	glm::vec3 lightPos = glm::vec3(0.5f, 0.5f, 0.5f);

	shaderProgram.Activate();
	glUniform4f(glGetUniformLocation(shaderProgram.ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(shaderProgram.ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);
	framebufferProgram.Activate();
	glUniform1i(glGetUniformLocation(framebufferProgram.ID, "screenTexture"), 0);
	glUniform1f(glGetUniformLocation(framebufferProgram.ID, "gamma"), gamma);




	// Enables the Depth Buffer
	glEnable(GL_DEPTH_TEST);

	// Enables Multisampling
	glEnable(GL_MULTISAMPLE);

	// Enables Cull Facing
	glEnable(GL_CULL_FACE);
	// Keeps front faces
	glCullFace(GL_FRONT);
	// Uses counter clock-wise standard
	glFrontFace(GL_CCW);


	// Creates camera object
	Camera camera(width, height, glm::vec3(0.0f, 0.0f, 2.0f));



	// Prepare framebuffer rectangle VBO and VAO
	unsigned int rectVAO, rectVBO;
	glGenVertexArrays(1, &rectVAO);
	glGenBuffers(1, &rectVBO);
	glBindVertexArray(rectVAO);
	glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), &rectangleVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	// Prepare VAO/VBO for drawing the Bezier curve (dynamic)
	unsigned int curveVAO, curveVBO;
	glGenVertexArrays(1, &curveVAO);
	glGenBuffers(1, &curveVBO);
	glBindVertexArray(curveVAO);
	glBindBuffer(GL_ARRAY_BUFFER, curveVBO);
	glBufferData(GL_ARRAY_BUFFER, 0, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);



	// Variables to create periodic event for FPS displaying
	double prevTime = 0.0;
	double crntTime = 0.0;
	double timeDiff;
	// Tracks time between frames for smooth input (seconds)
	double lastFrameTime = glfwGetTime();
	// Keeps track of the amount of frames in timeDiff
	unsigned int counter = 0;



	// Create Frame Buffer Object
	unsigned int FBO;
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	// Create Framebuffer Texture
	unsigned int framebufferTexture;
	glGenTextures(1, &framebufferTexture);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferTexture);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGB16F, width, height, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Prevents edge bleeding
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferTexture, 0);

	// Create Render Buffer Object
	unsigned int RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);


	// Error checking framebuffer
	auto fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer error: " << fboStatus << std::endl;

	// Create Frame Buffer Object
	unsigned int postProcessingFBO;
	glGenFramebuffers(1, &postProcessingFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, postProcessingFBO);

	// Create Framebuffer Texture
	unsigned int postProcessingTexture;
	glGenTextures(1, &postProcessingTexture);
	glBindTexture(GL_TEXTURE_2D, postProcessingTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, postProcessingTexture, 0);

	// Error checking framebuffer
	fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fboStatus != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Post-Processing Framebuffer error: " << fboStatus << std::endl;




	// Paths to textures
	std::string parentDir = (fs::current_path().fs::path::parent_path()).string();
	std::string diffusePath = "/Resources/YoutubeOpenGL 27 - Normal Maps/textures/diffuse.png";
	std::string normalPath = "/Resources/YoutubeOpenGL 27 - Normal Maps/textures/normal.png";
	std::string displacementPath = "/Resources/YoutubeOpenGL 28 - Parallax Occlusion Mapping/textures/displacement.png";

	std::vector<Texture> textures =
	{
		Texture((parentDir + diffusePath).c_str(), "diffuse", 0)
	};

	// Plane with the texture
	Mesh plane(vertices, indices, textures);
	// Normal map for the plane
	Texture normalMap((parentDir + normalPath).c_str(), "normal", 1);
	Texture displacementMap((parentDir + displacementPath).c_str(), "displacement", 2);

	// (No model loaded here; we will move the existing plane along the Bezier curve)

	// Bezier curve control points and duration for animation
	glm::vec3 bezierP0 = glm::vec3(-1.5f, -0.5f, 0.0f);
	glm::vec3 bezierP1 = glm::vec3(-0.5f, 1.0f, 0.0f);
	glm::vec3 bezierP2 = glm::vec3(0.5f, -1.0f, 0.0f);
	glm::vec3 bezierP3 = glm::vec3(1.5f, 0.5f, 0.0f);
	float bezierDuration = 6.0f; // seconds to complete the curve

	// Cubic Bezier evaluation lambda (returns position at t in [0,1])
	auto cubicBezier = [](const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float t)
		{
			float u = 1.0f - t;
			return (u * u * u) * p0 + (3.0f * u * u * t) * p1 + (3.0f * u * t * t) * p2 + (t * t * t) * p3;
		};



	// Main while loop
	while (!glfwWindowShouldClose(window))
	{
		// Updates counter and times
		crntTime = glfwGetTime();
		timeDiff = crntTime - prevTime;
		counter++;

		if (timeDiff >= 1.0 / 30.0)
		{
			// Creates new title
			std::string FPS = std::to_string((1.0 / timeDiff) * counter);
			std::string ms = std::to_string((timeDiff / counter) * 1000);
			std::string newTitle = "YoutubeOpenGL - " + FPS + "FPS / " + ms + "ms";
			glfwSetWindowTitle(window, newTitle.c_str());

			// Resets times and counter
			prevTime = crntTime;
			counter = 0;

			// Use this if you have disabled VSync
			//camera.Inputs(window);
		}


		// Bind the custom framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		// Specify the color of the background
		glClearColor(pow(0.07f, gamma), pow(0.13f, gamma), pow(0.17f, gamma), 1.0f);
		// Clean the back buffer and depth buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		// Enable depth testing since it's disabled when drawing the framebuffer rectangle
		glEnable(GL_DEPTH_TEST);

		// Handles camera inputs (delete this if you have disabled VSync)
		camera.Inputs(window);
		// per-frame delta for keyboard zoom
		double now = glfwGetTime();
		float deltaFrame = (float)(now - lastFrameTime);
		lastFrameTime = now;
		// Keyboard zoom: Z to zoom in, X to zoom out
		if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		{
			cameraFOV -= 30.0f * deltaFrame; // degrees per second
		}
		if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		{
			cameraFOV += 30.0f * deltaFrame;
		}
		cameraFOV = glm::clamp(cameraFOV, 15.0f, 90.0f);
		// Updates and exports the camera matrix to the Vertex Shader using current FOV
		camera.updateMatrix(cameraFOV, 0.1f, 100.0f);


		shaderProgram.Activate();
		normalMap.Bind();
		glUniform1i(glGetUniformLocation(shaderProgram.ID, "normal0"), 1);
		displacementMap.Bind();
		glUniform1i(glGetUniformLocation(shaderProgram.ID, "displacement0"), 2);

		// Compute position along Bezier curve and draw the model translated there
		float t = fmod((float)crntTime, bezierDuration) / bezierDuration; // normalized [0,1]
		glm::vec3 bezierPos = cubicBezier(bezierP0, bezierP1, bezierP2, bezierP3, t);
		// Draw the plane translated along the Bezier curve
		plane.Draw(shaderProgram, camera, glm::mat4(1.0f), bezierPos);

		// Draw Bezier curve as a line strip for visualization (not culled)
		std::vector<glm::vec3> curvePoints;
		const int curveResolution = 128; // increase for smoother curve
		for (int i = 0; i <= curveResolution; ++i)
		{
			float tt = (float)i / (float)curveResolution;
			curvePoints.push_back(cubicBezier(bezierP0, bezierP1, bezierP2, bezierP3, tt));
		}

		// Upload curve points
		glBindBuffer(GL_ARRAY_BUFFER, curveVBO);
		glBufferData(GL_ARRAY_BUFFER, curvePoints.size() * sizeof(glm::vec3), curvePoints.data(), GL_DYNAMIC_DRAW);
		// Use simple shader to draw line (reuse framebufferProgram or shaderProgram depending on availability)
		// We'll use shaderProgram but set model/view/proj as needed
		shaderProgram.Activate();
		// push identity model and camera matrix already set by camera.updateMatrix
		glDisable(GL_CULL_FACE);
		glLineWidth(2.0f);
		glBindVertexArray(curveVAO);
		glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)curvePoints.size());
		glBindVertexArray(0);
		glEnable(GL_CULL_FACE);

		// Make it so the multisampling FBO is read while the post-processing FBO is drawn
		glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, postProcessingFBO);
		// Conclude the multisampling and copy it to the post-processing FBO
		glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);


		// Bind the default framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// Draw the framebuffer rectangle
		framebufferProgram.Activate();
		glBindVertexArray(rectVAO);
		glDisable(GL_DEPTH_TEST); // prevents framebuffer rectangle from being discarded
		glBindTexture(GL_TEXTURE_2D, postProcessingTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);


		// Swap the back buffer with the front buffer
		glfwSwapBuffers(window);
		// Take care of all GLFW events
		glfwPollEvents();
	}



	// Delete all the objects we've created
	shaderProgram.Delete();
	glDeleteFramebuffers(1, &FBO);
	glDeleteFramebuffers(1, &postProcessingFBO);
	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();
	return 0;
}