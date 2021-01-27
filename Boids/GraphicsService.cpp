#include "GraphicsService.h"

ControlService* GraphicsService::cService = nullptr;

GLuint GraphicsService::loadBMP_custom(const char* imagepath)
{
	uint8_t header[54]; // 54 Byte Header each BMP file has
	// uint32_t data_pos; // Pointer to the start of data
	uint32_t width, height;
	uint32_t image_size; // width*height*3
	uint8_t* data; // the actual image data

	FILE* file = fopen(imagepath, "rb");
	if (!file)
	{
		std::cout << "Failed to load bmp, cancelling." << std::endl;
		return false;
	}
	
	if (fread(header, 1, 54, file) != 54)
	{
		std::cout << "Not an actual .bmp file, cancelling." << std::endl;
		fclose(file);
		return false;
	}

	if (header[0] != 'B' || header[1] != 'M')
	{
		std::cout << "Not an actual .bmp file, cancelling." << std::endl;
		fclose(file);
		return false;
	}


	// gets relevant information from the bmp file
	//data_pos = *(int*) & (header[0x0A]); // not quite sure why we need this.
	image_size = *(int*) & (header[0x22]);
	width = *(int*) & (header[0x12]);
	height = *(int*) & (header[0x16]);
	

	std::cout << "Read bmp file " << imagepath << std::endl;
	std::cout << width << " wide and " << height << " high" << std::endl;
	
	if (!image_size)
		image_size = width * height * 3;
	/*if (!data_pos)
		data_pos = 54; // Defined in the standard
*/

	data = new uint8_t[image_size];
	fread(data, 1, image_size, file); //read all the data
	fclose(file);
	
	GLuint textureID;
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	glGenerateMipmap(GL_TEXTURE_2D);
	
	
	return textureID;
}

GLuint GraphicsService::loadDDS(const char* imagepath)
{
	uint8_t header[124]; // defined by the standard

	FILE* fp;

	fp = fopen(imagepath, "rb");
	if (fp == NULL)
		return 0;

	char filecode[4];
	fread(filecode, 1, 4, fp);
	if (strncmp(filecode, "DDS", 3) != 0) // C-Style because the tutorial ... or I don't even know
	{
		fclose(fp);
		std::cout << "DDS file read error" << std::endl;
		return 0;
	}

	fread(&header, 124, 1, fp);

	uint32_t height = *(uint32_t*)&(header[8]);
	uint32_t width = *(uint32_t*) & (header[12]);
	uint32_t linear_size = *(uint32_t*) & (header[16]);
	uint32_t mip_map_count = *(uint32_t*) & (header[24]);
	uint32_t four_cc = *(uint32_t*) & (header[80]);

	uint8_t* buffer;
	uint32_t buffer_size;

	buffer_size = mip_map_count > 1 ? linear_size * 2 : linear_size;
	buffer = (uint8_t*)malloc(buffer_size * sizeof(uint8_t));
	fread(buffer, 1, buffer_size, fp);
	fclose(fp);

	uint32_t components = (four_cc == 0x31545844) ? 3 : 4; // 0x31545844 = DXT1 in ascii
	uint32_t format;

	switch (four_cc)
	{
	case 0x31545844: // DXT1
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case 0x33545844: // DXT3
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case 0x35545844: // DXT5
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	default:
		free(buffer);
		return 0;
	}

	GLuint textureID;
	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	uint32_t block_size = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
	uint32_t offset = 0;

	// loading mipmaps

	for (uint32_t level = 0; level < mip_map_count && (width || height); ++level)
	{
		uint32_t size = ((width + 3) / 4) * ((height + 3) / 4) * block_size;
		glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height, 0, size, buffer + offset);

		offset += size;
		width /= 2;
		height /= 2;
		
		// if texture isn't power of 2:
		if (width < 1) width = 1;
		if (height < 1) height = 1;
	}
	free(buffer);

	return textureID;
}

// TODO Refactor this and determine the correct order of function calls
void GraphicsService::initialize() {
	std::cout << "Initializing GLFW..." << std::endl;

	GraphicsService::setControlService(ControlService::getInstance());
	GraphicsService::cService->initialize();

	glfwSetErrorCallback(error_callback);

	if (!glfwInit()) { // GLFW did not successfully initialize
		exit(EXIT_FAILURE);
	}

	// GLFW successfully initialized

	glfwWindowHint(GLFW_SAMPLES, 4); //4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	window = glfwCreateWindow(1024, 768, "Cooler Titel", NULL, NULL);

	if (!window) { // Window creation failed
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	ControlService::setWindow(window);

	glfwSetKeyCallback(window, key_callback);

	glfwMakeContextCurrent(window);

	// Should we use a loader library, this is where to initialize it
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK) {
		std::cout << "Failed to initialize GLEW" << std::endl;
		exit(EXIT_FAILURE);
	}
	glfwSetCursorPos(window, 1024 / 2, 768 / 2); // TODO: Fix this meme

	std::cout << "Successfully initialized GLFW" << std::endl;
}


