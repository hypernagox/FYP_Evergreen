#include "ServerCorePch.h"
#include "Benchmarker.h"

namespace ServerCore
{
	Benchmarker::~Benchmarker() noexcept
	{
		ClearAndGetBenchmarkResult(0);
	}

	void Benchmarker::RecordBenchmark(const std::string_view func_name, const uint64_t dt_) noexcept
	{
		auto& data = m_mapForBenchmark[func_name.data()];
		data.accTime += dt_;
		++data.callCount;
	}

	void Benchmarker::ClearAndGetBenchmarkResult(const uint64_t id_, std::map<std::string, BenchData>* const global_result) const noexcept
	{
		if (global_result)
		{
			std::cout << std::format("Thread {} Func Log: \n", id_);
		}
		else
		{
			if (m_mapForBenchmark.empty())return;
			std::cout << std::format("Entitiy ID {} Func Log: \n", id_);
		}
		for (const auto& [funcName, record] : m_mapForBenchmark)
		{
			if (global_result)
			{
				auto& global_data = global_result->operator[](funcName);
				global_data.accTime += record.accTime;
				global_data.callCount += record.callCount;
			}

			std::cout << std::format("FuncName: {}, CallCount: {}, TotalCallTime: {}ms, Avg: {:.6f} ms\n"
				, funcName, record.callCount, record.accTime, static_cast<double>(record.accTime) / static_cast<double>(record.callCount));
		}
		std::cout << std::format("\n -----ID {} Func Log End----- \n\n\n", id_);
	}

	BenchmarkMgr::~BenchmarkMgr() noexcept
	{
		std::atomic_thread_fence(std::memory_order_seq_cst);

		std::cout << std::endl;

		std::map<std::string, Benchmarker::BenchData> global_bench_data;

		for (int i = 0; i < NUM_OF_THREADS; ++i)
		{
			m_bench_marker[i].ClearAndGetBenchmarkResult(i + 1, &global_bench_data);
			m_bench_marker[i].m_mapForBenchmark.clear();
		}

		std::cout << "Total Result \n\n";

		for (const auto& [funcName, record] : global_bench_data)
		{
			std::cout << std::format("FuncName: {}, CallCount: {}, TotalCallTime: {}ms, Avg: {:.6f} ms\n"
				, funcName, record.callCount, record.accTime, static_cast<double>(record.accTime) / static_cast<double>(record.callCount));
		}
	}

	BenchmarkRAII::~BenchmarkRAII() noexcept
	{
		const auto dt = GetTickCount64() - start_time;
		if (bench_marker)
		{
			bench_marker->RecordBenchmark(func_name, dt);
		}
		else
		{
			std::cout << std::format("Func Name: {} => {} ms \n", func_name, dt);
		}
	}

	[[nodiscard]] BenchmarkRAII StartBenchmarkGlobal(const char* const func_name) noexcept
	{
		return BenchmarkRAII{ func_name,&Mgr(BenchmarkMgr)->m_bench_marker[ServerCore::ThreadMgr::GetCurThreadIdx()] };
	}
}