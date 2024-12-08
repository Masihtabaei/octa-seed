#define NUM_THREADS_X 128
#define NUM_THREADS_Y 1
#define NUM_THREADS_Z 1

#define MAX_NUM_VERTICES 3
#define MAX_NUM_TRIANGLES 1

static const float3 userDefinedVertices[] = { { 0.0f, 0.25f, 0.0f }, { 0.25f, -0.25f, 0.0f }, { -0.25f, -0.25f, 0.0f } };
static const float3 userDefinedColor = float3(1.0f, 0.0f, 0.0f);

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
        MeshShaderOutput firstVertex;
        firstVertex.position = float4(userDefinedVertices[0].x, userDefinedVertices[0].y, userDefinedVertices[0].z, 1.0f);
    
        MeshShaderOutput secondVertex;
        secondVertex.position = float4(userDefinedVertices[1].x, userDefinedVertices[1].y, userDefinedVertices[1].z, 1.0f);
    
        MeshShaderOutput thirdVertex;
        thirdVertex.position = float4(userDefinedVertices[2].x, userDefinedVertices[2].y, userDefinedVertices[2].z, 1.0f);
    
        triangleVertices[0] = firstVertex;
        triangleVertices[1] = secondVertex;
        triangleVertices[2] = thirdVertex;
    }

    if (threadIdInsideItsGroup.x <= MAX_NUM_TRIANGLES)
        triangleIndices[0] = uint3(0, 1, 2);

}

float4 PS_main(MeshShaderOutput input)
    : SV_TARGET
{
    return float4(userDefinedColor, 1.0f);
}
