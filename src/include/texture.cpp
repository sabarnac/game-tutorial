#ifndef INCLUDE_TEXTURE_CPP
#define INCLUDE_TEXTURE_CPP

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <memory>

#include <GL/glew.h>

/**
 * Class for containing the details of the shader.
 */
class TextureDetails
{
	// Let the shader manager access private variables.
	friend class TextureManager;

private:
	// The name of the texture.
	std::string textureName;
	// The file path to the texture data.
	std::string textureFilePath;

	// The ID of the texture containing the texture data.
	GLuint textureId;

public:
	TextureDetails(GLuint textureId, std::string textureName, std::string textureFilePath)
			: textureId(textureId),
				textureName(textureName),
				textureFilePath(textureFilePath) {}

	/**
   * Get the name of the texture.
   * 
   * @return The texture name.
   */
	GLuint getTextureId()
	{
		return textureId;
	}

	/**
   * Get the ID of the shader program.
   * 
   * @return The shader program ID.
   */
	std::string getTextureName()
	{
		return textureName;
	}
};

/**
 * A manager class for managing textures used by models.
 */
class TextureManager
{
private:
	// Singleton instance of the texture manager.
	static TextureManager instance;

	// A map of created textures.
	std::map<std::string, std::shared_ptr<TextureDetails>> namedTextures;
	// A map counting the references to the created textures.
	std::map<std::string, int> namedTextureReferences;

	/**
	 * Create a 2D texture of the given width and height, and store the data of the texture.
	 * 
	 * @param textureName  The name of the texture being loaded.
	 * @param textureData  The data of the texture to store.
	 * @param width        The width of the texture.
	 * @param height       The height of the texture.
	 * 
	 * @return The ID of the texture.
	 */
	GLuint create2dTexture(std::string textureName, unsigned char *textureData, unsigned int width, unsigned int height)
	{
		// Define a variable for storing the texture ID.
		GLuint textureId;
		// Create a new texture and store the ID.
		glGenTextures(1, &textureId);
		// Bind the texture as a 2D texture.
		glBindTexture(GL_TEXTURE_2D, textureId);
		// Create a 2D RGB texture image on the bounded texture.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, textureData);

		// Provide parameters for behaviour when reading coordinates that are out-of-bounds,
		//   as well as algorithms to use for maginifcation and minification.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		// Generate mip-maps for the texture.
		glGenerateMipmap(GL_TEXTURE_2D);

		// Unbind the texture now that we're done.
		glBindTexture(GL_TEXTURE_2D, 0);

		// Return the ID of the created texture.
		return textureId;
	}

	/**
	 * Load the BMP image and create a texture for it.
	 * 
	 * @param textureName      The name of the texture being loaded.
	 * @param textureFilePath  The file path to the texture data.
	 * 
	 * @return The ID of the texture.
	 */
	GLuint loadBmpTexture(std::string textureName, std::string textureFilePath)
	{
		// Define vectors for storing the BMP metadata information.
		unsigned char header[54];
		unsigned int dataPos;
		unsigned int imageSize;
		unsigned int width, height;

		// Open the BMP file.
		auto file = fopen(textureFilePath.c_str(), "rb");
		// Check if the file is accessible.
		if (!file)
		{
			// Could not read the BMP file. Time to crash.
			std::cout << "Failed at texture 1" << std::endl;
			exit(1);
		}

		// Read the first 54 bytes of the file (contains the BMP header).
		auto readBytes = fread(header, 1, 54, file);
		// Check if we managed to read the first 54 bytes.
		if (readBytes != 54)
		{
			// Could not read the BMP file. Time to crash.
			fclose(file);
			std::cout << "Failed at texture 2" << std::endl;
			exit(1);
		}
		// Check if the first two characters of the header start with "BM".
		if (header[0] != 'B' || header[1] != 'M')
		{
			// Invalid file. Time to crash.
			fclose(file);
			std::cout << "Failed at texture 3" << std::endl;
			exit(1);
		}
		// Check if number of bits per pixel is 24 (1 byte per color channel).
		if (*(int *)&(header[0x1C]) != 24)
		{
			// Cannot support color format. Time to crash.
			fclose(file);
			std::cout << "Failed at texture 4" << std::endl;
			exit(1);
		}
		// Check if compression is enabled.
		if (*(int *)&(header[0x1E]) != 0)
		{
			// Cannot support compressed BMPs. Time to crash.
			fclose(file);
			std::cout << "Failed at texture 5" << std::endl;
			exit(1);
		}

		// Grab the BMP metadata information
		dataPos = *(int *)&(header[0x0A]);
		imageSize = *(int *)&(header[0x22]);
		width = *(int *)&(header[0x12]);
		height = *(int *)&(header[0x16]);

		// Some BMP files can be misformatted, so guess missing information.
		if (imageSize == 0)
		{
			// Image size would be width times height. But since each pixel contains 3 bytes of information
			//   (one per color channel), multiply that result by 3.
			imageSize = width * height * 3;
		}
		if (dataPos == 0)
		{
			// Data should start right after the BMP header, which is located at the start of the file and is 54 bytes in size.
			// So read from that point after.
			dataPos = 54;
		}

		// Create a buffer for storing the texture RBG data.
		auto textureData = new unsigned char[imageSize];

		// Read the texture data from the file and store into the buffer.
		fread(textureData, 1, imageSize, file);

		// Close the file now that we're done reading it.
		fclose(file);

		// Create the 2D texture and store the texture data there.
		auto textureId = create2dTexture(textureName, textureData, width, height);

		// Delete the texture data file we're storing since we no longer need it.
		delete[] textureData;

		// Return the ID of the created texture.
		return textureId;
	}

