#ifndef SIMPLYFLAT_TEXT_MODULE
#define SIMPLYFLAT_TEXT_MODULE

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

#include <gl\gl.h>
#include <gl\glu.h>

struct fontData
{
    float height;
    GLuint* textures;
    GLuint listBase;

    bool init(const char* fontOrFileName, uint32 height);
    void cleanUp();
};

#endif
