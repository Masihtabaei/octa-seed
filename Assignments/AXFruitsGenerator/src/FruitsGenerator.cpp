#include <fstream>
#include <gimslib/d3d/DX12App.hpp>
#include <gimslib/d3d/DX12Util.hpp>
#include <gimslib/types.hpp>
#include <gimslib/ui/ExaminerController.hpp>
#include <imgui.h>
#include <iostream>

using namespace gims;

class SphereRenderer : public DX12App
{
private:
  struct UiData
  {
    f32v3 m_backgroundColor     = {0.0f, 0.0f, 0.0f};
    i32   m_intraLevelOfDetails = 1;
    bool   m_flatShading = false;
    f32v3 m_firstControlPoint   = f32v3(0.0f, 0.0f, -0.3f);
    f32v3 m_secondControlPoint  = f32v3(1.0f, 0.0f, -0.7f);
    f32v3 m_thirdControlPoint   = f32v3(1.0f, 0.0f, 0.3f);
    f32v3 m_fourthControlPoint  = f32v3(0.f, 0.0f, 1.0f);
    char   m_shaderName[100]     = "default";
  };

  UiData m_uiData;

  ComPtr<ID3D12PipelineState> m_pipelineState;
  ComPtr<ID3D12PipelineState> m_wireFramePipelineState;
  ComPtr<ID3D12RootSignature> m_rootSignature;

  gims::ExaminerController m_examinerController;

  void createRootSignature()
  {
    CD3DX12_ROOT_PARAMETER rootParameters[1] = {};
    rootParameters[0].InitAsConstants(49, 0, 0, D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
    descRootSignature.Init(1, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> rootBlob, errorBlob;
    D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob);

    getDevice()->CreateRootSignature(0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(),
                                     IID_PPV_ARGS(&m_rootSignature));

    std::cout << "Root signature created successfully!" << std::endl;
  }


  std::string FormatFloat3(const f32v3& controlPoint)
  {
    return std::format("{:.3f}, {:.3f}, {:.3f}", controlPoint[0], controlPoint[1], controlPoint[2]);
  }


  void generateShaderFile()
  {

    std::string shaderCode = R"(static const uint NUM_THREADS_X = 128;
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
    return P0 +
           t * (-3 * P0 + 3 * P1) +
           T_QUADRAT * (3 * P0 - 6 * P1 + 3 * P2) +
           T_CUBED * (-P0 + 3 * P1 - 3 * P2 + P3);
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
    float3 n = normalize(cross(ddx(evaluatedCoordinates), ddy(evaluatedCoordinates)));
    
    float3 v = normalize(-input.viewSpacePosition);
    float3 h = normalize(l + v);
    float f_diffuse = max(0.0f, dot(n, l));
    float f_specular = pow(max(0.0f, dot(n, h)), SPECULAR_EXPONENT.w);
    return float4(AMBIENT_COLOR + f_diffuse * DIFFUSE_COLOR * TEXTURE_COLOR +
                      f_specular * SPECULAR_EXPONENT.xyz, 1);
})";

    static int  fileCounter = 0;
    std::string filePath    = std::format("../../../assignments/AXFruitsGenerator/"
                                             "{}.hlsl",
                                                m_uiData.m_shaderName);

    std::ofstream outfile(filePath);
    if (!outfile.is_open())
    {
      throw std::ios_base::failure("Failed to open output file: " + filePath);
    }

    const std::string searchString   = "static const float3 TEXTURE_COLOR = float3(1.0f, 0.0f, 0.0f);";
    size_t            insertPosition = shaderCode.find(searchString);

    const std::string generatedContent = std::format("\nstatic const float3 P0 = float3({});\n"
                                                     "static const float3 P1 = float3({});\n"
                                                     "static const float3 P2 = float3({});\n"
                                                     "static const float3 P3 = float3({});\n",
                                                     FormatFloat3(m_uiData.m_firstControlPoint),
                                                     FormatFloat3(m_uiData.m_secondControlPoint),
                                                     FormatFloat3(m_uiData.m_thirdControlPoint),
                                                     FormatFloat3(m_uiData.m_fourthControlPoint));

    shaderCode.insert(insertPosition + searchString.size(), "\n" + generatedContent);

    outfile << shaderCode;
    outfile.close();
  }

  void createPipeline()
  {
    const auto meshShader = compileShader(
        L"../../../assignments/AXFruitsGenerator/shaders/Fruits.hlsl", L"MS_main", L"ms_6_5");
    const auto pixelShader = compileShader(
        L"../../../assignments/AXFruitsGenerator/shaders/Fruits.hlsl", L"PS_main", L"ps_6_5");

    D3DX12_MESH_SHADER_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature                         = m_rootSignature.Get();
    psoDesc.MS                                     = HLSLCompiler::convert(meshShader);
    psoDesc.PS                                     = HLSLCompiler::convert(pixelShader);
    psoDesc.RasterizerState                        = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.FillMode               = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode               = D3D12_CULL_MODE_BACK;
    psoDesc.BlendState                             = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DSVFormat                              = getDX12AppConfig().depthBufferFormat;
    psoDesc.DepthStencilState                      = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable          = TRUE;
    psoDesc.DepthStencilState.StencilEnable        = FALSE;
    psoDesc.SampleMask                             = UINT_MAX;
    psoDesc.NumRenderTargets                       = 1;
    psoDesc.RTVFormats[0]                          = getDX12AppConfig().renderTargetFormat;
    psoDesc.DSVFormat                              = getDX12AppConfig().depthBufferFormat;
    psoDesc.SampleDesc.Count                       = 1;
    psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
    psoDesc.DepthStencilState.DepthWriteMask                 = D3D12_DEPTH_WRITE_MASK_ALL;

    auto psoStream = CD3DX12_PIPELINE_MESH_STATE_STREAM(psoDesc);

    D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
    streamDesc.pPipelineStateSubobjectStream = &psoStream;
    streamDesc.SizeInBytes                   = sizeof(psoStream);

    throwIfFailed(getDevice()->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_pipelineState)));

    std::cout << "Pipeline created successfully!" << std::endl;
  }

