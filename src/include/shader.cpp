#ifndef INCLUDE_SHADER_CPP
#define INCLUDE_SHADER_CPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h>

/**
 * Class for containing the details of the shader.
 */
class ShaderDetails
{
	// Let the shader manager access private variables.
	friend class ShaderManager;

private:
	// The name of the shader.
	std::string shaderName;
	// The file path to the vertex shader.
	std::string vertexShaderFilePath;
	// The file path to the geometry shader.
	std::string geometryShaderFilePath;
	// The file path to the fragment shader.
	std::string fragmentShaderFilePath;

	// The ID of the shader program.
	GLuint shaderId;

public:
	ShaderDetails(GLuint shaderId, std::string shaderName, std::string vertexShaderFilePath, std::string geometryShaderFilePath, std::string fragmentShaderFilePath)
			: shaderId(shaderId),
				shaderName(shaderName),
				vertexShaderFilePath(vertexShaderFilePath),
				geometryShaderFilePath(geometryShaderFilePath),
				fragmentShaderFilePath(fragmentShaderFilePath) {}

	/**
   * Get the name of the shader program.
   * 
   * @return The shader program name.
   */
	std::string getShaderName()
	{
		return shaderName;
	}

	/**
   * Get the ID of the shader program.
   * 
   * @return The shader program ID.
   */
	GLuint getShaderId()
	{
		return shaderId;
	}
};

/**
 * A manager class for managing shaders used by models.
 */
class ShaderManager
{
private:
	// Singleton instance of the shader manager.
	static ShaderManager instance;

	// A map of created shaders.
	std::map<std::string, std::shared_ptr<ShaderDetails>> namedShaders;
	// A map counting the references to the created shaders.
	std::map<std::string, int> namedShaderReferences;

	/**
	 * Read the shader code from the given shader file.
	 * 
	 * @param shaderName      The name of the shader program being loaded.
	 * @param shaderFilePath  The file path to the shader code.
	 * 
	 * @return The shader code.
	 */
	std::string loadShaderCode(std::string shaderName, std::string shaderFilePath)
	{
		// Create an input file stream for reading the shader file.
		std::ifstream shaderStream(shaderFilePath, std::ios::in);
		// Check if the input file stream is open.
		if (!shaderStream.is_open())
		{
			// Couldn't open the input file stream. Time to crash.
			std::cout << shaderName << std::endl
								<< "Failed at shader 1" << std::endl;
			exit(1);
		}

		// Define a stringstream to read the file contents into.
		std::stringstream sstr;
		// Push the contents of the input file stream into the string stream.
		sstr << shaderStream.rdbuf();
		// Close the input file stream.
		shaderStream.close();

		// Return the string contents of the string stream.
		return sstr.str();
	}

	/**
	 * Compiles the given shader code to the given shader.
	 * 
	 * @param shaderName  The name of the shader program being compiled.
	 * @param shaderCode  The shader code.
	 * @param shaderId    The ID of the shader.
	 */
	void compileShader(std::string shaderName, std::string shaderCode, GLuint shaderId)
	{
		// Convert the shader source code string into a character array.
		auto sourcePointer = shaderCode.c_str();
		// Set the source code of the shader code to the given shader code.
		glShaderSource(shaderId, 1, &sourcePointer, NULL);
		// Compile the shader.
		glCompileShader(shaderId);

		// Define variables for capturing the compilation result information.
		auto result = GL_FALSE;
		int infoLogLength;
		// Read the compilation result information for the shader.
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
		// Check if there is an error.
		if (infoLogLength > 0)
		{
			// There is an error. Time to crash.
			std::vector<char> shaderErrorMessage(infoLogLength + 1);
			glGetShaderInfoLog(shaderId, infoLogLength, NULL, &shaderErrorMessage[0]);
			std::cout << shaderName << std::endl
								<< &shaderErrorMessage[0] << std::endl
								<< "Failed at shader 2" << std::endl;
			exit(1);
		}
	}

	/**
	 * Creates a shader program using the list of given shaders (vertex, geometry, fragment).
	 * 
	 * @param shaderName  The name of the shader program being compiled.
	 * @param shaderIds   The IDs of the shaders to link together.
	 */
	GLuint createProgram(std::string shaderName, std::vector<GLuint> shaderIds)
	{
		// Create a new shader program.
		auto programId = glCreateProgram();
		// Iterate through the IDs of the shader.
		for (auto shaderId = shaderIds.begin(); shaderId != shaderIds.end(); shaderId++)
		{
			// Attach the shader to the main shader program.
			glAttachShader(programId, *shaderId);
		}
		// Link the shader together.
		glLinkProgram(programId);

		// Define variables for capturing the compilation result information.
		auto result = GL_FALSE;
		int infoLogLength;
		// Read the compilation result information for the shader program.
		glGetProgramiv(programId, GL_LINK_STATUS, &result);
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
		// Check if there is an error.
		if (infoLogLength > 0)
		{
			// There is an error. Time to crash.
			std::vector<char> programErrorMessage(infoLogLength + 1);
			glGetProgramInfoLog(programId, infoLogLength, NULL, &programErrorMessage[0]);
			std::cout << shaderName << std::endl
								<< &programErrorMessage[0] << std::endl
								<< "Failed at shader 3" << std::endl;
			exit(1);
		}

		// Return the ID of the created shader program.
		return programId;
	}

