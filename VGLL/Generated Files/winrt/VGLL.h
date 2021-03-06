// WARNING: Please don't edit this file. It was generated by C++/WinRT v2.0.210806.1

#pragma once
#ifndef WINRT_VGLL_H
#define WINRT_VGLL_H
#include "winrt/base.h"
static_assert(winrt::check_version(CPPWINRT_VERSION, "2.0.210806.1"), "Mismatched C++/WinRT headers.");
#define CPPWINRT_VERSION "2.0.210806.1"
#include "winrt/impl/VGLL.2.h"
namespace winrt::impl
{
    template <typename D> WINRT_IMPL_AUTO(double) consume_VGLL_ISimpleMath<D>::add(double firstNumber, double secondNumber) const
    {
        double result{};
        check_hresult(WINRT_IMPL_SHIM(winrt::VGLL::ISimpleMath)->add(firstNumber, secondNumber, &result));
        return result;
    }
    template <typename D> WINRT_IMPL_AUTO(double) consume_VGLL_ISimpleMath<D>::subtract(double firstNumber, double secondNumber) const
    {
        double result{};
        check_hresult(WINRT_IMPL_SHIM(winrt::VGLL::ISimpleMath)->subtract(firstNumber, secondNumber, &result));
        return result;
    }
    template <typename D> WINRT_IMPL_AUTO(double) consume_VGLL_ISimpleMath<D>::multiply(double firstNumber, double secondNumber) const
    {
        double result{};
        check_hresult(WINRT_IMPL_SHIM(winrt::VGLL::ISimpleMath)->multiply(firstNumber, secondNumber, &result));
        return result;
    }
    template <typename D> WINRT_IMPL_AUTO(double) consume_VGLL_ISimpleMath<D>::divide(double firstNumber, double secondNumber) const
    {
        double result{};
        check_hresult(WINRT_IMPL_SHIM(winrt::VGLL::ISimpleMath)->divide(firstNumber, secondNumber, &result));
        return result;
    }
    template <typename D>
    struct produce<D, winrt::VGLL::ISimpleMath> : produce_base<D, winrt::VGLL::ISimpleMath>
    {
        int32_t __stdcall add(double firstNumber, double secondNumber, double* result) noexcept final try
        {
            typename D::abi_guard guard(this->shim());
            *result = detach_from<double>(this->shim().add(firstNumber, secondNumber));
            return 0;
        }
        catch (...) { return to_hresult(); }
        int32_t __stdcall subtract(double firstNumber, double secondNumber, double* result) noexcept final try
        {
            typename D::abi_guard guard(this->shim());
            *result = detach_from<double>(this->shim().subtract(firstNumber, secondNumber));
            return 0;
        }
        catch (...) { return to_hresult(); }
        int32_t __stdcall multiply(double firstNumber, double secondNumber, double* result) noexcept final try
        {
            typename D::abi_guard guard(this->shim());
            *result = detach_from<double>(this->shim().multiply(firstNumber, secondNumber));
            return 0;
        }
        catch (...) { return to_hresult(); }
        int32_t __stdcall divide(double firstNumber, double secondNumber, double* result) noexcept final try
        {
            typename D::abi_guard guard(this->shim());
            *result = detach_from<double>(this->shim().divide(firstNumber, secondNumber));
            return 0;
        }
        catch (...) { return to_hresult(); }
    };
}
WINRT_EXPORT namespace winrt::VGLL
{
}
namespace std
{
#ifndef WINRT_LEAN_AND_MEAN
    template<> struct hash<winrt::VGLL::ISimpleMath> : winrt::impl::hash_base {};
    template<> struct hash<winrt::VGLL::SimpleMath> : winrt::impl::hash_base {};
#endif
}
#endif
