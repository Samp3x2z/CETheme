

# JSON theme format

The active CET project discovers every `.json` file in the runtime folder:

```text
Cyberpunk 2077/bin/x64/plugins/cyber_engine_tweaks/themes/
```





Add another valid JSON file to this directory and it becomes available in **Settings → Theme**. Theme appearance is data-driven: colors and supported Dear ImGui style properties are read from JSON. You do not need to add a C++ preset or edit C++ when creating another theme.

## Schema

Each theme is a JSON object. For readability, keep its top-level sections in this order:

1. `name` — Theme name/metadata.
2. `style` — Dear ImGui style properties supported by the bundled ImGui version.
3. `colors` — color values keyed by Dear ImGui color names **without** the `ImGuiCol_` prefix. Each value is an RGBA array of four numbers in the inclusive range `0.0`–`1.0`.

Style value types:

| JSON type | ImGui property type | Example |
|---|---|---|
| Number | Float / integer / enum / flags | `"WindowRounding": 12.0` |
| Two-number array | `ImVec2` | `"WindowPadding": [8.0, 8.0]` |
| Boolean | Boolean | `"AntiAliasedFill": true` |




Enum and flag properties are represented by their integer values. Only properties supported by the ImGui version used to compile this checkout can be applied. Unknown JSON properties are retained when CET saves a theme, which allows additional metadata and future extension.

The shipped themes contain all 63 color entries and the style properties handled by this fork's JSON reader/writer. If a color entry is missing or malformed (wrong type, wrong component count, non-finite value, or a component outside `0.0`–`1.0`), CET leaves that color at the ImGui default rather than attempting an unsafe conversion.

## Example

```json
{
    "name": "My Theme",
    "style": {
        "WindowRounding": 12.0,
        "FrameRounding": 12.0,
        "WindowPadding": [8.0, 8.0],
        "AntiAliasedFill": true,
        "WindowMenuButtonPosition": 0
    },
    "colors": {
        "Text": [1.0, 1.0, 1.0, 1.0],
        "WindowBg": [0.04, 0.04, 0.05, 1.0],
        "ButtonActive": [1.0, 0.0, 0.0, 0.29411765]
    }
}
```



## Loading and saving

- At game startup, CET loads the theme named in `config.json` and applies its JSON values.

- If the saved theme is missing or invalid, CET attempts to load `Dark.json`.

- In CET Settings, **Save** writes the selected theme's complete supported color/style state back to its JSON file and saves the selected theme/font in `config.json`.

- Changing the font requires restarting the game. Theme selection is applied by the theme reload path.

The included `Dark.json`, `Classic.json`, `Light.json` are examples and starting points. Duplicate one, give it a new filename, then edit its `colors` and `style` values.
