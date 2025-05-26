#pragma once

#include "exporter_base.h"

class RiggedMeshExporter : public ExporterBase
{
public:
	RiggedMeshExporter();

public:
	void Export(const aiScene& scene, const std::filesystem::path& outputPath) override;
};