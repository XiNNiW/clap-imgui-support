#include "clap/ext/timer-support.h"
#include "imgui.h"
#include <cstddef>
#include <imgui-clap-support/imgui-clap-editor.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include "GLFW/glfw3.h"
#include "imgui.h"

#include "imgui_internal.h" // so we can get the viewport associated with an ImGui window
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
namespace ClapSupport
{
#define TIMER_MS 30
static void glfw_error_callback(int error, const char *description)
{
    fprintf(stderr, "GLFW error %d: %s\n", error, description);
}

struct GLFWContext
{
    ImGuiContext *_imguiContext = nullptr;
    imgui_clap_editor *_editor = nullptr;
    GLFWwindow *_windowHandle = nullptr;
    const clap_host *_clap_host = nullptr;
    clap_id timer_id = 0;
    bool valid = false;
    // clap_plugin_timer_support gui__timer_support;
    GLFWContext(imgui_clap_editor *e, clap_xwnd win, const clap_host *host)
        : _editor(e), _clap_host(host)
    {
        valid = _initialize(e, win);
    }
    bool resizeWindow(int width, int height) { return false; }
    void beforeDelete()
    {

        // TODO: implement beforeDelete
        if (!_windowHandle)
            return;

        ImGui::SetCurrentContext(_imguiContext);

        deleteTimer();

        ImGui::DestroyContext();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();

        glfwDestroyWindow(_windowHandle);
        _windowHandle = nullptr;
        glfwTerminate();

        ImGui::DestroyContext();
        _imguiContext = nullptr;
    }
    bool createTimer()
    {

        unsigned int ms = TIMER_MS;

        clap_host_timer_support *timer_support =
            (clap_host_timer_support *)_clap_host->get_extension(_clap_host,
                                                                 CLAP_EXT_TIMER_SUPPORT);
        return timer_support && timer_support->register_timer(_clap_host, ms, &timer_id);
    }

  private:
    void onTimer() {}
    void deleteTimer()
    {

        clap_host_timer_support *timer_support =
            (clap_host_timer_support *)_clap_host->get_extension(_clap_host,
                                                                 CLAP_EXT_TIMER_SUPPORT);
        if (timer_support)
            timer_support->unregister_timer(_clap_host, timer_id);
        timer_id = 0;
    }
    bool _initialize(imgui_clap_editor *e, clap_xwnd win)
    {

        IMGUI_CHECKVERSION();

        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            return false;

        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        // register window class
        //  _windowClass = {sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L,   0L,
        //                   GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
        //                   _T("ImGui-CLAP-DX12-WindowClass"), nullptr};
        //  ::RegisterClassEx(&_windowClass);

        // get client rect, windows specific??
        //    RECT clientRect;
        //    ::GetClientRect(windowHandleParent, &clientRect);

        // create window
        //     _windowHandle = ::CreateWindow(_windowClass.lpszClassName, _T("ImGui Clap Saw Demo"),
        //         WS_CHILD | WS_VISIBLE, 0, 0, clientRect.right - clientRect.left,
        //         clientRect.bottom - clientRect.top, windowHandleParent, NULL,
        //                                   _windowClass.hInstance, nullptr);
        //     ::SetWindowLongPtr(_windowHandle, GWLP_USERDATA, (LONG_PTR)this);
        // invisible top level window
        _windowHandle = glfwCreateWindow(1, 1, "ImGui Backend", NULL, NULL);
        if (!_windowHandle)
            return false;
        glfwMakeContextCurrent(_windowHandle);
        glfwSwapInterval(1);

        // set parent
        //     ::SetParent(_windowHandle, windowHandleParent);

        // update window
        //    ::UpdateWindow(_windowHandle);

        // set focus
        //     ::SetFocus(_windowHandle);

        _imguiContext = ImGui::CreateContext();
        ImGui::SetCurrentContext(_imguiContext);

        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        // io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // TODO: investigate—perhaps different versions of imgui between projects?
        //  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        //  io.ConfigViewportsNoAutoMerge = true;
        //  io.ConfigViewportsNoTaskBarIcon = true;
        //  io.ConfigViewportsNoDefaultParent = true;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        // ImGui::StyleColorsLight();

        // no idea... various initialization things??
        // globals.retain();
        // createHeapDescriptors();
        // createCommandListAndQueue();
        // createFences();
        // createSwapChain(_windowHandle);
        // createRenderTargets();

        // Setup Platform/Renderer backends

        ImGui_ImplGlfw_InitForOpenGL(_windowHandle, true);
        ImGui_ImplOpenGL3_Init(NULL);
        //    ImGui_ImplWin32_Init(_windowHandle);
        //    ImGui_ImplDX12_Init(globals._device, DX12Globals::numOfFramesInFlight,
        //                        DXGI_FORMAT_R8G8B8A8_UNORM, _srvDescHeap,
        //                        _srvDescHeap->GetCPUDescriptorHandleForHeapStart(),
        //                        _srvDescHeap->GetGPUDescriptorHandleForHeapStart());

        if (!createTimer())
            return false;

        return true;
    }
};

}; // namespace ClapSupport

bool imgui_clap_guiCreateWith(imgui_clap_editor *e, const clap_host *host)
{
    IMGUI_CHECKVERSION();
    e->onGuiCreate();
    return true;
}
void imgui_clap_guiDestroyWith(imgui_clap_editor *e, const clap_host *host)
{
    e->onGuiDestroy();
    auto context = (ClapSupport::GLFWContext *)e->ctx;
    context->beforeDelete();
    delete context;
    e->ctx = nullptr;
}
bool imgui_clap_guiSetParentWith(imgui_clap_editor *e, const clap_window *win,
                                 const clap_host *host)
{
    e->ctx = (void *)new ClapSupport::GLFWContext(e, win->x11, host);
    return true;
}
bool imgui_clap_guiSetSizeWith(imgui_clap_editor *e, int width, int height)
{
    if (auto context = (ClapSupport::GLFWContext *)e->ctx)
    {
        return context->resizeWindow(width, height);
    }

    return false;
}
