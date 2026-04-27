# HPT
Header-only High Precision Timer lib for effortless, on the fly, execution time measurements of code blocks, using only standard C++ 20, and x86 intrinsics.

## Why use it ?
1. **Cycle level precision measurements** ( define _LOWER_PRECISION_MIN_OVERHEAD_ before including the library to get that behaviour ( tiny difference) ).
2. **Independent of underlying CPU clock speed** ( because the measurements are in cycles ).
3. **Option to display the results also in nanos** ( if the user provides the actual CPU speed clock on the running core ).
4. **Effortless usage** ( just add: _HPT::Timer T("Code Block Name");_ at the start of any code block ).
5. **Ultra light** ( negligible overhead of time ( like 70 - 74 cycles avg on a modern CPU ) and space ).
6. **Standalone and header only** ( only a C++ 20 supporting compiler needed without any extra dependencies ).
7. **Full stat results** ( totals, min, median, max, avg, std, 90th, 99th ).
8. **Option to stop and restart the timer inside the code block, and also reset the stats.**
9. **Easily turn off the measurements by simply defining the _TURN_OFF_MEASUREMENTS_ constant before including the library.**
10. **Can handle an enormous amount of measurements / calls** ( it uses frequency tables to store them ).

## Use case
This lib is best suited for accurate and consistent measuring production code with real world data.
It is NOT a benchmarking lib (although it can be used for that too).
The process is simple:
  1. Measure a specific code block in production with real world data, using the proper setup ( see Notes ).
  2. Make a code change in that block.
  3. Do step 1 again.
  4. See if that code change actually improved the stats you care about.
  5. If so, then keep that code change in production ( with a similar setup ) with this ( or similar ) data.

## Integration
Just copy the single header file ( _HPT.hpp_ ) in your include folder. Then include it in your code.

## API
```cpp
explicit Timer(const std::string& name = "Generic") noexcept;
void start(void) noexcept;
void stopAndRecord(void);
void setName(const std::string& name);
const std::string& getName(void) const noexcept;
static void clearMeasurements(const std::string& name = "Generic");
static void clearAllMeasurements(void);
static void setStatHighlight(const Stats& stat);
static void resetStatHighlight(const Stats& stat);
static void clearStatHighlights(void);
static void printResults(const size_t cpuSpeedInMGHz = 0, const size_t zeroCodeCycles = 0, std::ostream& os = std::cout);
static void printNotes(std::ostream& os = std::cout) noexcept;
```

## Simplest Example Usage
```cpp
// #define TURN_OFF_MEASUREMENTS

#include "HPT.hpp"

void FunctionToMeasure(size_t N)
{
  HPT::Timer T("FunctionToMeasure");

  // any code here...
}

int main(void)
{
  for ( size_t index{}; index < 1'000'000; ++index )
    FunctionToMeasure(index);

  HPT::PrintResults();

  std::exit(EXIT_SUCCESS);
}
```

## Sampe Output (Main Functions Of A Limit Order Book With Real World Data)
![Project logo](/measurements_.jpg)

## Notes
1. To get the measurements also in nanoseconds, please provide the positive _'cpuSpeedInMGHz'_ argument in _'PrintResults'_ (assuming CPU has invariant TSC support).
2. Make sure 'PrintResults' is called only once, after all the measured code blocks have executed, to avoid conflicts.
3. To find the actual real time speed in MHz, of all CPU cores, run this on Linux:

   _watch -n.1 "grep \"^[c]pu MHz\" /proc/cpuinfo"_
4. For more accurate results:
   1. Do not perform nested measurements.
   2. Measure only from one thread.
   3. Provide the _'zeroCodeCycles'_ (measurement of an empty code block) argument in _'PrintResults'_.
   4. Measure as few code blocks as possible at the same time.
   5. Keep the measured block names as short as possible (SSO).
   6. Disable hyperthreading.
   7. Disable turbo boost and force the _'performance'_ governor.
   8. Make sure the measuring thread has the highest priority and is pinned to an isolated CPU core throughout the measuring period.
   9. Keep running your benchmarks on the same core for consistency.
