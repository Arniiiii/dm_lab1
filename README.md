# data mining lab1

## Requirements

1. A modern C++ compiler
2. CMake
3. next libs installed:
    1. Boost (math(for knn), dynamic_bitset(for decision_trees))
    2. fmt ( because g++ doesn't support std::format(...) for ranges at the moment )

## How to build

Note: `$(nproc)` is equal to number of threads on your CPU. If you are using a POSIX shell, the function is usually defined.

1. git clone the project.
2. `cmake -S . -B ./build -DBUILD_TESTING=1 -DBUILD_SHARED_LIBS=1`
3. `cmake --build ./build -j $(nproc)` ( to build (if `-DBUILD_TESTING=1` was added, it will build tests.))
4. `ctest --test-dir ./build -j $(nproc) --verbose` ( to test )
5. `cmake --install ./build ` ( to install . `--prefix ./a_folder` to install to a prefix.)

## License

I don't care about the code, so BSD-2
