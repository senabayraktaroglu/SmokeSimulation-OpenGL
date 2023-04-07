
#include "Fluid.h"

static GLuint QuadVao;
static GLuint VisualizeProgram;
static FboInfo Obstacles, HiresObstacles,Circles;

FboInfo Divergence;


FboInfo AVelocity, ADensity, APressure, ATemperature, BVelocity, BDensity, BPressure, BTemperature;

void Swap(FboInfo& A, FboInfo& B)
{
    FboInfo temp = A;
    A = B;
    B = temp;
}


const char* PezInitialize(int width, int height)
{
    int w = GridWidth;
    int h = GridHeight;

    AVelocity = initiliazeFbo(w, h);
    ADensity = initiliazeFbo(w, h);
    APressure = initiliazeFbo(w, h);
    ATemperature = initiliazeFbo(w, h);


    BVelocity = initiliazeFbo(w, h);
    BDensity = initiliazeFbo(w, h);
    BPressure = initiliazeFbo(w, h);
    BTemperature = initiliazeFbo(w, h);


    Divergence = initiliazeFbo(w, h);
    InitSlabOps();
    VisualizeProgram = CreateProgram("Fluid2.Vertex", 0, "Fluid2.Visualize");

   
    w = ViewportWidth * 2;
    h = ViewportHeight * 2;

    QuadVao = CreateQuad();

    return "Fluid Demo";
}

void PezUpdate(unsigned int elapsedMicroseconds)
{
    glViewport(0, 0, GridWidth, GridHeight);

    Advect(AVelocity, AVelocity, BVelocity, VelocityDissipation);
    Swap(AVelocity, BVelocity);

    Advect(AVelocity, ATemperature,  BTemperature, TemperatureDissipation);
    Swap(ATemperature, BTemperature);

    Advect(AVelocity, ADensity, BDensity, DensityDissipation);    
    Swap(ADensity, BDensity);

    ApplyBuoyancy(AVelocity, ATemperature, ADensity, BVelocity);
    Swap(AVelocity, BVelocity);
   
    ApplyImpulse(ATemperature, ImpulsePosition, ImpulseTemperature);
    ApplyImpulse(ADensity, ImpulsePosition, ImpulseDensity);
    
    ComputeDivergence(AVelocity,  Divergence);
    ClearSurface(APressure, 0);
    
    for (int i = 0; i < 50; ++i) {
        Jacobi(APressure, Divergence, BPressure);
        Swap(APressure, BPressure);
    }
    
    SubtractGradient(AVelocity, APressure,  BVelocity);
    Swap(AVelocity, BVelocity);
    
    
}

void PezRender(GLuint windowFbo)
{
    // Bind visualization shader and set up blend state:
    glUseProgram(VisualizeProgram);
    GLint scale = glGetUniformLocation(VisualizeProgram, "Scale");
    glUniform2f(scale, 1.0f / ViewportWidth, 1.0f / ViewportHeight);
    glEnable(GL_BLEND);

    // Set render target to the backbuffer:
    glViewport(0, 0, ViewportWidth, ViewportHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, windowFbo);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    
    glBindTexture(GL_TEXTURE_2D, ADensity.TextureHandle);
    glUniform2f(scale, 1.0f / ViewportWidth, 1.0f / ViewportHeight);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    

    glDisable(GL_BLEND);
}

void PezHandleMouse(int x, int y, int action)
{
}
