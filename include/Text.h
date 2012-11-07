#ifndef SIMPLYFLAT_TEXT_MODULE
#define SIMPLYFLAT_TEXT_MODULE

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>

#include <GL/gl.h>
#include <GL/glu.h>

#ifndef FW_DONTCARE
  #define FW_DONTCARE 0
#endif

struct fontData
{
    fontData()
    {
        for (uint32 i = 0; i < 65536; i++)
        {
            textures[i] = 0;
            listIDs[i]  = 0;
        }
    }
    float height;
    GLuint textures[65536];
    GLuint listIDs[65536];

    FT_Face m_face;

    bool init(const char* fontOrFileName, uint32 height, uint16 bold = FW_DONTCARE, bool italic = false, bool underline = false, bool strikeout = false);
    void makeDisplayList(unsigned short ch);
    void cleanUp();
};

#endif
