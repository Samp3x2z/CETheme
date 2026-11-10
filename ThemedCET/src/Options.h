#pragma once

#include "Image.h"

struct Paths;

struct PatchesSettings
{
    void Load(const nlohmann::json& aConfig);
    nlohmann::json Save() const;
    void ResetToDefaults();

    [[nodiscard]] auto operator<=>(const PatchesSettings&) const = default;

    bool AsyncCompute{false};
    bool Antialiasing{false};
    bool DisableVignette{false};
    bool DisableBoundaryTeleport{false};
    bool DisableWin7Vsync{false};
};

struct FontSettings
{
    void Load(const nlohmann::json& aConfig);
    nlohmann::json Save() const;
    void ResetToDefaults();

    [[nodiscard]] auto operator<=>(const FontSettings&) const = default;

    std::string Path{};
    std::string Language{"Default"};
    float BaseSize{18.0f};
    int32_t OversampleHorizontal{3};
    int32_t OversampleVertical{1};
};

struct ThemeSettings
{
    void Load(const nlohmann::json& aConfig);
    nlohmann::json Save() const;
    void ResetToDefaults();
    [[nodiscard]] auto operator<=>(const ThemeSettings&) const = default;
    std::string Name{"Dark.json"};
};

struct DeveloperSettings
{
    void Load(const nlohmann::json& aConfig);
    nlohmann::json Save() const;
    void ResetToDefaults();

    [[nodiscard]] auto operator<=>(const DeveloperSettings&) const = default;

    bool RemoveDeadBindings{true};
    bool EnableImGuiAssertions{false};
    bool DumpGameOptions{false};
    uint64_t MaxLinesConsoleHistory{1000};
    bool PersistentConsole{true};
    bool EnableJIT{true};
};

struct Options
{
    Options(Paths& aPaths);
    ~Options() = default;

    void Load();
    void Save() const;
    void ResetToDefaults();

    Image GameImage;
    bool ExeValid{false};

    PatchesSettings Patches{};
    FontSettings Font{};
    ThemeSettings Theme{};
    DeveloperSettings Developer{};

private:
    Paths& m_paths;
};
