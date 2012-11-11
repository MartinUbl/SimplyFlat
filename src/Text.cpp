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

void fontData::makeDisplayList(unsigned short ch, uint8 index)
{
    if (index >= MAX_FA)
        return;

    int i = (int)ch;
    if (FT_Load_Glyph(m_face[index], FT_Get_Char_Index(m_face[index], i), FT_LOAD_DEFAULT))
        return;

    FT_Glyph glyph;
    if (FT_Get_Glyph(m_face[index]->glyph, &glyph))
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
    float width = ((float)divpow2(m_face[index]->glyph->advance.x, 7))*ih;
    float aHeight = (pheight > ((float)bitmap.rows)*ih) ? pheight : ((float)bitmap.rows)*ih;

    glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT);

    // create gl texture
    glGenTextures(1, &(textures[index][i]));

    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, textures[index][i]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, pwidth, pheight, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expanded_data);

    glPopAttrib();

    delete[] expanded_data;

    // create display list
    listIDs[index][i] = glGenLists(1);

    glNewList(listIDs[index][i], GL_COMPILE);

    glBindTexture(GL_TEXTURE_2D, textures[index][i]);

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
    glTranslatef((float)divpow2(m_face[index]->glyph->advance.x*2, 7), 0.0f, 0.0f);

    glEndList();

    charWidth[index][i] = divpow2(m_face[index]->glyph->advance.x*2, 7);
}

bool PrepareFontData(HDC* hDC, HFONT* fnt, uint32* size, uint8** data)
{
    SelectObject((*hDC), (*fnt));
    (*size) = GetFontData((*hDC), 0, 0, NULL, 0);
    if ((*size) == GDI_ERROR)
        return false;

    (*data) = new uint8[(*size)];
    uint32 res = GetFontData((*hDC), 0, 0, (*data), (*size));
    if (res == GDI_ERROR)
        return false;

    return true;
}

bool fontData::init(const char *fontOrFileName, uint32 height)
{
    this->height = (float)height;

    FT_Library library;
    if (FT_Init_FreeType(&library))
        return false;

    // if == 0, then OK, otherwise error
    if (FT_New_Face(library, fontOrFileName, 0, &m_face[FA_NORMAL]) != 0)
    {
        // Following code is suitable only for Windows
#ifdef _WIN32
        HDC hDC = CreateCompatibleDC(NULL);

        HFONT hFont = CreateFont(height, 5, 0, 0, FW_DONTCARE, FALSE,
                                                  FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_PRECIS,
                                                  CLIP_TT_ALWAYS, CLEARTYPE_QUALITY,
                                                  VARIABLE_PITCH, fontOrFileName);

        if (hFont)
        {
            uint32 size;
            uint8* data = NULL;

            if (!PrepareFontData(&hDC, &hFont, &size, &data))
            {
                DeleteObject(hFont);
                DeleteDC(hDC);
                return false;
            }

            FT_Face pf;

            if (FT_New_Memory_Face(library, (FT_Byte*)data, size, 0, &pf))
            {
                DeleteObject(hFont);
                DeleteDC(hDC);
                return false;
            }

            m_face[FA_NORMAL] = pf;
            pf = NULL;

            LOGFONT fontAttributes = { 0 };
            ::GetObject(hFont, sizeof(fontAttributes), &fontAttributes);
            fontAttributes.lfWeight = FW_BOLD;

            hFont = ::CreateFontIndirect(&fontAttributes);

            if (!PrepareFontData(&hDC, &hFont, &size, &data))
            {
                DeleteObject(hFont);
                DeleteDC(hDC);
                return false;
            }

            FT_New_Memory_Face(library, (FT_Byte*)data, size, 0, &pf);

            m_face[FA_BOLD] = pf;
            pf = NULL;

            ::GetObject(hFont, sizeof(fontAttributes), &fontAttributes);
            fontAttributes.lfItalic = TRUE;

            hFont = ::CreateFontIndirect(&fontAttributes);

            if (!PrepareFontData(&hDC, &hFont, &size, &data))
            {
                DeleteObject(hFont);
                DeleteDC(hDC);
                return false;
            }

            FT_New_Memory_Face(library, (FT_Byte*)data, size, 0, &pf);

            m_face[FA_BOLD_AND_ITALIC] = pf;
            pf = NULL;

            ::GetObject(hFont, sizeof(fontAttributes), &fontAttributes);
            fontAttributes.lfWeight = FW_DONTCARE;
            fontAttributes.lfItalic = TRUE;

            hFont = ::CreateFontIndirect(&fontAttributes);

            if (!PrepareFontData(&hDC, &hFont, &size, &data))
            {
                DeleteObject(hFont);
                DeleteDC(hDC);
                return false;
            }

            FT_New_Memory_Face(library, (FT_Byte*)data, size, 0, &pf);

            m_face[FA_ITALIC] = pf;
            pf = NULL;

            DeleteObject(hFont);
            DeleteDC(hDC);
        }
        else
        {
            DeleteDC(hDC);
            return false;
        }
#else
        return false;
#endif
    }

    for (uint32 i = FA_NORMAL; i < MAX_FA; i++)
        FT_Set_Char_Size(m_face[i], height << 6, height << 6, 96, 96);

    // For now, render only first 256 characters (for each style) to speed it up
    for (uint32 i = 0; i < 256; i++)
    {
        makeDisplayList(i, FA_NORMAL);
        makeDisplayList(i, FA_BOLD);
        makeDisplayList(i, FA_BOLD_AND_ITALIC);
        makeDisplayList(i, FA_ITALIC);
    }

    return true;
}

