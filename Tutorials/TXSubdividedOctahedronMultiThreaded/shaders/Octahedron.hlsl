#define NUM_THREADS_X 8
#define NUM_THREADS_Y 1
#define NUM_THREADS_Z 1

#define MAX_NUM_VERTICES 6
#define MAX_NUM_TRIANGLES 8

static const float3 userDefinedVertices[] = { { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };
static const uint3 userDefinedIndices[] = { { 1, 5, 2 }, { 2, 5, 3 }, { 3, 5, 4 }, { 4, 5, 1 }, { 1, 0, 4 }, { 4, 0, 3 }, { 3, 0, 2 }, { 2, 0, 1 } };
static const float3 userDefinedColor = float3(0.2f, 0.2f, 0.2f);


cbuffer PerMeshConstants : register(b0)
{
    float4x4 transformationMatrix;
}

struct MeshShaderOutput
{
    float4 position : SV_POSITION;
};


[outputtopology("triangle")]
[numthreads(NUM_THREADS_X, NUM_THREADS_Y, NUM_THREADS_Z)]
void MS_main(
    in uint3 threadIdInsideItsGroup : SV_GroupThreadID,
    out vertices MeshShaderOutput triangleVertices[MAX_NUM_VERTICES],
    out indices uint3 triangleIndices[MAX_NUM_TRIANGLES]
)
{
    SetMeshOutputCounts(MAX_NUM_VERTICES, MAX_NUM_TRIANGLES);
    if (threadIdInsideItsGroup.x < MAX_NUM_VERTICES)
    {
        triangleVertices[threadIdInsideItsGroup.x].position = mul(transformationMatrix, float4(userDefinedVertices[threadIdInsideItsGroup.x], 1.0f));
    }
    triangleIndices[threadIdInsideItsGroup.x] = userDefinedIndices[threadIdInsideItsGroup.x];


}

float4 PS_main(MeshShaderOutput input)
    : SV_TARGET
{
    return float4(userDefinedColor, 1.0f);
}
