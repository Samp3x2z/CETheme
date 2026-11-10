#include <stdafx.h>

#include "D3D12.h"
#include "Options.h"
#include "Utils.h"

#include <CET.h>
#include <imgui_impl/dx12.h>
#include <imgui_impl/win32.h>
#include <window/window.h>

#include <cmath>
#include <limits>


namespace
{
bool ThemeReadFloat(const nlohmann::json& aValues, const char* acName, float& aOut)
{
    const auto it = aValues.find(acName);
    if (it == aValues.end() || !it->is_number())
        return false;

    const auto value = it->get<double>();
    const auto converted = static_cast<float>(value);
    if (!std::isfinite(value) || !std::isfinite(converted))
        return false;

    aOut = converted;
    return true;
}

bool ThemeReadVec2(const nlohmann::json& aValues, const char* acName, ImVec2& aOut)
{
    const auto it = aValues.find(acName);
    if (it == aValues.end() || !it->is_array() || it->size() < 2 ||
        !(*it)[0].is_number() || !(*it)[1].is_number())
        return false;

    const auto x = (*it)[0].get<double>();
    const auto y = (*it)[1].get<double>();
    const auto fx = static_cast<float>(x);
    const auto fy = static_cast<float>(y);
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(fx) || !std::isfinite(fy))
        return false;

    aOut = ImVec2(fx, fy);
    return true;
}

bool ThemeReadBool(const nlohmann::json& aValues, const char* acName, bool& aOut)
{
    const auto it = aValues.find(acName);
    if (it == aValues.end() || !it->is_boolean())
        return false;

    aOut = it->get<bool>();
    return true;
}

bool ThemeReadInt(const nlohmann::json& aValues, const char* acName, int& aOut)
{
    const auto it = aValues.find(acName);
    if (it == aValues.end() || !it->is_number_integer())
        return false;

    const auto value = it->get<int64_t>();
    if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max())
        return false;

    aOut = static_cast<int>(value);
    return true;
}

bool ThemeReadColor(const nlohmann::json& aColors, const char* acName, ImVec4& aOut)
{
    const auto it = aColors.find(acName);
    if (it == aColors.end() || !it->is_array() || it->size() != 4)
        return false;

    float components[4]{};
    for (size_t i = 0; i < 4; ++i)
    {
        if (!(*it)[i].is_number())
            return false;

        const auto value = (*it)[i].get<double>();
        if (!std::isfinite(value) || value < 0.0 || value > 1.0)
            return false;

        components[i] = static_cast<float>(value);
    }

    aOut = ImVec4(components[0], components[1], components[2], components[3]);
    return true;
}

