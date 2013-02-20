#include <Defines.h>
#include <SimplyFlat.h>

void SimplyFlat::BeforeDraw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
}

void SimplyFlat::AfterDraw()
{
#ifdef _WIN32
    SwapBuffers(hDC);
#else
    glutSwapBuffers();
#endif
}

int32 SimplyFlat::BuildFont(const char *fontFileOrName, uint32 height, uint16 bold, bool italic, bool underline, bool strikeout)
{
    int32 pos = (int32)Drawing->fontDataMapSize();

    fontData* fd = new fontData;
    if (fd->init(fontFileOrName, height))
    {
        Drawing->SetFontData(pos, fd);
        return pos;
    }
    return -1;
}

SimplyFlat::t_Drawing::t_Drawing()
{
    m_fontPrecache = true;
}

void SimplyFlat::t_Drawing::DrawRectangle(uint32 x, uint32 y, uint32 width, uint32 height, uint32 color, uint32 texture)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    if (texture > 0 && SF->TextureStorage->GetGLTextureID(texture) > 0)
    {
        if (color)
            glColor4ub(GET_COLOR_R(color), GET_COLOR_G(color), GET_COLOR_B(color), GET_COLOR_A(color));
        else
            glColor3ub(255, 255, 255);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D , SF->TextureStorage->GetGLTextureID(texture));
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
        glColor4ub(GET_COLOR_R(color), GET_COLOR_G(color), GET_COLOR_B(color), GET_COLOR_A(color));
    }

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2d(x, y);
        glTexCoord2f(1.0f, 0.0f); glVertex2d(x+width, y);
        glTexCoord2f(1.0f, 1.0f); glVertex2d(x+width, y+height);
        glTexCoord2f(0.0f, 1.0f); glVertex2d(x, y+height);
    glEnd();

    glDisable(GL_BLEND);
}

void SimplyFlat::t_Drawing::DrawRectangleGradient(uint32 x, uint32 y, uint32 width, uint32 height, uint32 colorSrc, uint32 colorDst, uint8 vertexOptions)
{
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glColor4ub(GET_COLOR_R(colorSrc), GET_COLOR_G(colorSrc), GET_COLOR_B(colorSrc), GET_COLOR_A(colorSrc));

    bool source = true;

    #define TO_SRC_CLR glColor4ub(GET_COLOR_R(colorSrc), GET_COLOR_G(colorSrc), GET_COLOR_B(colorSrc), GET_COLOR_A(colorSrc))
    #define TO_DST_CLR glColor4ub(GET_COLOR_R(colorDst), GET_COLOR_G(colorDst), GET_COLOR_B(colorDst), GET_COLOR_A(colorDst))

    glBegin(GL_QUADS);
        if (vertexOptions & VERT_LU)
            TO_DST_CLR;
        else
            TO_SRC_CLR;
        glTexCoord2f(0.0f, 0.0f); glVertex2d(x, y);

        if (vertexOptions & VERT_RU)
            TO_DST_CLR;
        else
            TO_SRC_CLR;
        glTexCoord2f(1.0f, 0.0f); glVertex2d(x+width, y);

        if (vertexOptions & VERT_RL)
            TO_DST_CLR;
        else
            TO_SRC_CLR;
        glTexCoord2f(1.0f, 1.0f); glVertex2d(x+width, y+height);

        if (vertexOptions & VERT_LL)
            TO_DST_CLR;
        else
            TO_SRC_CLR;
        glTexCoord2f(0.0f, 1.0f); glVertex2d(x, y+height);
    glEnd();

    #undef TO_SRC_CLR
    #undef TO_DST_CLR
}

void SimplyFlat::t_Drawing::DrawCircle(uint32 center_x, uint32 center_y, float radius, uint32 color)
{
    glColor3ub(uint8(color >> 16), uint8(color >> 8), uint8(color));

    glBegin(GL_TRIANGLE_FAN);
    glVertex2d(center_x, center_y);
    for (float c = 0.0f; c <= 2*M_PI+0.5f; c += 0.5f)
    {
        glVertex2d(center_x+radius*cos(c), center_y+radius*sin(c));
    }
    glEnd();
}

void SimplyFlat::t_Drawing::ClearColor(uint8 r, uint8 g, uint8 b)
{
    glClearColor(GLclampf(r)/255.0f, GLclampf(g)/255.0f, GLclampf(b)/255.0f, 0);
}
