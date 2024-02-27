# quad-world

Visualisation of quad-tree.

## Setup

```sh
python3 -m pip install --upgrade conan
conan profile detect
conan install --build=missing --output-folder=build .
cmake --preset conan-release
cmake --build --preset conan-release
```
