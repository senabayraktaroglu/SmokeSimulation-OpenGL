#include "Fluid.h"
#include <math.h>

struct ProgramsRec {
    GLuint Advect;
    GLuint Jacobi;
    GLuint SubtractGradient;
    GLuint ComputeDivergence;
    GLuint ApplyImpulse;
    GLuint ApplyBuoyancy;
} Programs;

static void ResetState()
{
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_BLEND);
}

void InitSlabOps()
{
    Programs.Advect = CreateProgram("Fluid2.Vertex", 0, "Fluid2.Advect");
    Programs.Jacobi = CreateProgram("Fluid2.Vertex", 0, "Fluid2.Jacobi");
    Programs.SubtractGradient = CreateProgram("Fluid2.Vertex", 0, "Fluid2.SubtractGradient");
    Programs.ComputeDivergence = CreateProgram("Fluid2.Vertex", 0, "Fluid2.ComputeDivergence");
    Programs.ApplyImpulse = CreateProgram("Fluid2.Vertex", 0, "Fluid2.Splat");
    Programs.ApplyBuoyancy = CreateProgram("Fluid2.Vertex", 0, "Fluid2.Buoyancy");
}

void SwapSurfaces(Slab* slab)
{
    FboInfo temp = slab->Ping;
    slab->Ping = slab->Pong;
    slab->Pong = temp;
}



void ClearSurface(FboInfo s, float v)
{
    glBindFramebuffer(GL_FRAMEBUFFER, s.FboHandle);
    glClearColor(v, v, v, v);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Advect(FboInfo velocity, FboInfo source,  FboInfo dest, float dissipation)
{
    GLuint p = Programs.Advect;
    glUseProgram(p);

    GLint rdx = glGetUniformLocation(p, "rdx");
    GLint timeStep = glGetUniformLocation(p, "TimeStep");
    GLint dissLoc = glGetUniformLocation(p, "Dissipation");
    GLint x = glGetUniformLocation(p, "x");
    //GLint obstaclesTexture = glGetUniformLocation(p, "Obstacles");

    glUniform2f(rdx, 1.0f / GridWidth, 1.0f / GridHeight);
    glUniform1f(timeStep, TimeStep);
    glUniform1f(dissLoc, dissipation);
    glUniform1i(x, 1);
    //glUniform1i(obstaclesTexture, 2);

    glBindFramebuffer(GL_FRAMEBUFFER, dest.FboHandle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, velocity.TextureHandle);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, source.TextureHandle);
    //glActiveTexture(GL_TEXTURE2);
    //glBindTexture(GL_TEXTURE_2D, obstacles.TextureHandle);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ResetState();
}

void Jacobi(FboInfo pressure, FboInfo divergence,  FboInfo dest)
{
    GLuint p = Programs.Jacobi;
    glUseProgram(p);

    GLint alpha = glGetUniformLocation(p, "Alpha");
    GLint inverseBeta = glGetUniformLocation(p, "InverseBeta");
    GLint dSampler = glGetUniformLocation(p, "Divergence");
    GLint oSampler = glGetUniformLocation(p, "Obstacles");

    glUniform1f(alpha, -CellSize * CellSize);
    glUniform1f(inverseBeta, 0.25f);
    glUniform1i(dSampler, 1);
    glUniform1i(oSampler, 2);

    glBindFramebuffer(GL_FRAMEBUFFER, dest.FboHandle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pressure.TextureHandle);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, divergence.TextureHandle);
    //glActiveTexture(GL_TEXTURE2);
    //glBindTexture(GL_TEXTURE_2D, obstacles.TextureHandle);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ResetState();
}

void SubtractGradient(FboInfo velocity, FboInfo pressure,  FboInfo dest)
{
    GLuint p = Programs.SubtractGradient;
    glUseProgram(p);

    GLint gradientScale = glGetUniformLocation(p, "GradientScale");
    glUniform1f(gradientScale, GradientScale);
    GLint halfCell = glGetUniformLocation(p, "HalfInverseCellSize");
    glUniform1f(halfCell, 0.5f / CellSize);
    GLint sampler = glGetUniformLocation(p, "Pressure");
    glUniform1i(sampler, 1);
    sampler = glGetUniformLocation(p, "Obstacles");
    glUniform1i(sampler, 2);

    glBindFramebuffer(GL_FRAMEBUFFER, dest.FboHandle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, velocity.TextureHandle);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pressure.TextureHandle);
    //glActiveTexture(GL_TEXTURE2);
    //glBindTexture(GL_TEXTURE_2D, obstacles.TextureHandle);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ResetState();
}

void ComputeDivergence(FboInfo velocity, FboInfo dest)
{
    GLuint p = Programs.ComputeDivergence;
    glUseProgram(p);

    GLint halfCell = glGetUniformLocation(p, "HalfInverseCellSize");
    glUniform1f(halfCell, 0.5f / CellSize);
    GLint sampler = glGetUniformLocation(p, "Obstacles");
    glUniform1i(sampler, 1);

    glBindFramebuffer(GL_FRAMEBUFFER, dest.FboHandle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, velocity.TextureHandle);
    //glActiveTexture(GL_TEXTURE1);
    //glBindTexture(GL_TEXTURE_2D, obstacles.TextureHandle);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ResetState();
}

void ApplyImpulse(FboInfo dest, Vector2 position, float value)
{
    GLuint p = Programs.ApplyImpulse;
    glUseProgram(p);

    GLint pointLoc = glGetUniformLocation(p, "Point");
    GLint radiusLoc = glGetUniformLocation(p, "Radius");
    GLint fillColorLoc = glGetUniformLocation(p, "FillColor");

    glUniform2f(pointLoc, (float) position.X, (float) position.Y);
    glUniform1f(radiusLoc, SplatRadius);
    glUniform3f(fillColorLoc, value, value, value);

    glBindFramebuffer(GL_FRAMEBUFFER, dest.FboHandle);
    glEnable(GL_BLEND);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ResetState();
}

void ApplyBuoyancy(FboInfo velocity, FboInfo temperature, FboInfo density, FboInfo dest)
{
    GLuint p = Programs.ApplyBuoyancy;
    glUseProgram(p);

    GLint tempSampler = glGetUniformLocation(p, "Temperature");
    GLint inkSampler = glGetUniformLocation(p, "Density");
    GLint ambTemp = glGetUniformLocation(p, "AmbientTemperature");
    GLint timeStep = glGetUniformLocation(p, "TimeStep");
    GLint sigma = glGetUniformLocation(p, "Sigma");
    GLint kappa = glGetUniformLocation(p, "Kappa");

    glUniform1i(tempSampler, 1);
    glUniform1i(inkSampler, 2);
    glUniform1f(ambTemp, AmbientTemperature);
    glUniform1f(timeStep, TimeStep);
    glUniform1f(sigma, SmokeBuoyancy);
    glUniform1f(kappa, SmokeWeight);

    glBindFramebuffer(GL_FRAMEBUFFER, dest.FboHandle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, velocity.TextureHandle);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, temperature.TextureHandle);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, density.TextureHandle);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ResetState();
}
