# Journal - Zhang Sheng (Part 1)

> AI development session journal
> Started: 2026-03-31

---



## Session 1: File content extractor framework (process-isolated)

**Date**: 2026-04-02
**Task**: File content extractor framework (process-isolated)

### Summary

Implemented a process-isolated file content extractor framework with IPC pipes + plugin loading; refactored process priority utilities into dfm-base; documented TextIndex service and added a content-index upgrade unit.

### Main Changes

| Area | Description |
|------|-------------|
| Extractor framework | Added `dde-file-manager-extractor` app with `libextractor` (`ControllerPipe`/`WorkerPipe`) and a plugin loader. |
| Text extraction | Added initial `text-extractor` plugin and a standalone test app for end-to-end IPC. |
| TextIndex integration | Updated TextIndex service docs and adjusted textindex usage after moving `ProcessPriorityManager`. |
| Upgrade tool | Added `ContentIndexUpgradeUnit` to support content index upgrades. |

**Updated Files (highlights)**:
- `src/apps/dde-file-manager-extractor/libextractor/controllerpipe.cpp`
- `src/apps/dde-file-manager-extractor/libextractor/workerpipe.cpp`
- `src/apps/dde-file-manager-extractor/plugins/text-extractor/textextractorplugin.h`
- `tests/extractor-test/mainwindow.cpp`
- `src/dfm-base/utils/processprioritymanager.h`
- `.trellis/spec/backend/textindex-service.md`


### Git Commits

| Hash | Message |
|------|---------|
| `c4b21417a` | (see git log) |
| `d827749b5` | (see git log) |
| `e5dee8b57` | (see git log) |
| `880924f74` | (see git log) |
| `0acf4c34c` | (see git log) |

### Testing

- [OK] (Add test results)

### Status

[OK] **Completed**

### Next Steps

- None - task complete


## Session 2: Add OCR extractor plugin

**Date**: 2026-04-03
**Task**: Add OCR extractor plugin

### Summary

(Add summary)

### Main Changes

| Area | Description |
|------|-------------|
| Extractor plugin | Added a new `ocr-extractor` plugin under `src/apps/dde-file-manager-extractor/plugins/ocr-extractor/` for image OCR text extraction. |
| OCR behavior | Reused a process-level `DOcr` instance, preferred `PPOCR_V5` with default fallback, and scaled large images before recognition. |
| Text cleanup | Filtered blank lines and short symbol-only noise lines such as `□`. |
| Configuration | Added `maxOcrImageSizeMB` and `supportedOcrImageExtensions` to the textindex dconfig schema. |
| Build & packaging | Registered the new plugin in CMake and added `libdtk6ocr-dev` to Debian build dependencies. |

**Verification**:
- Ran `cmake -S . -B build-ocr-check`
- Ran `cmake --build build-ocr-check --target ocr-extractor -j2`
- Validated the updated dconfig JSON structure with `python3 -m json.tool`


### Git Commits

| Hash | Message |
|------|---------|
| `a88bfa9a2` | (see git log) |

### Testing

- [OK] (Add test results)

### Status

[OK] **Completed**

### Next Steps

- None - task complete
