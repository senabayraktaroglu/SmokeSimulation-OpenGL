
-- Vertex

in vec4 Position;

void main()
{
    gl_Position = Position;
}

-- Fill

out vec3 FragColor;

void main()
{
    FragColor = vec3(1, 0, 0);
}

-- Advect

out vec4 xNew;

uniform sampler2D u;
uniform sampler2D x;

uniform vec2 rdx;
uniform float TimeStep;
uniform float Dissipation;


vec4 f4texRECTbilerp(sampler2D tex, vec2 pos)
{
    return texture2D(tex, pos);
}
vec4 textureRect(in sampler2D tex, vec2 rectangleCoord)
{
	return texture(tex, rectangleCoord / textureSize(tex, 0));
}
void main()
{
    vec2 coords = gl_FragCoord.xy;
    vec2 pos= rdx*coords - TimeStep * rdx * texture2D(u, rdx*coords).xy;
    xNew = Dissipation * f4texRECTbilerp(x, pos);
}



-- Jacobi

out vec4 FragColor;

uniform sampler2D Pressure;
uniform sampler2D Divergence;


uniform float Alpha;
uniform float InverseBeta;

void main()
{
    ivec2 P = ivec2(gl_FragCoord.xy);

    int lod = 0;
    vec4 xL = texelFetchOffset(Pressure, P, lod, ivec2(0, 1));
    vec4 xR = texelFetchOffset(Pressure, P, lod, ivec2(0, -1));
    vec4 xB = texelFetchOffset(Pressure, P, lod, ivec2(1, 0));
    vec4 xT = texelFetchOffset(Pressure, P, lod, ivec2(-1, 0));


    vec4 bC = texelFetch(Divergence, P, 0);
    FragColor = (xL + xR + xB + xT + Alpha * bC) * InverseBeta;
}

-- SubtractGradient

out vec2 FragColor;

uniform sampler2D Velocity;
uniform sampler2D Pressure;
uniform float GradientScale;

void main()
{
    ivec2 T = ivec2(gl_FragCoord.xy);

    float pT = texelFetchOffset(Pressure, T, 0, ivec2(0, 1)).r;
    float pB = texelFetchOffset(Pressure, T, 0, ivec2(0, -1)).r;
    float pR = texelFetchOffset(Pressure, T, 0, ivec2(1, 0)).r;
    float pL = texelFetchOffset(Pressure, T, 0, ivec2(-1, 0)).r;
    float pC = texelFetch(Pressure, T, 0).r;


    vec2 uNew = texelFetch(Velocity, T, 0).xy; 
    uNew -= vec2(pR - pL, pT - pB) * GradientScale;

    FragColor = uNew ;  

 
}

-- ComputeDivergence

out float FragColor;

uniform sampler2D Velocity;
uniform float HalfInverseCellSize;

void main()
{
    ivec2 T = ivec2(gl_FragCoord.xy);

    // Find neighboring velocities:
    vec2 vN = texelFetchOffset(Velocity, T, 0, ivec2(0, 1)).xy;
    vec2 vS = texelFetchOffset(Velocity, T, 0, ivec2(0, -1)).xy;
    vec2 vE = texelFetchOffset(Velocity, T, 0, ivec2(1, 0)).xy;
    vec2 vW = texelFetchOffset(Velocity, T, 0, ivec2(-1, 0)).xy;

    FragColor = HalfInverseCellSize * (vE.x - vW.x + vN.y - vS.y);
}

-- Splat

out vec4 FragColor;

uniform vec2 Point;
uniform float Radius;
uniform vec3 FillColor;

void main()
{
    float d = distance(Point, gl_FragCoord.xy);
    if (d < Radius) {
        float a = (Radius - d) * 0.5;
        a = min(a, 1.0);
        FragColor = vec4(FillColor, a);
    } else {
        FragColor = vec4(0);
    }
}

-- Buoyancy

out vec2 FragColor;
uniform sampler2D Velocity;
uniform sampler2D Temperature;
uniform sampler2D Density;
uniform float AmbientTemperature;
uniform float TimeStep;
uniform float Sigma;
uniform float Kappa;

void main()
{
    ivec2 TC = ivec2(gl_FragCoord.xy);
    float T = texelFetch(Temperature, TC, 0).r;
    vec2 V = texelFetch(Velocity, TC, 0).xy;

    FragColor = V;

    if (T > AmbientTemperature) {
        float D = texelFetch(Density, TC, 0).x;
        FragColor += (TimeStep * (T - AmbientTemperature) * Sigma - D * Kappa ) * vec2(0, 1);
    }
}

-- Visualize

out vec4 FragColor;
uniform sampler2D Sampler;
uniform vec2 Scale;

void main()
{
    float L = texture(Sampler, gl_FragCoord.xy * Scale).r;
    FragColor = vec4(1.0,1.0,1.0, L);
}
