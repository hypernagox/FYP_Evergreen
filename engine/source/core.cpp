#include "pch.h"
#include "core.h"

#include "singleton.h"
#include "resource_load.h"
#include "input.h"
#include "frame_resource.h"
#include "time_measure.h"
#include "scene.h"
#include "scene_object.h"
#include "mesh.h"
#include "shader.h"
#include "frame_debug.h"
#include "debug_console.h"
#include "audio.h"
#include "texture.h"
#include "font.h"
#include "shadow_map.h"
#include "screen_space_ao.h"
#include "deferred_renderer.h"
#include "motion_blur.h"
#include "post_process_bloom.h"
#include "post_process_fxaa.h"
#include "post_process_outline.h"

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace udsdx
{
	Core::Core()
	{
	}

	Core::~Core()
	{
		Singleton<TimeMeasure>::ReleaseInstance();
		Singleton<Resource>::ReleaseInstance();
		Singleton<Audio>::ReleaseInstance();
	}

	void Core::Initialize(HINSTANCE hInstance, HWND hWnd)
	{ ZoneScoped;
		m_hInstance = hInstance;
		m_hMainWnd = hWnd;

		m_windowedRect = { 0, 0, m_clientWidth, m_clientHeight };

#if _WIN32_WINNT >= 0x0A00 // _WIN32_WINNT_WIN10
		m_roInitialization = std::make_unique<Wrappers::RoInitializeWrapper>(RO_INIT_MULTITHREADED);
		assert(SUCCEEDED(*m_roInitialization));
#else
		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		assert(SUCCEEDED(hr));
#endif

		// Get title of window
		wchar_t buffer[256];
		GetWindowText(m_hMainWnd, buffer, 256);
		m_mainWndCaption = buffer;

		InitializeDirect3D();

		m_monoUploadBuffer = std::make_unique<MonoUploadBuffer>(m_d3dDevice.Get());

		for (int i = 0; i < FrameResourceCount; ++i)
		{
			m_frameResources[i] = std::make_unique<FrameResource>(m_d3dDevice.Get());
		}

		auto audio = Singleton<Audio>::GetInstance();
		auto resource = Singleton<Resource>::GetInstance();
		m_timeMeasure = Singleton<TimeMeasure>::GetInstance();

		m_graphicsMemory = std::make_unique<GraphicsMemory>(m_d3dDevice.Get());

		INSTANCE(Input)->Initialize(m_hMainWnd);
		resource->Initialize(m_d3dDevice.Get(), m_commandQueue.Get(), m_commandList.Get(), m_rootSignature.Get());

		CreateDescriptorHeaps();
		RegisterDescriptorsToHeaps();
		BuildConstantBuffers();
		InitializeSpriteBatch();

		OnResizeWindow(m_clientWidth, m_clientHeight);

		m_fenceEvent = ::CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		// Create frame debug window
		// m_frameDebug = std::make_unique<FrameDebug>(m_hInstance);
		InitImGui();
	}

	void Core::InitializeDirect3D()
	{ ZoneScoped;
	    UINT factoryFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		EnableDebugLayer();
		factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
		// Create DXGI Factory
		ThrowIfFailed(::CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&m_dxgiFactory)));

		// Before creating the device, log the adapter information.
		LogAdapterInfo();

		// Enumerate the adapters by GPU preference.
		ComPtr<IDXGIAdapter1> dxgiAdapter = nullptr;
		for (UINT adapterIndex = 0; SUCCEEDED(m_dxgiFactory->EnumAdapterByGpuPreference(adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&dxgiAdapter))); ++adapterIndex)
		{
			// Skip software adapters.
			DXGI_ADAPTER_DESC1 desc;
			if (FAILED(dxgiAdapter->GetDesc1(&desc)) || (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
			{
				continue;
			}

			// Check if the adapter supports Direct3D 12 then create the device.
			if (SUCCEEDED(::D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_d3dDevice))))
			{
				break;
			}
		}

		if (m_d3dDevice == nullptr)
		{
			// Fallback to WARP(Windows Advanced Rasterization Platform) device
			ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter)));

			// Create hardware device with WARP adapter
			ThrowIfFailed(::D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_d3dDevice)));
		}

		// Check for tearing support
		if (FAILED(m_dxgiFactory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &m_tearingSupport, sizeof(m_tearingSupport))))
		{
			m_tearingSupport = false;
		}

		// Get descriptor sizes for offsets
		m_rtvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_dsvDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_cbvSrvUavDescriptorSize = m_d3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// Check 4X MSAA quality support for our back buffer format.
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
		msQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		msQualityLevels.SampleCount = 4;
		msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msQualityLevels.NumQualityLevels = 0;

		ThrowIfFailed(m_d3dDevice->CheckFeatureSupport(
			D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
			&msQualityLevels,
			sizeof(msQualityLevels)
		));

		m_4xMsaaQuality = msQualityLevels.NumQualityLevels;
		assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");

		CreateCommandObjects();
		CreateSwapChain();
		BuildRootSignature();
		
		// Create Deferred Renderer
		m_deferredRenderer = std::make_unique<DeferredRenderer>(m_d3dDevice.Get());

		// Create Shadow Map
		m_shadowMap = std::make_unique<ShadowMap>(m_renderOptions.ShadowMapSize, m_renderOptions.ShadowMapSize, m_d3dDevice.Get());

		// Create Screen Space Ambient Occlusion
		m_screenSpaceAO = std::make_unique<ScreenSpaceAO>(m_d3dDevice.Get(), m_commandList.Get(), 1.0f);
		m_screenSpaceAO->BuildPipelineState(m_d3dDevice.Get(), m_rootSignature.Get());

		// Create Motion Blur
		m_motionBlur = std::make_unique<MotionBlur>(m_d3dDevice.Get(), m_commandList.Get());
		m_motionBlur->BuildPipelineState();
		m_postProcessBloom = std::make_unique<PostProcessBloom>(m_d3dDevice.Get(), m_commandList.Get());
		m_postProcessBloom->BuildPipelineState();
		m_postProcessFXAA = std::make_unique<PostProcessFXAA>(m_d3dDevice.Get(), m_commandList.Get());
		m_postProcessFXAA->BuildPipelineState();
		m_postProcessOutline = std::make_unique<PostProcessOutline>(m_d3dDevice.Get(), m_commandList.Get());
		m_postProcessOutline->BuildPipelineState();

		const char tracyQueueName[] = "D3D12 Graphics Queue";
		m_tracyQueueCtx = TracyD3D12Context(m_d3dDevice.Get(), m_commandQueue.Get());
		TracyD3D12ContextName(m_tracyQueueCtx, tracyQueueName, sizeof(tracyQueueName));
	}

	void Core::EnableDebugLayer()
	{
		ComPtr<ID3D12Debug> debugLayer0;
		ComPtr<ID3D12Debug1> debugLayer1;

		ThrowIfFailed(::D3D12GetDebugInterface(IID_PPV_ARGS(&debugLayer0)));
		ThrowIfFailed(debugLayer0->QueryInterface(IID_PPV_ARGS(&debugLayer1)));
		debugLayer0->EnableDebugLayer();
		debugLayer1->SetEnableGPUBasedValidation(true);
	}

	void Core::CreateCommandObjects()
	{ ZoneScoped;
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

		ThrowIfFailed(m_d3dDevice->CreateCommandQueue(
			&queueDesc,
			IID_PPV_ARGS(&m_commandQueue)
		));
		ThrowIfFailed(m_d3dDevice->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(m_directCmdListAlloc.GetAddressOf())
		));
		ThrowIfFailed(m_d3dDevice->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_directCmdListAlloc.Get(),
			nullptr,
			IID_PPV_ARGS(m_commandList.GetAddressOf())
		));

		// Create fence for cpu-gpu synchronization
		ThrowIfFailed(m_d3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

		m_commandList->Close();
	}

	void Core::RegisterUpdateCallback(std::function<void(const Time&)> callback)
	{
		m_updateCallback = callback;
	}

	void Core::CreateSwapChain()
	{ ZoneScoped;
		// Release the previous swapchain we will be recreating.
		m_swapChain.Reset();

		DXGI_SWAP_CHAIN_DESC1 desc;
		ZeroMemory(&desc, sizeof(desc));

		desc.Width = m_clientWidth;
		desc.Height = m_clientHeight;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = SwapChainBufferCount;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		desc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
		desc.SampleDesc.Quality = m_4xMsaaState ? m_4xMsaaQuality - 1 : 0;
		desc.Flags = m_tearingSupport ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		ComPtr<IDXGISwapChain1> swapChain1;

		// Note: Swap chain uses queue to perform flush.
		ThrowIfFailed(m_dxgiFactory->CreateSwapChainForHwnd(m_commandQueue.Get(), m_hMainWnd, &desc, nullptr, nullptr, &swapChain1));
		ThrowIfFailed(swapChain1.As(&m_swapChain));

		// Suppress the Alt+Enter fullscreen toggle for tearing support.
		if (m_tearingSupport)
		{
			m_dxgiFactory->MakeWindowAssociation(m_hMainWnd, DXGI_MWA_NO_ALT_ENTER);
		}
	}

	void Core::CreateDescriptorHeaps()
	{ ZoneScoped;
		std::vector<Texture*> textures = INSTANCE(Resource)->LoadAll<Texture>();

		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
		cbvHeapDesc.NumDescriptors = FrameResourceCount;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(m_cbvHeap.GetAddressOf())));

		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc;
		srvHeapDesc.NumDescriptors = static_cast<UINT>(textures.size() + 64);
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		srvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(m_srvHeap.GetAddressOf())));

		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		rtvHeapDesc.NumDescriptors = 32;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_rtvHeap.GetAddressOf())));

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
		dsvHeapDesc.NumDescriptors = 32;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		dsvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_dsvHeap.GetAddressOf())));
	}

	void Core::OnDestroy()
	{ ZoneScoped;
		if (m_d3dDevice != nullptr)
		{
			// Ensure that the GPU is no longer referencing resources that are about to be destroyed.
			FlushCommandQueue();
			SetWindowFullscreen(false);

			if (m_scene != nullptr)
			{
				m_scene->HandleDetach();
			}

			::CloseHandle(m_fenceEvent);

			// Release the context
			TracyD3D12Destroy(m_tracyQueueCtx);
			ReleaseImGui();

			m_graphicsMemory.reset();
		}
	}

	void Core::RegisterDescriptorsToHeaps()
	{ ZoneScoped;
		DescriptorParam descriptorParam = GetDescriptorParameters();

		m_deferredRenderer->BuildDescriptors(descriptorParam);
		m_shadowMap->BuildDescriptors(descriptorParam, m_d3dDevice.Get());
		m_screenSpaceAO->BuildDescriptors(descriptorParam, m_depthStencilBuffer.Get());
		m_motionBlur->BuildDescriptors(descriptorParam);
		m_postProcessBloom->BuildDescriptors(descriptorParam);
		m_postProcessFXAA->BuildDescriptors(descriptorParam);
		m_postProcessOutline->BuildDescriptors(descriptorParam);

		for (auto texture : INSTANCE(Resource)->LoadAll<Texture>())
		{
			texture->CreateShaderResourceView(m_d3dDevice.Get(), descriptorParam);
		}

		for (auto font : INSTANCE(Resource)->LoadAll<Font>())
		{
			font->CreateShaderResourceView(m_d3dDevice.Get(), descriptorParam);
		}

		ApplyDescriptorParameters(descriptorParam);
	}

	void Core::BuildConstantBuffers()
	{ ZoneScoped;
		UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(PassConstants));

		for (int frameIndex = 0; frameIndex < FrameResourceCount; ++frameIndex)
		{
			auto objectCB = m_frameResources[frameIndex]->GetObjectCB();
			D3D12_GPU_VIRTUAL_ADDRESS cbAddress = objectCB->Resource()->GetGPUVirtualAddress();

			// Offset to the ith object constant buffer in the buffer.
			auto handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetCPUDescriptorHandleForHeapStart());
			handle.Offset(frameIndex, m_cbvSrvUavDescriptorSize);

			D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
			cbvDesc.BufferLocation = cbAddress;
			cbvDesc.SizeInBytes = objCBByteSize;

			m_d3dDevice->CreateConstantBufferView(&cbvDesc, handle);
		}
	}

	void Core::BuildRootSignature()
	{ ZoneScoped;
		CD3DX12_ROOT_PARAMETER slotRootParameter[22];

		slotRootParameter[RootParam::PerObjectCBV].InitAsConstants(sizeof(ObjectConstants) / 4, 0);
		slotRootParameter[RootParam::PerCameraCBV].InitAsConstantBufferView(1);
		slotRootParameter[RootParam::BonesCBV].InitAsConstantBufferView(2, 0);
		slotRootParameter[RootParam::PrevBonesCBV].InitAsConstantBufferView(2, 1);
		slotRootParameter[RootParam::PerShadowCBV].InitAsConstantBufferView(3);
		slotRootParameter[RootParam::PerFrameCBV].InitAsConstantBufferView(4);

		CD3DX12_DESCRIPTOR_RANGE texRange[NumTextureSlots]{};
		for (int i = 0; i < NumTextureSlots; ++i)
		{
			texRange[i].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, i, 0);
			slotRootParameter[RootParam::SrcTexSRV_0 + i].InitAsDescriptorTable(1, &texRange[i]);
		}

		CD3DX12_STATIC_SAMPLER_DESC samplerDesc[] = {
			CD3DX12_STATIC_SAMPLER_DESC(
			0,
			D3D12_FILTER_ANISOTROPIC,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP),
		};

		CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(slotRootParameter), slotRootParameter, _countof(samplerDesc), samplerDesc,
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		);

		ComPtr<ID3DBlob> serializedRootSig = nullptr;
		ComPtr<ID3DBlob> errorBlob = nullptr;
		HRESULT hr = D3D12SerializeRootSignature(
			&rootSigDesc,
			D3D_ROOT_SIGNATURE_VERSION_1,
			serializedRootSig.GetAddressOf(),
			errorBlob.GetAddressOf()
		);

		if (errorBlob != nullptr)
		{
			::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(m_d3dDevice->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(m_rootSignature.GetAddressOf())
		));
	}

	void Core::LogAdapterInfo()
	{
		std::wstring text = L"DXGI Adapters:\n";

		// Output information
		IDXGIAdapter* adapter = nullptr;
		for (UINT i = 0; m_dxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC desc;
			adapter->GetDesc(&desc);
			text += L"\n";
			text += std::format(L"> Adapter: {}\n", desc.Description);
			text += std::format(L"  > Vendor ID: {}\n", desc.VendorId);
			text += std::format(L"  > Device ID: {}\n", desc.DeviceId);
			text += std::format(L"  > SubSys ID: {}\n", desc.SubSysId);
			text += std::format(L"  > Revision: {}\n", desc.Revision);
			text += std::format(L"  > Dedicated Video Memory: {} MB\n", desc.DedicatedVideoMemory >> 20);
			text += std::format(L"  > Dedicated System Memory: {} MB\n", desc.DedicatedSystemMemory >> 20);
			text += std::format(L"  > Shared System Memory: {} MB\n", desc.SharedSystemMemory >> 20);
			text += L"  > Outputs: ";

			IDXGIOutput* m_output = nullptr;
			UINT j = 0;
			for (; adapter->EnumOutputs(j, &m_output) != DXGI_ERROR_NOT_FOUND; ++j)
			{
				DXGI_OUTPUT_DESC desc;
				m_output->GetDesc(&desc);
				int x = desc.DesktopCoordinates.left;
				int y = desc.DesktopCoordinates.top;
				int w = desc.DesktopCoordinates.right - x;
				int h = desc.DesktopCoordinates.bottom - y;
				text += std::format(L"\n   > Output: {}\n", desc.DeviceName);
				text += std::format(L"     > Attached to Desktop: {}\n", desc.AttachedToDesktop ? L"True" : L"False");
				text += std::format(L"     > Desktop Coordinates: ({}, {}), ({} * {})", x, y, w, h);
				m_output->Release();
			}
			if (j == 0)
			{
				text += L"None";
			}
			text += L"\n";
			adapter->Release();
		}
		DebugConsole::Log(text);
	}

	void Core::InitializeSpriteBatch()
	{
		ResourceUploadBatch uploadBatch(m_d3dDevice.Get());

		uploadBatch.Begin();

		RenderTargetState rtState(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN);

		m_hudSpriteBatch = std::make_unique<SpriteBatch>(m_d3dDevice.Get(), uploadBatch, SpriteBatchPipelineStateDescription(rtState, &CommonStates::NonPremultiplied));
		m_hudSpriteBatchPremultipliedAlpha = std::make_unique<SpriteBatch>(m_d3dDevice.Get(), uploadBatch, SpriteBatchPipelineStateDescription(rtState));

		auto uploadResourcesFinished = uploadBatch.End(INSTANCE(Core)->GetCommandQueue());
		uploadResourcesFinished.wait();
	}

	void Core::DisplayFrameStats()
	{
		constexpr int RefreshRate = 15;
		static int frameCount = 0;

		static std::chrono::steady_clock timer;
		static std::chrono::steady_clock::time_point lc;
		std::chrono::steady_clock::time_point c = timer.now();

		double delta = std::chrono::duration_cast<std::chrono::duration<double>>(c - lc).count();
		if (delta * RefreshRate > 1.0)
		{
			int fps = static_cast<int>(round(1.0 / delta * frameCount));
			SetWindowText(m_hMainWnd, (m_mainWndCaption + std::format(L" [{} x {}] @ {} FPS", m_clientWidth, m_clientHeight, fps)).c_str());
			frameCount = 0;
			lc = c;
		}
		frameCount += 1;
	}

	void Core::ExecuteCommandList()
	{
		ThrowIfFailed(m_commandList->Close());
		ID3D12CommandList* cmdsLists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
	}

	void Core::FlushCommandQueue()
	{ ZoneScoped;
		// Advance the fence value to mark commands up to this fence point.
		m_currentFence++;

		// Add an instruction to the command queue to set a new fence point.
		// Because we are on the GPU time line, the new fence point won't be set until the GPU finishes processing all the commands prior to this Signal().
		ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), m_currentFence));

		// Check if the fence is still not advanced.
		if (m_fence->GetCompletedValue() < m_currentFence)
		{
			// Generate an event which is automatically fired when the GPU reaches the desired fence.
			ThrowIfFailed(m_fence->SetEventOnCompletion(m_currentFence, m_fenceEvent));

			// Wait until the GPU hits current fence event is fired.
			::WaitForSingleObject(m_fenceEvent, INFINITE);
		}
	}

	void Core::PrepareDirectCommandList()
	{
		// Reset the command allocator for the next frame.
		m_directCmdListAlloc->Reset();
		m_commandList->Reset(m_directCmdListAlloc.Get(), nullptr);
	}

	void Core::ExecuteAndFlushDirectCommandList()
	{ ZoneScoped;
		// Close the command list and execute it to begin the initial GPU setup.
		ExecuteCommandList();

		// Wait until all commands are finished.
		FlushCommandQueue();
	}

	void Core::SetScene(std::shared_ptr<Scene> scene)
	{
		assert(scene != nullptr && "Scene cannot be null.");
		FlushCommandQueue();
		if (nullptr != m_scene)
		{
			m_scene->HandleDetach();
		}
		m_scene = scene;
		m_scene->HandleAttach();
	}

	void Core::AcquireNextFrameResource()
	{ ZoneScopedC(0x249EA0);
		m_currFrameResourceIndex = (m_currFrameResourceIndex + 1) % FrameResourceCount;

		auto frameResource = CurrentFrameResource();
		if (frameResource->GetFence() != 0 && m_fence->GetCompletedValue() < frameResource->GetFence())
		{
			// Fire event when GPU hits current fence.
			ThrowIfFailed(m_fence->SetEventOnCompletion(frameResource->GetFence(), m_fenceEvent));

			// Wait until the GPU hits current fence event is fired.
			::WaitForSingleObject(m_fenceEvent, INFINITE);
		}

		SceneObject::GarbageCollector::Collect(m_currFrameResourceIndex);
	}

	void Core::Update()
	{ ZoneScopedC(0xFAAB36);
		DisplayFrameStats();

		// Advance the time measure
		m_timeMeasure->Tick();

		INSTANCE(Audio)->Update();
		INSTANCE(Input)->Update();

		std::shared_ptr<Scene> lastScene = m_scene;

		BroadcastUpdateMessage();
		m_scene->Update(m_timeMeasure->GetTime());
		// If the scene has changed while updating Scene, we need to update the new scene.
		if (m_scene != lastScene)
		{
			m_scene->Update(m_timeMeasure->GetTime());
		}
		m_scene->PostUpdate(m_timeMeasure->GetTime());

		// Toggle ImGui elements with F12 key (Debug feature)
		if (INSTANCE(Input)->GetKeyDown(Keyboard::F3))
		{
			m_drawImGUIElements = !m_drawImGUIElements;
		}
		if (m_drawImGUIElements)
		{
			ImGuiNewFrame();
		}

		// Update the constant buffer with the latest view and project matrix.
		UpdateMainPassCB();
	}

	void Core::BroadcastUpdateMessage()
	{ ZoneScoped;
		if (m_updateCallback)
		{
			m_updateCallback(m_timeMeasure->GetTime());
		}
	}

	void Core::Render()
	{ ZoneScopedC(0xF78104);
		TracyD3D12Collect(m_tracyQueueCtx);
		TracyD3D12NewFrame(m_tracyQueueCtx);

		auto frameResource = CurrentFrameResource();
		auto cmdListAlloc = frameResource->GetCommandListAllocator();
		auto objectCB = frameResource->GetObjectCB();

		// Ready all the resources for rendering.
		RenderParam param{
			.Device = m_d3dDevice.Get(),
			.CommandList = m_commandList.Get(),
			.RootSignature = m_rootSignature.Get(),
			.SRVDescriptorHeap = m_srvHeap.Get(),

			.Renderer = m_deferredRenderer.get(),
			.RenderOptions = &m_renderOptions,

			.AspectRatio = GetAspectRatio(),
			.FrameResourceIndex = m_currFrameResourceIndex,
			.RenderStageIndex = 0,
			.Time = m_timeMeasure->GetTime(),

			.Viewport = m_screenViewport,
			.ScissorRect = m_scissorRect,

			.UseFrustumCulling = true,

			.ConstantBufferView = objectCB->Resource()->GetGPUVirtualAddress(),
			.DepthStencilView = DepthStencilView(),
			.RenderTargetView = CurrentBackBufferView(),

			.RenderTargetResource = CurrentBackBuffer(),
			.DepthStencilResource = m_depthStencilBuffer.Get(),

			.SpriteBatchNonPremultipliedAlpha = m_hudSpriteBatch.get(),
			.SpriteBatchPreMultipliedAlpha = m_hudSpriteBatchPremultipliedAlpha.get(),

			.RenderShadowMap = m_shadowMap.get(),
			.RenderScreenSpaceAO = m_screenSpaceAO.get(),
			.RenderMotionBlur = m_motionBlur.get(),
			.RenderPostProcessBloom = m_postProcessBloom.get(),
			.RenderPostProcessFXAA = m_postProcessFXAA.get(),
			.RenderPostProcessOutline = m_postProcessOutline.get(),

			.TracyQueueContext = &m_tracyQueueCtx
		};

		// Command list allocators can only be reset when the associated 
		// command lists have finished execution on the GPU; apps should use 
		// fences to determine GPU execution progress.
		ThrowIfFailed(cmdListAlloc->Reset());

		// Resets a command list back to its initial state as if a new command list was just created.
		// ID3D12PipelineState: This is optional and can be NULL.
		// If NULL, the runtime sets a dummy initial pipeline state so that drivers don't have to deal with undefined state.
		ThrowIfFailed(m_commandList->Reset(cmdListAlloc, nullptr));

		ID3D12DescriptorHeap* descriptorHeaps[] = { m_srvHeap.Get() };
		m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

		// Indicate a state transition on the resource usage.
		// Transition the back buffer to make it ready for writing.
		param.CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			// Before state; if the resource is in this state, the resource transitions to the after state.
			// if the resource is not in this state, the resource is not transitioned.
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET
		));

		// Draw the scene objects. 
		m_scene->Render(param);

		if (m_drawImGUIElements)
		{
			// Draw the debug window with ImGui.
			ImGuiRender();
		}

		// indicate a state transition on the resource usage.
		// Transition the back buffer to make it ready for presentation. (Reading memory)
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			CurrentBackBuffer(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT
		));

		// Done recording commands.
		ThrowIfFailed(m_commandList->Close());

		// Add the command list to the queue for execution.
		// Can add multiple lists for optimization.
		ID3D12CommandList* cmdLists[] = { m_commandList.Get() };
		m_commandQueue->ExecuteCommandLists(_countof(cmdLists), cmdLists);

		{ ZoneScopedN("Present Swap Chain");
			// When using sync interval 0, it is recommended to always pass the tearing
			// flag when it is supported, even when presenting in windowed mode.
			// However, this flag cannot be used if the app is in fullscreen mode as a
			// result of calling SetFullscreenState.
			UINT presentFlags = (m_tearingSupport && !m_fullscreen) ? DXGI_PRESENT_ALLOW_TEARING : 0;

			// Sync interval: the way in which the presentation waits for the vertical sync period.
			// 0: No sync interval, the present occurs immediately. May cause tearing.
			// 1: Sync with the next vertical blanking period(V-Sync). May cause latency.
			ThrowIfFailed(m_swapChain->Present(0, presentFlags));
			// Swap the back and front buffers
			m_currBackBuffer = (m_currBackBuffer + 1) % SwapChainBufferCount;
		}

		// Advance the fence value to mark commands up to this fence point.
		// the GPU adds a command to set the fence value to the desired value.
		frameResource->SetFence(++m_currentFence);
		m_commandQueue->Signal(m_fence.Get(), m_currentFence);

		// Add the one-shot resource to the command queue for execution.
		m_graphicsMemory->Commit(m_commandQueue.Get());
	}

	void Core::UpdateMainPassCB()
	{ ZoneScoped;
		PassConstants passConstants;
		passConstants.TotalTime = m_timeMeasure->GetTime().totalTime;
		passConstants.DeltaTime = m_timeMeasure->GetTime().deltaTime;
		passConstants.MotionBlurFactor = MotionBlur::BlurTimeScale / m_timeMeasure->GetTime().deltaTime;
		passConstants.MotionBlurRadius = static_cast<float>(MotionBlur::MaxBlurRadius);
		passConstants.FogColor = m_renderOptions.FogColor;
		passConstants.FogSunColor = m_renderOptions.FogSunColor;
		passConstants.FogDensity = m_renderOptions.FogDensity;
		passConstants.FogHeightFalloff = m_renderOptions.FogHeightFalloff;
		passConstants.FogDistanceStart = m_renderOptions.FogDistanceStart;

		auto frameResource = CurrentFrameResource();
		frameResource->GetObjectCB()->CopyData(0, passConstants);
	}

	LRESULT Core::ProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{ ZoneScoped;
		if (m_drawImGUIElements && ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		{
			return true;
		}

		switch (message)
		{
		case WM_MOVE:
			m_clientPosX = LOWORD(lParam);
			m_clientPosY = HIWORD(lParam);
			break;

		case WM_SIZE:
			// Save the new client area dimensions.
			m_clientWidth = LOWORD(lParam);
			m_clientHeight = HIWORD(lParam);

			// Notify the display associated resources for the resize event.
			OnResizeWindow(m_clientWidth, m_clientHeight);
			break;

		// Catch this message so to prevent the window from becoming too small.
		case WM_GETMINMAXINFO:
			((MINMAXINFO*)lParam)->ptMinTrackSize.x = m_minClientWidth;
			((MINMAXINFO*)lParam)->ptMinTrackSize.y = m_minClientHeight;
			break;

		case WM_SYSCHAR:
			// Handle Alt + Enter key sequence
			if ((wParam == VK_RETURN) && (lParam & (1 << 29)))
			{
				if (m_d3dDevice && m_tearingSupport)
				{
					SetWindowFullscreen(!m_fullscreen);
					break;
				}
			}
			break;

		default:
			return INSTANCE(Input)->ProcessMessage(hWnd, message, wParam, lParam);
		}
		
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	void Core::SetWindowFullscreen(bool fullscreen)
	{
		if (fullscreen == m_fullscreen)
		{
			return;
		}

		if (!m_tearingSupport)
		{
			return;
		}

		m_fullscreen = fullscreen;
		if (fullscreen)
		{
			GetWindowRect(m_hMainWnd, &m_windowedRect);
			SetWindowLong(m_hMainWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW & ~(WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_SYSMENU | WS_THICKFRAME));

			RECT windowRect;
			if (m_swapChain)
			{
				ComPtr<IDXGIOutput> pOutput;
				ThrowIfFailed(m_swapChain->GetContainingOutput(pOutput.GetAddressOf()));
				DXGI_OUTPUT_DESC desc;
				ThrowIfFailed(pOutput->GetDesc(&desc));
				windowRect = desc.DesktopCoordinates;
			}
			// Fallback to EnumDisplaySettings implementation
			else
			{
				// Get the settings of the primary display
				DEVMODE devMode = {};
				devMode.dmSize = sizeof(DEVMODE);
				EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &devMode);

				windowRect = {
					devMode.dmPosition.x,
					devMode.dmPosition.y,
					devMode.dmPosition.x + static_cast<LONG>(devMode.dmPelsWidth),
					devMode.dmPosition.y + static_cast<LONG>(devMode.dmPelsHeight)
				};
			}

			SetWindowPos(
				m_hMainWnd,
				HWND_TOP,
				windowRect.left,
				windowRect.top,
				windowRect.right - windowRect.left,
				windowRect.bottom - windowRect.top,
				SWP_FRAMECHANGED | SWP_NOACTIVATE
			);
			ShowWindow(m_hMainWnd, SW_NORMAL);
		}
		else
		{
			SetWindowLong(m_hMainWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

			SetWindowPos(
				m_hMainWnd,
				HWND_NOTOPMOST,
				m_windowedRect.left,
				m_windowedRect.top,
				m_windowedRect.right - m_windowedRect.left,
				m_windowedRect.bottom - m_windowedRect.top,
				SWP_FRAMECHANGED | SWP_NOACTIVATE
			);
			ShowWindow(m_hMainWnd, SW_NORMAL);
		}
	}

	bool Core::OnResizeWindow(int width, int height)
	{ ZoneScoped;
		if (width <= 0 || height <= 0)
		{
			return false;
		}

		assert(m_d3dDevice);
		assert(m_swapChain);
		assert(m_directCmdListAlloc);

		// Flush before changing any resources.
		FlushCommandQueue();

		PrepareDirectCommandList();

		// Release the previous resources we will be recreating.
		for (int i = 0; i < SwapChainBufferCount; ++i)
		{
			m_swapChainBuffers[i].Reset();
		}
		m_depthStencilBuffer.Reset();

		// Get the description of the swap chain.
		DXGI_SWAP_CHAIN_DESC swapChainDesc;
		m_swapChain->GetDesc(&swapChainDesc);

		// Resize the swap chain.
		ThrowIfFailed(m_swapChain->ResizeBuffers(
			SwapChainBufferCount,
			width, height,
			swapChainDesc.BufferDesc.Format,
			swapChainDesc.Flags
		));

		m_currBackBuffer = 0;

		// Create Resources for Render Target Views.
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());
		for (UINT i = 0; i < SwapChainBufferCount; i++)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_swapChainBuffers[i])));
			m_d3dDevice->CreateRenderTargetView(m_swapChainBuffers[i].Get(), nullptr, rtvHeapHandle);
			rtvHeapHandle.Offset(1, m_rtvDescriptorSize);
		}

		// Create the depth/stencil buffer and view.
		D3D12_RESOURCE_DESC depthStencilDesc;
		depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthStencilDesc.Alignment = 0;			// alignment size of the resource. 0 means use default alignment.
		depthStencilDesc.Width = width;			// size of the width.
		depthStencilDesc.Height = height;		// size of the height.
		depthStencilDesc.DepthOrArraySize = 1;	// size of the depth.
		depthStencilDesc.MipLevels = 1;			// number of mip levels.

		// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
		// the depth buffer.  Therefore, because we need to create two views to the same resource:
		//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
		//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
		// we need to create the depth buffer resource with a typeless format.
		depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

		depthStencilDesc.SampleDesc.Count = m_4xMsaaState ? 4 : 1;
		depthStencilDesc.SampleDesc.Quality = m_4xMsaaState ? (m_4xMsaaQuality - 1) : 0;
		depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;				// layout of the texture to be used in the pipeline.
		depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;	// resource is used as a depth-stencil buffer.

		D3D12_CLEAR_VALUE optClear;
		optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;
		ThrowIfFailed(m_d3dDevice->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&depthStencilDesc,
			D3D12_RESOURCE_STATE_COMMON,
			&optClear,
			IID_PPV_ARGS(&m_depthStencilBuffer)
		));

		// Create descriptor to mip level 0 of entire resource using the format of the resource.
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.Texture2D.MipSlice = 0;
		m_d3dDevice->CreateDepthStencilView(m_depthStencilBuffer.Get(), &dsvDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

		// Transition the resource from its initial state to be used as a depth buffer.
		m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(
			m_depthStencilBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_DEPTH_WRITE
		));

		m_deferredRenderer->OnResize(width, height);
		m_deferredRenderer->RebuildDescriptors();
		m_screenSpaceAO->OnResize(width, height, m_d3dDevice.Get());
		m_screenSpaceAO->RebuildDescriptors(m_depthStencilBuffer.Get());
		m_motionBlur->OnResize(width, height);
		m_motionBlur->RebuildDescriptors();
		m_postProcessBloom->OnResize(width, height);
		m_postProcessBloom->RebuildDescriptors();
		m_postProcessFXAA->OnResize(width, height);
		m_postProcessFXAA->RebuildDescriptors();
		m_postProcessOutline->OnResize(width, height);
		m_postProcessOutline->RebuildDescriptors();

		ExecuteAndFlushDirectCommandList();

		// Update the viewport transform to cover the client area.
		m_screenViewport.TopLeftX = 0;
		m_screenViewport.TopLeftY = 0;
		m_screenViewport.Width = static_cast<float>(width);
		m_screenViewport.Height = static_cast<float>(height);
		m_screenViewport.MinDepth = 0.0f;
		m_screenViewport.MaxDepth = 1.0f;

		m_scissorRect = { 0, 0, width, height };

		return true;
	}

	void Core::InitImGui()
	{
		static ID3D12DescriptorHeap* g_pSrvHeap = m_srvHeap.Get();
		static UINT* g_pCbvSrvUavDescriptorSize = &m_cbvSrvUavDescriptorSize;
		static UINT* g_pSrvHeapSize = &m_srvHeapSize;

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

		ImGui_ImplDX12_InitInfo init_info = {};
		init_info.Device = m_d3dDevice.Get();
		init_info.CommandQueue = m_commandQueue.Get();
		init_info.NumFramesInFlight = FrameResourceCount;
		init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
		// Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
		// (current version of the backend will only allocate one descriptor, future versions will need to allocate more)
		init_info.SrvDescriptorHeap = g_pSrvHeap;
		init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) {
			*out_cpu_handle = CD3DX12_CPU_DESCRIPTOR_HANDLE(g_pSrvHeap->GetCPUDescriptorHandleForHeapStart(), *g_pSrvHeapSize, *g_pCbvSrvUavDescriptorSize);
			*out_gpu_handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(g_pSrvHeap->GetGPUDescriptorHandleForHeapStart(), *g_pSrvHeapSize, *g_pCbvSrvUavDescriptorSize);
			(*g_pSrvHeapSize)++;
			};
		init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE, D3D12_GPU_DESCRIPTOR_HANDLE) {
			(*g_pSrvHeapSize)--;
			};
		ImGui_ImplDX12_Init(&init_info);
		ImGui_ImplWin32_Init(m_hMainWnd);
	}

	extern unsigned long long g_sceneObjectCount;

	void Core::ImGuiNewFrame()
	{
		static std::array<float, 100> frameTimes;
		std::array<float, 100> frameTimeHistogram;
		static float smoothMaxFrameTime = 0.0f;

		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		// ImGui::ShowDemoWindow();
		// ImGui::ShowStyleEditor();

		// Draw histogram

		// Calculate the frame time
		std::copy(frameTimes.begin() + 1, frameTimes.end(), frameTimes.begin());
		frameTimes.back() = m_timeMeasure->GetTime().deltaTime;

		// Find the maximum frame time
		std::memset(frameTimeHistogram.data(), 0, frameTimeHistogram.size() * sizeof(float));
		float maxFrameTime = *std::max_element(frameTimes.begin(), frameTimes.end()) * 1.5f;
		smoothMaxFrameTime = std::lerp(smoothMaxFrameTime, maxFrameTime, 0.1f);

		// Update the histogram data
		for (size_t i = 0; i < frameTimes.size(); ++i)
		{
			int targetIndex = static_cast<int>((frameTimes[i] / smoothMaxFrameTime) * frameTimeHistogram.size());
			if (targetIndex >= 0 && targetIndex < frameTimeHistogram.size())
			{
				frameTimeHistogram[targetIndex]++;
			}
		}
		float maxHistogramValue = *std::max_element(frameTimeHistogram.begin(), frameTimeHistogram.end()) * 1.25f;

		static std::array<float, 100> frameTimesPsum;
		std::copy(frameTimes.begin(), frameTimes.end(), frameTimesPsum.begin());
		std::sort(frameTimesPsum.begin(), frameTimesPsum.end(), std::greater<float>());
		for (size_t i = 1; i < frameTimesPsum.size(); ++i)
		{
			frameTimesPsum[i] += frameTimesPsum[i - 1];
		}

		// Draw the histogram
		ImGui::Begin("Updown Studio Dubug Window");
		// Set plot color to white (without outlines)
		ImGui::Text("Frame Per Second 100%%: %.3f FPS", 100.0f / frameTimesPsum[99]);
		ImGui::Text("Frame Per Second 10%%:  %.3f FPS", 10.0f / frameTimesPsum[9]);
		ImGui::Text("Frame Per Second 1%%:   %.3f FPS", 1.0f / frameTimesPsum[0]);
		ImGui::Text("Allocated SceneObjects: %llu", g_sceneObjectCount);
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_PlotHistogramHovered, ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
		ImGui::PlotHistogram("Frame Times", frameTimes.data(), static_cast<int>(frameTimes.size()), 0, nullptr, 0.0f, smoothMaxFrameTime, ImVec2(0.0f, 100.0f));
		ImGui::PlotLines("Frame Time Histogram", frameTimeHistogram.data(), static_cast<int>(frameTimeHistogram.size()), 0, nullptr, 0.0f, maxHistogramValue, ImVec2(0.0f, 100.0f));
		ImGui::PopStyleColor(2);


		ImGui::Checkbox("Draw Shadow Map", &m_renderOptions.DrawShadowMap);
		bool changeSSAO = ImGui::Checkbox("Draw SSAO", &m_renderOptions.DrawSSAO);
		ImGui::Checkbox("Draw Motion Blur", &m_renderOptions.DrawMotionBlur);
		ImGui::Checkbox("Draw Post Process Bloom", &m_renderOptions.DrawBloom);
		ImGui::Checkbox("Draw Post Process FXAA", &m_renderOptions.DrawFXAA);
		ImGui::Checkbox("Draw Post Process Outline", &m_renderOptions.DrawOutline);

		static float exposure = m_postProcessBloom->GetExposure();
		if (ImGui::SliderFloat("Exposure", &exposure, 0.0f, 10.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
		{
			m_postProcessBloom->SetExposure(exposure);
		}
		
		static float bloomStrength = m_postProcessBloom->GetBloomStrength();
		if (ImGui::SliderFloat("Bloom Strength", &bloomStrength, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
		{
			m_postProcessBloom->SetBloomStrength(bloomStrength);
		}
		
		// Draw combobox for shadow map resolution. the options are (256, 512, 1024, 2048, 4096, 8192).
		static const char* shadowMapResolutions[] = { "256", "512", "1024", "2048", "4096", "8192" };
		std::string currentShadowMapResolution = std::to_string(m_renderOptions.ShadowMapSize);
		int selectedShadowMapResolution = static_cast<int>(std::distance(std::begin(shadowMapResolutions), std::find(std::begin(shadowMapResolutions), std::end(shadowMapResolutions), currentShadowMapResolution)));
		if (ImGui::Combo("Shadow Map Resolution", &selectedShadowMapResolution, shadowMapResolutions, IM_ARRAYSIZE(shadowMapResolutions)))
		{
			FlushCommandQueue();
			m_renderOptions.ShadowMapSize = static_cast<unsigned int>(std::stoi(shadowMapResolutions[selectedShadowMapResolution]));
			m_shadowMap->OnResize(m_renderOptions.ShadowMapSize, m_renderOptions.ShadowMapSize, m_d3dDevice.Get());
			m_shadowMap->RebuildDescriptors(m_d3dDevice.Get());
		}

		if (changeSSAO && !m_renderOptions.DrawSSAO)
		{
			PrepareDirectCommandList();
			m_screenSpaceAO->ClearSSAOMap(m_commandList.Get());
			ExecuteAndFlushDirectCommandList();
		}

		ImGui::ColorPicker4("Fog Color", reinterpret_cast<float*>(&m_renderOptions.FogColor), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaOpaque | ImGuiColorEditFlags_HDR);
		ImGui::ColorPicker4("Fog Sun Color", reinterpret_cast<float*>(&m_renderOptions.FogSunColor), ImGuiColorEditFlags_Float | ImGuiColorEditFlags_AlphaOpaque | ImGuiColorEditFlags_HDR);
		ImGui::SliderFloat("Fog Density", &m_renderOptions.FogDensity, 0.0f, 10000.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);
		ImGui::SliderFloat("Fog Height Falloff", &m_renderOptions.FogHeightFalloff, 0.0f, 10000.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);
		ImGui::SliderFloat("Fog Distance Start", &m_renderOptions.FogDistanceStart, 0.0f, 100.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

		// Set window position to top left corner
		ImGui::SetWindowPos(ImVec2(0, 0));
		// Set window size and makes it resizable
		ImGui::SetWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);

		ImGui::End();

		m_scene->OnDrawGizmos();
	}

	void Core::ImGuiRender()
	{
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_commandList.Get());
	}

	void Core::ReleaseImGui()
	{
		ImGui_ImplDX12_Shutdown();
		ImGui_ImplWin32_Shutdown();

		ImGui::DestroyContext();
	}

	HINSTANCE Core::GetInstance() const
	{
		return m_hInstance;
	}

	HWND Core::GetMainWindow() const
	{
		return m_hMainWnd;
	}

	ID3D12Device* Core::GetDevice() const
	{
		return m_d3dDevice.Get();
	}

	ID3D12CommandQueue* Core::GetCommandQueue() const
	{
		return m_commandQueue.Get();
	}

	ID3D12CommandAllocator* Core::GetCommandAllocator() const
	{
		return m_directCmdListAlloc.Get();
	}

	ID3D12GraphicsCommandList* Core::GetCommandList() const
	{
		return m_commandList.Get();
	}

	DeferredRenderer* Core::GetRenderer() const
	{
		return m_deferredRenderer.get();
	}

	ShadowMap* Core::GetShadowMap() const
	{
		return m_shadowMap.get();
	}

	ScreenSpaceAO* Core::GetScreenSpaceAO() const
	{
		return m_screenSpaceAO.get();
	}

	MonoUploadBuffer* Core::GetMonoUploadBuffer() const
	{
		return m_monoUploadBuffer.get();
	}

	ID3D12RootSignature* Core::GetRootSignature() const
	{
		return m_rootSignature.Get();
	}

	ID3D12Resource* Core::CurrentBackBuffer() const
	{
		return m_swapChainBuffers[m_currBackBuffer].Get();
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Core::CurrentBackBufferView() const
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
			m_currBackBuffer,
			m_rtvDescriptorSize
		);
	}

	FrameResource* Core::CurrentFrameResource() const
	{
		return m_frameResources[m_currFrameResourceIndex].get();
	}

	int Core::GetClientWidth() const
	{
		return m_clientWidth;
	}

	int Core::GetClientHeight() const
	{
		return m_clientHeight;
	}

	float Core::GetAspectRatio() const
	{
		return static_cast<float>(m_clientWidth) / m_clientHeight;
	}

	void Core::SetClearColor(const Color& clearColor)
	{
		m_clearColor = clearColor;
	}
	
	void Core::SetClearColor(float r, float g, float b)
	{
		m_clearColor = Color(r, g, b, 1.0f);
	}

	D3D12_CPU_DESCRIPTOR_HANDLE Core::DepthStencilView() const
	{
		return m_dsvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	DescriptorParam Core::GetDescriptorParameters() const
	{
		DescriptorParam descriptorParam{
			.CbvCpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetCPUDescriptorHandleForHeapStart(), m_cbvHeapSize, m_cbvSrvUavDescriptorSize),
			.SrvCpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvHeap->GetCPUDescriptorHandleForHeapStart(), m_srvHeapSize, m_cbvSrvUavDescriptorSize),
			.RtvCpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_rtvHeapSize, m_rtvDescriptorSize),
			.DsvCpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), m_dsvHeapSize, m_dsvDescriptorSize),
			.CbvGpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetGPUDescriptorHandleForHeapStart(), m_cbvHeapSize, m_cbvSrvUavDescriptorSize),
			.SrvGpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_srvHeap->GetGPUDescriptorHandleForHeapStart(), m_srvHeapSize, m_cbvSrvUavDescriptorSize),
			.CbvSrvUavDescriptorSize = m_cbvSrvUavDescriptorSize,
			.RtvDescriptorSize = m_rtvDescriptorSize,
			.DsvDescriptorSize = m_dsvDescriptorSize
		};

		return descriptorParam;
	}

	void Core::ApplyDescriptorParameters(const DescriptorParam& param)
	{
		m_cbvHeapSize = static_cast<UINT>(param.CbvCpuHandle.ptr - m_cbvHeap->GetCPUDescriptorHandleForHeapStart().ptr) / m_cbvSrvUavDescriptorSize;
		m_srvHeapSize = static_cast<UINT>(param.SrvCpuHandle.ptr - m_srvHeap->GetCPUDescriptorHandleForHeapStart().ptr) / m_cbvSrvUavDescriptorSize;
		m_rtvHeapSize = static_cast<UINT>(param.RtvCpuHandle.ptr - m_rtvHeap->GetCPUDescriptorHandleForHeapStart().ptr) / m_rtvDescriptorSize;
		m_dsvHeapSize = static_cast<UINT>(param.DsvCpuHandle.ptr - m_dsvHeap->GetCPUDescriptorHandleForHeapStart().ptr) / m_dsvDescriptorSize;
	}

	RenderOptions& Core::GetRenderOptionsRef()
	{
		return m_renderOptions;
	}

	int Core::GetClientPosX() const
	{
		return m_clientPosX;
	}

	int Core::GetClientPosY() const
	{
		return m_clientPosY;
	}
}