glm::vec3 cubePositions[] = {
	glm::vec3(0.0f,  0.0f,  0.0f),
	glm::vec3(2.0f,  5.0f, -15.0f),
	glm::vec3(-1.5f, -2.2f, -2.5f),
	glm::vec3(-3.8f, -2.0f, -12.3f),
	glm::vec3(2.4f, -0.4f, -3.5f),
	glm::vec3(-1.7f,  3.0f, -7.5f),
	glm::vec3(1.3f, -2.0f, -2.5f),
	glm::vec3(1.5f,  2.0f, -2.5f),
	glm::vec3(1.5f,  0.2f, -1.5f),
	glm::vec3(-1.3f,  1.0f, -1.5f)
};

void GraphicsService::run() {
	
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f); // Set Background color

	// Enable depth test
	glEnable(GL_DEPTH_TEST);

	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Tells the gpu to not draw triangles behind the camera
	glEnable(GL_CULL_FACE);

	GLuint vertexArrayID;
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = this->loadShaders("SimpleVertexShader.glsl", "SimpleFragmentShader.glsl");
	
	// Get a handle for our "MVP" uniform
	GLuint matrixID = glGetUniformLocation(programID, "MVP");
	GLuint viewMatrixID = glGetUniformLocation(programID, "V");
	GLuint modelMatrixID = glGetUniformLocation(programID, "M");
	
	// Load the texture
	GLuint texture = loadDDS("uvmap.DDS");

	// Get a handle for our "Texture" uniform
	GLuint textureId = glGetUniformLocation(programID, "myTextureSampler");

	// Read our .obj file
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;
	bool res = ObjectLoader::loadOBJ("Hammer.obj", vertices, uvs, normals);

	// Loading Vertices
	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	// Loading UVs
	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

	// Loading Normals
	GLuint normalBuffer;
	glGenBuffers(1, &normalBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
	glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);

	// Get a handle for our "LightPosition" uniform
	glUseProgram(programID);
	GLuint LightID = glGetUniformLocation(programID, "LightPostion_worldspace");

	double lastTime = glfwGetTime();
	int nbFrames = 0;

	unsigned long counter = 0;

	while (!glfwWindowShouldClose(window)) {
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0) {
			printf("%f ms/frame\n", 1000.0 / double(nbFrames));
			nbFrames = 0;
			lastTime += 1.0;
		}

		counter++;


		// Clearing the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// User shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		GraphicsService::cService->ComputeMatricesFromInput();
		glm::mat4 ProjectionMatrix = ControlService::getProjectionMatrix();
		glm::mat4 ViewMatrix = ControlService::getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		//ModelMatrix = glm::translate(ModelMatrix, glm::vec3(counter / 100.0, 0, 0));
		glm::mat4 mvp = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader in the "MVP" uniform
		glUniformMatrix4fv(matrixID, 1, GL_FALSE, &mvp[0][0]);
		glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		// Lighting
		glm::vec3 lightPos = glm::vec3(4, 4, 4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		// Set our texture sampler to use Texture Unit 0
		glUniform1i(textureId, 0);
		
		// Setup Vertex Buffer
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glVertexAttribPointer(
			0, 
			3,
			GL_FLOAT, 
			GL_FALSE, 
			0, 
			(void*) 0);

		// Setup UV Buffer
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,
			2,
			GL_FLOAT, 
			GL_FALSE,
			0, 
			(void*) 0);

		// Normal Buffer
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, normalBuffer);
		glVertexAttribPointer(
			2,
			3, 
			GL_FLOAT, 
			GL_FALSE,
			0, 
			(void*) 0);

		
		for (unsigned int i = 0; i < 10; i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			if (i == 4) {
				mvp = glm::translate(mvp, glm::vec3(counter / 100.0, 0, 0));
			} else {
				mvp = glm::translate(mvp, cubePositions[i]);
			}

			float angle = 20.0f * i;
			mvp = glm::rotate(mvp, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			glUniformMatrix4fv(matrixID, 1, GL_FALSE, &mvp[0][0]);

			glDrawArrays(GL_TRIANGLES, 0, vertices.size());
		}
		
		//mvp = glm::translate(mvp, glm::vec3(5, -10, 0));
		//glUniformMatrix4fv(matrixID, 1, GL_FALSE, &mvp[0][0]);

		glDrawArrays(GL_TRIANGLES, 0, vertices.size());

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();
	}

	glDeleteBuffers(1, &vertexBuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &normalBuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &texture);
	glDeleteVertexArrays(1, &vertexArrayID);

	glfwDestroyWindow(window);
	glfwTerminate();
}

void GraphicsService::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	GraphicsService& instance = GraphicsService::getInstance();
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	
	// TODO Read in arrow keys for movement in scene
}

GLuint GraphicsService::loadShaders(const char* vertex_file_path, const char* fragment_file_path) {
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
	} else {
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}

	// Link the program
	printf("Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}