#include "stdafx.h"
#include "Core.h"

Core::Core()
{
	hWnd = 0;

	ClientWidth = FRAME_BUFFER_WIDTH;
	ClientHeight = FRAME_BUFFER_HEIGHT;

	dxgiFactory = NULL;
	dxgiSwapChain = NULL;
	Device = NULL;

	CommandAllocator = NULL;
	CommandQueue = NULL;
	CommandList = NULL;
	PipelineState = NULL;

	for (int i = 0; i < NumOfSwapChainBuffers; ++i)
		RenderTargetBuffers[i] = NULL;
	RtvDescriptorHeap = NULL;
	RtvDescriptorIncrementSize = 0;

	DepthStencilBuffer = NULL;
	DsvDescriptorHeap = NULL;
	DsvDescriptorIncrementSize = 0;

	SwapChainBufferIndex = 0;

	Fence = NULL;
	FenceEvent = NULL;
	for (int i = 0; i < NumOfSwapChainBuffers; ++i)
		FenceValues[i] = 0;
}

int Core::Init(HWND pa_hWnd, HINSTANCE pa_hInstance)
{
	hWnd = pa_hWnd;
	hInstance = pa_hInstance;

	CreateDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDescriptorHeaps();
	CreateSwapChain();
	CreateDSV();



	BuildObjects();

	return S_OK;
}

void Core::OnDestroy()
{
	WaitForGpu();

	::CloseHandle(FenceEvent);

	for (int i = 0; i < NumOfSwapChainBuffers; ++i)
		if (RenderTargetBuffers[i])
			RenderTargetBuffers[i]->Release();
	if (RtvDescriptorHeap) RtvDescriptorHeap->Release();

	if (DepthStencilBuffer) DepthStencilBuffer->Release();
	if (DsvDescriptorHeap) DsvDescriptorHeap->Release();

	if (CommandAllocator) CommandAllocator->Release();
	if (CommandQueue) CommandQueue->Release();
	if (CommandList) CommandList->Release();
	if (PipelineState) PipelineState->Release();

	if (Fence) Fence->Release();

	//오류 방지용 전체화면 해제
	dxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (dxgiSwapChain) dxgiSwapChain->Release();
	if (dxgiFactory) dxgiFactory->Release();
	if (Device) Device->Release();

#if defined(_DEBUG)
	IDXGIDebug1* dxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&dxgiDebug);
	HRESULT hResult = dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
	dxgiDebug->Release();
#endif
}

void Core::Progress()
{
	HRESULT hResult = CommandAllocator->Reset();
	hResult = CommandList->Reset(CommandAllocator, NULL);

	D3D12_RESOURCE_BARRIER ResourceBarrier;
	::ZeroMemory(&ResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	ResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	ResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	ResourceBarrier.Transition.pResource = RenderTargetBuffers[SwapChainBufferIndex];
	ResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	ResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	ResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	CommandList->ResourceBarrier(1, &ResourceBarrier);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
		RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (SwapChainBufferIndex * RtvDescriptorIncrementSize);
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	CommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, FALSE, &d3dDsvCPUDescriptorHandle);
	float ClearColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	CommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, ClearColor, 0, NULL);
	CommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	// 렌더

	ResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	ResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	ResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	CommandList->ResourceBarrier(1, &ResourceBarrier);
	hResult = CommandList->Close();

	ID3D12CommandList* CommandLists[] = { CommandList };
	CommandQueue->ExecuteCommandLists(_countof(CommandLists), CommandLists);

	WaitForGpu();
	dxgiSwapChain->Present(0, 0);
	GoNextFrame();
}

void Core::CreateDevice()
{
	HRESULT hResult;
	UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
	ID3D12Debug* DebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&DebugController);
	if (DebugController)
	{
		DebugController->EnableDebugLayer();
		DebugController->Release();
	}
	dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hResult = ::CreateDXGIFactory2(dxgiFactoryFlags, __uuidof(IDXGIFactory4), (void**)&dxgiFactory);

	IDXGIAdapter1* Adapter = NULL;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != dxgiFactory->EnumAdapters1(i, &Adapter); ++i) {
		DXGI_ADAPTER_DESC1 AdapterDesc;
		Adapter->GetDesc1(&AdapterDesc);
		if (AdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(Adapter, D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), (void**)&Device))) break;
	}

	if (!Adapter) {
		dxgiFactory->EnumWarpAdapter(__uuidof(IDXGIAdapter1), (void**)&Adapter);
		D3D12CreateDevice(Adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**)&Device);
	}

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS MsaaQualityLevels;
	MsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	MsaaQualityLevels.SampleCount = 4;
	MsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	MsaaQualityLevels.NumQualityLevels = 0;
	Device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&MsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	Msaa4xQualityLevels = MsaaQualityLevels.NumQualityLevels;
	Msaa4xEnable = (Msaa4xQualityLevels > 0) ? true : false;

	hResult = Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&Fence);
	FenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	if (Adapter) Adapter->Release();
}