void ThemeApplyJson(ImGuiStyle& aStyle, const nlohmann::json& acTheme)
{
    if (acTheme.contains("colors") && acTheme["colors"].is_object())
    {
        const auto& colors = acTheme["colors"];
        for (int i = 0; i < ImGuiCol_COUNT; ++i)
        {
            const char* colorName = ImGui::GetStyleColorName(static_cast<ImGuiCol>(i));
            ImVec4 color{};
            if (ThemeReadColor(colors, colorName, color))
                aStyle.Colors[i] = color;
        }
    }

    if (!acTheme.contains("style") || !acTheme["style"].is_object())
        return;

    const auto& values = acTheme["style"];
#define CET_THEME_READ_FLOAT(name) { float value{}; if (ThemeReadFloat(values, #name, value)) aStyle.name = value; }
#define CET_THEME_READ_VEC2(name) { ImVec2 value{}; if (ThemeReadVec2(values, #name, value)) aStyle.name = value; }
#define CET_THEME_READ_BOOL(name) { bool value{}; if (ThemeReadBool(values, #name, value)) aStyle.name = value; }
#define CET_THEME_READ_INT(name) { int value{}; if (ThemeReadInt(values, #name, value)) aStyle.name = static_cast<decltype(aStyle.name)>(value); }

#if IMGUI_VERSION_NUM >= 19200
    CET_THEME_READ_FLOAT(FontSizeBase);
    CET_THEME_READ_FLOAT(FontScaleMain);
    CET_THEME_READ_FLOAT(FontScaleDpi);
    CET_THEME_READ_FLOAT(WindowBorderHoverPadding);
    CET_THEME_READ_FLOAT(ScrollbarPadding);
    CET_THEME_READ_FLOAT(ImageRounding);
    CET_THEME_READ_FLOAT(ImageBorderSize);
    CET_THEME_READ_FLOAT(TabMinWidthBase);
    CET_THEME_READ_FLOAT(TabMinWidthShrink);
    CET_THEME_READ_FLOAT(TabCloseButtonMinWidthSelected);
    CET_THEME_READ_FLOAT(TabCloseButtonMinWidthUnselected);
    CET_THEME_READ_FLOAT(TreeLinesSize);
    CET_THEME_READ_FLOAT(TreeLinesRounding);
    CET_THEME_READ_FLOAT(MenuItemRounding);
    CET_THEME_READ_FLOAT(SelectableRounding);
    CET_THEME_READ_FLOAT(DragDropTargetRounding);
    CET_THEME_READ_FLOAT(DragDropTargetBorderSize);
    CET_THEME_READ_FLOAT(DragDropTargetPadding);
    CET_THEME_READ_FLOAT(ColorMarkerSize);
    CET_THEME_READ_FLOAT(InputTextCursorSize);
    CET_THEME_READ_FLOAT(SeparatorSize);
    CET_THEME_READ_BOOL(DockingNodeHasCloseButton);
    CET_THEME_READ_INT(TreeLinesFlags);
#else
    CET_THEME_READ_FLOAT(TabMinWidthForCloseButton);
#endif

    CET_THEME_READ_FLOAT(Alpha);
    CET_THEME_READ_FLOAT(DisabledAlpha);
    CET_THEME_READ_VEC2(WindowPadding);
    CET_THEME_READ_FLOAT(WindowRounding);
    CET_THEME_READ_FLOAT(WindowBorderSize);
    CET_THEME_READ_VEC2(WindowMinSize);
    CET_THEME_READ_VEC2(WindowTitleAlign);
    CET_THEME_READ_INT(WindowMenuButtonPosition);
    CET_THEME_READ_FLOAT(ChildRounding);
    CET_THEME_READ_FLOAT(ChildBorderSize);
    CET_THEME_READ_FLOAT(PopupRounding);
    CET_THEME_READ_FLOAT(PopupBorderSize);
    CET_THEME_READ_VEC2(FramePadding);
    CET_THEME_READ_FLOAT(FrameRounding);
    CET_THEME_READ_FLOAT(FrameBorderSize);
    CET_THEME_READ_VEC2(ItemSpacing);
    CET_THEME_READ_VEC2(ItemInnerSpacing);
    CET_THEME_READ_VEC2(CellPadding);
    CET_THEME_READ_VEC2(TouchExtraPadding);
    CET_THEME_READ_FLOAT(IndentSpacing);
    CET_THEME_READ_FLOAT(ColumnsMinSpacing);
    CET_THEME_READ_FLOAT(ScrollbarSize);
    CET_THEME_READ_FLOAT(ScrollbarRounding);
    CET_THEME_READ_FLOAT(GrabMinSize);
    CET_THEME_READ_FLOAT(GrabRounding);
    CET_THEME_READ_FLOAT(LogSliderDeadzone);
    CET_THEME_READ_FLOAT(TabRounding);
    CET_THEME_READ_FLOAT(TabBorderSize);
    CET_THEME_READ_FLOAT(TabBarBorderSize);
    CET_THEME_READ_FLOAT(TabBarOverlineSize);
    CET_THEME_READ_FLOAT(TableAngledHeadersAngle);
    CET_THEME_READ_VEC2(TableAngledHeadersTextAlign);
    CET_THEME_READ_INT(ColorButtonPosition);
    CET_THEME_READ_VEC2(ButtonTextAlign);
    CET_THEME_READ_VEC2(SelectableTextAlign);
    CET_THEME_READ_FLOAT(SeparatorTextBorderSize);
    CET_THEME_READ_VEC2(SeparatorTextAlign);
    CET_THEME_READ_VEC2(SeparatorTextPadding);
    CET_THEME_READ_VEC2(DisplayWindowPadding);
    CET_THEME_READ_VEC2(DisplaySafeAreaPadding);
    CET_THEME_READ_FLOAT(DockingSeparatorSize);
    CET_THEME_READ_FLOAT(MouseCursorScale);
    CET_THEME_READ_BOOL(AntiAliasedLines);
    CET_THEME_READ_BOOL(AntiAliasedLinesUseTex);
    CET_THEME_READ_BOOL(AntiAliasedFill);
    CET_THEME_READ_FLOAT(CurveTessellationTol);
    CET_THEME_READ_FLOAT(CircleTessellationMaxError);
    CET_THEME_READ_FLOAT(HoverStationaryDelay);
    CET_THEME_READ_FLOAT(HoverDelayShort);
    CET_THEME_READ_FLOAT(HoverDelayNormal);
    CET_THEME_READ_INT(HoverFlagsForTooltipMouse);
    CET_THEME_READ_INT(HoverFlagsForTooltipNav);
#undef CET_THEME_READ_FLOAT
#undef CET_THEME_READ_VEC2
#undef CET_THEME_READ_BOOL
#undef CET_THEME_READ_INT
}

