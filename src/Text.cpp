#include <SimplyFlat.h>
#include <Text.h>

#include <locale.h>

inline int next_p2(int a)
{
    int rval = 1;
    while (rval < a)
        rval <<= 1;

    return rval;
}

static inline int divpow2(int x, int divisor)
{
    int sign = ((unsigned int)x) >> 31;
    x += sign;
    x >>= divisor;
    return x;
}

void fontData::makeDisplayList(unsigned short ch)
{
    int i = (int)ch;
    if (FT_Load_Glyph(m_face, FT_Get_Char_Index(m_face, i), FT_LOAD_DEFAULT))
        return;

    FT_Glyph glyph;
    if (FT_Get_Glyph(m_face->glyph, &glyph))
        return;

    FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, 0, 1);
    FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;

    FT_Bitmap& bitmap = bitmap_glyph->bitmap;

    int pwidth = next_p2(bitmap.width);
    int pheight = next_p2(bitmap.rows);

    GLubyte* expanded_data = new GLubyte[2 * pwidth * pheight];

    for (int j = 0; j < pheight ; j++)
    {
        for (int i = 0; i < pwidth; i++)
        {
            expanded_data[2 * (i + j * pwidth)] = 255;
            expanded_data[2 * (i + j * pwidth) + 1] = (i >= bitmap.width || j >= bitmap.rows) ? 0 : bitmap.buffer[i + bitmap.width * j];
        }
    }

    // store char width and adjust max height
    // note .5f
    float ih = 1.0f/((float)height);
    float width = ((float)divpow2(m_face->glyph->advance.x, 7))*ih;
    float aHeight = (pheight > ((float)bitmap.rows)*ih) ? pheight : ((float)bitmap.rows)*ih;

    glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT);

    // create gl texture
    glGenTextures(1, &(textures[i]));

    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, textures[i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pwidth, pheight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data);

    glPopAttrib();

    delete[] expanded_data;

    // create display list
    listIDs[i] = glGenLists(1);

    glNewList(listIDs[i], GL_COMPILE);

    glBindTexture(GL_TEXTURE_2D, textures[i]);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    // adjust position to account for texture padding
    glTranslatef((float)bitmap_glyph->left, 0.0f, 0.0f);
    glTranslatef(0.0f, (float)(bitmap_glyph->top-bitmap.rows), 0.0f);

    // work out texcoords
    float tx=((float)bitmap.width)/((float)pwidth);
    float ty=((float)bitmap.rows)/((float)pheight);

    // render
    // note .5f
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(0.0f, (float)bitmap.rows);
        glTexCoord2f(0.0f, ty);
        glVertex2f(0.0f, 0.0f);
        glTexCoord2f(tx, ty);
        glVertex2f((float)bitmap.width, 0.0f);
        glTexCoord2f(tx, 0.0f);
        glVertex2f((float)bitmap.width, (float)bitmap.rows);
    glEnd();

    glPopMatrix();

    // move position for the next character
    // note extra div 2
    glTranslatef((float)divpow2(m_face->glyph->advance.x*2, 7), 0.0f, 0.0f);

    glEndList();
}

bool fontData::init(const char *fontOrFileName, uint32 height, uint16 bold, bool italic, bool underline, bool strikeout)
{
    this->height = (float)height;

    FT_Library library;
    if (FT_Init_FreeType(&library))
        return false;

    FT_Face face;

    // if == 0, then OK, otherwise error
    if (FT_New_Face(library, fontOrFileName, 0, &face) != 0)
    {
        // Following code is suitable only for Windows
#ifdef _WIN32
        char buf[1024];
        GetWindowsDirectoryA(buf,1024);
        strcat(buf, "\\fonts\\");
        strcat(buf, fontOrFileName);
        strcat(buf, ".ttf");

        if (FT_New_Face(library, buf, 0, &face) != 0)
        {
            HDC hDC = CreateCompatibleDC(NULL);

            HFONT hFont = CreateFont(0, 0, 0, 0, bold, italic,
                                                   underline, strikeout, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
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
#else
        return false;
#endif
    }

#ifndef _WIN32
    if (bold > FW_DONTCARE)
        face->style_flags |= FT_STYLE_FLAG_BOLD;
#endif

    FT_Set_Char_Size(face, height << 6, height << 6, 96, 96);

    m_face = face;

    // For now, render only first 256 characters to speed it up
    for (uint32 i = 0; i < 256; i++)
        makeDisplayList(i);

    return true;
}

void fontData::cleanUp()
{
    FT_Done_Face(m_face);

    for (uint32 i = 0; i < 65536; i++)
    {
        if (textures[i] != 0)
            glDeleteTextures(1, &textures[i]);
        if (listIDs[i] != 0)
            glDeleteLists(listIDs[i], 1);
    }
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

void SimplyFlat::t_Drawing::PrintText(uint32 fontId, uint32 x, uint32 y, const wchar_t *fmt, ...)
{
    fontData* fd = m_fontDataMap[fontId];

    //GLuint font = fd->listBase;
    float h = fd->height/.63f;

    y -= (uint32)fd->height;

    wchar_t text[256];
    va_list ap;

    if (fmt == NULL)
        *text=0;
    else
    {
        va_start(ap, fmt);
            vswprintf(text, 99999999, fmt, ap);
        va_end(ap);
    }

    const wchar_t *start_line = text;
    std::vector<std::wstring> lines;
    for (const wchar_t *c = text; *c; c++)
    {
        if (*c == '\n')
        {
            std::wstring line;
            for (const wchar_t *n = start_line; n < c; n++)
                line.append(1,*n);

            lines.push_back(line);
            start_line = c+1;
        }
    }

    if (start_line)
        lines.push_back(start_line);

    // Render additional characters if needed
    for (int i = 0; i < lines.size(); i++)
    {
        const wchar_t *str = lines[i].c_str();
        for (unsigned int j = 0; j < wcslen(str); j++)
        {
            if (fd->listIDs[str[j]] == 0)
                fd->makeDisplayList(str[j]);
        }
    }

    pushScreenCoordinateMatrix(&x, &y);

    glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (int i = 0; i < lines.size(); i++)
    {
        glPushMatrix();
        glLoadIdentity();
        glTranslatef((GLfloat)x,(GLfloat)(y-h*i),0);
        //glMultMatrixf(modelview_matrix);

        glListBase(0u);
        const wchar_t *str = lines[i].c_str();
        for (unsigned int j = 0; j < wcslen(str); j++)
            glCallList(fd->listIDs[str[j]]);

        glPopMatrix();
    }

    glPopAttrib();

    pop_projection_matrix();
}
