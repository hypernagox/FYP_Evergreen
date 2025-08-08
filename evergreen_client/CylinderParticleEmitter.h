#pragma once

#include "pch.h"
#include "SphereParticleEmitter.h"

class CylinderParticleEmitter : public udsdx::RendererBase
{
public:
	void OnInitialize() override;
	void Update(const udsdx::Time& time, udsdx::Scene& scene) override;
	void PostUpdate(const udsdx::Time& time, udsdx::Scene& scene) override;
	void Render(udsdx::RenderParam& param, int parameter) override;

public:
	void UpdateTransformCache() override;

public:
	void SetDrawCount(unsigned int count) { m_drawCount = count; }
	void SetTexture(udsdx::Texture* texture) { m_texture = texture; }
	void SetColor(const udsdx::Vector3& color) { m_color = color; }
	SphereParticleEmitterParameter& GetEmitterParameter() { return m_emitterParameter; }
	void SetEmitLoop(bool loop) { m_emitLoop = loop; }
	void SetOrientedByDirection(bool oriented) { m_orientedByDirection = oriented; }
	void SetHorizontalBillboard(bool horizontal) { m_horizontalBillboard = horizontal; }
	void SetVerticalBillboard(bool vertical) { m_verticalBillboard = vertical; }
	void SetAutoDestroy(bool autoDestroy) { m_autoDestroy = autoDestroy; }
	void Play();

private:
	static ComPtr<ID3D12PipelineState> m_pipelineState;
	static std::default_random_engine m_randomEngine;
	SphereParticleEmitterParameter m_emitterParameter;
	unsigned int m_seed = 0;
	unsigned int m_drawCount = 1;
	udsdx::Texture* m_texture = nullptr;
	udsdx::Vector3 m_color = udsdx::Vector3(1.0f, 1.0f, 1.0f);
	bool m_emitLoop = true;
	bool m_orientedByDirection = false;
	bool m_horizontalBillboard = false;
	bool m_verticalBillboard = false;
	bool m_isPlaying = false;
	bool m_autoDestroy = false;
};