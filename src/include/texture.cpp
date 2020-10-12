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
	// The ID of the texture containing the texture data.
	const GLuint textureId;

	// The name of the texture.
	const std::string textureName;
	// The file path to the texture data.
	const std::string textureFilePath;

public:
	TextureDetails(const GLuint &textureId, const std::string &textureName, const std::string &textureFilePath)
			: textureId(textureId),
				textureName(textureName),
				textureFilePath(textureFilePath) {}

	/**
   * Get the name of the texture.
   * 
   * @return The texture name.
   */
	const GLuint &getTextureId() const
	{
		return textureId;
	}

	/**
   * Get the ID of the shader program.
   * 
   * @return The shader program ID.
   */
	const std::string &getTextureName() const
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
	std::map<const std::string, const std::shared_ptr<const TextureDetails>> namedTextures;
	// A map counting the references to the created textures.
	std::map<const std::string, int> namedTextureReferences;

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
	GLuint create2dTexture(const unsigned char *const textureData, const unsigned int &width, const unsigned int &height)
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
	GLuint loadBmpTexture(const std::string &textureName, const std::string &textureFilePath)
	{
		// Define vectors for storing the BMP metadata information.
		unsigned char header[54];
		unsigned int dataPos;
		unsigned int imageSize;
		unsigned int width, height;

		// Open the BMP file.
		const auto file = fopen(textureFilePath.c_str(), "rb");
		// Check if the file is accessible.
		if (!file)
		{
			// Could not read the BMP file. Time to crash.
			std::cout << textureName << std::endl
								<< "Failed at texture 1" << std::endl;
			exit(1);
		}

		// Read the first 54 bytes of the file (contains the BMP header).
		const auto readBytes = fread(header, 1, 54, file);
		// Check if we managed to read the first 54 bytes.
		if (readBytes != 54)
		{
			// Could not read the BMP file. Time to crash.
			fclose(file);
			std::cout << textureName << std::endl
								<< "Failed at texture 2" << std::endl;
			exit(1);
		}
		// Check if the first two characters of the header start with "BM".
		if (header[0] != 'B' || header[1] != 'M')
		{
			// Invalid file. Time to crash.
			fclose(file);
			std::cout << textureName << std::endl
								<< "Failed at texture 3" << std::endl;
			exit(1);
		}
		// Check if number of bits per pixel is 24 (1 byte per color channel).
		if (*(int *)&(header[0x1C]) != 24)
		{
			// Cannot support color format. Time to crash.
			fclose(file);
			std::cout << textureName << std::endl
								<< "Failed at texture 4" << std::endl;
			exit(1);
		}
		// Check if compression is enabled.
		if (*(int *)&(header[0x1E]) != 0)
		{
			// Cannot support compressed BMPs. Time to crash.
			fclose(file);
			std::cout << textureName << std::endl
								<< "Failed at texture 5" << std::endl;
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
		const auto textureData = new unsigned char[imageSize];

		// Read the texture data from the file and store into the buffer.
		fread(textureData, 1, imageSize, file);

		// Close the file now that we're done reading it.
		fclose(file);

		// Create the 2D texture and store the texture data there.
		const auto textureId = create2dTexture(textureData, width, height);

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
	TextureManager(const TextureManager &) = delete;

	/**
	 * Load and create an texture from the given texture file path. If an texture with the same name was already created,
	 * return the same texture.
	 * 
	 * @param textureName      The name of the texture.
	 * @param textureFilePath  The file path to the texture data.
	 * 
	 * @return The details of the loaded texture.
	 */
	const std::shared_ptr<const TextureDetails> &create2dTexture(const std::string &textureName, const std::string &textureFilePath)
	{
		// Check if an texture with the name already exists.
		const auto existingTexture = namedTextures.find(textureName);
		if (existingTexture != namedTextures.end())
		{
			// Texture already loaded. Increase its reference count and return it.
			namedTextureReferences[textureName]++;
			return existingTexture->second;
		}

		// Load the BMP image file and store its details.
		const GLuint textureId = loadBmpTexture(textureName, textureFilePath);

		// Create a new texture details with the captured data.
		const auto newTexture = std::make_shared<const TextureDetails>(textureId, textureName, textureFilePath);

		// Insert the newly created texture into the map of created textures.
		namedTextures.insert(std::make_pair(textureName, newTexture));
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
	const std::shared_ptr<const TextureDetails> &getTextureDetails(const std::string &textureName) const
	{
		return namedTextures.at(textureName);
	}

	/**
	 * Delete a reference to the texture, and destroy it if no more references are present.
	 * 
	 * @param textureDetails  The details of the texture to destroy.
	 */
	void destroyTexture(const std::shared_ptr<const TextureDetails> &textureDetails)
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