# Baseline inventory

Recorded on 2026-07-14 from the repository root.

## Legacy structure

- Public headers: `include/tc_runtime/`.
- Lifecycle and scheduling: `src/runtime/`.
- Platform backends: `src/backends/`.
- Graphics contexts: `src/graphics/`.
- Renderers: `src/renderers/`.
- Root selections: `TC_PLATFORM`, `TC_BACKEND`, `TC_GRAPHICS`, and
  `TC_RENDERER`.

The complete legacy-symbol and source-file inventories are retained locally in
`legacy-symbols.txt` and `files.txt` beside this record. They are intentionally
ignored build artifacts.

## Reproducible baseline attempt

```
cmake -S . -B build/baseline-desktop-cpu \\
  -DTC_PLATFORM=DESKTOP -DTC_BACKEND=SDL -DTC_GRAPHICS=CPU \\
  -DTC_RENDERER=SKIA -DCMAKE_BUILD_TYPE=Debug
```

Result: configuration stopped before generation because no TotalCross Skia
archive and no `skia` CMake package target are installed. SDL3 was discovered
before that check. This is an environment limitation, not a source-build
failure. No desktop baseline can be compiled until the required Skia artifact
is supplied.
