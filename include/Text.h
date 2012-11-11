#ifndef SIMPLYFLAT_TEXT_MODULE
#define SIMPLYFLAT_TEXT_MODULE

#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>
#include <freetype/ftbitmap.h>

#include <GL/gl.h>
#include <GL/glu.h>

#ifndef FW_DONTCARE
  #define FW_DONTCARE 0
#endif

enum FeatureArrayIndex
{
    FA_NORMAL = 0,
    FA_BOLD   = 1,
    FA_ITALIC = 2,
    FA_BOLD_AND_ITALIC = 3,
    MAX_FA    = 4
};

struct fontData
{
    fontData()
    {
        for (uint32 i = 0; i < 65536; i++)
        {
            for (uint32 j = FA_NORMAL; j < MAX_FA; j++)
            {
                textures[j][i] = 0;
                listIDs[j][i]  = 0;
            }
        }
    }
    float height;
    GLuint textures[MAX_FA][65536];
    GLuint listIDs[MAX_FA][65536];

    FT_Face m_face[MAX_FA];

    bool init(const char* fontOrFileName, uint32 height);
    void makeDisplayList(unsigned short ch, FeatureArrayIndex index = FA_NORMAL);
    void cleanUp();
};

#endif
