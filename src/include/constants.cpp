#ifndef INCLUDE_CONSTANTS_CPP
#define INCLUDE_CONSTANTS_CPP

/**
 * A bunch of constants that's required to be known globally.
 */

const int32_t WINDOW_WIDTH = 1024;
const int32_t WINDOW_HEIGHT = 768;
const int32_t MAX_CONE_LIGHTS = 2;
const int32_t MAX_POINT_LIGHTS = 6;
const int32_t MAX_LIGHTS = MAX_CONE_LIGHTS + MAX_POINT_LIGHTS;
const int32_t MAX_TEXT_LENGTH = 80;
const int32_t MAX_TEXT_CHARS = 10240;

int32_t VIEWPORT_WIDTH = WINDOW_WIDTH;
int32_t VIEWPORT_HEIGHT = WINDOW_HEIGHT;
int32_t FRAMEBUFFER_WIDTH = VIEWPORT_WIDTH;
int32_t FRAMEBUFFER_HEIGHT = VIEWPORT_HEIGHT;
int32_t TEXT_HEIGHT = VIEWPORT_HEIGHT / 26;
int32_t TEXT_WIDTH = VIEWPORT_WIDTH / 80;
int32_t SWAP_INTERVAL = 0;

#endif