#pragma once

#include <d3d12.h>
#include <imgui.h>

namespace dolos {

class D3D12ImGui {
 public:
  D3D12ImGui() noexcept;
  ~D3D12ImGui() noexcept;

  bool Initialize(ID3D12Device* device,
                  int num_frames_in_flight,
                  DXGI_FORMAT rtv_format,
                  ID3D12DescriptorHeap* cbv_srv_heap,
                  D3D12_CPU_DESCRIPTOR_HANDLE font_srv_cpu_desc_handle,
                  D3D12_GPU_DESCRIPTOR_HANDLE font_srv_gpu_desc_handle);
  void Shutdown();

  void NewFrame();
  void RenderDrawData(ImDrawData* draw_data, ID3D12GraphicsCommandList* graphics_command_list);

  bool CreateDeviceObjects();
  void InvalidateDeviceObjects();

 private:
  struct RenderBuffers {
    ID3D12Resource* IndexBuffer;
    ID3D12Resource* VertexBuffer;
    int IndexBufferSize;
    int VertexBufferSize;
  };

  struct VERTEX_CONSTANT_BUFFER {
    float mvp[4][4];
  };

  ID3D12Device* pd3dDevice;
  ID3D12RootSignature* pRootSignature;
  ID3D12PipelineState* pPipelineState;
  DXGI_FORMAT RTVFormat;
  ID3D12Resource* pFontTextureResource;
  D3D12_CPU_DESCRIPTOR_HANDLE hFontSrvCpuDescHandle;
  D3D12_GPU_DESCRIPTOR_HANDLE hFontSrvGpuDescHandle;

  RenderBuffers* pFrameResources;
  UINT numFramesInFlight;
  UINT frameIndex;

  void SetupRenderState(ImDrawData* draw_data, ID3D12GraphicsCommandList* ctx, RenderBuffers* fr);
  void CreateFontsTexture();
};

}  // namespace dolos