public:
  SphereRenderer(const DX12AppConfig createInfo)
      : DX12App(createInfo)
      , m_examinerController(true)
  {
    m_examinerController.setTranslationVector(f32v3(0.0f, 0.0f, 3.0f));
    createRootSignature();
    createPipeline();
  }

  void checkForMeshShaderSupport()
  {
    D3D12_FEATURE_DATA_D3D12_OPTIONS7 featureData = {};
    getDevice()->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS7, &featureData, sizeof(featureData));
    if (featureData.MeshShaderTier < D3D12_MESH_SHADER_TIER_1)
    {
      throw std::runtime_error("Your device sadly does not support mesh shaders!");
    }
  }

  virtual void onDraw()
  {
    if (!ImGui::GetIO().WantCaptureMouse)
    {
      bool pressed  = ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right);
      bool released = ImGui::IsMouseReleased(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Right);
      if (pressed || released)
      {
        bool left = ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Left);
        m_examinerController.click(pressed, left == true ? 1 : 2,
                                   ImGui::IsKeyDown(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_RightCtrl),
                                   getNormalizedMouseCoordinates());
      }
      else
      {
        m_examinerController.move(getNormalizedMouseCoordinates());
      }
    }

    const auto commandList = getCommandList();
    const auto rtvHandle   = getRTVHandle();
    const auto dsvHandle   = getDSVHandle();
    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    f32v4 clearColor = {m_uiData.m_backgroundColor[0], m_uiData.m_backgroundColor[1], m_uiData.m_backgroundColor[2],
                        1.0f};
    commandList->ClearRenderTargetView(rtvHandle, &clearColor.x, 0, nullptr);
    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

    commandList->RSSetViewports(1, &getViewport());
    commandList->RSSetScissorRects(1, &getRectScissor());
    const auto projectionMatrix =
        glm::perspectiveFovLH_ZO<f32>(glm::radians(45.0f), (f32)getWidth(), (f32)getHeight(), 0.0001f, 10000.0f);
    const auto viewMatrix = m_examinerController.getTransformationMatrix();

    commandList->SetPipelineState(m_pipelineState.Get());
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    commandList->SetGraphicsRoot32BitConstants(0, 16, &viewMatrix, 0);
    commandList->SetGraphicsRoot32BitConstants(0, 16, &projectionMatrix, 16);

    f32v4 p0 = f32v4(m_uiData.m_firstControlPoint, 0.0f);
    f32v4 p1 = f32v4(m_uiData.m_secondControlPoint, 0.0f);
    f32v4 p2 = f32v4(m_uiData.m_thirdControlPoint, 0.0f);
    f32v4 p3 = f32v4(m_uiData.m_fourthControlPoint, m_uiData.m_flatShading);
    commandList->SetGraphicsRoot32BitConstants(0, 4, &p0, 32);
    commandList->SetGraphicsRoot32BitConstants(0, 4, &p1, 36);
    commandList->SetGraphicsRoot32BitConstants(0, 4, &p2, 40);
    commandList->SetGraphicsRoot32BitConstants(0, 4, &p3, 44);

    commandList->SetGraphicsRoot32BitConstants(0, 1, &m_uiData.m_intraLevelOfDetails, 48);

    commandList->DispatchMesh(1, 1, 1);
  }

  virtual void onDrawUI()
  {
    ImGui::Begin("Information");
    ImGui::Text("Frame time: %f", 1.0f / ImGui::GetIO().Framerate * 1000.0f);
    ImGui::End();
    ImGui::Begin("Configuration");
    ImGui::ColorEdit3("Background Color", &m_uiData.m_backgroundColor[0]);
    ImGui::SliderInt("Inter Level of Detail", &m_uiData.m_intraLevelOfDetails, 1, 28);
    ImGui::Checkbox("Flat Shading", &m_uiData.m_flatShading);
    ImGui::SliderFloat3("First Control Point", &m_uiData.m_firstControlPoint.x, -5, 5);
    ImGui::SliderFloat3("Second Control Point", &m_uiData.m_secondControlPoint.x, -5, 5);
    ImGui::SliderFloat3("Third Control Point", &m_uiData.m_thirdControlPoint.x, -5, 5);
    ImGui::SliderFloat3("Foruth Control Point", &m_uiData.m_fourthControlPoint.x, -5, 5);
    ImGui::InputText("Shader Name", m_uiData.m_shaderName, 100);
    if (ImGui::Button("Save Shader"))
      generateShaderFile();
    ImGui::End();
  }
};

int main(int /* argc*/, char /* **argv */)
{
  gims::DX12AppConfig config;
  config.title    = L"Assignment X Fruits Generator";
  config.useVSync = false;
  try
  {
    SphereRenderer app(config);
    app.checkForMeshShaderSupport();
    app.run();
  }
  catch (const std::exception& e)
  {
    std::cerr << "Error: " << e.what() << "\n";
  }
  if (config.debug)
  {
    std::atexit(DX12Util::reportLiveObjects);
  }
  return 0;
}
