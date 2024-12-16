#define NUM_THREADS_X 10
#define NUM_THREADS_Y 10
#define NUM_THREADS_Z 1

#define PI radians(180.0f)
#define HALF_PI radians(90.0f)
#define MAX_NUM_VERTICES 100
#define MAX_NUM_TRIANGLES 256

static const float3 userDefinedColor = float3(0.6f, 0.6f, 0.6f);

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


[outputtopology("triangle")]
[numthreads(NUM_THREADS_X, NUM_THREADS_Y, NUM_THREADS_Z)]
void MS_main(
    in uint3 threadIdInsideItsGroup : SV_GroupThreadID,
    out vertices MeshShaderOutput triangleVertices[MAX_NUM_VERTICES],
    out indices uint3 triangleIndices[MAX_NUM_TRIANGLES]
)
{
    SetMeshOutputCounts(MAX_NUM_VERTICES, MAX_NUM_TRIANGLES);
    if ((threadIdInsideItsGroup.x < NUM_THREADS_X) && (threadIdInsideItsGroup.y < NUM_THREADS_Y))
    {
        float lon = (map(float(threadIdInsideItsGroup.x), 0.0f, float(NUM_THREADS_X - 1), -PI, +PI));
        float lat = (map(float(threadIdInsideItsGroup.y), 0.0f, float(NUM_THREADS_Y - 1), -HALF_PI, +HALF_PI));
        float xCoordinate = sin(lon) * cos(lat);
        float yCoordinate = sin(lon) * sin(lat);
        float zCoordinate = cos(lon);
        MeshShaderOutput currentPoint;
        currentPoint.position = mul(transformationMatrix, float4(xCoordinate, yCoordinate, zCoordinate, 1.0f));
        triangleVertices[threadIdInsideItsGroup.x + threadIdInsideItsGroup.y * NUM_THREADS_X] = currentPoint;
        if ((threadIdInsideItsGroup.y < (NUM_THREADS_Y - 1)) && (threadIdInsideItsGroup.x < (NUM_THREADS_X - 1)))
        {
            uint firstVertexId = threadIdInsideItsGroup.x + threadIdInsideItsGroup.y * NUM_THREADS_X;
            uint secondVertexId = firstVertexId + 1;
            uint thirdVertexId = firstVertexId + NUM_THREADS_X;
            uint fourthVertexId = secondVertexId + NUM_THREADS_X;
            triangleIndices[(threadIdInsideItsGroup.x + threadIdInsideItsGroup.y * NUM_THREADS_X) * 2] = uint3(firstVertexId, secondVertexId, thirdVertexId);
            triangleIndices[(threadIdInsideItsGroup.x + threadIdInsideItsGroup.y * NUM_THREADS_X) * 2 + 1] = uint3(secondVertexId, fourthVertexId, thirdVertexId);
        }
    }

}

float4 PS_main(MeshShaderOutput input)
    : SV_TARGET
{
    return float4(userDefinedColor, 1.0f);
}
