# Code Guidelines

These guidelines are inherited from the upstream CET codebase and apply to active source changes under `ThemedCET/`. Keep `Original/` unchanged.

## Language and design

- The project uses C++20.
- Use templates responsibly to avoid excessive compile times and binary bloat.
- Prefer single-responsibility components that are reusable and easy to extend.
- Keep names self-explanatory.
- Avoid exceptions and throwing STL operations where a non-throwing alternative is available.

## Naming

- Variables use lower camelCase: `someVariableName`.
- Function arguments are prefixed with `a`: `aFunctionArgument`.
- Const variables are prefixed with `c`: `cSomeInt`.
- Pointers are prefixed with `p`: `pSomePointer`.
- Combine prefixes where applicable, e.g. `acpSomeArgument` for a const pointer argument.
- Static variables use the `s_` prefix.
- Global variables use the `g_` prefix.
- Class names and function names start with an uppercase letter.
- Class members use the `m_` prefix, followed by the variable naming convention.

## Formatting

- Put the opening brace on a new line.
- Prefer clear, explicit types for primitive values; use `auto` when it improves readability for long or complex types.
