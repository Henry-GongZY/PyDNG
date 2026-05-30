<!-- rtk-instructions v2 -->
# RTK (Rust Token Killer) - Token-Optimized Commands

## Golden Rule

**Always prefix commands with `rtk`**. If RTK has a dedicated filter, it uses it. If not, it passes through unchanged. This means RTK is always safe to use.

**Important**: Even in command chains with `&&`, use `rtk`:
```bash
# ❌ Wrong
git add . && git commit -m "msg" && git push

# ✅ Correct
rtk git add . && rtk git commit -m "msg" && rtk git push
```

## RTK Commands by Workflow

### Build & Compile (80-90% savings)
```bash
rtk cargo build         # Cargo build output
rtk cargo check         # Cargo check output
rtk cargo clippy        # Clippy warnings grouped by file (80%)
rtk tsc                 # TypeScript errors grouped by file/code (83%)
rtk lint                # ESLint/Biome violations grouped (84%)
rtk prettier --check    # Files needing format only (70%)
rtk next build          # Next.js build with route metrics (87%)
```

### Test (60-99% savings)
```bash
rtk cargo test          # Cargo test failures only (90%)
rtk go test             # Go test failures only (90%)
rtk jest                # Jest failures only (99.5%)
rtk vitest              # Vitest failures only (99.5%)
rtk playwright test     # Playwright failures only (94%)
rtk pytest              # Python test failures only (90%)
rtk rake test           # Ruby test failures only (90%)
rtk rspec               # RSpec test failures only (60%)
rtk test <cmd>          # Generic test wrapper - failures only
```

### Git (59-80% savings)
```bash
rtk git status          # Compact status
rtk git log             # Compact log (works with all git flags)
rtk git diff            # Compact diff (80%)
rtk git show            # Compact show (80%)
rtk git add             # Ultra-compact confirmations (59%)
rtk git commit          # Ultra-compact confirmations (59%)
rtk git push            # Ultra-compact confirmations
rtk git pull            # Ultra-compact confirmations
rtk git branch          # Compact branch list
rtk git fetch           # Compact fetch
rtk git stash           # Compact stash
rtk git worktree        # Compact worktree
```

Note: Git passthrough works for ALL subcommands, even those not explicitly listed.

### GitHub (26-87% savings)
```bash
rtk gh pr view <num>    # Compact PR view (87%)
rtk gh pr checks        # Compact PR checks (79%)
rtk gh run list         # Compact workflow runs (82%)
rtk gh issue list       # Compact issue list (80%)
rtk gh api              # Compact API responses (26%)
```

### JavaScript/TypeScript Tooling (70-90% savings)
```bash
rtk pnpm list           # Compact dependency tree (70%)
rtk pnpm outdated       # Compact outdated packages (80%)
rtk pnpm install        # Compact install output (90%)
rtk npm run <script>    # Compact npm script output
rtk npx <cmd>           # Compact npx command output
rtk prisma              # Prisma without ASCII art (88%)
```

### Files & Search (60-75% savings)
```bash
rtk ls <path>           # Tree format, compact (65%)
rtk read <file>         # Code reading with filtering (60%)
rtk grep <pattern>      # Search grouped by file (75%). Format flags (-c, -l, -L, -o, -Z) run raw.
rtk find <pattern>      # Find grouped by directory (70%)
```

### Analysis & Debug (70-90% savings)
```bash
rtk err <cmd>           # Filter errors only from any command
rtk log <file>          # Deduplicated logs with counts
rtk json <file>         # JSON structure without values
rtk deps                # Dependency overview
rtk env                 # Environment variables compact
rtk summary <cmd>       # Smart summary of command output
rtk diff                # Ultra-compact diffs
```

### Infrastructure (85% savings)
```bash
rtk docker ps           # Compact container list
rtk docker images       # Compact image list
rtk docker logs <c>     # Deduplicated logs
rtk kubectl get         # Compact resource list
rtk kubectl logs        # Deduplicated pod logs
```

### Network (65-70% savings)
```bash
rtk curl <url>          # Compact HTTP responses (70%)
rtk wget <url>          # Compact download output (65%)
```

### Meta Commands
```bash
rtk gain                # View token savings statistics
rtk gain --history      # View command history with savings
rtk discover            # Analyze Claude Code sessions for missed RTK usage
rtk proxy <cmd>         # Run command without filtering (for debugging)
rtk init                # Add RTK instructions to CLAUDE.md
rtk init --global       # Add RTK to ~/.claude/CLAUDE.md
```

