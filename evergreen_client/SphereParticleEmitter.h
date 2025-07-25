#pragma once

#include "pch.h"

struct SphereParticleEmitterParameter
{
	float ElapsedTime = 0.0f;
	float RotationMin = 0.0f;
	float RotationMax = 0.0f;
	float RotationLifeExp = 0.0f;
	float LifeTimeMin = 0.0f;
	float LifeTimeMax = 0.0f;
	udsdx::Vector2 SizeMin = udsdx::Vector2(0.0f, 0.0f);
	udsdx::Vector2 SizeMax = udsdx::Vector2(1.0f, 1.0f);
	udsdx::Vector2 SizeLifeExp = udsdx::Vector2(0.0f, 0.0f);
	float AlphaLifeExp = 0.0f;
	float SpeedMin = 0.0f;
	float SpeedMax = 0.0f;
	float SpeedLifeExp = 1.0f;
};

class SphereParticleEmitter : public udsdx::RendererBase
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
	bool m_isPlaying = false;
	bool m_autoDestroy = false;
};