	/**
	 * Loads a shader program using the given vertex shader file and fragment shader file.
	 * 
	 * @param shaderName              The name of the shader program being loaded.
	 * @param vertexShaderFilePath    The file path to the vertex shader source code.
	 * @param fragmentShaderFilePath  The file path to the fragment shader source code.
	 * 
	 * @return The ID of the shader program.
	 */
	GLuint loadShaders(std::string shaderName, std::string vertexShaderFilePath, std::string fragmentShaderFilePath)
	{
		// Create a vertex shader.
		auto vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
		// Create a fragment shader.
		auto fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

		// Load the vertex shader code.
		auto vertexShaderCode = loadShaderCode(shaderName, vertexShaderFilePath);
		// Load the fragment shader code.
		auto fragmentShaderCode = loadShaderCode(shaderName, fragmentShaderFilePath);

		// Compile the vertex shader code.
		compileShader(shaderName, vertexShaderCode, vertexShaderId);
		// Compile the fragment shader code.
		compileShader(shaderName, fragmentShaderCode, fragmentShaderId);

		// Create the shader program using the vertex shader and fragment shader.
		auto programId = createProgram(shaderName, std::vector<GLuint>({vertexShaderId, fragmentShaderId}));

		// Detach and delete the vertex shader since it's no longer required.
		glDetachShader(programId, vertexShaderId);
		glDeleteShader(vertexShaderId);

		// Detach and delete the fragment shader since it's no longer required.
		glDetachShader(programId, fragmentShaderId);
		glDeleteShader(fragmentShaderId);

		// Return the shader program ID.
		return programId;
	}

	/**
	 * Loads a shader program using the given vertex shader file, geometry shader file and fragment shader file.
	 * 
	 * @param shaderName              The name of the shader program being loaded.
	 * @param vertexShaderFilePath    The file path to the vertex shader source code.
	 * @param geometryShaderFilePath  The file path to the geometry shader source code.
	 * @param fragmentShaderFilePath  The file path to the fragment shader source code.
	 * 
	 * @return The ID of the shader program.
	 */
	GLuint loadShaders(std::string shaderName, std::string vertexShaderFilePath, std::string geometryShaderFilePath, std::string fragmentShaderFilePath)
	{
		// Create a vertex shader.
		auto vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
		// Create a geometry shader.
		auto geometryShaderId = glCreateShader(GL_GEOMETRY_SHADER);
		// Create a fragment shader.
		auto fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

		// Load the vertex shader code.
		auto vertexShaderCode = loadShaderCode(shaderName, vertexShaderFilePath);
		// Load the geometry shader code.
		auto geometryShaderCode = loadShaderCode(shaderName, geometryShaderFilePath);
		// Load the fragment shader code.
		auto fragmentShaderCode = loadShaderCode(shaderName, fragmentShaderFilePath);

		// Compile the vertex shader code.
		compileShader(shaderName, vertexShaderCode, vertexShaderId);
		// Compile the geometry shader code.
		compileShader(shaderName, geometryShaderCode, geometryShaderId);
		// Compile the fragment shader code.
		compileShader(shaderName, fragmentShaderCode, fragmentShaderId);

		// Create the shader program using the vertex shader, geometry shader and fragment shader.
		auto programId = createProgram(shaderName, std::vector<GLuint>({vertexShaderId, geometryShaderId, fragmentShaderId}));

		// Detach and delete the vertex shader since it's no longer required.
		glDetachShader(programId, vertexShaderId);
		glDeleteShader(vertexShaderId);

		// Detach and delete the geometry shader since it's no longer required.
		glDetachShader(programId, geometryShaderId);
		glDeleteShader(geometryShaderId);

		// Detach and delete the fragment shader since it's no longer required.
		glDetachShader(programId, fragmentShaderId);
		glDeleteShader(fragmentShaderId);

		// Return the shader program ID.
		return programId;
	}

	ShaderManager()
			: namedShaders({}),
				namedShaderReferences({}) {}

public:
	// Preventing copying the shader manager, making sure only one instance can exist.
	ShaderManager(ShaderManager &) = delete;

