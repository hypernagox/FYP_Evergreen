#include "pch.h"
#include "MinimapRenderer.h"

using namespace udsdx;

MinimapRenderer::MinimapRenderer(ID3D12Device* device, UINT width, UINT height) : m_device(device), m_width(width), m_height(height)
{
	m_viewport = { 0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f };
	m_scissorRect = { 0, 0, (int)width, (int)height };

	auto rootParam = INSTANCE(Core)->GetDescriptorParameters();
	BuildRootSignature();
	BuildResources();
	BuildDescriptors(rootParam);
	INSTANCE(Core)->ApplyDescriptorParameters(rootParam);
	BuildPipelineStateObject();

	m_renderTargetTexture = std::make_unique<udsdx::Texture>(m_renderTarget.Get(), m_srvCpuHandle, m_srvGpuHandle);
	SetViewMatrix(udsdx::Vector3(0.0f, 100.0f, 0.0f), udsdx::Vector3(0.0f, -1.0f, 0.0f));

	m_markTexture = INSTANCE(Resource)->Load<udsdx::Texture>(RESOURCE_PATH(L"gui\\minimap\\monster_normal_icon.png"));
}

MinimapRenderer::~MinimapRenderer()
{
}

void MinimapRenderer::BuildRootSignature()
{
	CD3DX12_ROOT_PARAMETER slotRootParameter[2];
	CD3DX12_DESCRIPTOR_RANGE descriptorRange[1];
	descriptorRange[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);

	slotRootParameter[0].InitAsConstants(48, 0, 0, D3D12_SHADER_VISIBILITY_ALL);
	slotRootParameter[1].InitAsDescriptorTable(1, descriptorRange, D3D12_SHADER_VISIBILITY_PIXEL);

	// Create a static sampler
	CD3DX12_STATIC_SAMPLER_DESC samplerDesc(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
				D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(slotRootParameter), slotRootParameter, 1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_device->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(m_rootSignature.GetAddressOf())));
}

void MinimapRenderer::BuildResources()
{
	m_renderTarget.Reset();
	m_depthStencil.Reset();

	CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);

	D3D12_RESOURCE_DESC renderTargetDesc = {};
	renderTargetDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	renderTargetDesc.Alignment = 0;
	renderTargetDesc.Width = m_width;
	renderTargetDesc.Height = m_height;
	renderTargetDesc.DepthOrArraySize = 1;
	renderTargetDesc.MipLevels = 1;
	renderTargetDesc.Format = RENDER_FORMAT;
	renderTargetDesc.SampleDesc.Count = 1;
	renderTargetDesc.SampleDesc.Quality = 0;
	renderTargetDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	renderTargetDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

	CD3DX12_CLEAR_VALUE clearValue{ RENDER_FORMAT, RENDER_CLEAR_VALUE };

	ThrowIfFailed(m_device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&renderTargetDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		&clearValue,
		IID_PPV_ARGS(&m_renderTarget)));

	D3D12_RESOURCE_DESC depthStencilDesc = {};
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = m_width;
	depthStencilDesc.Height = m_height;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	CD3DX12_CLEAR_VALUE depthClearValue{ DEPTH_FORMAT, 1.0f, 0 };

	ThrowIfFailed(m_device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&m_depthStencil)));
}

void MinimapRenderer::BuildDescriptors(udsdx::DescriptorParam& descriptorParam)
{
	m_srvCpuHandle = descriptorParam.SrvCpuHandle;
	m_srvGpuHandle = descriptorParam.SrvGpuHandle;
	m_rtvCpuHandle = descriptorParam.RtvCpuHandle;
	m_dsvCpuHandle = descriptorParam.DsvCpuHandle;

	descriptorParam.SrvCpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
	descriptorParam.SrvGpuHandle.Offset(1, descriptorParam.CbvSrvUavDescriptorSize);
	descriptorParam.RtvCpuHandle.Offset(1, descriptorParam.RtvDescriptorSize);
	descriptorParam.DsvCpuHandle.Offset(1, descriptorParam.DsvDescriptorSize);

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = RENDER_FORMAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	m_device->CreateShaderResourceView(m_renderTarget.Get(), &srvDesc, m_srvCpuHandle);

	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = RENDER_FORMAT;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	m_device->CreateRenderTargetView(m_renderTarget.Get(), &rtvDesc, m_rtvCpuHandle);

	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
	dsvDesc.Format = DEPTH_FORMAT;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	m_device->CreateDepthStencilView(m_depthStencil.Get(), &dsvDesc, m_dsvCpuHandle);
}

