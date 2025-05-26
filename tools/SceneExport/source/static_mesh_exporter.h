#pragma once

#include "exporter_base.h"

class StaticMeshExporter : public ExporterBase
{
public:
	StaticMeshExporter();

public:
	void Export(const aiScene& scene, const std::filesystem::path& outputPath) override;
};