# Building Themed CET

The active Xmake project is `ThemedCET/`. The repository-root scripts are the recommended way to produce the complete install-ready package.

## Requirements

- Windows x64.
- Git for Windows.
- Xmake 3.1 or newer.
- Visual Studio 2022 / Build Tools with MSVC C++ and a Windows SDK.
- Internet access for the pinned RED4ext SDK submodule and Xmake dependencies.

## Recommended build

Run from the repository root:

```powershell
.\build-local.ps1
```

Or double-click `build-local.bat`.

The script:

1. Validates Git and Xmake.
2. Synchronizes and initializes `ThemedCET/vendor/RED4ext.SDK` at commit `5246e659d783e5834cc08a90e56c8932c9ed3449`.
3. Clears the Xmake configuration and rebuilds the ImGui package against the current checkout path.
4. Cleans and compiles the Release x64 target.
5. Runs Xmake's package hook.
6. Validates and stages the install payload in `build/bin/`.

The previous successful `build/` output is preserved if a new build fails. The script waits for Enter before closing.

## Manual Xmake workflow

From the active project directory:

```powershell
cd ThemedCET
xmake f -c -m release -y
xmake require -f -y imgui
xmake clean -a
xmake -y
xmake package -y
```

To generate Visual Studio project files:

```powershell
xmake project -k vsxmake
```

## Install

Copy the contents of repository-root `build/bin/` into the Cyberpunk 2077 game directory. The package includes the ASI loader file, CET plugin, fonts, scripts, themes, TweakDB data, and third-party notices.

Do not copy `ThemedCET/` or `Original/` into the game directory.

## Troubleshooting

- If the SDK initialization fails, check network access and run `git submodule sync --recursive` followed by `git submodule update --init --recursive -- ThemedCET/vendor/RED4ext.SDK`.
- If Xmake reuses stale dependency configuration after moving the repository, rerun the root build script; it clears the configuration and rebuilds ImGui.
- Inspect the first compiler/package error rather than only the final failure line.
- Xmake logs are normally under `%LOCALAPPDATA%\.xmake`.
