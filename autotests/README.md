# DDE File Manager Unit Tests

## Architecture

Tests link against pre-built shared library targets (e.g. `DFM6::framework`) rather than
recompiling source files. This means `src/` is compiled only once — test compilation is fast.

```
autotests/
├── CMakeLists.txt       # Test entry point (dfm_setup_test_env + add_subdirectory)
├── run-ut.sh            # One-click build + test + coverage
├── dfm_test_main.h      # DFM_TEST_MAIN() macro (GTest + QApplication)
├── dfm_asan_helper.h    # ASAN report helper
├── libs/                # Library unit tests
│   └── dfm-framework/   # dfm-framework tests
└── old/                 # Archived old tests (not in use)
```

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