nlohmann::json ThemeStyleToJson(const ImGuiStyle& acStyle)
{
    nlohmann::json style = nlohmann::json::object();
#define CET_THEME_WRITE_FLOAT(name) style[#name] = acStyle.name
#define CET_THEME_WRITE_VEC2(name) style[#name] = nlohmann::json::array({acStyle.name.x, acStyle.name.y})
#define CET_THEME_WRITE_BOOL(name) style[#name] = acStyle.name
#define CET_THEME_WRITE_INT(name) style[#name] = static_cast<int>(acStyle.name)

#if IMGUI_VERSION_NUM >= 19200
    CET_THEME_WRITE_FLOAT(FontSizeBase);
    CET_THEME_WRITE_FLOAT(FontScaleMain);
    CET_THEME_WRITE_FLOAT(FontScaleDpi);
    CET_THEME_WRITE_FLOAT(WindowBorderHoverPadding);
    CET_THEME_WRITE_FLOAT(ScrollbarPadding);
    CET_THEME_WRITE_FLOAT(ImageRounding);
    CET_THEME_WRITE_FLOAT(ImageBorderSize);
    CET_THEME_WRITE_FLOAT(TabMinWidthBase);
    CET_THEME_WRITE_FLOAT(TabMinWidthShrink);
    CET_THEME_WRITE_FLOAT(TabCloseButtonMinWidthSelected);
    CET_THEME_WRITE_FLOAT(TabCloseButtonMinWidthUnselected);
    CET_THEME_WRITE_FLOAT(TreeLinesSize);
    CET_THEME_WRITE_FLOAT(TreeLinesRounding);
    CET_THEME_WRITE_FLOAT(MenuItemRounding);
    CET_THEME_WRITE_FLOAT(SelectableRounding);
    CET_THEME_WRITE_FLOAT(DragDropTargetRounding);
    CET_THEME_WRITE_FLOAT(DragDropTargetBorderSize);
    CET_THEME_WRITE_FLOAT(DragDropTargetPadding);
    CET_THEME_WRITE_FLOAT(ColorMarkerSize);
    CET_THEME_WRITE_FLOAT(InputTextCursorSize);
    CET_THEME_WRITE_FLOAT(SeparatorSize);
    CET_THEME_WRITE_BOOL(DockingNodeHasCloseButton);
    CET_THEME_WRITE_INT(TreeLinesFlags);
#else
    CET_THEME_WRITE_FLOAT(TabMinWidthForCloseButton);
#endif

    CET_THEME_WRITE_FLOAT(Alpha);
    CET_THEME_WRITE_FLOAT(DisabledAlpha);
    CET_THEME_WRITE_VEC2(WindowPadding);
    CET_THEME_WRITE_FLOAT(WindowRounding);
    CET_THEME_WRITE_FLOAT(WindowBorderSize);
    CET_THEME_WRITE_VEC2(WindowMinSize);
    CET_THEME_WRITE_VEC2(WindowTitleAlign);
    CET_THEME_WRITE_INT(WindowMenuButtonPosition);
    CET_THEME_WRITE_FLOAT(ChildRounding);
    CET_THEME_WRITE_FLOAT(ChildBorderSize);
    CET_THEME_WRITE_FLOAT(PopupRounding);
    CET_THEME_WRITE_FLOAT(PopupBorderSize);
    CET_THEME_WRITE_VEC2(FramePadding);
    CET_THEME_WRITE_FLOAT(FrameRounding);
    CET_THEME_WRITE_FLOAT(FrameBorderSize);
    CET_THEME_WRITE_VEC2(ItemSpacing);
    CET_THEME_WRITE_VEC2(ItemInnerSpacing);
    CET_THEME_WRITE_VEC2(CellPadding);
    CET_THEME_WRITE_VEC2(TouchExtraPadding);
    CET_THEME_WRITE_FLOAT(IndentSpacing);
    CET_THEME_WRITE_FLOAT(ColumnsMinSpacing);
    CET_THEME_WRITE_FLOAT(ScrollbarSize);
    CET_THEME_WRITE_FLOAT(ScrollbarRounding);
    CET_THEME_WRITE_FLOAT(GrabMinSize);
    CET_THEME_WRITE_FLOAT(GrabRounding);
    CET_THEME_WRITE_FLOAT(LogSliderDeadzone);
    CET_THEME_WRITE_FLOAT(TabRounding);
    CET_THEME_WRITE_FLOAT(TabBorderSize);
    CET_THEME_WRITE_FLOAT(TabBarBorderSize);
    CET_THEME_WRITE_FLOAT(TabBarOverlineSize);
    CET_THEME_WRITE_FLOAT(TableAngledHeadersAngle);
    CET_THEME_WRITE_VEC2(TableAngledHeadersTextAlign);
    CET_THEME_WRITE_INT(ColorButtonPosition);
    CET_THEME_WRITE_VEC2(ButtonTextAlign);
    CET_THEME_WRITE_VEC2(SelectableTextAlign);
    CET_THEME_WRITE_FLOAT(SeparatorTextBorderSize);
    CET_THEME_WRITE_VEC2(SeparatorTextAlign);
    CET_THEME_WRITE_VEC2(SeparatorTextPadding);
    CET_THEME_WRITE_VEC2(DisplayWindowPadding);
    CET_THEME_WRITE_VEC2(DisplaySafeAreaPadding);
    CET_THEME_WRITE_FLOAT(DockingSeparatorSize);
    CET_THEME_WRITE_FLOAT(MouseCursorScale);
    CET_THEME_WRITE_BOOL(AntiAliasedLines);
    CET_THEME_WRITE_BOOL(AntiAliasedLinesUseTex);
    CET_THEME_WRITE_BOOL(AntiAliasedFill);
    CET_THEME_WRITE_FLOAT(CurveTessellationTol);
    CET_THEME_WRITE_FLOAT(CircleTessellationMaxError);
    CET_THEME_WRITE_FLOAT(HoverStationaryDelay);
    CET_THEME_WRITE_FLOAT(HoverDelayShort);
    CET_THEME_WRITE_FLOAT(HoverDelayNormal);
    CET_THEME_WRITE_INT(HoverFlagsForTooltipMouse);
    CET_THEME_WRITE_INT(HoverFlagsForTooltipNav);
#undef CET_THEME_WRITE_FLOAT
#undef CET_THEME_WRITE_VEC2
#undef CET_THEME_WRITE_BOOL
#undef CET_THEME_WRITE_INT
    return style;
}

