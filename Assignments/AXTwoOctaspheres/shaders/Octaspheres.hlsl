#define NUM_THREADS_X 9
#define NUM_THREADS_Y 9
#define NUM_THREADS_Z 1

#define NUM_OCTASPHERES 2

#define MAX_NUM_VERTICES NUM_OCTASPHERES * NUM_THREADS_X * NUM_THREADS_Y
#define MAX_NUM_TRIANGLES 2 * NUM_OCTASPHERES * ((NUM_THREADS_X - 1) * (NUM_THREADS_Y - 1))

static const float3 userDefinedColor = float3(1.0f, 0.0f, 0.0f);


cbuffer PerMeshConstants : register(b0)
{
    float4x4 transformationMatrix;
}

struct MeshShaderOutput
{
    float4 position : SV_POSITION;
};

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
    if (v.z < 0.0)
    {
        v.xy = (1.0 - abs(v.yx)) * signNotZero(v.xy);
    }
    return normalize(v);
}

[outputtopology("triangle")]
[numthreads(NUM_THREADS_X, NUM_THREADS_Y, NUM_THREADS_Z)]
void MS_main(
    in uint3 threadIdInsideItsGroup : SV_GroupThreadID,
    out vertices MeshShaderOutput triangleVertices[MAX_NUM_VERTICES],
    out indices uint3 triangleIndices[MAX_NUM_TRIANGLES]
)
{
    SetMeshOutputCounts(MAX_NUM_VERTICES, MAX_NUM_TRIANGLES);
    float xCoordinate = map(threadIdInsideItsGroup.x, 0.0f, NUM_THREADS_X - 1, -1.0f, 1.0f);
    float yCoordinate = map(threadIdInsideItsGroup.y, 0.0f, NUM_THREADS_Y - 1, -1.0f, 1.0f);
    float3 decodedResults = octDecode(float2(xCoordinate, yCoordinate));
    triangleVertices[threadIdInsideItsGroup.y * NUM_THREADS_X + threadIdInsideItsGroup.x].position = mul(transformationMatrix, float4(decodedResults.x, decodedResults.y, decodedResults.z, 1.0f));
    triangleVertices[threadIdInsideItsGroup.y * NUM_THREADS_X + threadIdInsideItsGroup.x + (MAX_NUM_VERTICES / 2)].position = mul(transformationMatrix, float4(decodedResults.x + 2.5f, decodedResults.y, decodedResults.z, 1.0f));
    if ((threadIdInsideItsGroup.x < (NUM_THREADS_X - 1)) && (threadIdInsideItsGroup.y < (NUM_THREADS_Y - 1)))
    {
        for (int i = 0; i < 2; i++)
        {
            uint currentVertexID = threadIdInsideItsGroup.y * NUM_THREADS_X + threadIdInsideItsGroup.x + i * (MAX_NUM_VERTICES / 2);
            uint nextMostRightVertexID = currentVertexID + 1;
            uint nextMostBottomVertexID = currentVertexID + NUM_THREADS_X;
            uint nextMostBottomRightVertexID = nextMostBottomVertexID + 1;
            if ((threadIdInsideItsGroup.x < (NUM_THREADS_X / 2) && threadIdInsideItsGroup.y < (NUM_THREADS_Y / 2)) || (threadIdInsideItsGroup.x >= (NUM_THREADS_X / 2)
             && threadIdInsideItsGroup.y >= (NUM_THREADS_Y / 2)))
            {
                triangleIndices[2 * (threadIdInsideItsGroup.y * (NUM_THREADS_X - 1) + threadIdInsideItsGroup.x) + i * (MAX_NUM_TRIANGLES / 2)] = uint3(currentVertexID, nextMostRightVertexID, nextMostBottomVertexID);
                triangleIndices[2 * (threadIdInsideItsGroup.y * (NUM_THREADS_X - 1) + threadIdInsideItsGroup.x) + 1 + i * (MAX_NUM_TRIANGLES / 2)] = uint3(nextMostRightVertexID, nextMostBottomRightVertexID, nextMostBottomVertexID);
            }
            else
            {
                triangleIndices[2 * (threadIdInsideItsGroup.y * (NUM_THREADS_X - 1) + threadIdInsideItsGroup.x) + i * (MAX_NUM_TRIANGLES / 2)] = uint3(currentVertexID, nextMostRightVertexID, nextMostBottomRightVertexID);
                triangleIndices[2 * (threadIdInsideItsGroup.y * (NUM_THREADS_X - 1) + threadIdInsideItsGroup.x) + 1 + i * (MAX_NUM_TRIANGLES / 2)] = uint3(nextMostBottomRightVertexID, nextMostBottomVertexID, currentVertexID);
            }
        }
    }
}

float4 PS_main(MeshShaderOutput input)
    : SV_TARGET
{
    return float4(userDefinedColor, 1.0f);
}
