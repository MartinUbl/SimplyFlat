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
    FA_NORMAL          = 0,
    FA_BOLD            = 1 << 0,
    FA_ITALIC          = 1 << 1,
    FA_BOLD_AND_ITALIC = FA_BOLD | FA_ITALIC,
    MAX_FA             = FA_BOLD_AND_ITALIC+1,

    FA_UNDERLINE       = 1 << 2,
    FA_STRIKEOUT       = 1 << 3,
    MAX_NONFT_FA       = FA_STRIKEOUT + 1
};

enum WordWrapLimits
{
    WW_NO_WRAP     = -1,
    WW_WRAP_CANVAS = 0
};

static const wchar_t* wrapAfterChar = L" ,.!?-:/\\";
static bool IsWrapChar(wchar_t ch)
{
    for (uint32 i = 0; i < wcslen(wrapAfterChar); i++)
        if (wrapAfterChar[i] == ch)
            return true;
    return false;
}

struct fontData
{
    fontData()
    {
        for (uint32 i = 0; i < 65536; i++)
        {
            for (uint32 j = FA_NORMAL; j < MAX_FA; j++)
            {
                textures[j][i]  = 0;
                listIDs[j][i]   = 0;
                charWidth[j][i] = 0;
            }
        }

        for (uint32 i = 0; i < MAX_FA; i++)
            m_face[i] = NULL;
    }
    float height;
    GLuint textures[MAX_FA][65536];
    GLuint listIDs[MAX_FA][65536];
    uint16 charWidth[MAX_FA][65536];

    FT_Face m_face[MAX_FA];

    bool init(const char* fontOrFileName, uint32 height);
    void makeDisplayList(unsigned short ch, uint8 index = FA_NORMAL);
    void cleanUp();
};

struct printTextData
{
    printTextData()
    {
        fontId = 0;
        feature = 0;
        colorize = false;
        color = 0;

        text = NULL;
    }

    uint32 fontId;   // font ID of this text segment
    uint8 feature;   // bold/italic/underline/strikeout

    bool colorize;   // if set, rendering engine will use 'color' to determine output color
    uint32 color;    // classical RGBA color

    wchar_t* text;
};

typedef std::vector<printTextData*> StyledTextList;

#endif
