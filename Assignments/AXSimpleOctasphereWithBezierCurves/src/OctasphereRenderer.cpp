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


  void GenerateShaderFile()
  {

    std::string shaderCode = R"(#define NUM_THREADS_X 11
#define NUM_THREADS_Y 11
#define NUM_THREADS_Z 1

static const float3 userDefinedColor = float3(1.0f, 0.0f, 0.0f);

cbuffer PerMeshConstants : register(b0)
{
    float4x4 transformationMatrix;
    int intraLOD;
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
    if (v.z < 0.0f)
    {
        v.xy = (1.0 - abs(v.yx)) * signNotZero(v.xy);
    }
    return normalize(v);
}

float3 evaluateQuadraticBezier(float t)
{
    float3 firstLerp = lerp(p0.xyz, p1.xyz, t);
    float3 secondLerp = lerp(p2.xyz, p3.xyz, t);
    float3 result = lerp(firstLerp, secondLerp, t);

    return result;
}

float3 evaluateCubicBezierCurve(float t)
{
    float3 firstLerp = lerp(p0.xyz, p1.xyz, t);
    float3 secondLerp = lerp(p1.xyz, p2.xyz, t);
    float3 thirdLerp = lerp(p2.xyz, p3.xyz, t);
    float3 fourthLerp = lerp(firstLerp, secondLerp, t);
    float3 fifthLerp = lerp(secondLerp, thirdLerp, t);
    float3 result = lerp(fourthLerp, fifthLerp, t);

    return result;
}

[outputtopology("triangle")]
[numthreads(NUM_THREADS_X, NUM_THREADS_Y, NUM_THREADS_Z)]
void MS_main(
    in uint3 threadIdInsideItsGroup : SV_GroupThreadID,
    out vertices MeshShaderOutput triangleVertices[NUM_THREADS_X * NUM_THREADS_Y],
    out indices uint3 triangleIndices[2 * (NUM_THREADS_X - 1) * (NUM_THREADS_Y - 1)]
)
{
    SetMeshOutputCounts(intraLOD * intraLOD, 2 * (intraLOD - 1) * (intraLOD - 1));
    if (threadIdInsideItsGroup.x < intraLOD && threadIdInsideItsGroup.y < intraLOD)
    {
        float xCoordinate = map(threadIdInsideItsGroup.x, 0.0f, intraLOD - 1, -1.0f, 1.0f);
        float yCoordinate = map(threadIdInsideItsGroup.y, 0.0f, intraLOD - 1, -1.0f, 1.0f);
        float3 decodedCoordinates = octDecode(float2(xCoordinate, yCoordinate));

        float t = (decodedCoordinates.z + 1.0f) / 2;
        float3 evaluatedCoordinates = evaluateCubicBezierCurve(t);
        float2 sc = float2(0.0f, 0.0f);
       
        sc = normalize(decodedCoordinates.yx);
        sc.x = clamp(sc.x, -1.0f, 1.0f);
        sc.y = sqrt(1 - sc.x * sc.x) * signNotZero(sc.y);
        evaluatedCoordinates.xy = mul(float2x2(sc.y, -sc.x, sc.x, sc.y), evaluatedCoordinates.xy);
   
        //float2 angle = atan2(decodedCoordinates.y, decodedCoordinates.x);
        //evaluatedCoordinates.xy = mul(float2x2(cos(angle.x), -sin(angle.x), sin(angle.x), cos(angle.x)), evaluatedCoordinates.xy);
        
        triangleVertices[threadIdInsideItsGroup.y * intraLOD + threadIdInsideItsGroup.x].position = mul(transformationMatrix, float4(evaluatedCoordinates, 1.0f));

        if ((threadIdInsideItsGroup.x < (intraLOD - 1)) && (threadIdInsideItsGroup.y < (intraLOD - 1)))
        {
            uint currentVertexID = threadIdInsideItsGroup.y * intraLOD + threadIdInsideItsGroup.x;
            uint nextMostRightVertexID = currentVertexID + 1;
            uint nextMostBottomVertexID = currentVertexID + intraLOD;
            uint nextMostBottomRightVertexID = nextMostBottomVertexID + 1;
            if ((threadIdInsideItsGroup.x < (intraLOD / 2) && threadIdInsideItsGroup.y < (intraLOD / 2)) || (threadIdInsideItsGroup.x >= (intraLOD / 2)
             && threadIdInsideItsGroup.y >= (intraLOD / 2)))
            {
                triangleIndices[2 * (threadIdInsideItsGroup.y * (intraLOD - 1) + threadIdInsideItsGroup.x)] = uint3(currentVertexID, nextMostRightVertexID, nextMostBottomVertexID).xyz;
                triangleIndices[2 * (threadIdInsideItsGroup.y * (intraLOD - 1) + threadIdInsideItsGroup.x) + 1] = uint3(nextMostRightVertexID, nextMostBottomRightVertexID, nextMostBottomVertexID).xyz;

            }
            else
            {
                triangleIndices[2 * (threadIdInsideItsGroup.y * (intraLOD - 1) + threadIdInsideItsGroup.x)] = uint3(currentVertexID, nextMostRightVertexID, nextMostBottomRightVertexID).xyz;
                triangleIndices[2 * (threadIdInsideItsGroup.y * (intraLOD - 1) + threadIdInsideItsGroup.x) + 1] = uint3(nextMostBottomRightVertexID, nextMostBottomVertexID, currentVertexID).xyz;
            }
        }
    }

}

float4 PS_main(MeshShaderOutput input)
    : SV_TARGET
{
    return float4(userDefinedColor, 1.0f);
})";

    static int  fileCounter = 0;
    std::string filePath    = std::format("D:/git-repositories/procedural-fruit-generation/assignments/"
                                             "AXSimpleStrawberryWithIntraLOD/shader-{}.hlsl",
                                          fileCounter++);

    std::ofstream outfile(filePath);
    if (!outfile.is_open())
    {
      throw std::ios_base::failure("Failed to open output file: " + filePath);
    }

    const std::string searchString   = "#define NUM_THREADS_Z 1";
    size_t            insertPosition = shaderCode.find(searchString);

    if (insertPosition == std::string::npos)
    {
      throw std::runtime_error("Search string not found in template content.");
    }

    const std::string generatedContent = std::format("\nstatic const float3 p0 = float3({});\n"
                                                     "static const float3 p1 = float3({});\n"
                                                     "static const float3 p2 = float3({});\n"
                                                     "static const float3 p3 = float3({});\n",
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
        L"../../../assignments/AXSimpleOctasphereWithBezierCurves/shaders/Octasphere.hlsl", L"MS_main", L"ms_6_5");
    const auto pixelShader = compileShader(
        L"../../../assignments/AXSimpleOctasphereWithBezierCurves/shaders/Octasphere.hlsl", L"PS_main", L"ps_6_5");

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

    auto m = f32v4(m_uiData.m_firstControlPoint, 0.0f);
    auto n = f32v4(m_uiData.m_secondControlPoint, 0.0f);
    auto p = f32v4(m_uiData.m_thirdControlPoint, 0.0f);
    auto q = f32v4(m_uiData.m_fourthControlPoint, m_uiData.m_flatShading);
    commandList->SetGraphicsRoot32BitConstants(0, 4, &m, 32);
    commandList->SetGraphicsRoot32BitConstants(0, 4, &n, 36);
    commandList->SetGraphicsRoot32BitConstants(0, 4, &p, 40);
    commandList->SetGraphicsRoot32BitConstants(0, 4, &q, 44);

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
    if (ImGui::Button("Save Shader"))
      GenerateShaderFile();
    ImGui::End();
  }
};

int main(int /* argc*/, char /* **argv */)
{
  gims::DX12AppConfig config;
  config.title    = L"Assignment X Simple Octasphere With Bezier Curves";
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
