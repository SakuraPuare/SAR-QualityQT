    # aarch64-w64-mingw32-gcc -> /usr/bin/aarch64-w64-mingw32-gcc
    # x86_64-w64-mingw32-gcc -> /usr/bin/x86_64-w64-mingw32-gcc
    # i686-w64-mingw32-gcc -> /usr/bin/i686-w64-mingw32-gcc
    #
    # 设置目标系统名称
    set(CMAKE_SYSTEM_NAME Windows)

    # 指定交叉编译器
    # 注意：根据你的 MinGW-w64 安装，这些路径和名称可能需要调整
    # 例如，可能是 x86_64-w64-mingw32-gcc, x86_64-w64-mingw32-g++, x86_64-w64-mingw32-windres
    set(TOOLCHAIN_PREFIX x86_64-w64-mingw32) # 或者 i686-w64-mingw32 用于 32 位
    set(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
    set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
    set(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)

    # 设置查找目标环境的根路径
    # 将 MinGW 的系统根目录添加到 CMAKE_PREFIX_PATH
    # CMake 会在这个目录下查找 lib/cmake/<PackageName> 等路径
    set(CMAKE_PREFIX_PATH /usr/${TOOLCHAIN_PREFIX})

    # 修改 find_library, find_program 等的搜索行为
    # 只在目标环境的根路径中查找 (CMAKE_PREFIX_PATH 也会被考虑)
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

    # (可选) 设置 Qt6, OpenCV, GDAL 的查找路径提示
    # 如果设置 CMAKE_PREFIX_PATH 后仍然找不到，可以尝试取消注释并指定更精确的路径
    # set(Qt6_DIR "/usr/${TOOLCHAIN_PREFIX}/lib/cmake/Qt6")
    # set(OpenCV_DIR "/usr/${TOOLCHAIN_PREFIX}/lib/cmake/opencv4") # 目录名可能是 opencv4 或其他
    # set(GDAL_DIR "/usr/${TOOLCHAIN_PREFIX}/lib/cmake/gdal")