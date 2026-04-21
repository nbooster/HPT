#ifndef _HPT_
#define _HPT_
#pragma once

/*
High Precision Timer using only standard C++ 20, and x86 intrinsics.
*/

#ifndef TURN_OFF_MEASUREMENTS

#include <set>
#include <cmath>
#include <string>
#include <iostream>
#include <unordered_map>
#include <x86gprintrin.h>

#endif

namespace HPT
{

#ifndef TURN_OFF_MEASUREMENTS

using TimerHMapType = std::unordered_map<std::string, std::unordered_map<size_t, size_t>>;

[[gnu::hot, gnu::always_inline]] inline uint64_t cycle_start(void)
{
    /*_mm_lfence();

    const auto tsc { __rdtsc() };

    _mm_lfence();

    return tsc;*/

	uint32_t lo, hi;

    asm volatile
    (
        "lfence\n\t"
        "rdtsc\n\t"
        "lfence\n\t"
        : "=a"(lo), "=d"(hi)
    );

    return (static_cast<uint64_t>(hi) << 32) bitor lo;
}

[[gnu::hot, gnu::always_inline]] inline uint64_t cycle_end(void)
{
    /*unsigned int aux;

    const auto tsc { __rdtscp(std::addressof(aux)) };

    _mm_lfence();

    return tsc;*/

	uint32_t lo, hi;
    
    asm volatile
    (
    	"rdtscp\n\t"
		"mov %%eax, %[lo]\n\t"
		"mov %%edx, %[hi]\n\t"
		"cpuid\n\t"
		: [lo] "=r" (lo), [hi] "=r" (hi)
		:
		: "%rax", "%rbx", "%rcx", "%rdx"
	);

    return (static_cast<uint64_t>(hi) << 32) bitor lo;
}

template<class T>
requires std::is_arithmetic_v<T> 
[[gnu::cold]] inline std::string formatWithCommasLocal(const T& value, const uint32_t& decimals = 2)
{
    auto strOriginal { std::to_string( value < 0 ? -value : value ) };

    const auto periodPos { strOriginal.find('.') };

    auto str { strOriginal.substr(0, periodPos) };

    auto strDecimal { periodPos not_eq std::string::npos ? strOriginal.substr(periodPos + 1, decimals) : std::string() };

    std::string sign { value < 0 ? "-" : "" };

    std::string result;

    size_t index { 1 };

    for ( auto rit { str.rbegin() }; rit not_eq str.rend(); ++rit, ++index )
    {
        result.insert(result.cbegin(), *rit);

        if ( index % 3 == 0 )
            result.insert(result.cbegin(), ',');
    }

    if ( *(result.cbegin()) == ',' )
        result = result.substr(1);
    
    result = sign + result + std::string(".") + strDecimal;

    return result.back() == '.' ? result.substr(0, result.size() - 1) : result ;
}

[[gnu::cold]] inline std::string cyclesToNanosString(const double& cycles, const size_t cpuSpeedInMGHz)
{
	if ( cpuSpeedInMGHz == 0 )
		return "";

	return std::string("( ") + formatWithCommasLocal((cycles * 1000) / static_cast<double>(cpuSpeedInMGHz)) + std::string(" nanos )");
}

[[gnu::cold]] inline void printStatsFromFreqTable(const std::string& name, std::unordered_map<size_t, size_t>& freqTable, const size_t cpuSpeedInMGHz, const size_t zeroCodeCycles, std::ostream& os)
{
	if ( freqTable.empty() )
		return;

	os << "\033[" << "1;4;33" << "m" << "\n" << name << "\033[m" << "\n\n";

	size_t totalCycles { 0 }, totalCalls { 0 }, callsSoFar { 0 };

	double devSum { 0 };

    std::set<std::decay_t<decltype(freqTable)>::value_type> sortedTimeEntries;

    for ( const auto& pair : freqTable )
    {
        totalCycles += static_cast<size_t>(pair.first - zeroCodeCycles) * pair.second;

        totalCalls += pair.second;

        sortedTimeEntries.emplace(pair);
    }

    const auto avg { static_cast<double>(totalCycles) / static_cast<double>(totalCalls) };

    const auto medianPosition { (totalCalls + 1) / 2 };

    const size_t nintyPosition { static_cast<size_t>(std::ceil(0.9 * static_cast<double>(totalCalls)) ) };

    const size_t nintyninePosition { static_cast<size_t>(std::ceil(0.99 * static_cast<double>(totalCalls)) ) };

    bool printMedian { true }, print90th { true }, print99th { true };

    const auto minResult { sortedTimeEntries.empty() ? 0 : sortedTimeEntries.cbegin()->first - zeroCodeCycles };

    os << "Min: " << formatWithCommasLocal(minResult) << " cycles " << cyclesToNanosString(static_cast<double>(minResult), cpuSpeedInMGHz) << "\n";

    for ( const auto& [nanos, calls] : sortedTimeEntries )
    {
        callsSoFar += calls;

        devSum += static_cast<double>(calls) * (static_cast<double>(nanos - zeroCodeCycles) - avg) * (static_cast<double>(nanos - zeroCodeCycles) - avg);

        if ( printMedian and callsSoFar >= medianPosition )
        {
        	const auto medianResult { nanos - zeroCodeCycles };

            os << "\033[" << "1;32" << "m" << "Median: " << formatWithCommasLocal(medianResult) << " cycles " << cyclesToNanosString(static_cast<double>(medianResult), cpuSpeedInMGHz) << "\033[m" << "\n";

            printMedian = false;
        }

        if ( print90th and callsSoFar >= nintyPosition )
        {
        	const auto nintyPercentileResult { nanos - zeroCodeCycles };

            os << "90th Percentile: " << formatWithCommasLocal(nintyPercentileResult) << " cycles " << cyclesToNanosString(static_cast<double>(nintyPercentileResult), cpuSpeedInMGHz) << "\n";

            print90th = false;
        }

        if ( print99th and callsSoFar >= nintyninePosition )
        {
        	const auto nintyNinePercentileResult { nanos - zeroCodeCycles };

            os << "99th Percentile: " << formatWithCommasLocal(nintyNinePercentileResult) << " cycles " << cyclesToNanosString(static_cast<double>(nintyNinePercentileResult), cpuSpeedInMGHz) << "\n";

            print99th = false;
        }
    }

    const auto maxResult { sortedTimeEntries.empty() ? 0 : sortedTimeEntries.crbegin()->first - zeroCodeCycles };

    os << "Max: " << formatWithCommasLocal(maxResult) << " cycles " << cyclesToNanosString(static_cast<double>(maxResult), cpuSpeedInMGHz) << "\n";

    os << "Avg: " << formatWithCommasLocal(avg) << " cycles " << cyclesToNanosString(avg, cpuSpeedInMGHz) << "\n";

    const auto stdDevResult { std::sqrt(devSum / static_cast<double>(totalCalls)) };

    os << "Std Dev: " << formatWithCommasLocal(stdDevResult) << cyclesToNanosString(stdDevResult, cpuSpeedInMGHz) << "\n";

    os << "Total: " << formatWithCommasLocal(totalCycles) << " cycles " << cyclesToNanosString(static_cast<double>(totalCycles), cpuSpeedInMGHz) << "\n";

    os << "Total Calls: " << formatWithCommasLocal(totalCalls) << "\n";

    os << "Total Distinct Values: " << formatWithCommasLocal(freqTable.size()) << "\n";

    os << std::endl;
}

#endif

class Timer final
{
	#ifndef TURN_OFF_MEASUREMENTS

