#define NUM_THREADS_X 8
#define NUM_THREADS_Y 1
#define NUM_THREADS_Z 1

#define MAX_NUM_VERTICES 48
#define MAX_NUM_TRIANGLES 32

static const float3 userDefinedVertices[] = { { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } };
static const uint3 userDefinedIndices[] = { { 1, 5, 2 }, { 2, 5, 3 }, { 3, 5, 4 }, { 4, 5, 1 }, { 1, 0, 4 }, { 4, 0, 3 }, { 3, 0, 2 }, { 2, 0, 1 } };
static const float3 userDefinedColor = float3(0.8f, 0.8f, 0.8f);


cbuffer PerMeshConstants : register(b0)
{
    float4x4 transformationMatrix;
    float customLength;
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
    uint3 originalIndices = userDefinedIndices[threadIdInsideItsGroup.x];
    
    float4 firstOriginalVertex = float4(userDefinedVertices[originalIndices.x], 1.0f);
    float4 secondOriginalVertex = float4(userDefinedVertices[originalIndices.y], 1.0f);
    float4 thirdOriginalVertex = float4(userDefinedVertices[originalIndices.z], 1.0f);
    
    float4 firstToSecondVertex = float4((firstOriginalVertex + secondOriginalVertex) / length((firstOriginalVertex + secondOriginalVertex)));
    float4 firstToThirdVertex = float4((firstOriginalVertex + thirdOriginalVertex) / length((firstOriginalVertex + thirdOriginalVertex)));
    float4 secondToThirdVertex = float4((secondOriginalVertex + thirdOriginalVertex) / length((secondOriginalVertex + thirdOriginalVertex)));

    triangleVertices[threadIdInsideItsGroup.x * 6].position = mul(transformationMatrix, firstOriginalVertex);
    triangleVertices[threadIdInsideItsGroup.x * 6 + 1].position = mul(transformationMatrix, firstToSecondVertex);
    triangleVertices[threadIdInsideItsGroup.x * 6 + 2].position = mul(transformationMatrix, secondOriginalVertex);
    triangleVertices[threadIdInsideItsGroup.x * 6 + 3].position = mul(transformationMatrix, secondToThirdVertex);
    triangleVertices[threadIdInsideItsGroup.x * 6 + 4].position = mul(transformationMatrix, thirdOriginalVertex);
    triangleVertices[threadIdInsideItsGroup.x * 6 + 5].position = mul(transformationMatrix, firstToThirdVertex);
    
    triangleIndices[threadIdInsideItsGroup.x * 4] = uint3(threadIdInsideItsGroup.x * 6, threadIdInsideItsGroup.x * 6 + 1, threadIdInsideItsGroup.x * 6 + 5);
    triangleIndices[threadIdInsideItsGroup.x * 4 + 1] = uint3(threadIdInsideItsGroup.x * 6 + 1, threadIdInsideItsGroup.x * 6 + 2, threadIdInsideItsGroup.x * 6 + 3);
    triangleIndices[threadIdInsideItsGroup.x * 4 + 2] = uint3(threadIdInsideItsGroup.x * 6 + 1, threadIdInsideItsGroup.x * 6 + 3, threadIdInsideItsGroup.x * 6 + 5);
    triangleIndices[threadIdInsideItsGroup.x * 4 + 3] = uint3(threadIdInsideItsGroup.x * 6 + 5, threadIdInsideItsGroup.x * 6 + 3, threadIdInsideItsGroup.x * 6 + 4);

}

float4 PS_main(MeshShaderOutput input)
    : SV_TARGET
{
    return float4(userDefinedColor, 1.0f);
}
