#ifndef INCLUDE_TEXT_CPP
#define INCLUDE_TEXT_CPP

#include <iostream>
#include <map>
#include <vector>
#include <memory>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "constants.cpp"
#include "common.cpp"
#include "window.cpp"
#include "shader.cpp"

/**
 * Class containing information about a text character.
 */
class TextCharacter
{
  // Let the text manager access private variables.
  friend class TextManager;
  friend class TextCharacterSet;

private:
  const unsigned char character;
  const uint32_t characterSetLayerId;
  const glm::vec2 size;
  const glm::vec2 bearing;
  const float_t advance;
  const glm::vec2 maxUv;

public:
  TextCharacter(const unsigned char &character,
                const glm::vec2 &size,
                const glm::vec2 &bearing,
                const float_t &advance,
                const glm::vec2 &maxUv,
                const uint32_t &characterSetLayerId)
      : character(character),
        characterSetLayerId(characterSetLayerId),
        size(size),
        bearing(bearing),
        advance(advance),
        maxUv(maxUv) {}

  const unsigned char &getCharacter() const
  {
    return character;
  }
};

class TextCharacterSet
{
  // Let the text manager access private variables.
  friend class TextManager;

private:
  const std::string fontId;
  const std::string fontFilePath;

  const GLuint characterTextureArrayId;

  std::map<const unsigned char, const TextCharacter> characterMap;

  GLuint createTextureArray()
  {
    GLuint newTextureArrayId;
    glGenTextures(1, &newTextureArrayId);
    return newTextureArrayId;
  }

  void loadFont(const std::string &fontId, const std::string &fontFilePath)
  {
    FT_Library freeType;
    if (FT_Init_FreeType(&freeType))
    {
      std::cout << fontId << std::endl
                << "Failed at text character set 1" << std::endl;
      exit(1);
    }

    FT_Face fontFace;
    if (FT_New_Face(freeType, fontFilePath.c_str(), 0, &fontFace))
    {
      std::cout << fontId << std::endl
                << "Failed at text character set 2" << std::endl;
      exit(1);
    }

    FT_Set_Pixel_Sizes(fontFace, 0, TEXT_HEIGHT);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glBindTexture(GL_TEXTURE_2D_ARRAY, characterTextureArrayId);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    float_t maxWidth = 0;
    float_t maxRows = 0;
    for (unsigned char character = 0; character < std::numeric_limits<unsigned char>::max(); character++)
    {
      if (FT_Load_Char(fontFace, character, FT_LOAD_RENDER))
      {
        std::cout << fontId << std::endl
                  << "Failed at text character set 3" << std::endl;
        continue;
      }

      maxWidth = glm::max(maxWidth, static_cast<float_t>(fontFace->glyph->bitmap.width));
      maxRows = glm::max(maxRows, static_cast<float_t>(fontFace->glyph->bitmap.rows));
    }

    std::vector<uint8_t> clearData(maxWidth * maxRows, 0);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RED, maxWidth, maxRows, std::numeric_limits<unsigned char>::max(), 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    for (unsigned char character = 0; character < std::numeric_limits<unsigned char>::max(); character++)
    {
      if (FT_Load_Char(fontFace, character, FT_LOAD_RENDER))
      {
        std::cout << fontId << std::endl
                  << "Failed at text character set 4" << std::endl;
        continue;
      }

      glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, static_cast<uint32_t>(character), maxWidth, maxRows, 1, GL_RED, GL_UNSIGNED_BYTE, &clearData[0]);
      glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, static_cast<uint32_t>(character), fontFace->glyph->bitmap.width, fontFace->glyph->bitmap.rows, 1, GL_RED, GL_UNSIGNED_BYTE, fontFace->glyph->bitmap.buffer);

      const TextCharacter textCharacter(
          character,
          glm::vec2(static_cast<float_t>(fontFace->glyph->bitmap.width), static_cast<float_t>(fontFace->glyph->bitmap.rows)),
          glm::vec2(static_cast<float_t>(fontFace->glyph->bitmap_left), static_cast<float_t>(fontFace->glyph->bitmap_top)),
          static_cast<float_t>(fontFace->glyph->advance.x) / 64.0f,
          glm::vec2(static_cast<float_t>(fontFace->glyph->bitmap.width) / maxWidth, static_cast<float_t>(fontFace->glyph->bitmap.rows) / maxRows),
          (uint32_t)character);
      characterMap.insert(std::pair<const unsigned char, const TextCharacter>(character, textCharacter));
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    FT_Done_Face(fontFace);
    FT_Done_FreeType(freeType);
  }

  TextCharacterSet(const std::string &fontId, const std::string &fontFilePath)
      : fontId(fontId),
        fontFilePath(fontFilePath),
        characterTextureArrayId(createTextureArray())
  {
    loadFont(fontId, fontFilePath);
  }