nlohmann::json ThemeColorsToJson(const ImGuiStyle& acStyle)
{
    nlohmann::json colors = nlohmann::json::object();
    for (int i = 0; i < ImGuiCol_COUNT; ++i)
    {
        const char* colorName = ImGui::GetStyleColorName(static_cast<ImGuiCol>(i));
        const auto& color = acStyle.Colors[i];
        colors[colorName] = nlohmann::json::array({color.x, color.y, color.z, color.w});
    }
    return colors;
}
} // namespace

bool D3D12::ResetState(const bool acDestroyContext)
{
    if (m_initialized)
    {
        std::lock_guard _(m_imguiLock);

        for (auto& drawData : m_imguiDrawDataBuffers)
        {
            for (auto i = 0; i < drawData.CmdListsCount; ++i)
                IM_DELETE(drawData.CmdLists[i]);
            drawData.Clear();
        }

        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();

        if (acDestroyContext)
            ImGui::DestroyContext();
    }

    m_frameContexts.clear();
    m_outSize = {0, 0};

    m_pd3d12Device.Reset();
    m_pd3dRtvDescHeap.Reset();
    m_pd3dSrvDescHeap.Reset();

    m_pCommandQueue.Reset();
    m_pdxgiSwapChain.Reset();

    m_initialized = false;

    return false;
}

