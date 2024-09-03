#include "pch.h"
#include "core.h"

#include "singleton.h"
#include "resource.h"
#include "input.h"
#include "frame_resource.h"
#include "time_measure.h"
#include "scene.h"
#include "mesh.h"
#include "shader.h"
#include "frame_debug.h"
#include "debug_console.h"
#include "audio.h"
#include "texture.h"
#include "shadow_map.h"
#include "screen_space_ao.h"

#include <assimp/DefaultLogger.hpp>

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

		for (int i = 0; i < FrameResourceCount; ++i)
		{
			m_frameResources[i] = std::make_unique<FrameResource>(m_d3dDevice.Get());
		}

		auto audio = Singleton<Audio>::CreateInstance();
		auto resource = Singleton<Resource>::CreateInstance();
		m_timeMeasure = Singleton<TimeMeasure>::CreateInstance();

#if defined(DEBUG) || defined(_DEBUG)
		Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
#endif
		INSTANCE(Input)->Initialize(m_hMainWnd);
		resource->Initialize(m_d3dDevice.Get(), m_commandQueue.Get(), m_commandList.Get(), m_rootSignature.Get());

#if defined(DEBUG) || defined(_DEBUG)
		Assimp::DefaultLogger::kill();
#endif

		BuildDescriptorHeaps();
		BuildConstantBuffers();

		ExecuteCommandList();
		OnResizeWindow(m_clientWidth, m_clientHeight);

		for (auto& resource_mesh : resource->LoadAll<Mesh>())
		{
			resource_mesh->DisposeUploaders();
		}

		// Reset the command list to prep for initialization commands.
		ThrowIfFailed(m_commandList->Reset(m_directCmdListAlloc.Get(), nullptr));

		m_fenceEvent = ::CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		// Create frame debug window
		m_frameDebug = std::make_unique<FrameDebug>(m_hInstance);
	}

	void Core::InitializeDirect3D()
	{ ZoneScoped;
#if defined(DEBUG) || defined(_DEBUG)
		EnableDebugLayer();
#endif
		// Create DXGI Factory
		ThrowIfFailed(::CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgiFactory)));
		// Create hardware device
		HRESULT hr = ::D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_d3dDevice));

		// Fallback to WARP device
		if (FAILED(hr))
		{
			ComPtr<IDXGIAdapter> adapter = nullptr;
			ThrowIfFailed(m_dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&adapter)));

			// Create hardware device with WARP adapter
			ThrowIfFailed(::D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_d3dDevice)));
		}

		m_tearingSupport = CheckTearingSupport();

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

#if defined(DEBUG) || defined(_DEBUG)
		LogAdapterInfo();
