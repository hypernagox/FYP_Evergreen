#pragma once
#include "Singleton.hpp"

namespace NagiocpX
{
	class Benchmarker
	{
	public:
		friend class BenchmarkMgr;
		friend class BenchmarkRAII;
		Benchmarker()noexcept = default;
		~Benchmarker()noexcept;
		struct BenchData {
			uint64_t accTime = 0;
			uint64_t callCount = 0;
		};
		void ClearAndGetBenchmarkResult(const uint64_t id_, const class ContentsEntity* const entity = nullptr, std::map<std::string, BenchData>* const global_result = nullptr)const noexcept;
	private:
		void RecordBenchmark(const std::string_view func_name, const uint64_t dt_)noexcept;
	private:
		std::map<std::string, BenchData> m_mapForBenchmark;
	};

	class BenchmarkMgr
		:public Singleton<BenchmarkMgr>
	{
		friend class Singleton;
		friend class BenchmarkRAII;
		BenchmarkMgr()noexcept = default;
		~BenchmarkMgr()noexcept;
		friend BenchmarkRAII StartBenchmarkGlobal(const char* const func_name)noexcept;
	private:
		Benchmarker m_bench_marker[NagiocpX::NUM_OF_THREADS];
	};

	class BenchmarkRAII
	{
	public:
		BenchmarkRAII(const char* const funcName_, Benchmarker* const bench_marker_ptr = nullptr)noexcept
			:bench_marker{ bench_marker_ptr }, start_time{ ::GetTickCount64() }, func_name{ funcName_ } {}
		BenchmarkRAII(const BenchmarkRAII&) = delete;
		BenchmarkRAII& operator=(const BenchmarkRAII&) = delete;
		BenchmarkRAII(BenchmarkRAII&&)noexcept = delete;
		BenchmarkRAII& operator=(BenchmarkRAII&&)noexcept = delete;
		~BenchmarkRAII()noexcept;
	private:
		const uint64_t start_time = ::GetTickCount64();
		const char* const func_name;
		Benchmarker* const bench_marker = nullptr;
	};

	[[nodiscard]] BenchmarkRAII StartBenchmarkGlobal(const char* const func_name)noexcept;
	[[nodiscard]] inline BenchmarkRAII StartBenchmark(const char* const func_name, Benchmarker* const bench_ptr)noexcept { return BenchmarkRAII{ func_name,bench_ptr }; }

	#define ON_RECORD_BENCHMARK

	#ifdef ON_RECORD_BENCHMARK
	#define DO_BENCH_GLOBAL(funcName)		const auto BENCH_RAII_GLOBAL_TEMP = NagiocpX::StartBenchmarkGlobal(funcName)
	#define DO_BENCH(funcName, bench_ptr)	const auto BENCH_RAII_TEMP = NagiocpX::StartBenchmark(funcName, bench_ptr)
	#define DO_BENCH_GLOBAL_THIS_FUNC		DO_BENCH_GLOBAL(__FUNCTION__)
	#define DO_BENCH_THIS_FUNC(bench_ptr)	DO_BENCH(__FUNCTION__, bench_ptr)
	#define DO_BENCH_THIS_FUNC_NOW			DO_BENCH(__FUNCTION__, nullptr)
	#else
	#define DO_BENCH_GLOBAL(funcName)
	#define DO_BENCH(funcName, bench_ptr)
	#define DO_BENCH_GLOBAL_THIS_FUNC
	#define DO_BENCH_THIS_FUNC(bench_ptr)
	#define DO_BENCH_THIS_FUNC_NOW
	#endif
}

