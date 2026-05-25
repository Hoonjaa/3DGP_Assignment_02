#pragma once
class Core
{
public:
	SINGLE(Core);

private:
	HINSTANCE hInstance;
	HWND hWnd;

	int ClientWidth;
	int ClientHeight;

	// 팩토리 / 디바이스 / 스왑체인
	IDXGIFactory4* dxgiFactory;
	IDXGISwapChain3* dxgiSwapChain;
	ID3D12Device* Device;

	// 다중 샘플링
	bool Msaa4xEnable = false;
	UINT Msaa4xQualityLevels = 0;

	// 스왑체인 총 갯수, 현재 버퍼
	static const UINT NumOfSwapChainBuffers = 2;
	UINT SwapChainBufferIndex;

	// 렌더 타겟 버퍼 배열, RTV 서술자 힙, RTV 서술자 크기
	ID3D12Resource* RenderTargetBuffers[NumOfSwapChainBuffers];
	ID3D12DescriptorHeap* RtvDescriptorHeap;
	UINT RtvDescriptorIncrementSize;

	// DS 버퍼, DSV 서술자 힙, DSV 서술자 크기
	ID3D12Resource* DepthStencilBuffer;
	ID3D12DescriptorHeap* DsvDescriptorHeap;
	UINT DsvDescriptorIncrementSize;

	// 커맨드 큐, 할당자, 리스트
	ID3D12CommandQueue* CommandQueue;
	ID3D12CommandAllocator* CommandAllocator;
	ID3D12GraphicsCommandList* CommandList;

	// 파이프라인
	ID3D12PipelineState* PipelineState;

	// 펜스, 펜스 값, 이벤트 핸들
	ID3D12Fence* Fence;
	UINT64 FenceValues[NumOfSwapChainBuffers];
	HANDLE FenceEvent;

public:
	int Init(HWND hWnd, HINSTANCE hInstance);
	void Progress();
	void OnDestroy();

	void CreateDevice();
	void CreateCommandQueueAndList();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateSwapChain();

	void CreateRTVs();
	void CreateDSV();

	void BuildObjects();

	void WaitForGpu();
	void GoNextFrame();
};

