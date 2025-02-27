static const uint NUM_THREADS_X = 128;
static const uint NUM_THREADS_Y = 1;
static const uint NUM_THREADS_Z = 1;
static const uint NUM_VERTICES = 256;
static const uint NUM_TRIANGLES = 256;

static const float INTER_DISTANCE = 2.5f;

static const float3 LIGHT_DIRECTION = float3(0.0f, 0.0f, -1.0f);
static const float3 AMBIENT_COLOR = float3(0.0f, 0.0f, 0.0f);
static const float3 DIFFUSE_COLOR = float3(1.0f, 1.0f, 1.0f);
static const float4 SPECULAR_EXPONENT = float4(1.0f, 1.0f, 1.0f, 128.0f);
static const float3 TEXTURE_COLOR = float3(1.0f, 0.0f, 0.0f);

cbuffer PerMeshConstants : register(b0)
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4 p0;
    float4 p1;
    float4 p2;
    float4 p3;
    int interLOD;
}

struct MeshShaderOutput
{
    float4 position : SV_POSITION;
    float3 viewSpacePosition : POSITION;
    float3 decodedCoordinates : TEXCOORD0;
    float positionOffset : TEXCOORD1;
};

int calculateIntraLOD(int n)
{
    return (n >= 121 ? 11 : n >= 64 ? 9 : n >= 42 ? 7 : n >= 16 ? 5 : 3);
}