	TextureManager()
			: namedTextures({}),
				namedTextureReferences({}) {}

public:
	// Preventing copying the texture manager, making sure only one instance can exist.
	TextureManager(TextureManager &) = delete;

	/**
	 * Load and create an texture from the given texture file path. If an texture with the same name was already created,
	 * return the same texture.
	 * 
	 * @param textureName      The name of the texture.
	 * @param textureFilePath  The file path to the texture data.
	 * 
	 * @return The details of the loaded texture.
	 */
	std::shared_ptr<TextureDetails> &create2dTexture(std::string textureName, std::string textureFilePath)
	{
		// Check if an texture with the name already exists.
		auto existingTexture = namedTextures.find(textureName);
		if (existingTexture != namedTextures.end())
		{
			// Texture already loaded. Increase its reference count and return it.
			namedTextureReferences[textureName]++;
			return existingTexture->second;
		}

		// Load the BMP image file and store its details.
		GLuint textureId = loadBmpTexture(textureName, textureFilePath);

		// Create a new texture details with the captured data.
		auto newTexture = std::make_shared<TextureDetails>(textureId, textureName, textureFilePath);

		// Insert the newly created texture into the map of created textures.
		namedTextures[textureName] = newTexture;
		// Set the reference count of the texture to 1.
		namedTextureReferences[textureName] = 1;

		// Return the texture details.
		return namedTextures[textureName];
	}

	/**
   * Return the texture created with the given name.
   * 
   * @param textureName  The name of the texture to return.
   * 
   * @return The texture created with the given name.
   */
	std::shared_ptr<TextureDetails> &getTextureDetails(std::string textureName)
	{
		return namedTextures[textureName];
	}

	/**
	 * Delete a reference to the texture, and destroy it if no more references are present.
	 * 
	 * @param textureDetails  The details of the texture to destroy.
	 */
	void destroyTexture(std::shared_ptr<TextureDetails> &textureDetails)
	{
		// Reduce the reference count of the texture.
		namedTextureReferences[textureDetails->getTextureName()]--;
		// Check if there are no more references to the texture.
		if (namedTextureReferences[textureDetails->getTextureName()] <= 0)
		{
			// No more references left, so time to clean.
			// Remove the texture from the created textures references map.
			namedTextureReferences.erase(textureDetails->getTextureName());
			// Remove the texture from the created textures map.
			namedTextures.erase(textureDetails->getTextureName());
			// Delete the texture containing the texture data.
			glDeleteTextures(1, &textureDetails->textureId);
		}
	}

	/**
   * Returns the singleton instance of the texture manager.
   * 
   * @return The texture manager singleton instance.
   */
	static TextureManager &getInstance()
	{
		return instance;
	}
};

// Initialize the texture manager singleton instance static variable.
TextureManager TextureManager::instance;

#endif