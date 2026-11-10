# Themed Cyber Engine Tweaks

A themed fork of [Cyber Engine Tweaks](https://github.com/maximegmd/CyberEngineTweaks) for Cyberpunk 2077.

The active project lives in `ThemedCET/`. `Original/` is retained as an upstream reference snapshot and is not part of the build. Do not edit `Original/`.

## Features

- **JSON-driven Dear ImGui themes:** theme colors and supported style properties—such as rounding, borders, spacing, and alignment—are stored in JSON files.
- **Automatic theme discovery:** every valid `.json` theme in the runtime `themes/` directory is detected automatically. Adding a theme does not require a C++ preset or source change.
- **Theme reload and save:** the current colors and supported style properties can be saved to the selected theme JSON.
- **Custom fonts:** `.ttf` and `.otf` files placed in the runtime `fonts/` directory are discovered by the font selector.
- **Persistent settings:** the selected theme and font are stored in CET's `config.json`. Font changes take effect after restarting the game.
- **Windows build and packaging scripts:** build and stage an install-ready package from the repository root.

## Repository layout

```text
Themed_CyberEngineTweaks/
├── ThemedCET/                 # Only active CET source/project tree
│   ├── src/
│   ├── themes/                # JSON themes and theme format documentation
│   ├── fonts/
│   ├── scripts/
│   ├── vendor/
│   │   ├── asiloader/
│   │   └── RED4ext.SDK/       # Pinned official Git submodule
│   └── xmake.lua
├── Original/                  # Upstream reference snapshot; do not edit
├── .gitmodules
├── build-local.ps1
├── build-local.bat
├── build/                     # Generated install-ready output (not committed)
└── README.md
```

## Requirements

Build on Windows x64 with:

- Git for Windows.
- Xmake 3.1 or newer.
- Visual Studio 2022 or compatible Build Tools, including the **Desktop development with C++** workload and a Windows SDK.
- Internet access on the first build to initialize the RED4ext SDK submodule and download Xmake dependencies/package assets.

The official RED4ext SDK is pinned to `5246e659d783e5834cc08a90e56c8932c9ed3449` at `ThemedCET/vendor/RED4ext.SDK`. The build script initializes it automatically.

To initialize it manually:

```powershell
git submodule sync --recursive
git submodule update --init --recursive -- ThemedCET/vendor/RED4ext.SDK
```

## Build an install-ready package

From the repository root, double-click `build-local.bat` or run:

```powershell
.\build-local.ps1
```

The script configures Xmake for Release, rebuilds the ImGui dependency for the current checkout path, compiles CET, runs the package hook, validates the output, and stages the result under `build/bin/`. It waits for Enter before closing, including after a failure. A failed build preserves the previous successful `build/` folder.

Expected install-ready structure:

```text
build/
└── bin/
    └── x64/
        ├── version.dll
        └── plugins/
            ├── cyber_engine_tweaks.asi
            ├── cyber_engine_tweaks.pdb
            └── cyber_engine_tweaks/
                ├── fonts/
                ├── scripts/
                ├── themes/
                ├── tweakdb/
                └── ThirdParty_LICENSES
```

Copy the **contents of `build/bin/`** into the Cyberpunk 2077 game directory, merging with existing folders. Do not copy the source tree itself.

## Themes

At runtime, themes are stored in:

```text
Cyberpunk 2077/bin/x64/plugins/cyber_engine_tweaks/themes/
```

The Settings menu discovers every `.json` file in that folder. The repository includes these presets:

- `Dark.json`
- `Classic.json`
- `Light.json`
- `DarkRed.json`

`Dark.json` is used as the fallback when the saved selection is missing or invalid.

### Theme JSON structure

Theme files contain two main data sections:

- `colors`: Dear ImGui color entries stored as RGBA arrays.
- `style`: supported Dear ImGui style properties, including values such as padding, spacing, rounding, and borders.

The engine validates the expected value types and ranges before applying theme data. Saving a theme writes the current colors and supported style properties back to the selected JSON file.

**Implementation note:** only the Dear ImGui style properties explicitly supported by the current implementation are loaded and serialized. This does not imply automatic support for every ImGui property, including future additions.

To add a theme, place another valid JSON file in the runtime `themes/` directory. No C++ preset is required. See [ThemedCET/themes/README.md](ThemedCET/themes/README.md) for the schema and examples.

## Custom fonts

Place `.ttf` or `.otf` files in:

```text
Cyberpunk 2077/bin/x64/plugins/cyber_engine_tweaks/fonts/
```

The font selector discovers these files automatically. Restart the game after changing the selected font.

## Configuration and troubleshooting

CET stores the selected theme and font in:

```text
Cyberpunk 2077/bin/x64/plugins/cyber_engine_tweaks/config.json
```

Theme appearance is stored in each theme JSON, not in C++ presets. Keep the `cyber_engine_tweaks/themes` and `fonts` folders when updating the plugin.

If the build fails, inspect the first compiler or packaging error above the final failure message. Xmake logs are available under `%LOCALAPPDATA%\\.xmake`.

---

# Changes from the upstream reference

This section documents the differences visible in the patch comparing `Original/` with `ThemedCET/`. The patch was supplied on **November 10, 2026**.

The supplied patch is approximately **28.3 MB** and contains **20,968 diff sections**. It is not a clean source-only diff: it also includes Xmake caches, build files, a packaging directory, and the RED4ext SDK contents. Generated files and dependencies are distinguished from the CET source changes below.

The line numbers shown are the **hunk ranges on the ThemedCET side**. They include context retained by Git around changed lines; not every line in a listed range was necessarily modified.

> **Scope limitation:** this comparison covers only `Original/` and `ThemedCET/`. It does not verify repository-root changes such as `build-local.ps1`, `build-local.bat`, this root README, or the root `.gitmodules` file.

## Change summary

The main functional changes add:

1. a Dear ImGui theme engine driven by JSON files;
2. theme reloading and saving for supported colors and style properties;
3. an **Interface** section in CET settings, with theme and font selection;
4. persistence of the selected theme and font in CET options;
5. runtime `themes/` and `fonts/` directory handling;
6. theme copying during Xmake packaging and installation;
7. additional compatibility/runtime adaptations and fork configuration changes.

## 1. Theme engine and interface

| File | ThemedCET line ranges | Description |
|---|---:|---|
| `src/d3d12/D3D12_Functions.cpp` | 9–317, 466–472, 586–708, 717–726, 759–765 | Adds JSON loading for ImGui colors and style properties, value validation, fallback to `Dark.json`, color/style serialization, and theme reload/save methods. The style is applied during initialization and can then be reloaded. |
| `src/d3d12/D3D12.h` | 15–22, 74–80 | Declares `ReloadTheme()` and `SaveTheme()`, and stores the active theme name. |
| `src/overlay/widgets/Settings.cpp` | 6–13, 109–179, 195–216, 219–226 | Adds the **Interface** section, detects theme JSON files and TTF/OTF fonts, displays the selectors, synchronizes selections with the options, and triggers theme saving. |
| `src/overlay/widgets/Settings.h` | 25–32 | Adds members required to track the selected theme and font. |
| `themes/Classic.json` | 1–506 | New ImGui Classic preset. |
| `themes/DarkRed.json` | 1–506 | New DarkRed preset. |
| `themes/Dark.json` | 1–506 | New Dark preset, used as the fallback theme. |
| `themes/Light.json` | 1–506 | New Light preset. |
| `themes/README.md` | 1–58 | Documents the JSON format and how to add themes. |

### JSON theme system

- The engine reads colors from the `colors` property and supported style properties from `style`.
- Colors are mapped to Dear ImGui names and stored as RGBA arrays.
- The code validates expected types and values before applying the data.
- If the selected theme is missing or invalid, the engine attempts to load `Dark.json`.
- Saving writes the current colors and supported style properties back to the selected JSON file.
- The interface discovers `.json` files in the runtime directory, so adding a valid preset does not require a new C++ preset.

**Important:** the code serializes only the Dear ImGui properties explicitly supported by its implementation. The patch does not establish that every existing or future ImGui property is automatically supported.

## 2. Options, paths, and fonts

| File | ThemedCET line ranges | Description |
|---|---:|---|
| `src/Options.h` | 34–48, 73–79 | Adds `ThemeSettings`, which stores the selected theme name and provides Load/Save/Reset methods; adds this settings block to `Options`. |
| `src/Options.cpp` | 47–60, 113–122, 125–131, 136–142 | Implements reading, writing, and resetting the selected theme in the CET configuration. |
| `src/Paths.h` | 13–19, 30–35 | Declares access to the themes path and adds the corresponding path member. |
| `src/Paths.cpp` | 45–52, 91–99 | Exposes the `Themes()` path and creates the runtime themes directory, alongside the fonts directory. |
| `src/d3d12/D3D12_Functions.cpp` | 466–472 | Changes the default font from `NotoSans-Regular.ttf` to `Rajdhani-SemiBold.ttf`. |
| `CetColorVariables.txt` | 1–60 | New reference file for CET color variables. |

The selected theme is persisted in the options under the `theme` key. Font selection is presented in the same settings section; available TTF and OTF files are detected in the fonts directory.

## 3. Additional source changes

These changes are present in the patch, but are not part of the core theme engine.

| File | ThemedCET line ranges | Observed difference |
|---|---:|---|
| `src/VKBindings.cpp` | 165–171 | Adjusts the initialization syntax of the variable holding the parsed JSON. |
| `src/reverse/RTTIExtender.cpp` | 446–488 | Replaces manual RTTI class creation/registration with `CreateScriptedClass()`, then checks that the retrieved class exists before registering its native functions. |
| `src/scripting/FunctionOverride.cpp` | 468–474, 491–497 | Simplifies lambda captures by removing redundant explicit captures. |
| `src/SharedSpinLock.cpp` | 1–2 | New small source file. |
| `xmake.lua` | 64–75, 104–126, 164–175, 185–192 | Makes RC resource files optional, handles clones where no Git tag is available by using a fallback version, and copies themes into packaging and installation directories. |

The changes in `RTTIExtender.cpp` and `FunctionOverride.cpp` are unrelated to theme appearance. They should be reviewed separately before any potential synchronization with upstream.

## 4. Documentation and generated configuration files

| File | ThemedCET line ranges | Description |
|---|---:|---|
| `BUILD.md` | 1–64 | Build documentation adapted for the active project. |
| `CODE_GUIDELINES.md` | 1–28 | Contribution guidelines adapted for the active project. |
| `src/config/CETVersion.h` | 1–21 | New version header generated for compilation. |
| `src/config/Resource.rc` | 1–31 | New Windows resource/version file included in the compiled tree. |

The `CETVersion.h` and `Resource.rc` files are generated/configuration files. They are listed for completeness, but should not be confused with the theme engine's functional code.

## 5. Reference-only files

The patch reports the following paths as present under `Original/` without an equivalent at the same relative path under `ThemedCET/`:

- `.gitattributes`
- `.gitignore`
- `.gitmodules`
- `LICENSE-EXCEPTION`
- `README.md`
- `LICENSE-MIT` (the patch shows a copy of the MIT license in the active packaging directory)

This describes the comparison between the two subtrees; it does **not** mean these files were necessarily deleted from the full Git repository. Some repository configuration files may have been moved or replaced at the root. This comparison alone cannot confirm that.

## 6. Generated files and dependencies excluded from source-change review

The supplied patch also includes many files that should not be mistaken for functional CET source changes:

- **`ThemedCET/.xmake/`**: Xmake caches and local configuration state.
- **`ThemedCET/build/`**: generated dependencies, intermediate files, metadata, and build outputs.
- **`ThemedCET/package/`**: packaging directory containing compiled binaries (`.asi`, `.pdb`, `.dll`), fonts, scripts, and copies of the themes.
- **`ThemedCET/vendor/RED4ext.SDK/`**: RED4ext SDK contents. The patch expands this dependency into thousands of files; these are not a set of CET engine modifications.

These items account for much of the patch's size and number of diff sections. They are not counted as source changes in sections 1–4.

## 7. Reproducing a cleaner source diff

From the repository root, compare the two directories while excluding local build output, packaging output, and the SDK dependency:

```bash
git diff --no-index --find-renames -- Original/ ThemedCET/ -- \
  ':!ThemedCET/.xmake/**' \
  ':!ThemedCET/build/**' \
  ':!ThemedCET/package/**' \
  ':!ThemedCET/vendor/**' \
  > changes-original-vs-fork-source.patch
```

To inspect one file:

```bash
git diff --no-index -- Original/src/d3d12/D3D12_Functions.cpp \
  ThemedCET/src/d3d12/D3D12_Functions.cpp
```

For exact added/removed line numbers, inspect the hunk headers (`@@ -old +new @@`) in the generated patch. The ranges in this README intentionally refer to complete hunk ranges, including context.

---

**Reference analyzed:** the `changes-original-vs-fork.patch` file supplied in the conversation and generated on November 10, 2026. This report reflects the differences visible in that patch; it does not claim to cover repository-root files that were not part of the comparison.
