#pragma once

#include "win32/win32_app.h"
#include "win32/render/win32_software_device.h"
#include "win32/input/win32_input_system.h"

template <typename Intf, typename Impl>
class PlatformFactory
{
    static_assert(std::is_base_of<Intf, Impl>::value, "Impl must inherit Intf");
public:
    template <typename... Args>
    static std::unique_ptr<Intf> create(Args&&... args);
};

///////////////////////////////////////////////////////////////////////////////
// AppFactory
///////////////////////////////////////////////////////////////////////////////
class AppFactory : public
#ifdef WIN32
    PlatformFactory<App, Win32App>
#else
#   error "No platform available for AppFactory"
#endif
{};

///////////////////////////////////////////////////////////////////////////////
// RenderDeviceFactory
///////////////////////////////////////////////////////////////////////////////
class RenderDeviceFactory : public
#ifdef WIN32
    PlatformFactory<RenderDevice, Win32SoftwareDevice>
#else
#   error "No platform available for RenderDeviceFactory"
#endif
{};

///////////////////////////////////////////////////////////////////////////////
// InputDeviceFactory
///////////////////////////////////////////////////////////////////////////////
class MouseDeviceFactory : public
#ifdef WIN32
    PlatformFactory<MouseDevice, Win32MouseDevice>
#else
#   error "No platform available for RenderDeviceFactory"
#endif
{};

class KeyboardDeviceFactory : public
#ifdef WIN32
    PlatformFactory<KeyboardDevice, Win32KeyboardDevice>
#else
#   error "No platform available for RenderDeviceFactory"
#endif
{};

///////////////////////////////////////////////////////////////////////////////
// PlatformFactory impl
///////////////////////////////////////////////////////////////////////////////
namespace detail
{
    template <typename T>
    struct PlatformTypeTraits;

#define DECL_TYPE_NAME(x) \
    template <> struct PlatformTypeTraits<x> \
    { static constexpr char* name = #x; }

    DECL_TYPE_NAME(Win32App);
    DECL_TYPE_NAME(Win32SoftwareDevice);
    DECL_TYPE_NAME(Win32MouseDevice);
    DECL_TYPE_NAME(Win32KeyboardDevice);
#undef DECL_TYPE_NAME

}

template <typename Intf, typename Impl>
template <typename... Args>
inline std::unique_ptr<Intf> PlatformFactory<Intf, Impl>::create(Args&&... args)
{
    dlog("PlatformFactory creating a %s...", detail::PlatformTypeTraits<Impl>::name);
    return std::unique_ptr<Intf>{ new Impl{ std::forward<Args>(args)... } };
}