public:
  const std::string &getFontId() const
  {
    return fontId;
  }

  const TextCharacter &getCharacter(const unsigned char &character) const
  {
    return characterMap.at(character);
  }
};

/**
 * Class for containing the details of the text to render.
 */
class TextDetails
{
private:
  // The text content to render.
  const std::string content;
  // The position of the text, with the origin being the bottom-left of the screen.
  const glm::vec2 position;
  // The normalized scale of the text, where a value of 1.0 is 100% the default size of the text.
  const float_t scale;

public:
  TextDetails(
      const std::string &content,
      const glm::vec2 &position,
      const float_t &scale)
      : content(content),
        position(position),
        scale(scale) {}

  /**
   * Get the content of the text.
   * 
   * @return The text content.
   */
  const std::string &getContent() const
  {
    return content;
  }

  /**
   * Get the position of the text, with the origin being the bottom-left of the screen.
   * 
   * @return The text position.
   */
  const glm::vec2 &getPosition() const
  {
    return position;
  }

  /**
   * Get the scale of the text, where a value of 1.0 is 100% the default size of the text.
   * 
   * @return The text scale.
   */
  const float_t &getScale() const
  {
    return scale;
  }
};

/**
 * A manager class for managing and trendering text.
 */
class TextManager
{
private:
  // The shader manager responsible for creating shader programs.
  ShaderManager &shaderManager;

  static TextManager instance;
  const static TextCharacterSet characterSet;

  // The shader program details of the model.
  const std::shared_ptr<const ShaderDetails> textShader;
  const glm::mat4 textProjectionMatrix;
  const GLuint textVertexBufferId;
  const GLuint textUvBufferId;
  const GLuint textUvLayerBufferId;

  std::vector<std::shared_ptr<const TextDetails>> textToRenderMap;

  void clearTextToRenderMap()
  {
    textToRenderMap.clear();
  }

  GLuint createTextVertexBuffer()
  {
    GLuint newBufferId;
    glGenBuffers(1, &newBufferId);

    glBindBuffer(GL_ARRAY_BUFFER, newBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float_t) * 6 * 2 * MAX_TEXT_CHARS, NULL, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return newBufferId;
  }

  GLuint createTextUvBuffer()
  {
    GLuint newBufferId;
    glGenBuffers(1, &newBufferId);

    glBindBuffer(GL_ARRAY_BUFFER, newBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float_t) * 6 * 2 * MAX_TEXT_CHARS, NULL, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return newBufferId;
  }

  GLuint createTextUvLayerBuffer()
  {
    GLuint newBufferId;
    glGenBuffers(1, &newBufferId);

    glBindBuffer(GL_ARRAY_BUFFER, newBufferId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float_t) * 6 * 1 * MAX_TEXT_CHARS, NULL, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return newBufferId;
  }

  TextManager() : shaderManager(ShaderManager::getInstance()),
                  textShader(shaderManager.createShaderProgram("Text", "assets/shaders/vertex/text.glsl", "assets/shaders/fragment/text.glsl")),
                  textProjectionMatrix(glm::ortho(0.0f, 1.0f * VIEWPORT_WIDTH, 0.0f, 1.0f * VIEWPORT_HEIGHT)),
                  textVertexBufferId(createTextVertexBuffer()),
                  textUvBufferId(createTextUvBuffer()),
                  textUvLayerBufferId(createTextUvLayerBuffer()) {}

public:
  /**
   * Returns the singleton instance of the text manager.
   * 
   * @return The text manager singleton instance.
   */
  static TextManager &getInstance()
  {
    return instance;
  }