bool D3D12::Initialize()
{
    if (m_initialized)
        return true;

    if (!m_pdxgiSwapChain)
        return false;

    const HWND hWnd = m_window.GetWindow();
    if (!hWnd)
    {
        Log::Warn("D3D12::Initialize() - window not yet hooked!");
        return false;
    }

    if (FAILED(m_pdxgiSwapChain->GetDevice(IID_PPV_ARGS(&m_pd3d12Device))))
    {
        Log::Error("D3D12::Initialize() - failed to get device!");
        return ResetState();
    }

    DXGI_SWAP_CHAIN_DESC sdesc;
    m_pdxgiSwapChain->GetDesc(&sdesc);

    if (hWnd != sdesc.OutputWindow)
        Log::Warn(
            "D3D12::Initialize() - output window of current swap chain does not match hooked window! Currently hooked "
            "to {} while swap chain output window is {}.",
            reinterpret_cast<void*>(hWnd), reinterpret_cast<void*>(sdesc.OutputWindow));

    m_outSize = {static_cast<LONG>(sdesc.BufferDesc.Width), static_cast<LONG>(sdesc.BufferDesc.Height)};

    const auto buffersCounts = std::min(sdesc.BufferCount, 3u);
    m_frameContexts.resize(buffersCounts);

    D3D12_DESCRIPTOR_HEAP_DESC srvdesc = {};
    srvdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvdesc.NumDescriptors = 200; // Same number as is used in scripting/Texture
    srvdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(m_pd3d12Device->CreateDescriptorHeap(&srvdesc, IID_PPV_ARGS(&m_pd3dSrvDescHeap))))
    {
        Log::Error("D3D12::Initialize() - failed to create SRV descriptor heap!");
        return ResetState();
    }

    D3D12_DESCRIPTOR_HEAP_DESC rtvdesc;
    rtvdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvdesc.NumDescriptors = buffersCounts;
    rtvdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvdesc.NodeMask = 1;
    if (FAILED(m_pd3d12Device->CreateDescriptorHeap(&rtvdesc, IID_PPV_ARGS(&m_pd3dRtvDescHeap))))
    {
        Log::Error("D3D12::Initialize() - failed to create RTV descriptor heap!");
        return ResetState();
    }

    const SIZE_T rtvDescriptorSize = m_pd3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < buffersCounts; i++)
    {
        auto& context = m_frameContexts[i];
        context.MainRenderTargetDescriptor = rtvHandle;
        m_pdxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&context.BackBuffer));
        m_pd3d12Device->CreateRenderTargetView(context.BackBuffer.Get(), nullptr, context.MainRenderTargetDescriptor);
        rtvHandle.ptr += rtvDescriptorSize;
    }

    for (auto& context : m_frameContexts)
    {
        if (FAILED(m_pd3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&context.CommandAllocator))))
        {
            Log::Error("D3D12::Initialize() - failed to create command allocator!");
            return ResetState();
        }

        if (FAILED(m_pd3d12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, context.CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&context.CommandList))) ||
            FAILED(context.CommandList->Close()))
        {
            Log::Error("D3D12::Initialize() - failed to create command list!");
            return ResetState();
        }
    }

    if (!InitializeImGui(buffersCounts))
    {
        Log::Error("D3D12::Initialize() - failed to initialize ImGui!");
        return ResetState();
    }

    Log::Info("D3D12::Initialize() - initialization successful!");
    m_initialized = true;

    OnInitialized.Emit();

    return true;
}