## Token Savings Overview

| Category | Commands | Typical Savings |
|----------|----------|-----------------|
| Tests | vitest, playwright, cargo test | 90-99% |
| Build | next, tsc, lint, prettier | 70-87% |
| Git | status, log, diff, add, commit | 59-80% |
| GitHub | gh pr, gh run, gh issue | 26-87% |
| Package Managers | pnpm, npm, npx | 70-90% |
| Files | ls, read, grep, find | 60-75% |
| Infrastructure | docker, kubectl | 85% |
| Network | curl, wget | 65-70% |

Overall average: **60-90% token reduction** on common development operations.
<!-- /rtk-instructions -->

---

# PyDNG Project Knowledge

## Overview

PyDNG (package name: **dngpy**) provides Python bindings for the Adobe DNG SDK via pybind11, enabling read/write of DNG (Digital Negative) raw image files from Python with NumPy integration.

- **Version**: 0.1.2 (pyproject.toml) / 0.1.0 (setup.py / `__init__.py` — should eventually sync)
- **Python**: >= 3.8 (CI builds cover cp38–cp313)
- **License**: Adobe DNG SDK license

## Repository Layout

```
PyDNG/
├── src/dngpy/            # Installable Python package (NOT "pydng"!)
│   ├── __init__.py       # Dng wrapper class with string-based pixel_type support
│   ├── __init__.pyi      # Type stubs
│   ├── _native.pyi       # Type stubs for the compiled extension
│   └── py.typed          # PEP 561 marker
├── bindings/             # C++ pybind11 layer
│   ├── include/          # dng.h, utils.h, pch.h
│   ├── src/              # pydng_bindings.cpp (pybind11 module), dng.cpp (Dng wrapper)
│   └── main.cpp          # dng_validate CLI entry point
├── cmake/                # CMake helper scripts (source lists, libjxl preload, copy scripts)
├── extern/               # Vendored dependencies (git submodules)
│   ├── dng_sdk/          # Adobe DNG SDK (source)
│   ├── pybind11/         # pybind11 v2.11.1
│   ├── xmp/              # Adobe XMP Toolkit
│   ├── libjxl/           # libjxl + highway + brotli (for JXL-compressed DNG)
│   ├── libjpeg/          # libjpeg (Unix: ExternalProject; Windows: compiled inline)
│   └── sample_files/     # 14 DNG test files covering various formats
├── examples/
│   ├── example_read_dng.py
│   └── example_write_dng.py
├── pyproject.toml        # scikit-build-core config (primary build config)
├── setup.py              # Legacy setuptools wrapper (for compatibility)
├── setup.cfg             # Empty / metadata-only
├── CMakeLists.txt        # Top-level CMake (builds libdng + _native.pyd)
└── .github/workflows/    # CI: builds core lib then cibuildwheel wheels (release only)
```

## Key Architecture

### C++ Layer (`bindings/`)

- **`Dng` struct** ([dng.h](bindings/include/dng.h)): Wraps `AutoPtr<dng_negative> negative`. Provides `Read`, `Write`, `SetData`, `GetData`, metadata/settings accessors.
- **`DngData` struct**: Holds raw pixel buffer (`void* ptr`) with dims and pixel type. Used as bridge between NumPy and DNG SDK.
- **`DngMeta` struct**: Flat metadata struct (make, model, ISO, exposure, etc.).
- **`DngGainMap` struct**: Gain map data for shading correction.
- **`pch.h`**: Precompiled header pulling in DNG SDK + STL + Windows headers.
- **`utils.h`**: String/encoding helpers (UTF-8 ↔ wchar_t for Windows paths, Bayer pattern parsing).

### pybind11 Bindings (`pydng_bindings.cpp`)

- Module name: `_native` (imported as `dngpy._native`)
- `DngMeta`, `DngData`, `DngGainMap` exposed as pybind11 classes
- `Dng` methods: `read`, `write`, `get_data`, `set_data` (lambda with dtype dispatch), `get_meta`/`set_meta`, `get/set_baseline_exposure`, `get/set_white_balance`, `get/set_gainmap`, `get/set_bayer_pattern`
- `ErrorCode` exposed as class with static readonly int constants

