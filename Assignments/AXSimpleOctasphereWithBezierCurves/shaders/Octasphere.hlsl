#define NUM_THREADS_X 11
#define NUM_THREADS_Y 11
#define NUM_THREADS_Z 1
#define NUM_VERTICES 256
#define NUM_INDEX_TRIPLES 256

static const float lightDirectionXCoordinate = float(0.0f);
static const float lightDirectionYCoordinate = float(0.0f);
static const float3 ambientColor = float3(0.0f, 0.0f, 0.0f);
static const float3 diffuseColor = float3(1.0f, 1.0f, 1.0f);
static const float4 specularAndExponent = float4(1.0f, 1.0f, 1.0f, 128.0f);
static const float3 userDefinedColor = float3(1.0f, 0.0f, 0.0f);

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
    float  positionOffset : TEXCOORD1;
};

int calculateIntraLOD(int n) 
{
    return (n >= 121 ? 11 : n >= 64 ? 9 : n >= 42 ? 7 : n >= 16 ? 5 : 3);
}

float map(float value, float min1, float max1, float min2, float max2)
{
    return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

float signNotZero(float k)
{
    return (k >= 0.0) ? 1.0 : -1.0;
}

float2 signNotZero(float2 v)
{
    return float2(signNotZero(v.x), signNotZero(v.y));
}

float3 octDecode(float2 o)
{
    float3 v = float3(o.x, o.y, 1.0 - abs(o.x) - abs(o.y));
    if (v.z < 0.0f)
    {
        v.xy = (1.0 - abs(v.yx)) * signNotZero(v.xy);
    }
    return normalize(v);
}

float3 evaluateCubicBezierCurve(float t)
{
    const float tQuadrat = t * t;
    const float tCubed = tQuadrat * t; 
    return p0.xyz + 
           t * (-3 * p0.xyz + 3 * p1.xyz) + 
           tQuadrat * (3 * p0.xyz - 6 * p1.xyz + 3 * p2.xyz) + 
           tCubed * (- p0.xyz + 3 * p1.xyz - 3 * p2.xyz + p3.xyz);
}

float3 calculateFruitCoordinates(float3 decodedCoordinates)
{
    float t = (decodedCoordinates.z + 1.0f) / 2;
    float3 evaluatedCoordinates = evaluateCubicBezierCurve(t);
    float2 sc = float2(0.0f, 0.0f);    
    sc = normalize(decodedCoordinates.yx);
    sc.x = clamp(sc.x, -1.0f, 1.0f);
    sc.y = sqrt(1 - sc.x * sc.x) * signNotZero(sc.y);
    evaluatedCoordinates.xy = mul(float2x2(sc.y, -sc.x, sc.x, sc.y), evaluatedCoordinates.xy);
    return evaluatedCoordinates;
}

[outputtopology("triangle")]
[numthreads(NUM_THREADS_X, NUM_THREADS_Y, NUM_THREADS_Z)]
void MS_main(
    in uint3 threadIdInsideItsGroup : SV_GroupThreadID,
    out vertices MeshShaderOutput triangleVertices[NUM_VERTICES],
    out indices uint3 triangleIndices[NUM_INDEX_TRIPLES]
)
{
    int intraLOD = calculateIntraLOD(128 / interLOD);
    
    if (threadIdInsideItsGroup.x >= intraLOD)
        return;
    if (threadIdInsideItsGroup.y >= intraLOD)
        return;
    
    SetMeshOutputCounts(interLOD * intraLOD * intraLOD, 2 * interLOD * (intraLOD - 1) * (intraLOD - 1));

    float xCoordinate = map(threadIdInsideItsGroup.x, 0.0f, intraLOD - 1, -1.0f, 1.0f);
    float yCoordinate = map(threadIdInsideItsGroup.y, 0.0f, intraLOD - 1, -1.0f, 1.0f);
    float3 decodedCoordinates = octDecode(float2(xCoordinate, yCoordinate));

    float3 evaluatedCoordinates = calculateFruitCoordinates(decodedCoordinates);
    for (int i = 0; i < interLOD; i++)
    {
        triangleVertices[threadIdInsideItsGroup.y * intraLOD + threadIdInsideItsGroup.x + i * (intraLOD * intraLOD)].position = mul(projectionMatrix, mul(viewMatrix, float4(evaluatedCoordinates.x + i * 2.5f, evaluatedCoordinates.y, evaluatedCoordinates.z, 1.0f)));
        triangleVertices[threadIdInsideItsGroup.y * intraLOD + threadIdInsideItsGroup.x + i * (intraLOD * intraLOD)].viewSpacePosition = mul(viewMatrix, float4(evaluatedCoordinates.x + i * 2.5f, evaluatedCoordinates.y, evaluatedCoordinates.z, 1.0f));
        triangleVertices[threadIdInsideItsGroup.y * intraLOD + threadIdInsideItsGroup.x + i * (intraLOD * intraLOD)].decodedCoordinates = decodedCoordinates;
        triangleVertices[threadIdInsideItsGroup.y * intraLOD + threadIdInsideItsGroup.x + i * (intraLOD * intraLOD)].positionOffset = i * 2.5f;
    }

    if (threadIdInsideItsGroup.x >= (intraLOD - 1))
        return;
    
    if (threadIdInsideItsGroup.y >= (intraLOD - 1))
        return;
    
    
    for (int i = 0; i < interLOD; i++)
    {
        uint currentVertexID = threadIdInsideItsGroup.y * intraLOD + threadIdInsideItsGroup.x + i * (intraLOD * intraLOD);
        uint nextMostRightVertexID = currentVertexID + 1;
        uint nextMostBottomVertexID = currentVertexID + intraLOD;
        uint nextMostBottomRightVertexID = nextMostBottomVertexID + 1;
        if ((threadIdInsideItsGroup.x < (intraLOD / 2) && threadIdInsideItsGroup.y < (intraLOD / 2)) || (threadIdInsideItsGroup.x >= (intraLOD / 2)
            && threadIdInsideItsGroup.y >= (intraLOD / 2)))
        {
            triangleIndices[2 * (threadIdInsideItsGroup.y * (intraLOD - 1) + threadIdInsideItsGroup.x) + i * 2  * (intraLOD - 1) * (intraLOD - 1)] = uint3(currentVertexID, nextMostRightVertexID, nextMostBottomVertexID).xyz;
            triangleIndices[2 * (threadIdInsideItsGroup.y * (intraLOD - 1) + threadIdInsideItsGroup.x) + 1 + i * 2 * (intraLOD - 1) * (intraLOD - 1)] = uint3(nextMostRightVertexID, nextMostBottomRightVertexID, nextMostBottomVertexID).xyz;
        }
        else
        {
            triangleIndices[2 * (threadIdInsideItsGroup.y * (intraLOD - 1) + threadIdInsideItsGroup.x) + i * 2  * (intraLOD - 1) * (intraLOD - 1)] = uint3(currentVertexID, nextMostRightVertexID, nextMostBottomRightVertexID).xyz;
            triangleIndices[2 * (threadIdInsideItsGroup.y * (intraLOD - 1) + threadIdInsideItsGroup.x) + 1 + i * 2 * (intraLOD - 1) * (intraLOD - 1)] = uint3(nextMostBottomRightVertexID, nextMostBottomVertexID, currentVertexID).xyz;
        }
    }

}

float4 PS_main(MeshShaderOutput input)
    : SV_TARGET
{
    float3 evaluatedCoordinates = calculateFruitCoordinates(input.decodedCoordinates);
    
    evaluatedCoordinates = mul(viewMatrix, float4(evaluatedCoordinates, 1.0f));
    evaluatedCoordinates.x += input.positionOffset;
    float3 lightDirection = float3(lightDirectionXCoordinate, lightDirectionYCoordinate, -1.0f);
    float3 l = normalize(lightDirection);
    float3 n = p3.w ? normalize(cross(ddx(input.viewSpacePosition), ddy(input.viewSpacePosition))) : normalize(cross(ddx(evaluatedCoordinates), ddy(evaluatedCoordinates)));
    
    float3 v = normalize(-input.viewSpacePosition);
    float3 h = normalize(l + v);
    float f_diffuse = max(0.0f, dot(n, l));
    float f_specular = pow(max(0.0f, dot(n, h)), specularAndExponent.w);
    float3 textureColor = float4(1.0f, 0.0f, 0.0f, 0.0f);
    return float4(ambientColor.xyz + f_diffuse * diffuseColor.xyz * textureColor.xyz +
                      f_specular * specularAndExponent.xyz, 1);
}
