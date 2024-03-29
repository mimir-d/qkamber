#pragma once

#ifdef WIN32
    #include "platform/win32/win32_app.h"
    #include "platform/win32/render/win32_software_device.h"
    #include "platform/win32/input/win32_input_device.h"
#else
    #include "platform/sdl/sdl_app.h"
    #include "platform/sdl/render/sdl_software_device.h"
    #include "platform/sdl/input/sdl_input_device.h"
#endif

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
#ifdef WIN32
    class AppFactory : public PlatformFactory<App, Win32App> {};
#else
    using AppFactory = PlatformFactory<App, SdlApp>;
#endif

///////////////////////////////////////////////////////////////////////////////
// RenderDeviceFactory
///////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
    class RenderDeviceFactory : public PlatformFactory<RenderDevice, Win32SoftwareDevice> {};
#else
    using RenderDeviceFactory = PlatformFactory<RenderDevice, SdlSoftwareDevice>;
#endif

///////////////////////////////////////////////////////////////////////////////
// InputDeviceFactory
///////////////////////////////////////////////////////////////////////////////
#ifdef WIN32
    class MouseDeviceFactory : public PlatformFactory<MouseDevice, Win32MouseDevice> {};
#else
    using MouseDeviceFactory = PlatformFactory<MouseDevice, SdlMouseDevice>;
#endif

#ifdef WIN32
    class KeyboardDeviceFactory : public PlatformFactory<KeyboardDevice, Win32KeyboardDevice> {};
#else
    using KeyboardDeviceFactory = PlatformFactory<KeyboardDevice, SdlKeyboardDevice>;
#endif

///////////////////////////////////////////////////////////////////////////////
// PlatformFactory impl
///////////////////////////////////////////////////////////////////////////////
namespace detail
{
    template <typename T>
    struct PlatformTypeTraits;

#define DECL_TYPE_NAME(x) \
    template <> struct PlatformTypeTraits<x> \
    { static constexpr char name[] = #x; }

#ifdef WIN32
    DECL_TYPE_NAME(Win32App);
    DECL_TYPE_NAME(Win32SoftwareDevice);
    DECL_TYPE_NAME(Win32MouseDevice);
    DECL_TYPE_NAME(Win32KeyboardDevice);
#else
    DECL_TYPE_NAME(SdlApp);
    DECL_TYPE_NAME(SdlSoftwareDevice);
    DECL_TYPE_NAME(SdlMouseDevice);
    DECL_TYPE_NAME(SdlKeyboardDevice);
#endif
#undef DECL_TYPE_NAME

}

template <typename Intf, typename Impl>
template <typename... Args>
inline std::unique_ptr<Intf> PlatformFactory<Intf, Impl>::create(Args&&... args)
{
    dlog("PlatformFactory creating a %s...", detail::PlatformTypeTraits<Impl>::name);
    return std::unique_ptr<Intf>{ new Impl{ std::forward<Args>(args)... } };
}