void MinimapRenderer::BuildPipelineStateObject()
{
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.InputLayout = { Vertex::DescriptionTable, Vertex::DescriptionTableSize };
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = TRUE;
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.RTVFormats[0] = RENDER_FORMAT;
		psoDesc.DSVFormat = DEPTH_FORMAT;

		{
			auto vsByteCode = DX::ReadData(L"compiled_shaders\\vs_minimap_pass.cso");
			auto psByteCode = DX::ReadData(L"compiled_shaders\\ps_minimap_pass.cso");

			psoDesc.VS =
			{
				reinterpret_cast<BYTE*>(vsByteCode.data()),
				vsByteCode.size()
			};
			psoDesc.PS =
			{
				reinterpret_cast<BYTE*>(psByteCode.data()),
				psByteCode.size()
			};

			ThrowIfFailed(m_device->CreateGraphicsPipelineState(
				&psoDesc,
				IID_PPV_ARGS(m_pipelineState.GetAddressOf())
			));
			m_pipelineState->SetName(L"MinimapRenderer::PassRender");
		}
	}

	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
		ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.InputLayout = { nullptr, 0 };
		psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		// typical blend state for gui rendering
		psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
		psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
		psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
		psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;


		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState.DepthEnable = FALSE; // Disable depth testing for minimap marks
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		psoDesc.NumRenderTargets = 1;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.RTVFormats[0] = RENDER_FORMAT;
		psoDesc.DSVFormat = DEPTH_FORMAT;

		{
			auto vsByteCode = DX::ReadData(L"compiled_shaders\\vs_minimapmark_pass.cso");
			auto gsByteCode = DX::ReadData(L"compiled_shaders\\gs_minimapmark_pass.cso");
			auto psByteCode = DX::ReadData(L"compiled_shaders\\ps_minimapmark_pass.cso");

			psoDesc.VS =
			{
				reinterpret_cast<BYTE*>(vsByteCode.data()),
				vsByteCode.size()
			};
			psoDesc.GS =
			{
				reinterpret_cast<BYTE*>(gsByteCode.data()),
				gsByteCode.size()
			};
			psoDesc.PS =
			{
				reinterpret_cast<BYTE*>(psByteCode.data()),
				psByteCode.size()
			};

			ThrowIfFailed(m_device->CreateGraphicsPipelineState(
				&psoDesc,
				IID_PPV_ARGS(m_markPipelineState.GetAddressOf())
			));
			m_markPipelineState->SetName(L"MinimapRenderer::PassRenderMark");
		}
	}
}

void MinimapRenderer::SetViewMatrix(const udsdx::Vector3& position, const udsdx::Vector3& forward)
{
	XMStoreFloat4x4(&m_viewMatrix, XMMatrixTranspose(XMMatrixLookToLH(
		DirectX::XMVectorSet(position.x, position.y, position.z, 1.0f),
		DirectX::XMVectorSet(forward.x, forward.y, forward.z, 0.0f),
		DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)
	)));
}

void MinimapRenderer::OnDetach()
{
	auto rootParam = INSTANCE(Core)->GetDescriptorParameters();

	rootParam.SrvCpuHandle.Offset(-1, rootParam.CbvSrvUavDescriptorSize);
	rootParam.SrvGpuHandle.Offset(-1, rootParam.CbvSrvUavDescriptorSize);
	rootParam.RtvCpuHandle.Offset(-1, rootParam.RtvDescriptorSize);
	rootParam.DsvCpuHandle.Offset(-1, rootParam.DsvDescriptorSize);

	INSTANCE(Core)->ApplyDescriptorParameters(rootParam);
}

