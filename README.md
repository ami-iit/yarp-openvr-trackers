# Installation

## OpenVR

````bash
git clone https://github.com/ValveSoftware/openvr  # --depth=1 for a quick clone
cd openvr

# Fix some upstream problems
# https://github.com/ValveSoftware/openvr/pull/1542
# https://github.com/ValveSoftware/openvr/issues/1594
cd src && ln -s vrcommon vrcore && cd -
sed -i "s|#include <string.h>|#include <string.h>\n#include <stdarg.h>\n|g" src/vrcore/strtools_public.cpp
cat <<EOF > src/vrcore/assert.h
#ifndef VR_CORE_ASSERT_H
#define VR_CORE_ASSERT_H

#include <iostream>
#include <string>

inline void AssertMsg(bool ok, const std::string& msg) {
    std::cout << msg << std::endl;
}

#endif // VR_CORE_ASSERT_H
EOF

cmake -S . -B build -GNinja -DCMAKE_INSTALL_PREFIX=/path/to/install -DINSTALL_PKGCONFIG_DIR=/path/to/install/share/pkgconfig -DCMAKE_BUILD_TYPE=Release
cmake --build ./build/
cmake --install ./build/
```

Add the following to the ``.bashrc``.
```bash
export OPENVR_DIR=/path/to/install
export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$OPENVR_DIR/share/pkgconfig/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$OPENVR_DIR/lib
```


