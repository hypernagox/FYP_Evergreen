#include "ServerCorePch.h"
#include "Benchmarker.h"
#include "TimeMgr.h"
#include "ThreadMgr.h"

namespace ServerCore
{
	constinit static std::ofstream* log_stream = nullptr;

	static void RecordLogToStream(const std::string_view log_str)noexcept
	{
		std::cout << log_str;
		if (log_stream)
			*log_stream << log_str;
	}

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

	void Benchmarker::ClearAndGetBenchmarkResult(const uint64_t id_, const class ContentsEntity* const entity, std::map<std::string, BenchData>* const global_result) const noexcept
	{
		
		if (global_result)
		{
			RecordLogToStream(std::format("Thread {} Func Log: \n", id_));
		}
		else
		{
			if (m_mapForBenchmark.empty())return;
			if (entity)std::cout << std::format("Entitiy ID {}, Group {}, Type {} Func Log: \n", id_, entity->GetPrimaryGroupType(), entity->GetDetailType());
			else std::cout << std::format("Entitiy ID {} Func Log: \n", id_);
		}
		for (const auto& [funcName, record] : m_mapForBenchmark)
		{
			if (global_result)
			{
				auto& global_data = global_result->operator[](funcName);
				global_data.accTime += record.accTime;
				global_data.callCount += record.callCount;
			}

			RecordLogToStream(std::format("FuncName: {}, CallCount: {}, TotalCallTime: {}ms, Avg: {:.6f} ms\n"
				, funcName, record.callCount, record.accTime, static_cast<double>(record.accTime) / static_cast<double>(record.callCount)));
		}
		RecordLogToStream(std::format("\n -----ID {} Func Log End----- \n\n\n", id_));
	}

	BenchmarkMgr::~BenchmarkMgr() noexcept
	{
		std::atomic_thread_fence(std::memory_order_seq_cst);

		log_stream = new std::ofstream{ "..\\Func_Time_Benchmark.txt" };

		RecordLogToStream("\n");

		std::map<std::string, Benchmarker::BenchData> global_bench_data;

		for (int i = 0; i < NUM_OF_THREADS; ++i)
		{
			m_bench_marker[i].ClearAndGetBenchmarkResult(i + 1, nullptr, &global_bench_data);
			m_bench_marker[i].m_mapForBenchmark.clear();
		}

		RecordLogToStream("Total Result \n\n");
		
		uint64_t total_func_time = 0;

		for (const auto& [funcName, record] : global_bench_data)
		{
			total_func_time += record.accTime;
			RecordLogToStream(std::format("FuncName: {}, CallCount: {}, TotalCallTime: {}ms, Avg: {:.6f} ms\n"
				, funcName, record.callCount, record.accTime, static_cast<double>(record.accTime) / static_cast<double>(record.callCount)));
		}

		RecordLogToStream(std::format("\n\nServer Running Time: {:.2f} seconds, Total Func Acc Time: {:.2f} seconds / {} ms\n\n", Mgr(TimeMgr)->GetServerTimeStamp() / 1000., total_func_time / 1000., total_func_time));

		RecordLogToStream("\n\nTotal Percentage\n\n");
		
		if (0 == total_func_time) {
			RecordLogToStream("Total Func Time is Zero\n");
			return;
		}

		for (const auto& [funcName, record] : global_bench_data)
		{
			RecordLogToStream(std::format("FuncName: {}, {:.2f}% \n"
				, funcName, ((float)record.accTime / total_func_time) * 100.f));
		}

		delete log_stream;
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