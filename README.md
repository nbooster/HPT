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

## Simplest Example Usage
```cpp
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