	static std::unordered_map<std::string, std::unordered_map<size_t, size_t>> FreqTables;

	std::string name;

	bool timerStopped;

	size_t timeStampStart;

	#endif

public:

	[[gnu::hot, gnu::always_inline]] inline explicit Timer(const std::string& name = "Generic") noexcept

	#ifndef TURN_OFF_MEASUREMENTS

	:name{ name }, timerStopped{ false }, timeStampStart{ cycle_start() }

	#endif
	{
		#ifndef TURN_OFF_MEASUREMENTS

		[[maybe_unused]] static const bool _ = [this]
		{
			#if defined(__GNUC__) or defined(__GNUG__)

			#pragma GCC diagnostic push
			#pragma GCC diagnostic ignored "-Wvolatile"

			#endif

			// Warm up the CPU just in case...

			for ( volatile int i = 0; i < 1'000'000; ++i );

			#if defined(__GNUC__) or defined(__GNUG__)

			#pragma GCC diagnostic pop

			#endif

			this->timeStampStart = cycle_start();

			return false; 
		}();

		#endif
	}

	[[gnu::hot, gnu::always_inline]] inline ~Timer()
	{
		#ifndef TURN_OFF_MEASUREMENTS

		const auto timeStampEnd { cycle_end() };

		[[maybe_unused]] static const bool _ = []
		{
			Timer::ClearAllMeasurements();

			return false; 
		}();

		if ( not this->timerStopped ) [[likely]]
			++(Timer::FreqTables[this->name][timeStampEnd - this->timeStampStart]);

		#endif
	}