void Core::CreateCommandQueueAndList()
{
	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc;
	::ZeroMemory(&CommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	CommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	CommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	HRESULT hResult = Device->CreateCommandQueue(&CommandQueueDesc, __uuidof(ID3D12CommandQueue), (void**)&CommandQueue);
	hResult = Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&CommandAllocator);
	hResult = Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, CommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&CommandList);
	hResult = CommandList->Close();
}

void Core::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC DescriptorHeapDesc;
	::ZeroMemory(&DescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	DescriptorHeapDesc.NumDescriptors = NumOfSwapChainBuffers;
	DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	DescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	DescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = Device->CreateDescriptorHeap(&DescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&RtvDescriptorHeap);
	RtvDescriptorIncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	DescriptorHeapDesc.NumDescriptors = 1;
	DescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = Device->CreateDescriptorHeap(&DescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&DsvDescriptorHeap);
	DsvDescriptorIncrementSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void Core::CreateSwapChain()
{
	RECT rcClient;
	::GetClientRect(hWnd, &rcClient);
	ClientWidth = rcClient.right - rcClient.left;
	ClientHeight = rcClient.bottom - rcClient.top;

	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	::ZeroMemory(&SwapChainDesc, sizeof(SwapChainDesc));
	SwapChainDesc.BufferCount = NumOfSwapChainBuffers;
	SwapChainDesc.BufferDesc.Width = ClientWidth;
	SwapChainDesc.BufferDesc.Height = ClientHeight;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	SwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	SwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	SwapChainDesc.OutputWindow = hWnd;
	SwapChainDesc.SampleDesc.Count = (Msaa4xEnable) ? 4 : 1;
	SwapChainDesc.SampleDesc.Quality = (Msaa4xEnable) ? (Msaa4xQualityLevels - 1) : 0;
	SwapChainDesc.Windowed = TRUE;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	HRESULT hResult = dxgiFactory->CreateSwapChain(CommandQueue, &SwapChainDesc, (IDXGISwapChain**)&dxgiSwapChain);
	SwapChainBufferIndex = dxgiSwapChain->GetCurrentBackBufferIndex();

	hResult = dxgiFactory->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);

	CreateRTVs();
}

void Core::CreateRTVs()
{
	D3D12_CPU_DESCRIPTOR_HANDLE RtvCPUDescriptorHandle = RtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < NumOfSwapChainBuffers; ++i) {
		dxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&RenderTargetBuffers[i]);
		Device->CreateRenderTargetView(RenderTargetBuffers[i], NULL, RtvCPUDescriptorHandle);
		RtvCPUDescriptorHandle.ptr += RtvDescriptorIncrementSize;
	}
}

void Core::CreateDSV()
{
	D3D12_RESOURCE_DESC ResourceDesc;
	ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	ResourceDesc.Alignment = 0;
	ResourceDesc.Width = ClientWidth;
	ResourceDesc.Height = ClientHeight;
	ResourceDesc.DepthOrArraySize = 1;
	ResourceDesc.MipLevels = 1;
	ResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	ResourceDesc.SampleDesc.Count = (Msaa4xEnable) ? 4 : 1;
	ResourceDesc.SampleDesc.Quality = (Msaa4xEnable) ? (Msaa4xQualityLevels - 1) : 0;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	ResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_HEAP_PROPERTIES HeapProperties;
	::ZeroMemory(&HeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProperties.CreationNodeMask = 1;
	HeapProperties.VisibleNodeMask = 1;

	D3D12_CLEAR_VALUE ClearValue;
	ClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	ClearValue.DepthStencil.Depth = 1.0f;
	ClearValue.DepthStencil.Stencil = 0;
	Device->CreateCommittedResource(&HeapProperties, D3D12_HEAP_FLAG_NONE,
		&ResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &ClearValue,
		__uuidof(ID3D12Resource), (void**)&DepthStencilBuffer);

	D3D12_CPU_DESCRIPTOR_HANDLE DsvCPUDescriptorHandle =
		DsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	Device->CreateDepthStencilView(DepthStencilBuffer, NULL, DsvCPUDescriptorHandle);
}

void Core::BuildObjects()
{
	CommandList->Reset(CommandAllocator, NULL);

	// 씬 초기화

	CommandList->Close();
	ID3D12CommandList* CommandLists[] = { CommandList };
	CommandQueue->ExecuteCommandLists(1, CommandLists);

	WaitForGpu();
}

void Core::WaitForGpu()
{
	UINT64 FenceValue = ++FenceValues[SwapChainBufferIndex];
	HRESULT hResult = CommandQueue->Signal(Fence, FenceValue);
	if (Fence->GetCompletedValue() < FenceValue) {
		hResult = Fence->SetEventOnCompletion(FenceValue, FenceEvent);
		::WaitForSingleObject(FenceEvent, INFINITE);
	}
}

void Core::GoNextFrame()
{
	SwapChainBufferIndex = dxgiSwapChain->GetCurrentBackBufferIndex();
	UINT64 FenceValue = ++FenceValues[SwapChainBufferIndex];
	HRESULT hResult = CommandQueue->Signal(Fence, FenceValue);
	if (Fence->GetCompletedValue() < FenceValue) {
		hResult = Fence->SetEventOnCompletion(FenceValue, FenceEvent);
		::WaitForSingleObject(FenceEvent, INFINITE);
	}
}