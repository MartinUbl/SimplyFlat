#include <SimplyFlat.h>
#include <Text.h>

inline int next_p2(int a)
{
    int rval = 1;
    while (rval < a)
        rval <<= 1;

    return rval;
}

void makeDisplayList(FT_Face face, char ch, GLuint list_base, GLuint *tex_base)
{
    if (FT_Load_Glyph(face, FT_Get_Char_Index( face, ch ), FT_LOAD_DEFAULT))
        return;
        //throw std::runtime_error("FT_Load_Glyph failed");

    FT_Glyph glyph;
    if (FT_Get_Glyph(face->glyph, &glyph))
        return;
        //throw std::runtime_error("FT_Get_Glyph failed");

    FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

    FT_Bitmap& bitmap = bitmap_glyph->bitmap;

    int width = next_p2(bitmap.width);
    int height = next_p2(bitmap.rows);

    GLubyte* expanded_data = new GLubyte[2 * width * height];

    for (int j = 0; j <height ; j++)
    {
        for (int i = 0; i < width; i++)
        {
            expanded_data[2 * (i + j * width)] = 255;
            expanded_data[2 * (i + j * width) + 1] = (i >= bitmap.width || j >= bitmap.rows) ? 0 : bitmap.buffer[i + bitmap.width * j];
        }
    }

    glBindTexture(GL_TEXTURE_2D, tex_base[ch]);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data);

    delete [] expanded_data;

    glNewList(list_base+ch,GL_COMPILE);

    glBindTexture(GL_TEXTURE_2D,tex_base[ch]);

    glPushMatrix();

    glTranslatef((GLfloat)bitmap_glyph->left,0,0);
    glTranslatef(0,(GLfloat)bitmap_glyph->top-bitmap.rows,0);

    float x = (float)bitmap.width / (float)width;
    float y = (float)bitmap.rows / (float)height;

    glBegin(GL_QUADS);
        glTexCoord2d(0,0); glVertex2f(0,(GLfloat)bitmap.rows);
        glTexCoord2d(0,y); glVertex2f(0,0);
        glTexCoord2d(x,y); glVertex2f((GLfloat)bitmap.width,0);
        glTexCoord2d(x,0); glVertex2f((GLfloat)bitmap.width,(GLfloat)bitmap.rows);
    glEnd();
    glPopMatrix();
    glTranslatef((GLfloat)(face->glyph->advance.x >> 6),0,0);

    glEndList();
}

bool fontData::init(const char *fontOrFileName, uint32 height)
{
    textures = new GLuint[128];

    this->height = (float)height;

    FT_Library library;
    if (FT_Init_FreeType(&library))
        return false;
        //throw std::runtime_error("FT_Init_FreeType failed");

    FT_Face face;

    // if == 0, then OK, otherwise error
    if (FT_New_Face(library, fontOrFileName, 0, &face) != 0)
    {
        HDC hDC = CreateCompatibleDC(NULL);

        HFONT hFont = CreateFont(0, 0, 0, 0, FW_DONTCARE, false,
                                               false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                                               CLIP_DEFAULT_PRECIS, 5,
                                               VARIABLE_PITCH, fontOrFileName);

        if (hFont)
        {
            SelectObject(hDC, hFont);
            uint32 size = GetFontData(hDC, 0, 0, NULL, 0);
            if (size == GDI_ERROR)
            {
                DeleteObject(hFont);
                DeleteDC(hDC);
                return false;
            }

            uint8* data = new uint8[size];
            uint32 res = GetFontData(hDC, 0, 0, data, size);
            if (res == GDI_ERROR)
            {
                DeleteObject(hFont);
                DeleteDC(hDC);
                return false;
            }

            DeleteObject(hFont);
            DeleteDC(hDC);

            if (FT_New_Memory_Face(library, (FT_Byte*)data, size, 0, &face))
                return false;
        }
        else
        {
            DeleteDC(hDC);
            return false;
        }
    }

    FT_Set_Char_Size(face, height << 6, height << 6, 96, 96);

    listBase = glGenLists(128);
    glGenTextures(128, textures);

    for (uint8 i = 0; i < 128; i++)
        makeDisplayList(face,i,listBase,textures);

    FT_Done_Face(face);

    FT_Done_FreeType(library);

    return true;
}

void fontData::cleanUp()
{
    glDeleteLists(listBase,128);
    glDeleteTextures(128,textures);
    delete [] textures;
}

inline void pushScreenCoordinateMatrix(uint32* x, uint32* y) {
    glPushAttrib(GL_TRANSFORM_BIT);
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(viewport[0], viewport[0]+viewport[2], viewport[1], viewport[3]);
    (*y) = viewport[3] - (*y);
    glPopAttrib();
}

inline void pop_projection_matrix() {
    glPushAttrib(GL_TRANSFORM_BIT);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

void SimplyFlat::t_Drawing::PrintText(uint32 fontId, uint32 x, uint32 y, const char *fmt, ...)
{
    fontData* fd = m_fontDataMap[fontId];

    pushScreenCoordinateMatrix(&x, &y);

    GLuint font = fd->listBase;
    float h = fd->height/.63f;

    y -= (uint32)fd->height;

    char text[256];
    va_list ap;

    if (fmt == NULL)
        *text=0;
    else
    {
        va_start(ap, fmt);
            vsprintf(text, fmt, ap);
        va_end(ap);
    }

    const char *start_line = text;
    std::vector<std::string> lines;
    for (const char *c = text; *c; c++)
    {
        if (*c == '\n')
        {
            std::string line;
            for (const char *n = start_line; n < c; n++)
                line.append(1,*n);

            lines.push_back(line);
            start_line = c+1;
        }
    }

    if (start_line)
        lines.push_back(start_line);

    glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glListBase(font);

    /*float modelview_matrix[16];
    glGetFloatv(GL_MODELVIEW_MATRIX, modelview_matrix);*/

    for (int i = 0; i < lines.size(); i++)
    {
        glPushMatrix();
        glLoadIdentity();
        glTranslatef((GLfloat)x,(GLfloat)(y-h*i),0);
        //glMultMatrixf(modelview_matrix);

        glCallLists(lines[i].length(), GL_UNSIGNED_BYTE, lines[i].c_str());
        glPopMatrix();
    }

    glPopAttrib();

    pop_projection_matrix();
}
