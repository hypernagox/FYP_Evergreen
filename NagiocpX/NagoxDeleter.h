#pragma once

namespace NagiocpX
{
	class ContentsEntity;

	class NagoxDeleter {
	public:
		virtual void ProcessDestroy(S_ptr<ContentsEntity> entity)noexcept = 0;
	};
}