void D3D12::ReloadFonts()
{
    std::lock_guard _(m_imguiLock);

    // TODO - scale also by DPI
    const auto [resx, resy] = m_outSize;
    const auto scaleFromReference = std::min(static_cast<float>(resx) / 1920.0f, static_cast<float>(resy) / 1080.0f);

    auto& io = ImGui::GetIO();
    io.Fonts->Clear();

    ImFontConfig config;
    const auto& fontSettings = m_options.Font;
    config.SizePixels = std::floorf(fontSettings.BaseSize * scaleFromReference);
    config.OversampleH = fontSettings.OversampleHorizontal;
    config.OversampleV = fontSettings.OversampleVertical;
    if (config.OversampleH == 1 && config.OversampleV == 1)
        config.PixelSnapH = true;
    config.MergeMode = false;

    // add default font
    const auto customFontPath = fontSettings.Path.empty() ? std::filesystem::path{} : GetAbsolutePath(UTF8ToUTF16(fontSettings.Path), m_paths.Fonts(), false);
    auto cetFontPath = GetAbsolutePath(L"Rajdhani-SemiBold.ttf", m_paths.Fonts(), false);
    const auto* cpGlyphRanges = io.Fonts->GetGlyphRangesDefault();
    if (customFontPath.empty())
    {
        if (!fontSettings.Path.empty())
            Log::Warn("D3D12::ReloadFonts() - Custom font path is invalid! Using default CET font.");

        if (cetFontPath.empty())
        {
            Log::Warn("D3D12::ReloadFonts() - Missing default fonts!");
            io.Fonts->AddFontDefault(&config);
        }
        else
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(cetFontPath.native()).c_str(), config.SizePixels, &config, cpGlyphRanges);
    }
    else
        io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(customFontPath.native()).c_str(), config.SizePixels, &config, cpGlyphRanges);

    if (fontSettings.Language == "ChineseFull")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansTC-Regular.otf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesChineseFull();
    }
    else if (fontSettings.Language == "ChineseSimplifiedCommon")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansSC-Regular.otf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesChineseSimplifiedCommon();
    }
    else if (fontSettings.Language == "Japanese")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansJP-Regular.otf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesJapanese();
    }
    else if (fontSettings.Language == "Korean")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansKR-Regular.otf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesKorean();
    }
    else if (fontSettings.Language == "Cyrillic")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSans-Regular.ttf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesCyrillic();
    }
    else if (fontSettings.Language == "Thai")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansThai-Regular.ttf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesThai();
    }
    else if (fontSettings.Language == "Vietnamese")
    {
        cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSans-Regular.ttf", m_paths.Fonts(), false);
        cpGlyphRanges = io.Fonts->GetGlyphRangesVietnamese();
    }
    else
    {
        switch (GetSystemDefaultLangID())
        {
        case MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_TRADITIONAL):
            cetFontPath = GetAbsolutePath(L"NotoSansTC-Regular.otf", m_paths.Fonts(), false);
            cpGlyphRanges = io.Fonts->GetGlyphRangesChineseFull();
            break;

        case MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED):
            cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansSC-Regular.otf", m_paths.Fonts(), false);
            cpGlyphRanges = io.Fonts->GetGlyphRangesChineseSimplifiedCommon();
            break;

        case MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT):
            cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansJP-Regular.otf", m_paths.Fonts(), false);
            cpGlyphRanges = io.Fonts->GetGlyphRangesJapanese();
            break;

        case MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT):
            cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansKR-Regular.otf", m_paths.Fonts(), false);
            cpGlyphRanges = io.Fonts->GetGlyphRangesKorean();
            break;

        case MAKELANGID(LANG_BELARUSIAN, SUBLANG_DEFAULT):
        case MAKELANGID(LANG_RUSSIAN, SUBLANG_DEFAULT):
            cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSans-Regular.ttf", m_paths.Fonts(), false);
            cpGlyphRanges = io.Fonts->GetGlyphRangesCyrillic();
            break;

        case MAKELANGID(LANG_THAI, SUBLANG_DEFAULT):
            cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSansThai-Regular.ttf", m_paths.Fonts(), false);
            cpGlyphRanges = io.Fonts->GetGlyphRangesThai();
            break;

        case MAKELANGID(LANG_VIETNAMESE, SUBLANG_DEFAULT):
            cetFontPath = GetAbsolutePath(m_paths.Fonts() / L"NotoSans-Regular.ttf", m_paths.Fonts(), false);
            cpGlyphRanges = io.Fonts->GetGlyphRangesVietnamese();
            break;
        }
    }

    // add extra glyphs from language font
    config.MergeMode = true;
    if (customFontPath.empty())
    {
        if (!fontSettings.Path.empty())
            Log::Warn("D3D12::ReloadFonts() - Custom font path is invalid! Using default CET font.");

        if (cetFontPath.empty())
        {
            Log::Warn("D3D12::ReloadFonts() - Missing fonts for extra language glyphs!");
            io.Fonts->AddFontDefault(&config);
        }
        else
            io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(cetFontPath.native()).c_str(), config.SizePixels, &config, cpGlyphRanges);
    }
    else
        io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(customFontPath.native()).c_str(), config.SizePixels, &config, cpGlyphRanges);

    // add icons from fontawesome4
    config.GlyphMinAdvanceX = config.SizePixels;
    static const ImWchar icon_ranges[] = {ICON_MIN_MD, ICON_MAX_MD, 0};
    auto cetIconPath = GetAbsolutePath(L"materialdesignicons.ttf", m_paths.Fonts(), false);
    io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(cetIconPath.native()).c_str(), config.SizePixels, &config, icon_ranges);
}