float map(float value, float min1, float max1, float min2, float max2)
{
    return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

float signNotZero(float scalar)
{
    return (scalar >= 0.0) ? 1.0 : -1.0;
}

float2 signNotZero(float2 vec)
{
    return float2(signNotZero(vec.x), signNotZero(vec.y));
}

float3 octDecode(float2 coordinates)
{
    float3 octahedronCoordinates = float3(coordinates.x, coordinates.y, 1.0 - abs(coordinates.x) - abs(coordinates.y));
    if (octahedronCoordinates.z < 0.0f)
    {
        octahedronCoordinates.xy = (1.0 - abs(octahedronCoordinates.yx)) * signNotZero(octahedronCoordinates.xy);
    }
    return normalize(octahedronCoordinates);
}

float3 evaluateCubicBezierCurve(float t)
{
    const float T_QUADRAT = t * t;
    const float T_CUBED = T_QUADRAT * t;
    return p0.xyz +
           t * (-3 * p0.xyz + 3 * p1.xyz) +
           T_QUADRAT * (3 * p0.xyz - 6 * p1.xyz + 3 * p2.xyz) +
           T_CUBED * (-p0.xyz + 3 * p1.xyz - 3 * p2.xyz + p3.xyz);
}

float3 calculateFruitCoordinates(float3 coordinates)
{
    const float T = (coordinates.z + 1.0f) / 2;
    float3 result = evaluateCubicBezierCurve(T);
    
    float2 sinusCosinus = float2(0.0f, 0.0f);
    sinusCosinus = normalize(coordinates.yx);
    sinusCosinus.x = clamp(sinusCosinus.x, -1.0f, 1.0f);
    sinusCosinus.y = sqrt(1 - sinusCosinus.x * sinusCosinus.x) * signNotZero(sinusCosinus.y);
    result.xy = mul(float2x2(sinusCosinus.y, -sinusCosinus.x, sinusCosinus.x, sinusCosinus.y), result.xy);
    
    return result;
}

[outputtopology("triangle")]
[numthreads(NUM_THREADS_X, NUM_THREADS_Y, NUM_THREADS_Z)]
void MS_main(
    in uint3 threadIdInsideItsGroup : SV_GroupThreadID,
    out vertices MeshShaderOutput triangleVertices[NUM_VERTICES],
    out indices uint3 triangleIndices[NUM_TRIANGLES]
)
{
    const uint INTRA_LOD = calculateIntraLOD(NUM_THREADS_X / interLOD);
    const uint INTRA_LOD_QUADRAT = INTRA_LOD * INTRA_LOD;
    const uint INTRA_LOD_DECREMENTED_QUADRAT = INTRA_LOD_QUADRAT - 2 * INTRA_LOD + 1;
    
    const uint X = threadIdInsideItsGroup.x % INTRA_LOD;
    const uint Y = threadIdInsideItsGroup.x / INTRA_LOD;
    
    if (X >= INTRA_LOD)
        return;
    if (Y >= INTRA_LOD)
        return;
    
    SetMeshOutputCounts(interLOD * INTRA_LOD * INTRA_LOD, 2 * interLOD * (INTRA_LOD - 1) * (INTRA_LOD - 1));

    const float X_REMAPPED = map(X, 0.0f, INTRA_LOD - 1, -1.0f, 1.0f);
    const float Y_REMAPPED = map(Y, 0.0f, INTRA_LOD - 1, -1.0f, 1.0f);
    const float3 SPHERICAL_COORDINATES = octDecode(float2(X_REMAPPED, Y_REMAPPED));

    float positionOffset = 0;
    uint index = Y * INTRA_LOD + X;
    float3 coordinates = calculateFruitCoordinates(SPHERICAL_COORDINATES);
    float4 viewSpacePosition = float4(0.0f, 0.0f, 0.0f, 0.0f);
    for (int i = 0; i < interLOD; i++)
    {
        viewSpacePosition = mul(viewMatrix, float4(coordinates, 1.0f));
        triangleVertices[index].position = mul(projectionMatrix, viewSpacePosition);
        triangleVertices[index].viewSpacePosition = viewSpacePosition;
        triangleVertices[index].decodedCoordinates = SPHERICAL_COORDINATES;
        triangleVertices[index].positionOffset = positionOffset;
        index += INTRA_LOD_QUADRAT;
        coordinates.x += INTER_DISTANCE;
        positionOffset += INTER_DISTANCE;

    }

    if (X >= (INTRA_LOD - 1))
        return;
    
    if (Y >= (INTRA_LOD - 1))
        return;
    
    uint current = Y * INTRA_LOD + X;
    uint right = current + 1;
    uint bottom = current + INTRA_LOD;
    uint bottomRight = bottom + 1;
    index = 2 * (Y * (INTRA_LOD - 1) + X);
    bool noFlipNeeded = true;
    for (int i = 0; i < interLOD; i++)
    {
        noFlipNeeded = (X < (INTRA_LOD / 2) && Y < (INTRA_LOD / 2)) || (X >= (INTRA_LOD / 2) && Y >= (INTRA_LOD / 2));
        triangleIndices[index] = noFlipNeeded ? uint3(current, right, bottom) : uint3(current, right, bottomRight);
        triangleIndices[index + 1] = noFlipNeeded ? uint3(right, bottomRight, bottom) : uint3(bottomRight, bottom, current);
        current += INTRA_LOD_QUADRAT;
        right = current + 1;
        bottom = current + INTRA_LOD;
        bottomRight = bottom + 1;
        index += 2 * INTRA_LOD_DECREMENTED_QUADRAT;

    }

}

float4 PS_main(MeshShaderOutput input)
    : SV_TARGET
{
    float3 evaluatedCoordinates = calculateFruitCoordinates(input.decodedCoordinates);
    
    evaluatedCoordinates = mul(viewMatrix, float4(evaluatedCoordinates, 1.0f));
    evaluatedCoordinates.x += input.positionOffset;
    float3 l = normalize(LIGHT_DIRECTION);
    float3 n = p3.w ? normalize(cross(ddx(input.viewSpacePosition), ddy(input.viewSpacePosition))) : normalize(cross(ddx(evaluatedCoordinates), ddy(evaluatedCoordinates)));
    
    float3 v = normalize(-input.viewSpacePosition);
    float3 h = normalize(l + v);
    float f_diffuse = max(0.0f, dot(n, l));
    float f_specular = pow(max(0.0f, dot(n, h)), SPECULAR_EXPONENT.w);
    return float4(AMBIENT_COLOR + f_diffuse * DIFFUSE_COLOR * TEXTURE_COLOR +
                      f_specular * SPECULAR_EXPONENT.xyz, 1);
}

