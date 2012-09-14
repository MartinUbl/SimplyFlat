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

uint32 SimplyFlat::BuildFont(const char *fontFile, uint32 height)
{
    uint32 pos = Drawing->fontDataMapSize();

    fontData* fd = new fontData;
    fd->init(fontFile, height);

    Drawing->SetFontData(pos, fd);

    return pos;
}

void SimplyFlat::t_Drawing::DrawRectangle(uint32 top, uint32 left, uint32 width, uint32 height, uint32 color)
{
    glBegin(GL_QUADS);
      glColor3ub(uint8(color >> 16), uint8(color >> 8), uint8(color));
        glTexCoord2f(1.0f, 1.0f); glVertex2d(top, left);
        glTexCoord2f(1.0f, 0.0f); glVertex2d(top, left+width);
        glTexCoord2f(0.0f, 0.0f); glVertex2d(top+height, left+width);
        glTexCoord2f(0.0f, 1.0f); glVertex2d(top+height, left);
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
    glClearColor(r, g, b, 0);
}