void D3D12::ReloadTheme(const bool aForce)
{
    if (ImGui::GetCurrentContext() == nullptr)
        return;

    std::lock_guard _(m_imguiLock);

    const auto themeName = m_options.Theme.Name.empty() ? std::string("Dark.json") : m_options.Theme.Name;
    if (!aForce && themeName == m_activeTheme)
        return;

    ImGuiStyle style{};
    nlohmann::json theme;
    auto loadTheme = [this](const std::string& aName, nlohmann::json& aTheme) -> bool
    {
        const auto path = m_paths.Themes() / UTF8ToUTF16(aName);
        std::ifstream input(path);
        if (!input)
            return false;

        aTheme = nlohmann::json::parse(input, nullptr, false);
        return !aTheme.is_discarded() && aTheme.is_object();
    };

    bool loaded = loadTheme(themeName, theme);
    if (!loaded && themeName != "Dark.json")
    {
        Log::Warn("D3D12::ReloadTheme() - Theme '{}' is missing or invalid; trying Dark.json.", themeName);
        loaded = loadTheme("Dark.json", theme);
    }

    if (loaded)
    {
        ThemeApplyJson(style, theme);
        Log::Info("D3D12::ReloadTheme() - Loaded JSON theme '{}'.", themeName);
    }
    else
    {
        Log::Error("D3D12::ReloadTheme() - No valid theme JSON could be loaded; retaining Dear ImGui constructor defaults.");
    }

    m_styleReference = style;
    const auto [resx, resy] = m_outSize;
    const auto scale = std::min(static_cast<float>(resx) / 1920.0f, static_cast<float>(resy) / 1080.0f);
    ImGui::GetStyle() = m_styleReference;
    ImGui::GetStyle().ScaleAllSizes(scale);
    m_activeTheme = themeName;
}

bool D3D12::SaveTheme(const std::string& aThemeName)
{
    if (ImGui::GetCurrentContext() == nullptr || aThemeName.empty())
        return false;

    std::lock_guard _(m_imguiLock);

    const std::filesystem::path fileName = UTF8ToUTF16(aThemeName);
    if (fileName.filename() != fileName || fileName.extension() != L".json")
    {
        Log::Error("D3D12::SaveTheme() - Refusing invalid theme filename '{}'.", aThemeName);
        return false;
    }

    const auto themePath = m_paths.Themes() / fileName;
    std::ifstream input(themePath);
    if (!input)
    {
        Log::Error("D3D12::SaveTheme() - Could not open theme '{}' for saving.", aThemeName);
        return false;
    }

    auto theme = nlohmann::json::parse(input, nullptr, false);
    if (theme.is_discarded() || !theme.is_object())
    {
        Log::Error("D3D12::SaveTheme() - Theme '{}' contains invalid JSON; refusing to overwrite it.", aThemeName);
        return false;
    }
    input.close();

    // Save the active in-memory style when this is the currently applied theme.
    // Otherwise normalize the selected JSON itself without changing ImGui mid-frame.
    ImGuiStyle styleToSave{};
    if (aThemeName == m_activeTheme)
        styleToSave = m_styleReference;
    else
        ThemeApplyJson(styleToSave, theme);

    theme.erase("preset");
    theme["colors"] = ThemeColorsToJson(styleToSave);

    nlohmann::json savedStyle = theme.contains("style") && theme["style"].is_object()
        ? theme["style"]
        : nlohmann::json::object();
    const auto currentStyle = ThemeStyleToJson(styleToSave);
    for (auto it = currentStyle.begin(); it != currentStyle.end(); ++it)
        savedStyle[it.key()] = it.value();
    theme["style"] = std::move(savedStyle);

    std::ofstream output(themePath, std::ios::trunc);
    if (!output)
    {
        Log::Error("D3D12::SaveTheme() - Could not write theme '{}'.", aThemeName);
        return false;
    }

    output << theme.dump(4) << std::endl;
    if (!output.good())
    {
        Log::Error("D3D12::SaveTheme() - Failed while writing theme '{}'.", aThemeName);
        return false;
    }

    Log::Info("D3D12::SaveTheme() - Saved complete colors and style values to '{}'.", aThemeName);
    return true;
}

