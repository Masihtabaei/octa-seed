#define NUM_THREADS_X 128
#define NUM_THREADS_Y 1
#define NUM_THREADS_Z 1

#define MAX_NUM_VERTICES 4
#define MAX_NUM_TRIANGLES 2

static const float X = 0.8f;
static const float3 userDefinedVertices[] = { {-X, -X, 0.0f}, {+X, +X, 0.0f}, {+X, -X, 0.0f}, {-X, +X, 0.0f} };
static const uint3 userDefineIndices[] = { {0, 1, 2}, {0, 3, 1} };
static const float3 userDefinedColor = float3(0.0f, 1.0f, 0.0f);

struct MeshShaderOutput
{
  float4 position : SV_POSITION;
};

[outputtopology("triangle")]
[numthreads(NUM_THREADS_X, NUM_THREADS_Y, NUM_THREADS_Z)]
void MS_main(
    in uint3 threadIdInsideItsGroup: SV_GroupThreadID,
    out vertices MeshShaderOutput triangleVertices[MAX_NUM_VERTICES],
    out indices uint3 triangleIndices[MAX_NUM_TRIANGLES]
)
{
    SetMeshOutputCounts(MAX_NUM_VERTICES, MAX_NUM_TRIANGLES);
    
    if (threadIdInsideItsGroup.x <= MAX_NUM_VERTICES)
    {
        MeshShaderOutput currentVertex;
        currentVertex.position = float4(userDefinedVertices[threadIdInsideItsGroup.x].x, userDefinedVertices[threadIdInsideItsGroup.x].y, userDefinedVertices[threadIdInsideItsGroup.x].z, 1.0f);
    
        triangleVertices[threadIdInsideItsGroup.x] = currentVertex;
    }

    if (threadIdInsideItsGroup.x <= MAX_NUM_TRIANGLES)
        triangleIndices[threadIdInsideItsGroup.x] = userDefineIndices[threadIdInsideItsGroup.x];

}

float4 PS_main(MeshShaderOutput input)
    : SV_TARGET
{
    return float4(userDefinedColor, 1.0f);
}
