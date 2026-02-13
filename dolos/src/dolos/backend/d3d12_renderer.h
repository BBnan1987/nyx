#pragma once

#include "dolos/backend/d3d12_imgui.h"
#include "dolos/backend/renderer.h"
#include "dolos/backend/win32_window.h"
#include "dolos/detour.h"

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>
#include <vector>

using Microsoft::WRL::ComPtr;

namespace dolos {

class D3D12Renderer : public Renderer {
 public:
  D3D12Renderer(Win32Window* window, nyx::NyxImGui* nyx_imgui) noexcept;
  virtual ~D3D12Renderer() noexcept;

  D3D12Renderer(const D3D12Renderer&) = delete;
  D3D12Renderer& operator=(const D3D12Renderer&) = delete;
  D3D12Renderer(D3D12Renderer&&) = delete;
  D3D12Renderer& operator=(D3D12Renderer&&) = delete;

  bool Initialize() override;
  void Shutdown() override;

  HRESULT Present(IDXGISwapChain* self, UINT SyncInterval, UINT Flags);
  HRESULT ResizeBuffers(
      IDXGISwapChain* self, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
  void ExecuteCommandLists(ID3D12CommandQueue* self, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);

 private:
  Win32Window* window_;
  nyx::NyxImGui* nyx_imgui_;
  D3D12ImGui imgui_;

  HRESULT CreateRenderTargets(IDXGISwapChain* pSwapChain);
  void ResetRenderTargets();

  struct FrameContext {
    ComPtr<ID3D12CommandAllocator> allocator;
    ComPtr<ID3D12Resource> rtv;
    D3D12_CPU_DESCRIPTOR_HANDLE rtd;
  };

  std::vector<FrameContext> frames_;
  ComPtr<ID3D12DescriptorHeap> rtv_heap_;
  ComPtr<ID3D12DescriptorHeap> srv_heap_;
  ComPtr<ID3D12GraphicsCommandList> command_list_;
  size_t fixme_shutdown_ticks_;

  Detour<decltype(&IDXGISwapChain::Present)> present_;
  Detour<decltype(&IDXGISwapChain::ResizeBuffers)> resize_buffers_;
  Detour<decltype(&ID3D12CommandQueue::ExecuteCommandLists)> execute_command_lists_;

  ID3D12CommandQueue* command_queue_;  // DO NOT RELEASE!
};

}  // namespace dolos
