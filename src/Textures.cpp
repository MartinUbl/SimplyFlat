#include <Defines.h>
#include <SimplyFlat.h>

uint32 SimplyFlat::t_TextureStorage::LoadTexture(const char* path, uint32 flags)
{
    if (!path || strlen(path) < 2)
        return 0;

    // ID 0 --> blank
    if (m_textureMap.size() == 0)
        m_textureMap.resize(1);

    flags |= SOIL_FLAG_POWER_OF_TWO;

    uint32 channels = SOIL_LOAD_AUTO;
    if (flags & IMAGELOAD_GREYSCALE)
        channels = SOIL_LOAD_LA;

    uint32 id = m_textureMap.size();
    m_textureMap.resize(id+1);

    flags |= SOIL_FLAG_MIPMAPS;

    m_textureMap[id] = SOIL_load_OGL_texture(path, channels, SOIL_CREATE_NEW_ID, flags);

    return id;
}

uint32 SimplyFlat::t_TextureStorage::GetGLTextureID(uint32 id)
{
    if (m_textureMap.size() <= id)
        return 0;

    return m_textureMap[id];
}
