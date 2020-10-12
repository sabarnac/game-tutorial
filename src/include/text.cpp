#ifndef INCLUDE_TEXT_CPP
#define INCLUDE_TEXT_CPP

#include <iostream>
#include <map>
#include <vector>
#include <memory>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

/**
 * Class containing information about a text character.
 */
class TextCharacter
{
  // Let the text manager access private variables.
  friend class TextManager;
  friend class TextCharacterSet;

private:
  const char character;
  const unsigned int characterSetLayerId;
  const glm::vec2 size;
  const glm::vec2 bearing;
  const unsigned int advance;
  const glm::vec2 maxUv;

public:
  TextCharacter(const char &character,
                const glm::vec2 &size,
                const glm::vec2 &bearing,
                const unsigned int &advance,
                const glm::vec2 &maxUv,
                const unsigned int &characterSetLayerId)
      : character(character),
        characterSetLayerId(characterSetLayerId),
        size(size),
        bearing(bearing),
        advance(advance),
        maxUv(maxUv) {}

  double getCharacter() const
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

  std::map<const char, const TextCharacter> characterMap;

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

    FT_Set_Pixel_Sizes(fontFace, 0, 100);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glBindTexture(GL_TEXTURE_2D_ARRAY, characterTextureArrayId);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned int maxWidth = 0;
    unsigned int maxRows = 0;
    for (char character = 0; character < std::numeric_limits<char>::max(); character++)
    {
      if (FT_Load_Char(fontFace, character, FT_LOAD_RENDER))
      {
        std::cout << fontId << std::endl
                  << "Failed at text character set 3" << std::endl;
        continue;
      }

      maxWidth = glm::max(maxWidth, fontFace->glyph->bitmap.width);
      maxRows = glm::max(maxRows, fontFace->glyph->bitmap.rows);
    }

    std::vector<uint8_t> clearData(maxWidth * maxRows, 0);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RED, maxWidth, maxRows, std::numeric_limits<char>::max(), 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    for (char character = 0; character < std::numeric_limits<char>::max(); character++)
    {
      if (FT_Load_Char(fontFace, character, FT_LOAD_RENDER))
      {
        std::cout << fontId << std::endl
                  << "Failed at text character set 4" << std::endl;
        continue;
      }

      glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, (unsigned int)character, maxWidth, maxRows, 1, GL_RED, GL_UNSIGNED_BYTE, &clearData[0]);
      glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, (unsigned int)character, fontFace->glyph->bitmap.width, fontFace->glyph->bitmap.rows, 1, GL_RED, GL_UNSIGNED_BYTE, fontFace->glyph->bitmap.buffer);

      const TextCharacter textCharacter(
          character,
          glm::vec2(fontFace->glyph->bitmap.width, fontFace->glyph->bitmap.rows),
          glm::vec2(fontFace->glyph->bitmap_left, fontFace->glyph->bitmap_top),
          fontFace->glyph->advance.x,
          glm::vec2(fontFace->glyph->bitmap.width / maxWidth, fontFace->glyph->bitmap.rows / maxRows),
          (unsigned int)character);
      characterMap.insert(std::pair<const char, const TextCharacter>(character, textCharacter));
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

  const TextCharacter &getCharacter(const char &character)
  {
    return characterMap.at(character);
  }
};

/**
 * Class for containing the details of the text to render.
 */
class TextDetails
{
  // Let the text manager access private variables.
  friend class TextManager;

private:
  // The text content to render.
  const std::string content;
  // The position of the text, with the origin being the top-left of the screen.
  const glm::vec2 position;

public:
  TextDetails(
      const std::string &content,
      const glm::vec2 &position)
      : content(content),
        position(position) {}

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
   * Get the position of the text, with the origin being the top-left of the screen.
   * 
   * @return The text position.
   */
  const glm::vec2 &getPosition() const
  {
    return position;
  }
};

/**
 * A manager class for managing and trendering text.
 */
class TextManager
{
private:
  static TextManager instance;
  const static TextCharacterSet characterSet;

  std::vector<std::shared_ptr<const TextDetails>> textToRenderMap;

  void clearTextToRenderMap()
  {
    textToRenderMap.clear();
  }

  TextManager()
  {
  }

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

    clearTextToRenderMap();
  }

  void addText(const std::string &content, const glm::vec2 &position)
  {
    textToRenderMap.push_back(std::make_shared<const TextDetails>(content, position));
  }
};

// Initialize the text character set static variable.
const TextCharacterSet TextManager::characterSet = TextCharacterSet("Roboto", "assets/fonts/Roboto-Regular.ttf");
// Initialize the text manager singleton instance static variable.
TextManager TextManager::instance;

#endif