bool D3D12::InitializeImGui(size_t aBuffersCounts)
{
    std::lock_guard _(m_imguiLock);

    // TODO - scale also by DPI
    const auto [resx, resy] = m_outSize;
    const auto scaleFromReference = std::min(static_cast<float>(resx) / 1920.0f, static_cast<float>(resy) / 1080.0f);

    if (ImGui::GetCurrentContext() == nullptr)
    {
        // do this once, do not repeat context creation!
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

    }

    m_activeTheme.clear();
    ReloadTheme(true);

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    if (!ImGui_ImplWin32_Init(m_window.GetWindow()))
    {
        Log::Error("D3D12::InitializeImGui() - ImGui_ImplWin32_Init call failed!");
        return false;
    }

    if (!ImGui_ImplDX12_Init(
            m_pd3d12Device.Get(), static_cast<int>(aBuffersCounts), DXGI_FORMAT_R8G8B8A8_UNORM, m_pd3dSrvDescHeap.Get(), m_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
            m_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart()))
    {
        Log::Error("D3D12::InitializeImGui() - ImGui_ImplDX12_Init call failed!");
        ImGui_ImplWin32_Shutdown();
        return false;
    }

    ReloadFonts();

    if (!ImGui_ImplDX12_CreateDeviceObjects(m_pCommandQueue.Get()))
    {
        Log::Error("D3D12::InitializeImGui() - ImGui_ImplDX12_CreateDeviceObjects call failed!");
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        return false;
    }

    return true;
}

void D3D12::PrepareUpdate()
{
    if (!m_initialized)
        return;

    std::lock_guard _(m_imguiLock);

    ReloadTheme();
    ImGui_ImplWin32_NewFrame(m_outSize);
    ImGui::NewFrame();

    CET::Get().GetOverlay().Update();

    CET::Get().GetVM().Draw();

    ImGui::Render();

    auto& drawData = m_imguiDrawDataBuffers[2];

    for (auto i = 0; i < drawData.CmdListsCount; ++i)
        IM_DELETE(drawData.CmdLists[i]);
    drawData.Clear();

    drawData = *ImGui::GetDrawData();

    ImVector<ImDrawList*> copiedDrawLists;
    copiedDrawLists.resize(drawData.CmdListsCount);

    for (auto i = 0; i < drawData.CmdListsCount; ++i)
        copiedDrawLists[i] = drawData.CmdLists[i]->CloneOutput();
    drawData.CmdLists = std::move(copiedDrawLists);

    std::swap(m_imguiDrawDataBuffers[1], m_imguiDrawDataBuffers[2]);
}

void D3D12::Update()
{
    // swap staging ImGui buffer with render ImGui buffer
    {
        std::lock_guard _(m_imguiLock);
        ImGui_ImplDX12_NewFrame(m_pCommandQueue.Get());
        if (m_imguiDrawDataBuffers[1].Valid)
        {
            std::swap(m_imguiDrawDataBuffers[0], m_imguiDrawDataBuffers[1]);
            m_imguiDrawDataBuffers[1].Valid = false;
        }
    }

    if (!m_imguiDrawDataBuffers[0].Valid)
        return;

    assert(m_pdxgiSwapChain);
    auto& frameContext = m_frameContexts[m_pdxgiSwapChain->GetCurrentBackBufferIndex()];
    frameContext.CommandAllocator->Reset();

    D3D12_RESOURCE_BARRIER barrier;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = frameContext.BackBuffer.Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    ID3D12DescriptorHeap* heaps[] = {m_pd3dSrvDescHeap.Get()};

    frameContext.CommandList->Reset(frameContext.CommandAllocator.Get(), nullptr);
    frameContext.CommandList->ResourceBarrier(1, &barrier);
    frameContext.CommandList->SetDescriptorHeaps(1, heaps);
    frameContext.CommandList->OMSetRenderTargets(1, &frameContext.MainRenderTargetDescriptor, FALSE, nullptr);

    ImGui_ImplDX12_RenderDrawData(&m_imguiDrawDataBuffers[0], frameContext.CommandList.Get());

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    frameContext.CommandList->ResourceBarrier(1, &barrier);
    frameContext.CommandList->Close();

    ID3D12CommandList* commandLists[] = {frameContext.CommandList.Get()};
    m_pCommandQueue->ExecuteCommandLists(1, commandLists);
}