	/**
	 * Load and create a shader program from the given shader file paths. If a shader program with the same name was already created,
	 * return the same shader program.
	 * 
	 * @param shaderName              The name of the shader program being loaded.
	 * @param vertexShaderFilePath    The file path to the vertex shader source code.
	 * @param fragmentShaderFilePath  The file path to the fragment shader source code.
	 * 
	 * @return The details of the loaded shader program.
	 */
	std::shared_ptr<ShaderDetails> &createShaderProgram(std::string shaderName, std::string vertexShaderFilePath, std::string fragmentShaderFilePath)
	{
		// Check if an shader program with the name already exists.
		auto existingShader = namedShaders.find(shaderName);
		if (existingShader != namedShaders.end())
		{
			// Shader already loaded. Increase its reference count and return it.
			namedShaderReferences[shaderName]++;
			return existingShader->second;
		}

		// Load the shader program and store its details.
		auto shaderProgramId = loadShaders(shaderName, vertexShaderFilePath, fragmentShaderFilePath);

		// Create a new shader program details with the captured data.
		auto newShader = std::make_shared<ShaderDetails>(shaderProgramId, shaderName, vertexShaderFilePath, "", fragmentShaderFilePath);

		// Insert the newly created shader program into the map of created shader programs.
		namedShaders[shaderName] = newShader;
		// Set the reference count of the shader program to 1.
		namedShaderReferences[shaderName] = 1;

		// Return the shader program details.
		return namedShaders[shaderName];
	}

	/**
	 * Load and create a shader program from the given shader file paths. If a shader program with the same name was already created,
	 * return the same shader program.
	 * 
	 * @param shaderName              The name of the shader program being loaded.
	 * @param vertexShaderFilePath    The file path to the vertex shader source code.
	 * @param geometryShaderFilePath  The file path to the geometry shader source code.
	 * @param fragmentShaderFilePath  The file path to the fragment shader source code.
	 * 
	 * @return The details of the loaded shader program.
	 */
	std::shared_ptr<ShaderDetails> &createShaderProgram(std::string shaderName, std::string vertexShaderFilePath, std::string geometryShaderFilePath, std::string fragmentShaderFilePath)
	{
		// Check if an shader program with the name already exists.
		auto existingShader = namedShaders.find(shaderName);
		if (existingShader != namedShaders.end())
		{
			// Shader already loaded. Increase its reference count and return it.
			namedShaderReferences[shaderName]++;
			return existingShader->second;
		}

		// Load the shader program and store its details.
		auto shaderProgramId = loadShaders(shaderName, vertexShaderFilePath, geometryShaderFilePath, fragmentShaderFilePath);

		// Create a new shader program details with the captured data.
		auto newShader = std::make_shared<ShaderDetails>(shaderProgramId, shaderName, vertexShaderFilePath, geometryShaderFilePath, fragmentShaderFilePath);

		// Insert the newly created shader program into the map of created shader programs.
		namedShaders[shaderName] = newShader;
		// Set the reference count of the shader program to 1.
		namedShaderReferences[shaderName] = 1;

		// Return the shader program details.
		return namedShaders[shaderName];
	}

	/**
   * Return the shader program created with the given name.
   * 
   * @param shaderName  The name of the shader program to return.
   * 
   * @return The shader program created with the given name.
   */
	std::shared_ptr<ShaderDetails> &getShaderDetails(std::string shaderName)
	{
		return namedShaders[shaderName];
	}

	/**
	 * Delete a reference to the shader program, and destroy it if no more references are present.
	 * 
	 * @param shaderDetails  The details of the shader program to destroy.
	 */
	void destroyShaderProgram(std::shared_ptr<ShaderDetails> &shaderDetails)
	{
		// Reduce the reference count of the shader program.
		namedShaderReferences[shaderDetails->getShaderName()]--;
		// Check if there are no more references to the shader program.
		if (namedShaderReferences[shaderDetails->getShaderName()] <= 0)
		{
			// No more references left, so time to clean.
			// Remove the shader program from the created shader programs references map.
			namedShaderReferences.erase(shaderDetails->getShaderName());
			// Remove the shader program from the created shader programs map.
			namedShaders.erase(shaderDetails->getShaderName());
			// Delete the shader program.
			glDeleteProgram(shaderDetails->shaderId);
		}
	}

	/**
   * Returns the singleton instance of the shader manager.
   * 
   * @return The shader manager singleton instance.
   */
	static ShaderManager &getInstance()
	{
		return instance;
	}
};

// Initialize the shader manager singleton instance static variable.
ShaderManager ShaderManager::instance;

#endif