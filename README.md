# HPT
Header Only High Precision Timer Lib for effortless, on the fly, execution time measurements of code blocks, using only standard C++ 20, and x86 intrinsics.

## Why use it ?
1. Ultra precise measurements ( cycle level precision ).
2. Independent of underlying Hardware ( because the measurements are in cycles ).
3. Option to display the results also in nanos ( if the user provides the actual CPU speed clock on the running core ).
4. Effortless usage ( just add: HPT::Timer T("Code Block Name"); at the start of any code block ).
5. Ultra light ( negligible overhead of time and space ).
6. Standalone and header only ( only a C++ 20 supporting compiler needed without any extra dependencies ).
7. Full stat results ( totals, min, median, max, avg, std, 90th, 99th ).
8. Option to stop and restart the timer inside the code block, and also reset the stats.
9. Easily turn off the measurements by simply defining the TURN_OFF_MEASUREMENTS constant before including the library.
10. Can handle an enormous amount of measurements / calls ( it uses frequency tables to store them ).

## API
```cpp
explicit Timer(const std::string& name = "Generic") noexcept;
void start(void) noexcept;
void stopAndRecord(void);
static void ClearMeasurements(const std::string& name = "Generic");
static void ClearAllMeasurements(void);
static void PrintResults(const size_t cpuSpeedInMGHz = 0, const size_t zeroCodeCycles = 0, std::ostream& os = std::cout, const bool printNotes = true);
```

## Simplest Example Usage
```cpp
// #define TURN_OFF_MEASUREMENTS

#include "HPT.hpp"

void DummyFunction(size_t N)
{
  HPT::Timer T("DummyFunction");

  // any code here...
}

int main(void)
{
  for ( size_t index{}; index < 1'000'000; ++index )
    DummyFunction(index);

  HPT::PrintResults();

  std::exit(EXIT_SUCCESS);
}
```

## Notes
1. To get the measurements also in nanoseconds, please provide the positive 'cpuSpeedInMGHz' argument in 'PrintResults' (assuming CPU has invariant TSC support).
2. For more accurate results:
   1. Do not perform nested measurements.
   2. Measure only from one thread.
   3. Provide the 'zeroCodeCycles' (measurement of an empty code block) argument in 'PrintResults'.
   4. Measure as few code blocks as possible at the same time.
   5. Keep the measured block names as short as possible (SSO).
   6. Disable hyperthreading.
   7. Disable turbo boost and force the 'performance' governor.
   8. Make sure the measuring thread has the highest priority and is pinned to an isolated CPU core throughout the measuring period.
   9. Keep running your benchmarks on the same core for consistency.
