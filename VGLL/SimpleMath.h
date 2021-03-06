// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#include "SimpleMath.g.h"

namespace winrt::VGLL::implementation
{
    struct SimpleMath: SimpleMathT<SimpleMath>
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
    struct SimpleMath: SimpleMathT<SimpleMath, implementation::SimpleMath>
    {
    };
}
