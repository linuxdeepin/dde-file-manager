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


## Session 3: TextIndex 架构重构与规范更新

**Date**: 2026-04-10
**Task**: TextIndex 架构重构与规范更新

### Summary

(Add summary)

### Main Changes

## 完成工作

### 1. 代码重构 (已提交)

| 变更 | 描述 |
|------|------|
| 统一提取器 | 移除 LegacyDocUtilsExtractor，所有提取通过 ProcessExtractor |
| 进程隔离 | ProcessExtractorProxy 管理子进程，60s 空闲超时，120s 请求超时 |
| 依赖简化 | 移除 docparser 依赖，提取逻辑移至 dde-file-manager-extractor |

### 2. 规范文档更新

**文件**: `.trellis/spec/backend/textindex-service.md` (664 行)

**新增章节**:
- 完整架构图 (DBus → Runtime → Extractor → Subprocess)
- ProcessExtractor 进程提取器设计
- ControllerPipe IPC 协议 (状态码: S/F/f/D/B)
- dde-file-manager-extractor 工具架构
- 双索引模式 (文本 + OCR)
- 完整配置项列表 (15项)

### 3. 变更文件

| 文件 | 操作 |
|------|------|
| `src/services/textindex/extractor/processextractor.cpp` | 重构 (+296/-323) |
| `src/services/textindex/core/indexruntime.cpp` | 简化选择器 |
| `src/services/textindex/utils/docutils.cpp` | 删除提取函数 |
| `src/services/textindex/dependencies.cmake` | 移除 docparser |
| `.trellis/spec/backend/textindex-service.md` | 完整重写 |

### 4. 归档任务

- `04-09-textindex-process-extractor` → `archive/2026-04/`


### Git Commits

| Hash | Message |
|------|---------|
| `e136b97d3` | (see git log) |
| `81c24eac5` | (see git log) |

### Testing

- [OK] (Add test results)

### Status

[OK] **Completed**

### Next Steps

- None - task complete


## Session 4: TextIndex modify time field alignment

**Date**: 2026-04-11
**Task**: TextIndex modify time field alignment

### Summary

(Add summary)

### Main Changes

| Area | Description |
|------|-------------|
| Content index | Replaced legacy `modified` stored field with `Content::kModifyTime` numeric field |
| OCR index | Added `OcrText::kModifyTime` numeric field |
| Update check | Updated `checkNeedUpdate()` to read the new per-profile modify-time field |
| Verification | Rebuilt `dde-filemanager-textindex` successfully in `obj-x86_64-linux-gnu` |

**Updated Files**:
- `src/services/textindex/document/contentdocumentbuilder.cpp`
- `src/services/textindex/document/ocrdocumentbuilder.cpp`
- `src/services/textindex/task/taskhandler.cpp`


### Git Commits

| Hash | Message |
|------|---------|
| `918a8670c` | (see git log) |

### Testing

- [OK] (Add test results)

### Status

[OK] **Completed**

### Next Steps

- None - task complete


## Session 5: Fix textindex directory move path invalidation

**Date**: 2026-04-16
**Task**: Fix textindex directory move path invalidation

### Summary

(Add summary)

### Main Changes

| Area | Description |
|------|-------------|
| Root cause | Queued and in-flight incremental indexing tasks kept stale old paths after a directory rename/move, while `DirectoryMoveProcessor` only migrated documents that already existed in the index. |
| Implementation | Added queue path rewrite handling for directory move events and queued a compensation `UpdateFileList` task for moved target directories to recover files missed during in-flight indexing. |
| Design | Kept `TaskManager` as a thin coordinator and moved pure directory-move path rewrite logic into `src/services/textindex/task/taskqueueutils.*`. |
| Verification | Rebuilt the `dde-filemanager-textindex` target successfully with CMake. |

**Updated Files**:
- `src/services/textindex/task/taskmanager.cpp`
- `src/services/textindex/task/taskmanager.h`
- `src/services/textindex/task/taskqueueutils.cpp`
- `src/services/textindex/task/taskqueueutils.h`
- `.trellis/tasks/04-16-fix-textindex-path-invalidation-on-directory-change/prd.md`


### Git Commits

| Hash | Message |
|------|---------|
| `2e0908993` | (see git log) |

### Testing

- [OK] (Add test results)

### Status

[OK] **Completed**

### Next Steps

- None - task complete
