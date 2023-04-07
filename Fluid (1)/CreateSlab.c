#include "Fluid.h"



Slab CreateSlab(GLsizei width, GLsizei height)
{
    Slab slab;
    slab.Ping = initiliazeFbo(width, height);
    slab.Pong = initiliazeFbo(width, height);
    return slab;
}

Surface CreateSurface(GLsizei width, GLsizei height, int numComponents)
{
    GLuint fboHandle;
    glGenFramebuffers(1, &fboHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);

    GLuint textureHandle;
    glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    const int UseHalfFloats = 1;
    if (UseHalfFloats) {
        switch (numComponents) {
            case 1: glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, width, height, 0, GL_RED, GL_HALF_FLOAT, 0); break;
            case 2: glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_HALF_FLOAT, 0); break;
            case 3: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_HALF_FLOAT, 0); break;
            case 4: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_HALF_FLOAT, 0); break;
            default: PezFatalError("Illegal slab format.");
        }
    } else {
        switch (numComponents) {
            case 1: glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, 0); break;
            case 2: glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, 0); break;
            case 3: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, 0); break;
            case 4: glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0); break;
            default: PezFatalError("Illegal slab format.");
        }
    }

    PezCheckCondition(GL_NO_ERROR == glGetError(), "Unable to create normals texture");

    GLuint colorbuffer;
    glGenRenderbuffers(1, &colorbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, colorbuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureHandle, 0);
    PezCheckCondition(GL_NO_ERROR == glGetError(), "Unable to attach color buffer");
    
    PezCheckCondition(GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_FRAMEBUFFER), "Unable to create FBO.");
    Surface surface = { fboHandle, textureHandle, numComponents };

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return surface;
}


FboInfo initiliazeFbo(int w, int h)
{
    GLuint FboHandle;
    GLuint TextureHandle;
    GLuint depthBuffer;
    int width;
    int height;
    bool isComplete;

    isComplete = false;
    width = w;
    height = h;
    // Generate two textures and set filter parameters (no storage allocated yet)
    glGenTextures(1, &TextureHandle);
    glBindTexture(GL_TEXTURE_2D, TextureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenTextures(1, &depthBuffer);
    glBindTexture(GL_TEXTURE_2D, depthBuffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // allocate storage for textures
    glBindTexture(GL_TEXTURE_2D, TextureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // generate a depth texture
    glBindTexture(GL_TEXTURE_2D, depthBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT,
        nullptr);

    ///////////////////////////////////////////////////////////////////////
    // Generate and bind framebuffer
    ///////////////////////////////////////////////////////////////////////
    // >>> @task 1
    //...
    glGenFramebuffers(1, &FboHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, FboHandle);
    // bind the texture as color attachment 0 (to the currently bound framebuffer)
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TextureHandle, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    // bind the texture as depth attachment (to the currently bound framebuffer)
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthBuffer, 0);

    // check if framebuffer is complete

    // bind default framebuffer, just in case.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    FboInfo newFbo = {FboHandle,TextureHandle,depthBuffer,width, height,isComplete};
    return newFbo;
}
