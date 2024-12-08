#include <gimslib/d3d/DX12App.hpp>
#include <gimslib/d3d/DX12Util.hpp>
#include <gimslib/types.hpp>
#include <imgui.h>
#include <gimslib/ui/ExaminerController.hpp>
#include <iostream>
using namespace gims;

class SphereRenderer : public DX12App
{
private:
  struct UiData
  {
    f32v3 m_backgroundColor = {0.0f, 0.0f, 0.0f};    
    float m_radius            = 1.0f;
    int  m_interSphereLOD = 1;
  };

  UiData m_uiData;

  ComPtr<ID3D12PipelineState> m_pipelineState;
  ComPtr<ID3D12PipelineState> m_wireFramePipelineState;
  ComPtr<ID3D12RootSignature> m_rootSignature;

  gims::ExaminerController m_examinerController;

  void createRootSignature()
  {
    CD3DX12_ROOT_PARAMETER rootParameters[1] = {};
    rootParameters[0].InitAsConstants(18, 0, 0, D3D12_SHADER_VISIBILITY_ALL);

    CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
    descRootSignature.Init(1, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);

    ComPtr<ID3DBlob> rootBlob, errorBlob;
    D3D12SerializeRootSignature(&descRootSignature, D3D_ROOT_SIGNATURE_VERSION_1, &rootBlob, &errorBlob);

    getDevice()->CreateRootSignature(0, rootBlob->GetBufferPointer(), rootBlob->GetBufferSize(),
                                     IID_PPV_ARGS(&m_rootSignature));

    std::cout << "Root signature created successfully!" << std::endl;
  }

  void createPipeline()
  {
    const auto meshShader =
        compileShader(L"../../../assignments/AXUVSphereWithInterLOD/shaders/Sphere.hlsl",
                      L"MS_main", L"ms_6_5");
    const auto pixelShader =
        compileShader(L"../../../assignments/AXUVSphereWithInterLOD/Shaders/Sphere.hlsl",
                      L"PS_main", L"ps_6_5");

    D3DX12_MESH_SHADER_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature                         = m_rootSignature.Get();
    psoDesc.MS                                     = HLSLCompiler::convert(meshShader);
    psoDesc.PS                                     = HLSLCompiler::convert(pixelShader);
    psoDesc.RasterizerState                        = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.FillMode               = D3D12_FILL_MODE_WIREFRAME;
    psoDesc.RasterizerState.CullMode               = D3D12_CULL_MODE_NONE;
    psoDesc.BlendState                             = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DSVFormat                              = getDX12AppConfig().depthBufferFormat;
    psoDesc.DepthStencilState.DepthEnable          = FALSE;
    psoDesc.DepthStencilState.StencilEnable        = FALSE;
    psoDesc.SampleMask                             = UINT_MAX;
    psoDesc.NumRenderTargets                       = 1;
    psoDesc.RTVFormats[0]                          = getRenderTarget()->GetDesc().Format;
    psoDesc.DSVFormat                              = getDepthStencil()->GetDesc().Format;
    psoDesc.SampleDesc.Count                       = 1;

    auto psoStream = CD3DX12_PIPELINE_MESH_STATE_STREAM(psoDesc);

    D3D12_PIPELINE_STATE_STREAM_DESC streamDesc;
    streamDesc.pPipelineStateSubobjectStream = &psoStream;
    streamDesc.SizeInBytes                   = sizeof(psoStream);

    throwIfFailed(getDevice()->CreatePipelineState(&streamDesc, IID_PPV_ARGS(&m_pipelineState)));

    std::cout << "Pipeline created successfully!" << std::endl;
  }

public:
  SphereRenderer(const DX12AppConfig createInfo)
      : DX12App(createInfo),
      m_examinerController(true)
  {
    m_examinerController.setTranslationVector(f32v3(0.0f, 0.0f, 4.0f));
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
    
    f32v4 clearColor = {m_uiData.m_backgroundColor[0], m_uiData.m_backgroundColor[1], m_uiData.m_backgroundColor[2], 1.0f};
    commandList->ClearRenderTargetView(rtvHandle, &clearColor.x, 0, nullptr);
    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
    
    
    commandList->RSSetViewports(1, &getViewport());
    commandList->RSSetScissorRects(1, &getRectScissor());

    const auto projectionMatrix =
        glm::perspectiveFovLH_ZO<f32>(glm::radians(45.0f), (f32)getWidth(), (f32)getHeight(), 0.0001f, 10000.0f);
    const auto viewMatrix = m_examinerController.getTransformationMatrix();

    commandList->SetPipelineState(m_pipelineState.Get());
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());
    const auto accumulatedTransformation = projectionMatrix * viewMatrix;
    commandList->SetGraphicsRoot32BitConstants(0, 16, &accumulatedTransformation, 0);
    commandList->SetGraphicsRoot32BitConstants(0, 1, &m_uiData.m_radius, 16);
    commandList->SetGraphicsRoot32BitConstants(0, 1, &m_uiData.m_interSphereLOD, 17);
    commandList->DispatchMesh(m_uiData.m_interSphereLOD, m_uiData.m_interSphereLOD, 1);

  }

  virtual void onDrawUI()
  {
    ImGui::Begin("Information");
    ImGui::Text("Frame time: %f", 1.0f / ImGui::GetIO().Framerate * 1000.0f);
    ImGui::End();    
    ImGui::Begin("Configuration");
    ImGui::ColorEdit3("Background Color", &m_uiData.m_backgroundColor[0]);
    ImGui::SliderFloat("Radius", &m_uiData.m_radius, 0.1f, 1.0f);
    ImGui::SliderInt("Inter Sphere LOD", &m_uiData.m_interSphereLOD, 1, 18);
    ImGui::End();        
  }
};

int main(int /* argc*/, char /* **argv */)
{
  gims::DX12AppConfig config;
  config.title    = L"Assignmeent X UV Sphere With Inter LOD";
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