	[[gnu::hot, gnu::always_inline]] inline void start(void) noexcept
	{
		#ifndef TURN_OFF_MEASUREMENTS

		this->timerStopped = false;

		this->timeStampStart = cycle_start();

		#endif
	}

	[[gnu::hot, gnu::always_inline]] inline void stopAndRecord(void)
	{
		#ifndef TURN_OFF_MEASUREMENTS

		const auto timeStampEnd { cycle_end() };

		if ( not this->timerStopped ) [[likely]]
			++(Timer::FreqTables[this->name][timeStampEnd - this->timeStampStart]);

		this->timerStopped = true;

		#endif
	}

	[[gnu::cold]] static void ClearMeasurements(const std::string& name = "Generic")
	{
		#ifndef TURN_OFF_MEASUREMENTS

		Timer::FreqTables[name].clear();

		#endif
	}

	[[gnu::cold]] static void ClearAllMeasurements(void)
	{
		#ifndef TURN_OFF_MEASUREMENTS

		for ( auto& [name, freqTable] : Timer::FreqTables )
			freqTable.clear();

		#endif
	}

	[[gnu::cold]] static void PrintResults(const size_t cpuSpeedInMGHz = 0, const size_t zeroCodeCycles = 0, std::ostream& os = std::cout, const bool printNotes = true)
	{
		#ifndef TURN_OFF_MEASUREMENTS

		for ( auto& [name, freqTable] : Timer::FreqTables )
		{
			printStatsFromFreqTable(name, freqTable, cpuSpeedInMGHz, zeroCodeCycles, os);

			freqTable.clear();
		}

		[[maybe_unused]] static const bool _ = [&os, printNotes]
		{
			if ( not printNotes )
				return false;

			os << 
			"\n\n*To get the measurements also in nanoseconds,\n please provide the positive 'cpuSpeedInMGHz' argument in 'PrintResults' (assuming CPU has invariant TSC support).\n"
			"\n\n**For more accurate results: "
			"\n  do not perform nested measurements,"
			"\n  measure only from one thread,"
			"\n  provide the 'zeroCodeCycles' (measurement of an empty code block) argument in 'PrintResults',"
			"\n  measure as few code blocks as possible at the same time,"
			"\n  keep the measured block names as short as possible (SSO),"
			"\n  disable hyperthreading,"
			"\n  make sure the measuring thread was pinned to an isolated CPU core throughout the measuring period,"
			"\n  with the highest priority,"
			"\n  and keep running your benchmarks on the same core for consistency.\n\n";

			return false; 
		}();

		#endif
	}
};

inline TimerHMapType Timer::FreqTables;

}; // namespace HPT

#endif
