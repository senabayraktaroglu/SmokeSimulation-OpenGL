#include "Pez.h"

typedef struct Surface_ {
    GLuint FboHandle;
    GLuint TextureHandle;
    int NumComponents;
} Surface;

typedef struct FboInfo_
{
    GLuint FboHandle;
    GLuint TextureHandle;
    GLuint depthBuffer;
    int width;
    int height;
    bool isComplete;
} FboInfo;
typedef struct Slab_ {
    FboInfo Ping;
    FboInfo Pong;
} Slab;

typedef struct Vector2_ {
    int X;
    int Y;
} Vector2;



#define CellSize (1.25f)
#define ViewportWidth (PEZ_VIEWPORT_WIDTH)
#define ViewportHeight (PEZ_VIEWPORT_HEIGHT)
#define GridWidth (ViewportWidth / 2)
#define GridHeight (ViewportHeight / 2)
#define SplatRadius ((float) GridWidth / 4.0f)

static const float AmbientTemperature = 0.0f;
static const float ImpulseTemperature = 10.0f;
static const float ImpulseDensity = 1.0f;
static const int NumJacobiIterations = 40;
static const float TimeStep = 0.125f;
static const float SmokeBuoyancy = 1.0f;
static const float SmokeWeight = 0.05f;
static const float GradientScale = 1.125f / CellSize;
static const float TemperatureDissipation = 0.99f;
static const float VelocityDissipation = 0.99f;
static const float DensityDissipation = 0.9999f;
static const Vector2 ImpulsePosition = { GridWidth / 2, - (int) SplatRadius / 2};

static const int PositionSlot = 0;

GLuint CreateQuad();
GLuint CreateProgram(const char* vsKey, const char* gsKey, const char* fsKey);
Slab CreateSlab(GLsizei width, GLsizei height);
FboInfo initiliazeFbo(int w, int h);
void CreateObstacles(FboInfo dest, int width, int height);
void CreateCircle(FboInfo dest, int width, int height);
void InitSlabOps();
void SwapSurfaces(Slab* slab);
void ClearSurface(FboInfo s, float value);
void Advect(FboInfo velocity, FboInfo source,  FboInfo dest, float dissipation);
void Jacobi(FboInfo pressure, FboInfo divergence, FboInfo dest);
void SubtractGradient(FboInfo velocity, FboInfo pressure,  FboInfo dest);
void ComputeDivergence(FboInfo velocity, FboInfo dest);
void ApplyImpulse(FboInfo dest, Vector2 position, float value);
void ApplyBuoyancy(FboInfo velocity, FboInfo temperature, FboInfo density, FboInfo dest);
