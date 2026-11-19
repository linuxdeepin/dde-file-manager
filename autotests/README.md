# DDE File Manager Unit Tests

## Architecture

Tests link against pre-built shared library targets (e.g. `DFM6::framework`) rather than
recompiling source files. This means `src/` is compiled only once — test compilation is fast.

```
autotests/
├── CMakeLists.txt       # Test entry point (dfm_setup_test_env + add_subdirectory)
├── run-ut.sh            # One-click build + test + coverage; supports incremental runs
├── dfm_test_main.h      # DFM_TEST_MAIN() macro (GTest + QApplication)
├── dfm_asan_helper.h    # ASAN report helper
├── libs/                # Library unit tests
│   ├── dfm-base/        #   test-dfm-base
│   ├── dfm-extension/   #   test-dfm-extension-{emblemicon,file,menu,window,global}
│   ├── dfm-framework/   #   test-dfm-framework-{backtrace,event,lifecycle,listener,log}
│   └── textindex/       #   test-textindex
└── old/                 # Archived old tests (not in use)
```

Suites are split per source submodule (mirroring `src/` layout) so incremental CI can
run only what a change affects. Each `dfm_add_test(... COVERS <dir>)` records its
tested source dirs into `build-autotests/ut-suite-map.tsv`, which `run-ut.sh` consumes
for changed-file → suite mapping.

## Build & Run

### Option A: Integrated with the project

```bash
# From project root
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DOPT_ENABLE_BUILD_UT=ON
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure
```

### Option B: Using the script

```bash
# Build and run all tests
./autotests/run-ut.sh

# Run tests without rebuilding
./autotests/run-ut.sh --run-only

# Build with coverage and generate HTML report
./autotests/run-ut.sh --coverage
```

### Incremental runs (CI local minimal testing)

Run only the suites affected by changed files/directories:

```bash
# Changed file(s) or directories (space or comma separated)
./autotests/run-ut.sh --changed-files src/dfm-framework/lifecycle/plugin.cpp
./autotests/run-ut.sh --changed-files src/dfm-extension/menu/ src/dfm-base

# From a git diff (merge request / push)
./autotests/run-ut.sh --git-diff origin/master

# From stdin (e.g. CI provides the changed-file list)
git diff --name-only origin/master | ./autotests/run-ut.sh --changed-files -

# Dry run: print selected suites and why, execute nothing
./autotests/run-ut.sh --list --changed-files src/dfm-base

# Policy when no suite matches a code change (default: all = run everything)
./autotests/run-ut.sh --on-no-match=skip|strict|all --changed-files src/apps/...
```

Mapping rules (in order):
1. Non-code changes (debian/, docs/, images, translations, ...) select nothing.
2. Shared test infra (test main, stubs, CMake test macros) selects all suites.
3. Path-prefix match against `ut-suite-map.tsv` (suite test dirs + `COVERS` dirs).
4. Include-affinity: suites whose test sources `#include` the changed file.
5. Unmatched code changes follow `--on-no-match` (default: full run, fail-safe).

The coverage report (if generated) is at `build-autotests/coverage/html/index.html`.

## Writing New Tests

### 1. Create a test directory

```
autotests/libs/<library-name>/
    CMakeLists.txt
    main.cpp
    test_*.cpp
```

### 2. CMakeLists.txt

```cmake
file(GLOB_RECURSE TEST_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
)

dfm_add_test(test-<library-name>
    SOURCES ${TEST_SOURCES}
    LINK_LIBRARIES <DFM::AliasTarget> Qt6::Test
)
```

Available library targets:
- `DFM6::base` — dfm6-base library
- `DFM6::framework` — dfm6-framework library
- `DFM::extension` — dfm-extension library

### 3. main.cpp

```cpp
#include "dfm_test_main.h"
DFM_TEST_MAIN(<unit_id>)
```

### 4. Test files

Use standard GTest macros (`TEST`, `TEST_F`, `EXPECT_*`, `ASSERT_*`).

For function stubbing, include `"stubext.h"` and use `StubExt::set_lamda()`:

```cpp
#include "stubext.h"

TEST(SomeTest, Example) {
    stub_ext::StubExt stub;
    stub.set_lamda(&SomeClass::method, [](SomeClass *self) -> ReturnType {
        __DBG_STUB_INVOKE__
        return mockValue;
    });
    // ... test code ...
}
```

## Dependencies

- GTest (`libgtest-dev`, `gtest`)
- lcov + genhtml (`lcov`) — only needed for coverage