#endif
		CreateCommandObjects();
		CreateSwapChain();
		CreateRtvAndDsvDescriptorHeaps();
		BuildRootSignature();
		
		// Create Shadow Map
		m_shadowMap = std::make_unique<ShadowMap>(8192u, 8192u, m_d3dDevice.Get());
		m_shadowMap->BuildPipelineState(m_d3dDevice.Get(), m_rootSignature.Get());

		// Create Screen Space Ambient Occlusion
		m_screenSpaceAO = std::make_unique<ScreenSpaceAO>(m_d3dDevice.Get(), m_commandList.Get(), 0.5f);
		m_screenSpaceAO->BuildPipelineState(m_d3dDevice.Get(), m_rootSignature.Get());

		const char tracyQueueName[] = "D3D12 Graphics Queue";
		m_tracyQueueCtx = TracyD3D12Context(m_d3dDevice.Get(), m_commandQueue.Get());
		TracyD3D12ContextName(m_tracyQueueCtx, tracyQueueName, sizeof(tracyQueueName));

		if (m_tearingSupport)
		{
			m_dxgiFactory->MakeWindowAssociation(m_hMainWnd, DXGI_MWA_NO_ALT_ENTER);
		}
	}

	void Core::EnableDebugLayer()
	{
		ThrowIfFailed(::D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugLayer)));
		m_debugLayer->EnableDebugLayer();
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

		// Reset the command list to prep for initialization commands.
		ThrowIfFailed(m_commandList->Reset(m_directCmdListAlloc.Get(), nullptr));
	}

	void Core::RegisterUpdateCallback(std::function<void(const Time&)> callback)
	{
		m_updateCallback = callback;
	}

	void Core::CreateSwapChain()
	{ ZoneScoped;
		// Release the previous swapchain we will be recreating.
		m_swapChain.Reset();

		DXGI_SWAP_CHAIN_DESC sd;
		sd.BufferDesc.Width = m_clientWidth;
		sd.BufferDesc.Height = m_clientHeight;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

		// Use 4X MSAA
		if (m_4xMsaaState)
		{
			sd.SampleDesc.Count = 4;
			sd.SampleDesc.Quality = m_4xMsaaQuality - 1;
		}
		// No MSAA
		else
		{
			sd.SampleDesc.Count = 1;
			sd.SampleDesc.Quality = 0;
		}

		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.BufferCount = SwapChainBufferCount;
		sd.OutputWindow = m_hMainWnd;
		sd.Windowed = true;
		sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		sd.Flags = m_tearingSupport ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

		// Note: Swap chain uses queue to perform flush.
		ThrowIfFailed(m_dxgiFactory->CreateSwapChain(m_commandQueue.Get(), &sd, m_swapChain.GetAddressOf()));
	}

	void Core::CreateRtvAndDsvDescriptorHeaps()
	{ ZoneScoped;
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		rtvHeapDesc.NumDescriptors = SwapChainBufferCount + 4;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_rtvHeap.GetAddressOf())));

		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
		dsvHeapDesc.NumDescriptors = 2;
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
			::CloseHandle(m_fenceEvent);

			// Release the context
			TracyD3D12Destroy(m_tracyQueueCtx);
		}
	}

	void Core::BuildDescriptorHeaps()
	{ ZoneScoped;

		std::vector<Texture*> textures = INSTANCE(Resource)->LoadAll<Texture>();

		// Need a CBV descriptor for each frame resource,
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
		cbvHeapDesc.NumDescriptors = FrameResourceCount;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		cbvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(m_cbvHeap.GetAddressOf())));

		// Create SRV heap
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc;
		srvHeapDesc.NumDescriptors = static_cast<UINT>(textures.size() + 6);
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		srvHeapDesc.NodeMask = 0;
		ThrowIfFailed(m_d3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(m_srvHeap.GetAddressOf())));

		DescriptorParam descriptorParam{
			.CbvCpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetCPUDescriptorHandleForHeapStart()),
			.SrvCpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_srvHeap->GetCPUDescriptorHandleForHeapStart()),
			.RtvCpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), SwapChainBufferCount, m_rtvDescriptorSize),
			.DsvCpuHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_dsvHeap->GetCPUDescriptorHandleForHeapStart(), 1, m_dsvDescriptorSize),

			.CbvGpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_cbvHeap->GetGPUDescriptorHandleForHeapStart()),
			.SrvGpuHandle = CD3DX12_GPU_DESCRIPTOR_HANDLE(m_srvHeap->GetGPUDescriptorHandleForHeapStart()),

			.CbvSrvUavDescriptorSize = m_cbvSrvUavDescriptorSize,
			.RtvDescriptorSize = m_rtvDescriptorSize,
			.DsvDescriptorSize = m_dsvDescriptorSize
		};

		m_shadowMap->BuildDescriptors(descriptorParam, m_d3dDevice.Get());
		m_screenSpaceAO->BuildDescriptors(descriptorParam, m_depthStencilBuffer.Get());

		for (auto texture : textures)
		{
			texture->CreateShaderResourceView(m_d3dDevice.Get(), descriptorParam);
		}
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
		CD3DX12_ROOT_PARAMETER slotRootParameter[6];

		CD3DX12_DESCRIPTOR_RANGE texTable;
		texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);
		CD3DX12_DESCRIPTOR_RANGE shadowMapTable;
		shadowMapTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 1);

		slotRootParameter[0].InitAsConstants(sizeof(ObjectConstants) / 4, 0);
		slotRootParameter[1].InitAsConstants(sizeof(CameraConstants) / 4, 1);
		slotRootParameter[2].InitAsConstantBufferView(2);
		slotRootParameter[3].InitAsConstantBufferView(3);
		slotRootParameter[4].InitAsDescriptorTable(1, &texTable);
		slotRootParameter[5].InitAsDescriptorTable(1, &shadowMapTable);

		CD3DX12_STATIC_SAMPLER_DESC samplerDesc[] = {
			CD3DX12_STATIC_SAMPLER_DESC(
			0,
			D3D12_FILTER_MIN_MAG_MIP_POINT,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP),
			CD3DX12_STATIC_SAMPLER_DESC(
			1,
			D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,
			0.0f,
			16,
			D3D12_COMPARISON_FUNC_LESS_EQUAL,
			D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK)
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

	bool Core::CheckTearingSupport() const
	{
#ifndef PIXSUPPORT
		ComPtr<IDXGIFactory6> factory;
		HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
		BOOL allowTearing = FALSE;
		if (SUCCEEDED(hr))
		{
			hr = factory->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &allowTearing, sizeof(allowTearing));
		}
		return SUCCEEDED(hr) && allowTearing;
#else
		return true;
#endif
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

	void Core::SetScene(std::shared_ptr<Scene> scene)
	{
		m_scene = scene;
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
	}

	void Core::Update()
	{ ZoneScopedC(0xFAAB36);
		DisplayFrameStats();

		// Advance the time measure
		m_timeMeasure->Tick();

		INSTANCE(Audio)->Update();
		INSTANCE(Input)->Update();

		BroadcastUpdateMessage();
		m_scene->Update(m_timeMeasure->GetTime());

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

			.AspectRatio = static_cast<float>(m_clientWidth) / m_clientHeight,
			.FrameResourceIndex = m_currFrameResourceIndex,

			.Viewport = m_screenViewport,
			.ScissorRect = m_scissorRect,

			.UseFrustumCulling = true,

			.ConstantBufferView = objectCB->Resource()->GetGPUVirtualAddress(),
			.DepthStencilView = DepthStencilView(),
			.RenderTargetView = CurrentBackBufferView(),

			.RenderShadowMap = m_shadowMap.get(),
			.RenderScreenSpaceAO = m_screenSpaceAO.get(),

			.TracyQueueContext = &m_tracyQueueCtx
		};

		ThrowIfFailed(cmdListAlloc->Reset());
		// Resets a command list back to its initial state as if a new command list was just created.
		// ID3D12PipelineState: This is optional and can be NULL.
		// If NULL, the runtime sets a dummy initial pipeline state so that drivers don't have to deal with undefined state.
		ThrowIfFailed(m_commandList->Reset(cmdListAlloc, nullptr));

		ID3D12DescriptorHeap* descriptorHeaps[] = { m_srvHeap.Get() };
		m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

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

		// Frame debug
		m_frameDebug->Update(m_timeMeasure->GetTime());
	}

	void Core::UpdateMainPassCB()
	{ ZoneScoped;
		PassConstants passConstants;
		passConstants.TotalTime = m_timeMeasure->GetTime().totalTime;

		auto frameResource = CurrentFrameResource();
		frameResource->GetObjectCB()->CopyData(0, passConstants);
	}

	bool Core::ProcessMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{ ZoneScoped;
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
		default:
			return INSTANCE(Input)->ProcessMessage(hWnd, message, wParam, lParam);
		}
		
		return true;
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

		ThrowIfFailed(m_commandList->Reset(m_directCmdListAlloc.Get(), nullptr));

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

		m_screenSpaceAO->OnResize(width, height, m_d3dDevice.Get());
		m_screenSpaceAO->RebuildDescriptors(m_depthStencilBuffer.Get());

		// Execute the resize commands.
		ExecuteCommandList();
		// Wait until resize is complete.
		FlushCommandQueue();

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

	int Core::GetClientPosX() const
	{
		return m_clientPosX;
	}

	int Core::GetClientPosY() const
	{
		return m_clientPosY;
	}
}
