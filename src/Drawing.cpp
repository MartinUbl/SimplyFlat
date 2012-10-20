#include <Defines.h>
#include <SimplyFlat.h>

void SimplyFlat::BeforeDraw()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
}

void SimplyFlat::AfterDraw()
{
    SwapBuffers(hDC);
}

int32 SimplyFlat::BuildFont(const char *fontFileOrName, uint32 height, uint16 bold, bool italic, bool underline, bool strikeout)
{
    int32 pos = (int32)Drawing->fontDataMapSize();

    fontData* fd = new fontData;
    if (fd->init(fontFileOrName, height, bold, italic, underline, strikeout))
    {
        Drawing->SetFontData(pos, fd);
        return pos;
    }
    return -1;
}

void SimplyFlat::t_Drawing::DrawRectangle(uint32 x, uint32 y, uint32 width, uint32 height, uint32 color, uint32 texture)
{
    if (texture > 0 && SF->TextureStorage->GetGLTextureID(texture) > 0)
    {
        glColor3ub(255, 255, 255);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D , SF->TextureStorage->GetGLTextureID(texture));
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
        glColor3ub(uint8(color >> 16), uint8(color >> 8), uint8(color));
    }

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2d(x, y);
        glTexCoord2f(1.0f, 0.0f); glVertex2d(x+width, y);
        glTexCoord2f(1.0f, 1.0f); glVertex2d(x+width, y+height);
        glTexCoord2f(0.0f, 1.0f); glVertex2d(x, y+height);
    glEnd();
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
