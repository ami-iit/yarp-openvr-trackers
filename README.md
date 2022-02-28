# Installation

## OpenVR

```bash
git clone https://github.com/ami-iit/openvr  # --depth=1 for a quick clone
cd openvr

# GNU/Linux and macOS
cmake -S . -B build -GNinja -DCMAKE_INSTALL_PREFIX:PATH=/path/to/install -DCMAKE_BUILD_TYPE=Release
cmake --build ./build/
cmake --install ./build/

# Windows
# Configure first with cmake-gui to detect the compiler
cmake -S . -B build -DCMAKE_INSTALL_PREFIX:PATH=/path/to/install -DBUILD_SHARED_LIBS:BOOL=ON
cmake --build build/ --config Release
cmake --install build/ --config Release
```

Add the following to the `.bashrc`.

```bash
# GNU/Linux
export OPENVR_DIR=/path/to/install
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$OPENVR_DIR/share/pkgconfig/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OPENVR_DIR/lib
```
