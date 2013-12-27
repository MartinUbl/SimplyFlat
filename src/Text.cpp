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

    // the minimum character width/height is 2
    if (pwidth < 2)
        pwidth = 2;
    if (pheight < 2)
        pheight = 2;

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
    glTranslatef(0.0f, (float)(-bitmap_glyph->top), 0.0f);

    // work out texcoords
    float tx=((float)bitmap.width)/((float)pwidth);
    float ty=((float)bitmap.rows)/((float)pheight);

    // render
    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(0.0f, 0.0f);
        glTexCoord2f(0.0f, ty);
        glVertex2f(0.0f, (float)bitmap.rows);
        glTexCoord2f(tx, ty);
        glVertex2f((float)bitmap.width, (float)bitmap.rows);
        glTexCoord2f(tx, 0.0f);
        glVertex2f((float)bitmap.width, 0.0f);
    glEnd();

    glPopMatrix();

    // move position for the next character
    // note extra div 2
    glTranslatef((float)divpow2(m_face[index]->glyph->advance.x*2, 7), 0.0f, 0.0f);

    glEndList();

    charWidth[index][i] = divpow2(m_face[index]->glyph->advance.x*2, 7);
}

#ifdef _WIN32
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
#endif

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
    if (SF->Drawing->IsFontPrecaching())
    {
        for (uint32 i = 0; i < 256; i++)
        {
            makeDisplayList(i, FA_NORMAL);
            makeDisplayList(i, FA_BOLD);
            makeDisplayList(i, FA_BOLD_AND_ITALIC);
            makeDisplayList(i, FA_ITALIC);
        }
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

void SimplyFlat::t_Drawing::PrintText(int32 fontId, int32 x, int32 y, uint8 feature, int32 wordWrapLimit, const wchar_t *fmt, ...)
{
    // do not allow to draw not rendered or saved font
    if (fontId < 0 || m_fontDataMap.size() <= fontId)
        return;

    uint32 canvasEnd = sSimplyFlat->GetScreenWidth();
    if (wordWrapLimit == WW_WRAP_CANVAS)
        wordWrapLimit = canvasEnd;

    canvasEnd = (int32)(canvasEnd * 1.0f/m_persistentScale);

    // Sign for calculating with supplied beginning final x in wordWrapLimit instead of dynamic x
    bool wrapMovedStart = false;
    if (wordWrapLimit <= 0)
        wrapMovedStart = true;

    fontData* fd = m_fontDataMap[fontId];

    uint32 i, k, linewidth, tmpwidth, line, chr;

    float h = fd->height/.63f;

    bool underline = ((feature & FA_UNDERLINE) > 0);
    bool strikeout = ((feature & FA_STRIKEOUT) > 0);

    // Now we have to cut highest bits - leave only bits 0 and 1 (so it can make numbers 0,1,2,3 as index for printing)
    feature &= (MAX_FA - 1);

    y += (uint32)fd->height;

    wchar_t str[2048];
    va_list ap;

    if (fmt == NULL)
        *str=0;
    else
    {
        va_start(ap, fmt);
            vswprintf(str, 99999999, fmt, ap);
        va_end(ap);
    }

    std::vector<uint32> lineWidths;
    std::vector<uint32> lineCharCount;

    // Render additional characters if needed
    for (int i = 0; i < wcslen(str); i++)
    {
        if (fd->listIDs[feature][str[i]] == 0)
            fd->makeDisplayList(str[i], feature);

//        lineWidths[i] += fd->charWidth[feature][str[j]];
    }

    uint32 origX = x;
    uint32 origY = y;

    glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glPushMatrix();
    glTranslatef((GLfloat)x,(GLfloat)y,0);

    linewidth = 0;
    line = 0;
    chr = 0;
    for (i = 0; i < wcslen(str); )
    {
        if (wordWrapLimit != WW_NO_WRAP)
        {
            tmpwidth = 0;
            if (IsWrapChar(str[i]) && i+1 < wcslen(str))
            {
                tmpwidth += fd->charWidth[feature][str[i]];

                for (k = i+1; k < wcslen(str); k++)
                {
                    tmpwidth += fd->charWidth[feature][str[k]];

                    if (IsWrapChar(str[k]))
                        break;
                }

                if ((wrapMovedStart ? x : -wordWrapLimit)+linewidth+tmpwidth > (wrapMovedStart ? canvasEnd : wordWrapLimit))
                {
                    glCallList(fd->listIDs[feature][str[i]]);

                    glTranslatef(-(GLfloat)linewidth, h*(line+1), 0);

                    line++;
                    lineWidths.resize(line);
                    lineWidths[line-1] = linewidth;
                    linewidth = 0;
                    i++;
                    lineCharCount.resize(line);
                    lineCharCount[line-1] = chr;
                    chr = 0;

                    continue;
                }
            }
        }

        if (str[i] == L'\n')
        {
            glTranslatef(-(GLfloat)linewidth, h*(line+1), 0);

            line++;
            lineWidths.resize(line);
            lineWidths[line-1] = linewidth;
            linewidth = 0;
            i++;
            lineCharCount.resize(line);
            lineCharCount[line-1] = chr;
            chr = 0;

            continue;
        }
        linewidth += fd->charWidth[feature][str[i]];
        glCallList(fd->listIDs[feature][str[i]]);
        chr++;
        i++;
    }

    lineWidths.resize(line+1);
    lineWidths[line] = linewidth;
    lineCharCount.resize(line+1);
    lineCharCount[line] = chr;

    glPopMatrix();

    glPopAttrib();

    if (underline)
    {
        glDisable(GL_TEXTURE_2D);

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

        float upos = - float(fd->height) / 3;
        float thick = float(divpow2(fd->m_face[feature]->underline_thickness*2, 7))*1.5f;

        for (uint32 i = 0; i < lineWidths.size(); i++)
        {
            if (lineWidths[i] > 0)
            {
                float lineAdd = (float(lineWidths[i])/float(lineCharCount[i]))*0.35f;

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

void SimplyFlat::t_Drawing::PrintStyledText(uint32 x, uint32 y, int32 wordWrapLimit, StyledTextList* printList)
{
    if (!printList || printList->empty())
        return;

    uint32 canvasEnd = sSimplyFlat->GetScreenWidth();
    if (wordWrapLimit == WW_WRAP_CANVAS)
        wordWrapLimit = canvasEnd;

    canvasEnd = (int32)(canvasEnd * 1.0f/m_persistentScale);

    // Sign for calculating with supplied beginning final x in wordWrapLimit instead of dynamic x
    bool wrapMovedStart = false;
    if (wordWrapLimit <= 0)
        wrapMovedStart = true;

    uint32 i, j, k, linewidth, tmpwidth;

    // dynamically allocate fields to store needed data
    fontData** fd = new fontData*[printList->size()];
    float* h = new float[printList->size()];
    bool* underline = new bool[printList->size()];
    bool* strikeout = new bool[printList->size()];

    float highest = 0;

    for (i = 0; i < printList->size(); i++)
    {
        // If some of print elements are NULL (not valid), do not print anything - it's some kind of fault in function usage
        if ((*printList)[i] == NULL
            // or if some of the fonts aren't loaded yet, or they're invalid
            || (*printList)[i]->fontId < 0 || m_fontDataMap.size() <= (*printList)[i]->fontId)
        {
            delete[] fd;
            delete[] h;
            delete[] underline;
            delete[] strikeout;
            return;
        }

        fd[i] = m_fontDataMap[(*printList)[i]->fontId];
        h[i] = fd[i]->height/.63f;
        underline[i] = (((*printList)[i]->feature & FA_UNDERLINE) > 0);
        strikeout[i] = (((*printList)[i]->feature & FA_STRIKEOUT) > 0);

        if (fd[i]->height > highest)
            highest = fd[i]->height;

        // Now we have to cut highest bits - leave only bits 0 and 1 (so it can make numbers 0,1,2,3 as index for printing)
        (*printList)[i]->feature &= (MAX_FA - 1);
    }

    y += (uint32)highest;

    // Render additional characters if needed
    for (i = 0; i < printList->size(); i++)
    {
        const wchar_t *str = (*printList)[i]->text;

        for (j = 0; j < wcslen(str); j++)
        {
            if (fd[i]->listIDs[(*printList)[i]->feature][str[j]] == 0)
                fd[i]->makeDisplayList(str[j], (*printList)[i]->feature);
        }
    }

    uint32 origX = x;
    uint32 origY = y;

    glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT  | GL_ENABLE_BIT | GL_TRANSFORM_BIT);
    glMatrixMode(GL_MODELVIEW);
    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    uint32 line = 0;
    j = 0;

    glPushMatrix();
    glTranslatef((GLfloat)x,(GLfloat)y,0);

    linewidth = 0;
    for (i = 0; i < printList->size(); )
    {
        if ((*printList)[i]->colorize)
            glColor4ub(GET_COLOR_R((*printList)[i]->color), GET_COLOR_G((*printList)[i]->color), GET_COLOR_B((*printList)[i]->color), GET_COLOR_A((*printList)[i]->color));

        glListBase(0u);
        const wchar_t *str = (*printList)[i]->text;

        for (; j < wcslen(str); j++)
        {
            if (wordWrapLimit != WW_NO_WRAP)
            {
                tmpwidth = 0;
                if (IsWrapChar(str[j]) && j+1 < wcslen(str))
                {
                    tmpwidth += fd[i]->charWidth[(*printList)[i]->feature][str[j]];

                    for (k = j+1; k < wcslen(str); k++)
                    {
                        tmpwidth += fd[i]->charWidth[(*printList)[i]->feature][str[k]];

                        if (IsWrapChar(str[k]))
                            break;
                    }

                    if ((wrapMovedStart ? -wordWrapLimit : x)+linewidth+tmpwidth > (wrapMovedStart ? canvasEnd : wordWrapLimit))
                    {
                        glCallList(fd[i]->listIDs[(*printList)[i]->feature][str[j]]);
                        glTranslatef(-(GLfloat)linewidth, h[i]*(line+1), 0);

                        line++;
                        linewidth = 0;
                        j++;

                        goto continueLabel;
                    }
                }
            }

            if (str[j] == L'\n')
            {
                glTranslatef(-(GLfloat)linewidth, h[i]*(line+1), 0);

                line++;
                linewidth = 0;
                j++;

                goto continueLabel;
            }
            linewidth += fd[i]->charWidth[(*printList)[i]->feature][str[j]];
            glCallList(fd[i]->listIDs[(*printList)[i]->feature][str[j]]);
        }

        j = 0;

        i++;

continueLabel:;
    }

    glPopMatrix();

    glPopAttrib();

/*
    if (underline)
    {

    }

    if (strikeout)
    {

    }
*/

    delete[] fd;
    delete[] h;
    delete[] underline;
    delete[] strikeout;
}

uint32 SimplyFlat::t_Drawing::GetTextWidth(int32 fontId, uint32 feature, const wchar_t *fmt, ...)
{
    if (fontId < 0 || m_fontDataMap.size() <= fontId)
        return 0;

    fontData* fd = m_fontDataMap[fontId];
    feature &= (MAX_FA - 1);

    wchar_t str[2048];
    va_list ap;

    if (fmt == NULL)
        *str=0;
    else
    {
        va_start(ap, fmt);
            vswprintf(str, 99999999, fmt, ap);
        va_end(ap);
    }

    uint32 width = 0;

    for (uint32 i = 0; i < wcslen(str); i++)
    {
        if (fd->listIDs[feature][str[i]] == 0)
            fd->makeDisplayList(str[i], feature);

        width += fd->charWidth[feature][str[i]];
    }

    return width;
}

uint32 SimplyFlat::t_Drawing::GetTextAmountToWidth(int32 fontId, uint32 feature, uint32 width, const wchar_t *fmt, ...)
{
    if (fontId < 0 || m_fontDataMap.size() <= fontId)
        return 0;

    fontData* fd = m_fontDataMap[fontId];
    feature &= (MAX_FA - 1);

    wchar_t str[2048];
    va_list ap;

    if (fmt == NULL)
        *str=0;
    else
    {
        va_start(ap, fmt);
            vswprintf(str, 99999999, fmt, ap);
        va_end(ap);
    }

    uint32 pwidth = 0;

    for (uint32 i = 0; i < wcslen(str); i++)
    {
        if (fd->listIDs[feature][str[i]] == 0)
            fd->makeDisplayList(str[i], feature);

        pwidth += fd->charWidth[feature][str[i]];
        if (pwidth > width)
        {
            if (i > 0)
                return (i-1);
            else
                return 0;
        }
    }

    return wcslen(str);
}

uint32 SimplyFlat::t_Drawing::GetTextAmountToWidthFromEnd(int32 fontId, uint32 feature, uint32 width, const wchar_t *fmt, ...)
{
    if (fontId < 0 || m_fontDataMap.size() <= fontId)
        return 0;

    fontData* fd = m_fontDataMap[fontId];
    feature &= (MAX_FA - 1);

    wchar_t str[2048];
    va_list ap;

    if (fmt == NULL)
        *str=0;
    else
    {
        va_start(ap, fmt);
            vswprintf(str, 99999999, fmt, ap);
        va_end(ap);
    }

    uint32 pwidth = 0;

    for (int32 i = wcslen(str)-1; i >= 0; i--)
    {
        if (fd->listIDs[feature][str[i]] == 0)
            fd->makeDisplayList(str[i], feature);

        pwidth += fd->charWidth[feature][str[i]];
        if (pwidth > width)
        {
            if (i < wcslen(str))
                return wcslen(str)-i-1;
            else
                return 0;
        }
    }

    return wcslen(str);
}

uint32 SimplyFlat::t_Drawing::GetFontHeight(int32 fontId)
{
    if (fontId < 0 || m_fontDataMap.size() <= fontId)
        return 0;

    fontData* fd = m_fontDataMap[fontId];

    return (uint32)fd->height;
}