void MinimapRenderer::PassRender(udsdx::RenderParam& renderParam, const std::vector<Vector3>& marks)
{
	CD3DX12_RESOURCE_BARRIER barrier[2];
	barrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTarget.Get(),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		D3D12_RESOURCE_STATE_RENDER_TARGET);
	barrier[1] = CD3DX12_RESOURCE_BARRIER::Transition(
		m_renderTarget.Get(),
		D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_GENERIC_READ);

	ID3D12GraphicsCommandList* pCommandList = renderParam.CommandList;

	pCommandList->SetGraphicsRootSignature(m_rootSignature.Get());

	pCommandList->ResourceBarrier(1, &barrier[0]);

	pCommandList->ClearRenderTargetView(m_rtvCpuHandle, RENDER_CLEAR_VALUE, 0, nullptr);
	pCommandList->ClearDepthStencilView(m_dsvCpuHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	pCommandList->OMSetRenderTargets(1, &m_rtvCpuHandle, true, &m_dsvCpuHandle);

	pCommandList->RSSetViewports(1, &m_viewport);
	pCommandList->RSSetScissorRects(1, &m_scissorRect);

	pCommandList->SetPipelineState(m_pipelineState.Get());

	pCommandList->SetGraphicsRoot32BitConstants(0, 16, &m_worldMatrix, 0);
	pCommandList->SetGraphicsRoot32BitConstants(0, 16, &m_viewMatrix, 16);
	pCommandList->SetGraphicsRoot32BitConstants(0, 16, &m_projectionMatrix, 32);
	pCommandList->SetGraphicsRootDescriptorTable(1, m_markTexture->GetSrvGpu());

	// Draw the minimap mesh
	auto vbv = m_minimapMesh->VertexBufferView();
	auto ibv = m_minimapMesh->IndexBufferView();

	pCommandList->IASetVertexBuffers(0, 1, &vbv);
	pCommandList->IASetIndexBuffer(&ibv);
	pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	const auto& submeshes = m_minimapMesh->GetSubmeshes();
	for (size_t index = 0; index < submeshes.size(); ++index)
	{
		const auto& submesh = submeshes[index];
		pCommandList->DrawIndexedInstanced(submesh.IndexCount, 1, submesh.StartIndexLocation, submesh.BaseVertexLocation, 0);
	}

	// Draw minimap marks
	pCommandList->SetPipelineState(m_markPipelineState.Get());

	pCommandList->IASetVertexBuffers(0, 0, nullptr);
	pCommandList->IASetIndexBuffer(nullptr);
	pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	std::vector<Vector3> sortedMarks = marks;
	Vector3 forward = Vector3(m_viewMatrix.m[2][0], m_viewMatrix.m[2][1], m_viewMatrix.m[2][2]);
	std::sort(sortedMarks.begin(), sortedMarks.end(), [&forward](const Vector3& lhs, const Vector3& rhs) {
		return lhs.Dot(forward) > rhs.Dot(forward); // Sort by projection onto the forward vector
	});
	for (const auto& mark : sortedMarks)
	{
		XMMATRIX worldMatrix = XMMatrixTranspose(XMMatrixTranslation(mark.x, mark.y, mark.z));
		pCommandList->SetGraphicsRoot32BitConstants(0, 16, &worldMatrix, 0);

		pCommandList->DrawInstanced(1, 1, 0, 0);
	}

	pCommandList->ResourceBarrier(1, &barrier[1]);
}

void MinimapRenderer::SetMinimapEnvironment(const EnvironmentParameters& environmentParams)
{
	m_worldMatrix = (
		Matrix4x4::CreateScale(environmentParams.TerrainSize) *
		Matrix4x4::CreateTranslation(environmentParams.TerrainOffset, 0.0f, environmentParams.TerrainOffset)).Transpose();
	XMStoreFloat4x4(&m_projectionMatrix, XMMatrixTranspose(XMMatrixOrthographicLH(256.0f, 256.0f, -1000.0f, 1000.0f)));
}