  void render()
  {
    std::vector<float_t> characterVertices({});
    std::vector<float_t> characterUvs({});
    std::vector<float_t> characterUvLayers({});
    for (auto &textLine : textToRenderMap)
    {
      auto bufferExhaused = false;

      auto startX = textLine->getPosition().x;
      for (auto &ch : textLine->getContent())
      {
        if (characterVertices.size() / (6 * 2) >= MAX_TEXT_CHARS)
        {
          bufferExhaused = true;
          break;
        }

        const auto textCharacter = characterSet.getCharacter(ch);

        {
          const auto xPos = startX + (textCharacter.bearing.x * textLine->getScale());
          const auto yPos = (textLine->getPosition().y * TEXT_HEIGHT) - ((textCharacter.size.y - textCharacter.bearing.y) * textLine->getScale());

          const auto width = textCharacter.size.x * textLine->getScale();
          const auto height = textCharacter.size.y * textLine->getScale();

          characterVertices.push_back(xPos);
          characterVertices.push_back(yPos + height);
          characterVertices.push_back(xPos);
          characterVertices.push_back(yPos);
          characterVertices.push_back(xPos + width);
          characterVertices.push_back(yPos);

          characterVertices.push_back(xPos);
          characterVertices.push_back(yPos + height);
          characterVertices.push_back(xPos + width);
          characterVertices.push_back(yPos);
          characterVertices.push_back(xPos + width);
          characterVertices.push_back(yPos + height);
        }

        {
          characterUvs.push_back(0.0f);
          characterUvs.push_back(0.0f);
          characterUvs.push_back(0.0f);
          characterUvs.push_back(textCharacter.maxUv.y);
          characterUvs.push_back(textCharacter.maxUv.x);
          characterUvs.push_back(textCharacter.maxUv.y);

          characterUvs.push_back(0.0f);
          characterUvs.push_back(0.0f);
          characterUvs.push_back(textCharacter.maxUv.x);
          characterUvs.push_back(textCharacter.maxUv.y);
          characterUvs.push_back(textCharacter.maxUv.x);
          characterUvs.push_back(0.0f);
        }

        {
          characterUvLayers.push_back(static_cast<float_t>(textCharacter.characterSetLayerId));
          characterUvLayers.push_back(static_cast<float_t>(textCharacter.characterSetLayerId));
          characterUvLayers.push_back(static_cast<float_t>(textCharacter.characterSetLayerId));
          characterUvLayers.push_back(static_cast<float_t>(textCharacter.characterSetLayerId));
          characterUvLayers.push_back(static_cast<float_t>(textCharacter.characterSetLayerId));
          characterUvLayers.push_back(static_cast<float_t>(textCharacter.characterSetLayerId));
        }

        startX += textCharacter.advance * textLine->getScale();
      }

      if (bufferExhaused)
      {
        break;
      }
    }

    if (characterVertices.empty())
    {
      return;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Render text
    glUseProgram(textShader->getShaderId());

    const auto textTextureId = glGetUniformLocation(textShader->getShaderId(), "textTexture");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, characterSet.characterTextureArrayId);
    glUniform1i(textTextureId, 0);

    const auto projectionId = glGetUniformLocation(textShader->getShaderId(), "projection");
    glUniformMatrix4fv(projectionId, 1, GL_FALSE, &textProjectionMatrix[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, textVertexBufferId);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float_t) * characterVertices.size(), &characterVertices[0]);

    glBindBuffer(GL_ARRAY_BUFFER, textUvBufferId);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float_t) * characterUvs.size(), &characterUvs[0]);

    glBindBuffer(GL_ARRAY_BUFFER, textUvLayerBufferId);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float_t) * characterUvLayers.size(), &characterUvLayers[0]);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    { // Define vertex attribute arrays that contains the vertex position, UV coordinates, and normal vector data of the model.
      VertexAttributeArray vertexArray("VertexArray", textVertexBufferId, 2);
      VertexAttributeArray uvArray("UvArray", textUvBufferId, 2);
      VertexAttributeArray normalArray("UvLayerArray", textUvLayerBufferId, 1);

      // Enable them so that it can be used by the GPU.
      vertexArray.enableAttribute();
      uvArray.enableAttribute();
      normalArray.enableAttribute();

      glDrawArrays(GL_TRIANGLES, 0, characterVertices.size() / 2);
    }

    glDisable(GL_BLEND);

    clearTextToRenderMap();
  }

  void addText(const std::string &content, const glm::vec2 &position, const float_t &scale)
  {
    textToRenderMap.push_back(std::make_shared<const TextDetails>(content, position, scale));
  }
};

// Initialize the text character set static variable.
const TextCharacterSet TextManager::characterSet = TextCharacterSet("Roboto", "assets/fonts/Roboto-Regular.ttf");
// Initialize the text manager singleton instance static variable.
TextManager TextManager::instance;

#endif