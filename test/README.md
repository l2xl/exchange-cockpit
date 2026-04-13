# Tests

Uses Catch2 v3 test framework. Each test is a separate executable target defined in the root CMakeLists.txt via `add_test(target_name test_file)`.

## Test placement rule

Test files must mirror the `src/` subfolder structure. A test for source in `src/<component>/` goes to `test/<component>/`. Provider-specific tests (e.g. ByBit) use `test/bybit/` matching `src/data/bybit/`.

## Adding a new test

In root `CMakeLists.txt` add a single line inside the `if(NOT ANDROID)` block:
```cmake
add_test(test_<name> test/<component>/test_<name>.cpp)
```
The `add_test` CMake function creates an executable, links `core`, `Catch2::Catch2WithMain`, and all common dependencies automatically.

## Building a single test

```sh
cmake --build cmake-build-debug-clang-19 --target test_<name>
```

## Running a test

```sh
./cmake-build-debug-clang-19/test_<name>
```

Catch2 CLI options apply, e.g. `--list-tests`, `-c "section name"`, `[tag]`.