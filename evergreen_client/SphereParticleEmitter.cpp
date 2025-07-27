#include "pch.h"
#include "SphereParticleEmitter.h"

using namespace udsdx;

ComPtr<ID3D12PipelineState> SphereParticleEmitter::m_pipelineState = nullptr;
std::default_random_engine SphereParticleEmitter::m_randomEngine{};

static ComPtr<ID3D12PipelineState> InitPipelineState(ComPtr<ID3D12Device> device)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));

	psoDesc.InputLayout.pInputElementDescs = nullptr;
	psoDesc.InputLayout.NumElements = 0;
	psoDesc.pRootSignature = INSTANCE(Core)->GetRootSignature();
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = true;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.StencilEnable = false;

	psoDesc.BlendState.RenderTarget[0].BlendEnable = true;
	psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	psoDesc.NumRenderTargets = 1;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R11G11B10_FLOAT;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	auto vsByteCode = DX::ReadData(L"compiled_shaders\\vs_sphere_particle_emitter.cso");
	auto gsByteCode = DX::ReadData(L"compiled_shaders\\gs_particle_emitter.cso");
	auto psByteCode = DX::ReadData(L"compiled_shaders\\ps_particle_emitter.cso");

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

	ComPtr<ID3D12PipelineState> pipelineState;
	ThrowIfFailed(device->CreateGraphicsPipelineState(
		&psoDesc,
		IID_PPV_ARGS(pipelineState.GetAddressOf())
	));

	return pipelineState;
}

void SphereParticleEmitter::OnInitialize()
{
	m_renderGroup = udsdx::RenderGroup::Forward;
	m_topology = D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
	m_seed = static_cast<unsigned int>(std::uniform_int_distribution(0x0000, 0xFFFF)(m_randomEngine));

	if (m_pipelineState == nullptr)
	{
		m_pipelineState = InitPipelineState(INSTANCE(Core)->GetDevice());
	}
}

void SphereParticleEmitter::Update(const udsdx::Time& time, udsdx::Scene& scene)
{
	if (m_isPlaying)
	{
		m_emitterParameter.ElapsedTime += time.deltaTime;
		if (m_emitterParameter.ElapsedTime >= m_emitterParameter.LifeTimeMax && !m_emitLoop)
		{
			m_isPlaying = false;
			m_emitterParameter.ElapsedTime = 0.0f;
			if (m_autoDestroy)
			{
				GetSceneObject()->RemoveFromParent();
			}
		}
	}
}

void SphereParticleEmitter::PostUpdate(const udsdx::Time& time, udsdx::Scene& scene)
{
	RendererBase::PostUpdate(time, scene);

	if (m_isPlaying)
	{
		scene.EnqueueRenderObject(this, m_renderGroup, m_pipelineState.Get(), nullptr, 0);
	}
}

void SphereParticleEmitter::Render(udsdx::RenderParam& param, int parameter)
{
	unsigned int flags =
		static_cast<unsigned int>(m_emitLoop) |
		static_cast<unsigned int>(m_orientedByDirection) << 1 |
		static_cast<unsigned int>(m_verticalBillboard) << 2;

	ObjectConstants objectConstants;
	objectConstants.World = m_transformCache.Transpose();
	objectConstants.World.m[3][0] = m_color.x;
	objectConstants.World.m[3][1] = m_color.y;
	objectConstants.World.m[3][2] = m_color.z;
	objectConstants.World.m[3][3] = std::bit_cast<float>(flags);
	memcpy(&objectConstants.PrevWorld, &m_emitterParameter, sizeof(Matrix4x4));

	param.CommandList->SetGraphicsRoot32BitConstants(RootParam::PerObjectCBV, sizeof(ObjectConstants) / 4, &objectConstants, 0);

	param.CommandList->IASetVertexBuffers(0, 0, nullptr);
	param.CommandList->IASetIndexBuffer(nullptr);
	param.CommandList->IASetPrimitiveTopology(m_topology);

	param.CommandList->SetGraphicsRootDescriptorTable(RootParam::SrcTexSRV_0, m_texture->GetSrvGpu());
	param.CommandList->DrawInstanced(m_drawCount, 1, m_seed, 0);
}

void SphereParticleEmitter::UpdateTransformCache()
{
	m_transformCache = GetSceneObject()->GetTransform()->GetWorldSRTMatrix(false);
}

void SphereParticleEmitter::Play()
{
	m_isPlaying = true;
	m_emitterParameter.ElapsedTime = 0.0f;
}
