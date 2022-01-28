#pragma once
#include "SimpleMath.g.h"

// Note: Remove this static_assert after copying these generated source files to your project.
// This assertion exists to avoid compiling these generated source files directly.
static_assert(false, "Do not compile generated C++/WinRT source files directly");

namespace winrt::VGLL::implementation
{
    struct SimpleMath : SimpleMathT<SimpleMath>
    {
        SimpleMath() = default;

        double add(double firstNumber, double secondNumber);
        double subtract(double firstNumber, double secondNumber);
        double multiply(double firstNumber, double secondNumber);
        double divide(double firstNumber, double secondNumber);
    };
}
namespace winrt::VGLL::factory_implementation
{
    struct SimpleMath : SimpleMathT<SimpleMath, implementation::SimpleMath>
    {
    };
}
