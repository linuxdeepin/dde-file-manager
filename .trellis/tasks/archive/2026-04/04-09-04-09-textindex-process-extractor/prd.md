# Textindex Migrate To Process Extractor

## Goal

Refactor the textindex extraction pipeline so both content indexing and OCR indexing use `ProcessExtractor` backed by `ControllerPipe` and `dde-file-manager-extractor`, replacing the legacy `DocUtils`-based content extraction path.

## What I Already Know

- `IndexRuntime` currently routes `IndexProfile::Type::Content` to `LegacyDocUtilsExtractor` and `IndexProfile::Type::Ocr` to `ProcessExtractor`.
- `ProcessExtractor` currently creates a stack-local `ControllerPipe`, starts `dde-file-manager-extractor` for each `extract()` call, waits with a local `QEventLoop`, then stops the process.
- `extract()` is invoked from textindex worker-thread code paths such as `taskhandler.cpp` and `moveprocessor.cpp`, so the process-driving logic must be safe outside the main thread.
- `tests/extractor` already demonstrates the intended `ControllerPipe` usage pattern with a longer-lived subprocess.
- `DocUtils` is still needed for Lucene document field-copy helpers, but its legacy extraction-related APIs should be removed if no longer used.

## Assumptions

- We will keep the public synchronous `IndexExtractor::extract()` interface unchanged to minimize blast radius in indexing logic.
- A per-`IndexRuntime` reusable extractor process is acceptable and desirable; separate content and OCR runtimes may each own their own extractor lifecycle.
- The extractor subprocess should be stopped after about 60 seconds of inactivity, and restarted transparently on the next request.

## Open Questions

- None blocking at the moment. Proceed with the runtime-owned reusable `ProcessExtractor` design unless new constraints appear during implementation.

## Requirements

- Replace content profile extraction with `ProcessExtractor`.
- Remove `LegacyDocUtilsExtractor`.
- Remove `DocUtils` extraction-related interfaces and implementation code that only existed for legacy extraction.
- Refactor `ProcessExtractor` so it reuses a started `dde-file-manager-extractor` process across requests.
- Automatically stop the extractor process if it stays idle for 60 seconds after the last completed request.
- Keep extraction callable from non-main-thread indexing code without relying on fragile direct connections in the caller thread.
- Preserve current indexing call sites and `IndexExtractor::extract()` contract.
- Keep failures observable through clear error strings and logging.

## Acceptance Criteria

- [ ] Content index extraction no longer depends on `LegacyDocUtilsExtractor` or `DocUtils::extractFileContent`.
- [ ] `IndexRuntime` uses `ProcessExtractor` for both content and OCR profiles.
- [ ] `ProcessExtractor` does not start and stop the extractor subprocess on every successful request.
- [ ] The extractor subprocess is released after the configured idle timeout with no pending work.
- [ ] Text extraction still works through existing indexing flows and move/update paths.
- [ ] Obsolete legacy extractor files and docutils extraction APIs are removed from the codebase.

## Definition Of Done

- Code compiles for the touched targets.
- Relevant autotests or targeted verification complete for the changed area.
- Logging and error-handling follow existing backend conventions.
- No unrelated files are modified.

## Out Of Scope

- Reworking the extractor IPC protocol itself.
- Broad textindex spec refresh in Trellis docs.
- Changing Lucene document schema or index field semantics.

## Technical Approach

Implement a reusable `ProcessExtractor` with an internal worker object that owns `ControllerPipe` in a dedicated thread. Keep a synchronous `extract()` facade by forwarding requests to that worker via blocking queued invocation. The worker is responsible for starting the subprocess on demand, handling request completion/failure, and arming an idle timer that stops the subprocess after 60 seconds without new work.

## Decision (ADR-lite)

**Context**: The legacy content extractor path is outdated, while the current process-based extractor is functionally aligned with the desired architecture but wastes resources by spawning a subprocess for each file and mixes synchronous waiting logic into caller threads.

**Decision**: Use one reusable `ProcessExtractor` implementation for both content and OCR indexing, with runtime-owned lifecycle and an internal dedicated thread for IPC/process management.

**Consequences**: This keeps business-layer call sites stable and avoids per-request process startup overhead, but introduces a small amount of concurrency/lifecycle management inside the extractor implementation.

## Technical Notes

- Key files inspected:
  - `src/services/textindex/core/indexruntime.h`
  - `src/services/textindex/core/indexruntime.cpp`
  - `src/services/textindex/extractor/processextractor.h`
  - `src/services/textindex/extractor/processextractor.cpp`
  - `src/services/textindex/extractor/legacydocutilsextractor.*`
  - `src/services/textindex/utils/docutils.*`
  - `src/apps/dde-file-manager-extractor/libextractor/controllerpipe.*`
  - `src/services/textindex/task/taskhandler.cpp`
  - `src/services/textindex/task/moveprocessor.cpp`
- Existing tests referencing old extraction behavior live in `autotests/services/textindex/test_docutils.cpp`.