### Python Layer (`src/dngpy/__init__.py`)

- `Dng` inherits from `_native.Dng`, adds `set_data()` wrapper that accepts string-based pixel types (`"uint8"`, `"uint16"`, `"int16"`, `"uint32"`, `"float32"`)
- `PIXEL_TYPE_MAP`: string → int mapping
- DLL directory registration for Windows via `os.add_dll_directory()`

### Pixel Types

| String      | Code | DNG Constant | C Type     |
|-------------|------|-------------|------------|
| `"uint8"`   | 1    | ttByte      | uint8_t    |
| `"uint16"`  | 3    | ttShort     | uint16_t   |
| `"int16"`   | 8    | ttSShort    | int16_t    |
| `"uint32"`  | 4    | ttLong      | uint32_t   |
| `"float32"` | 11   | ttFloat     | float      |

## CI / Release Pipeline

- **Trigger**: `on: release: types: [published]` — ONLY builds on published GitHub Releases
- **Two-stage**: Stage 1 builds `libdng` shared library per platform; Stage 2 uses cibuildwheel to build wheels linking prebuilt core
- **Platforms**: Windows (MSVC x64), Linux (manylinux2014 x86_64), macOS (arm64)
- **Wheels + sdist** uploaded to GitHub Release; published to PyPI via trusted publishing

## C++ Build System

- **CMake** >= 3.15, C++14 required
- Top-level targets: `dng` (shared lib), `_native` (pybind11 module), `dng_validate` (CLI test tool, optional)
- Build options: `BUILD_PYTHON_BINDINGS` (default ON), `BUILD_DNG_LIBRARY` (default ON), `BUILD_DNG_VALIDATE` (default OFF), `PREBUILT_DNG_PATH`
- Windows: zlib + libjpeg compiled inline; libjxl via ExternalProject
- Linux: libjpeg via ExternalProject (autotools); libjxl via ExternalProject (CMake)
- macOS: similar to Linux, with `@loader_path` install names for wheel portability

## Quick Debug Build Workflow (Windows, Python 3.13)

**Never use `pip install` during development — it's too slow.** Use cmake directly and copy manually:

```bash
# Build only the native extension (incremental, fast)
cmake --build build --target _native --config Release

# Copy to the ACTUAL install location (NOT site-packages!)
# The Python package is installed to: $CONDA_PREFIX/Lib/dngpy/
cp build/Release/_native.cp313-win_amd64.pyd "$CONDA_PREFIX/Lib/dngpy/"
cp build/Release/dng.dll "$CONDA_PREFIX/Lib/dngpy/"
rm -rf "$CONDA_PREFIX/Lib/dngpy/__pycache__"
```

For other Python versions, the `.pyd` filename suffix changes (e.g., `cp312-win_amd64.pyd`).

**IMPORTANT**: The conda install path is `<conda_env>/Lib/dngpy/`, NOT `<conda_env>/Lib/site-packages/dngpy/`. Use `python -c "import dngpy._native; print(dngpy._native.__file__)"` to verify the load path.

## Known Issues & Gotchas

1. **`negative` null pointer on fresh Dng()**: The `Dng()` default constructor leaves `negative` as null. Only `Read()` initializes it. Fixed in `SetData` (added null-check + `Make_dng_negative()` init). Other setters (`SetBaselineExposure`, `SetWhiteBalance`, etc.) still lack this guard — call `set_data()` first or they will crash.

2. **`SetMeta` is a no-op**: The C++ implementation is empty. Metadata set via `set_meta()` is NOT actually applied to the negative. This needs to be implemented.

3. **Version mismatch**: `pyproject.toml` = 0.1.2, `setup.py` = 0.1.0, `__init__.py` = 0.1.0.

4. **Manylinux external projects**: libjpeg and libjxl are built as `ExternalProject` dependencies, which adds significant time to the first build.

5. **Windows DLL resolution**: `dng.dll` must be in the same directory as `_native.pyd` (handled by post-build copy + `os.add_dll_directory`).

## Testing

- No automated test suite yet. Manual testing via `examples/` scripts.
- `extern/sample_files/` contains 14 DNG test files covering: JXL linear raw (int/float), JXL Bayer raw, PGTM2 profiles (int8/uint16/float16/float32), ImageSequenceInfo, ImageStats, HDR/SDR profiles.
