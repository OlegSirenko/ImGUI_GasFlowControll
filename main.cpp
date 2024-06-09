// Dear ImGui: standalone example application for DirectX 9

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "winsock2.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <iostream>
#include <vector>
#include <chrono>
#include "implot.h"
#include "ControlPanel.h"
#include "PlotWindow.h"
#include "mainMenu.h"
#include "PID.h"
#include <cmath>
#include <deque>
#include "resources/bruceFontEmbedded_utf_8.cpp"
#include "resources/ExoFontEmbedded_utf8.cpp"
#include "ServerModule.h"
#include <thread>


// Data
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();



//void handle_events(bool&, SDL_Window*);
void update_plot_windows(std::shared_ptr<tcp_server>& server,
                         std::unordered_map<tcp_connection::pointer, std::unique_ptr<PlotWindow>>& plotWindowsMap,
                         int window_width, int window_height, int window_position_x,
                         int window_position_y, bool attach_window);

void render_windows(std::shared_ptr<tcp_server>& server, std::unordered_map<tcp_connection::pointer, std::unique_ptr<PlotWindow>>& plotWindowsMap,  long current_time);
void embraceTheDarkness();

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);



// Main code
int main(int, char**)
{
    // Create application window
    //ImGui_ImplWin32_EnableDpiAwareness();

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr),
                       static_cast<HICON>(LoadImage(nullptr, _T("GasFlowControlIcon.ico"), IMAGE_ICON, 0, 0,
                                                    LR_DEFAULTSIZE | LR_LOADFROMFILE)),
                       nullptr, nullptr, nullptr, L"Monitoring for Gas FLow control system",
                       static_cast<HICON>(LoadImage(nullptr, _T("GasFlowControlIcon.ico"), IMAGE_ICON, 0, 0,
                                                    LR_DEFAULTSIZE | LR_LOADFROMFILE)) };
    ::RegisterClassExW(&wc);


    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Monitoring for Gas FLow control system", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, nullptr, nullptr, wc.hInstance, nullptr);


    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }


    // Show the window
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext(); //
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    io.ConfigViewportsNoTaskBarIcon = true;
    //io.ConfigViewportsNoDefaultParent = true;
    //io.ConfigDockingAlwaysTabBar = true;
    //io.ConfigDockingTransparentPayload = true;
    //io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports; // FIXME-DPI: Experimental.

    // Setup Dear ImGui style
    embraceTheDarkness();
    ImGui::StyleColorsLight();


    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 2.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    io.Fonts->AddFontFromMemoryCompressedTTF(ExoFont_compressed_data, ExoFont_compressed_size, 17);

    // Our state
    bool show_demo_window = false;
    bool show_another_window = false;
    bool show_plot_window = true;
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.90f);


    // Main loop
    bool done = false;
    bool connection_emitted = false;

    int window_height, window_width, window_position_x, window_position_y;

    auto start = std::chrono::system_clock::now();

    double setpoint = 0.0;

    // Init server context and thread
    auto io_context = std::make_shared<boost::asio::io_context>();
    auto server = std::make_shared<tcp_server>(*io_context);



    std::unique_ptr<ControlPanel> controlPanel = std::make_unique<ControlPanel>(window_width, window_height, window_position_x, window_position_y);

    std::unordered_map<tcp_connection::pointer, std::unique_ptr<PlotWindow>> plotWindowsMap;

    std::vector<std::string> logs = {
            "Application opened successfully",
            "Logging started...",
    };


    // Start server
    std::thread server_thread([&io_context, &logs] {
//        std::cout<<"Waiting for data... "<<std::endl;
        logs.emplace_back("Server started");
//        logs.emplace_back(); // ip address of server.
        io_context->run();
        logs.emplace_back("Server closed");
    });

    while (!done){
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        auto now = std::chrono::system_clock::now();  // Calculate the time elapsed since the start of the application in seconds
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - start);
        auto current_time = elapsed.count();

        mainMenu::Render();

        connection_emitted = server->get_connections_count() > 0;
        controlPanel->num_connections = server->get_connections_count();
        if(connection_emitted){
            update_plot_windows(server, plotWindowsMap, window_width, window_height, window_position_x, window_position_y, true);
            // Render each PlotWindow
            auto connections = server->get_connections();

            render_windows(server, plotWindowsMap, current_time);
        }
        controlPanel->Render(connection_emitted, logs);


        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        g_pSwapChain->Present(1, 0); // Present with vsync
    }

    // Cleanup
    io_context->stop();
    server_thread.join();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions
bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
        case WM_SIZE:
            if (wParam == SIZE_MINIMIZED)
                return 0;
            g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
            g_ResizeHeight = (UINT)HIWORD(lParam);
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
        case WM_DPICHANGED:
            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
            {
                //const int dpi = HIWORD(wParam);
                //printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
                const RECT* suggested_rect = (RECT*)lParam;
                ::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
            }
            break;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}


void update_plot_windows(std::shared_ptr<tcp_server>& server, std::unordered_map<tcp_connection::pointer, std::unique_ptr<PlotWindow>>& plotWindowsMap, int window_width, int window_height, int window_position_x, int window_position_y, bool attach_window) {
    auto connections = server->get_connections();
    // Remove any PlotWindows that don't have an associated connection
    for (auto it = plotWindowsMap.begin(); it != plotWindowsMap.end(); ) {
        if (std::find(connections.begin(), connections.end(), it->first) == connections.end()) {
            it = plotWindowsMap.erase(it);
        } else {
            ++it;
        }
    }

    // Add a PlotWindow for any new connections
    for (const auto& connection : connections) {
        if (plotWindowsMap.find(connection) == plotWindowsMap.end()) {
            plotWindowsMap[connection] = std::make_unique<PlotWindow>(window_width, window_height, window_position_x, window_position_y, attach_window);
        }
    }
}


void render_windows(std::shared_ptr<tcp_server>& server, std::unordered_map<tcp_connection::pointer, std::unique_ptr<PlotWindow>>& plotWindowsMap,  long current_time) {
    auto connections = server->get_connections();
    for (auto & connection : connections) {
        auto& plotWindow = plotWindowsMap[connection];
        std::string data(connection->get_latest_data());
        if(!data.empty()){
            double current_data_from_connection = std::stod(data);
            std::string plot_window_name = connection->get_ip() + ":" + std::to_string(connection->get_port());
            plotWindow->Render(current_time, current_data_from_connection, plot_window_name);
            std::cout<<"PID Value: "<<plotWindow->pid.GetSteerValue()<<std::endl;
        }
    }
}