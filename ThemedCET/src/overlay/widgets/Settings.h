#pragma once

#include "Widget.h"

#include <Options.h>

struct LuaVM;
struct Settings : Widget
{
    Settings(Options& aOptions, LuaVM& aVm);
    ~Settings() override = default;

    WidgetResult OnDisable() override;

    void Load();
    void Save() const;
    void ResetToDefaults();

protected:
    void OnUpdate() override;
    WidgetResult OnPopup() override;

private:
    void UpdateAndDrawSetting(const std::string& acLabel, const std::string& acTooltip, bool& aCurrent, const bool& acSaved);
    void RefreshInterfaceFileLists();

    PatchesSettings m_patches;
    DeveloperSettings m_developer;
    std::string m_theme;
    std::string m_font;
    std::vector<std::string> m_themes;
    std::vector<std::string> m_fonts;
    bool m_interfaceListsLoaded{false};

    Options& m_options;
    LuaVM& m_vm;

    TChangedCBResult m_popupResult{TChangedCBResult::APPLY};
    bool m_madeChanges{false};
    bool m_openChangesModal{true};
};
