#ifndef GALLIUM__GPU__COMPUTEPIPELINE_H
#define GALLIUM__GPU__COMPUTEPIPELINE_H
#pragma once

#include <gallium/gpu/pipeline.h>

#include <memory>

namespace ga::gpu
{
	class Device;

	struct ComputePipelineInfo
	{
		PipelineStageDesc stage;
	};

	class ComputePipeline
	{
		struct Impl;
		std::unique_ptr<Impl> m_pImpl;

	public:
		ComputePipeline(Device& device, const ComputePipelineInfo& info);
		~ComputePipeline();

		const Impl& GetImpl() const;
	};
}

#endif /* GALLIUM__GPU__COMPUTEPIPELINE_H */
