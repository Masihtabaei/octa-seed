#define NUM_THREADS_X 128
#define NUM_THREADS_Y 1
#define NUM_THREADS_Z 1

#define TWO_PI 360
#define MAX_NUM_VERTICES 90
#define MAX_NUM_TRIANGLES 89

static const float3 userDefinedColor = float3(0.8f, 0.2f, 0.0f);
static const float3 circleCenter = float3(0.0f, 0.0f, 0.0f);
static const float circleRadius = 0.5f;

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
        if (threadIdInsideItsGroup.x == 0)
        {
            MeshShaderOutput circleCenterPoint;
            circleCenterPoint.position = float4(circleCenter.x, circleCenter.y, circleCenter.z, 1.0f);
            triangleVertices[0] = circleCenterPoint;
        }
        else
        {
            float valueInRadians = radians(threadIdInsideItsGroup.x * (TWO_PI / MAX_NUM_VERTICES));
            MeshShaderOutput currentCirclePoint;
            float xCoordinate = cos(valueInRadians);
            float yCoordinate = sin(valueInRadians);
            currentCirclePoint.position = float4(xCoordinate, yCoordinate, 0.0f, 1.0f);
            triangleVertices[threadIdInsideItsGroup.x] = currentCirclePoint;
            if (threadIdInsideItsGroup.x == MAX_NUM_TRIANGLES)
            {
                triangleIndices[threadIdInsideItsGroup.x - 1] = uint3(0, 1, threadIdInsideItsGroup.x - 1);               
            }
            else
            {
                triangleIndices[threadIdInsideItsGroup.x - 1] = uint3(0, threadIdInsideItsGroup.x, threadIdInsideItsGroup.x - 1);              
            }
        }
    }        

}

float4 PS_main(MeshShaderOutput input)
    : SV_TARGET
{
    return float4(userDefinedColor, 1.0f);
}