void fontData::cleanUp()
{
    for (uint32 i = FA_NORMAL; i < MAX_FA; i++)
        FT_Done_Face(m_face[i]);

    for (uint32 i = 0; i < 65536; i++)
    {
        for (uint32 j = FA_NORMAL; j < MAX_FA; j++)
        {
            if (textures[j][i] != 0)
                glDeleteTextures(1, &textures[j][i]);
            if (listIDs[j][i] != 0)
                glDeleteLists(listIDs[j][i], 1);
        }
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

void SimplyFlat::t_Drawing::PrintText(uint32 fontId, uint32 x, uint32 y, uint8 feature, const wchar_t *fmt, ...)
{
    fontData* fd = m_fontDataMap[fontId];

    //GLuint font = fd->listBase;
    float h = fd->height/.63f;

    bool underline = ((feature & FA_UNDERLINE) > 0);
    bool strikeout = ((feature & FA_STRIKEOUT) > 0);

    // Now we have to cut highest bits - leave only bits 0 and 1 (so it can make numbers 0,1,2,3 as index for printing)
    feature &= (MAX_FA - 1);

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
    std::vector<uint32> lineWidths;
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
    {
        lines.push_back(start_line);
        lineWidths.push_back(0);
    }

    // Render additional characters if needed
    for (int i = 0; i < lines.size(); i++)
    {
        const wchar_t *str = lines[i].c_str();
        for (unsigned int j = 0; j < wcslen(str); j++)
        {
            if (fd->listIDs[feature][str[j]] == 0)
                fd->makeDisplayList(str[j], feature);

            lineWidths[i] += fd->charWidth[feature][str[j]];
        }

        lineWidths[i] = (uint32)((float)lineWidths[i]);
    }

    uint32 origX = x;
    uint32 origY = y;

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
            glCallList(fd->listIDs[feature][str[j]]);

        glPopMatrix();
    }

    glPopAttrib();

    pop_projection_matrix();

    glLoadIdentity();

    if (underline)
    {
        glDisable(GL_TEXTURE_2D);
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        float upos = -float(divpow2(fd->m_face[feature]->underline_position*2, 8));
        float thick = float(divpow2(fd->m_face[feature]->underline_thickness*2, 7));

        for (uint32 i = 0; i < lineWidths.size(); i++)
        {
            if (lineWidths[i] > 0)
            {
                glBegin(GL_QUADS);
                    glVertex2d(x,               origY+h*i+upos);
                    glVertex2d(x+lineWidths[i], origY+h*i+upos);
                    glVertex2d(x+lineWidths[i], origY+h*i+upos+thick);
                    glVertex2d(x,               origY+h*i+upos+thick);
                glEnd();
            }
        }
    }

    if (strikeout)
    {
        glDisable(GL_TEXTURE_2D);
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        float upos = - float(fd->height) / 3;
        float thick = float(divpow2(fd->m_face[feature]->underline_thickness*2, 7))*1.5f;

        for (uint32 i = 0; i < lineWidths.size(); i++)
        {
            if (lineWidths[i] > 0)
            {
                float lineAdd = (float(lineWidths[i])/float(lines[i].size()))*0.35f;

                glBegin(GL_QUADS);
                    glVertex2d(x-lineAdd,                      origY+h*i+upos);
                    glVertex2d(x+float(lineWidths[i])+lineAdd, origY+h*i+upos);
                    glVertex2d(x+float(lineWidths[i])+lineAdd, origY+h*i+upos+thick);
                    glVertex2d(x-lineAdd,                      origY+h*i+upos+thick);
                glEnd();
            }
        }
    }
}
