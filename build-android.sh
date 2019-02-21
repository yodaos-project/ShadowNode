NDK="/Users/qile222/Projects/android-ndk-r16b"
PLATFORM_DESCRIPTOR=aarch64-android-linux
SHADOWNODE_ROOT=$(dirname $0)
cmake \
    -H. \
    -B./build-android_aarch64 \
    -DCMAKE_BUILD_TYPE="Release" \
    -DCMAKE_CXX_FLAGS="-O3" \
    -DCMAKE_TOOLCHAIN_FILE=$NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI="armeabi-v7a" \
    -DANDROID_STL="c++_static" \
    -DANDROID_TOOLCHAIN="clang" \
    -DANDROID_PLATFORM="android-14" \
    -DTARGET_ARCH=aarch64 \
    -DPLATFORM_DESCRIPTOR=$PLATFORM_DESCRIPTOR \
    -DFEATURE_DEBUGGER=OFF \
    -DCMAKE_BUILD_TYPE=Release \
    -DTARGET_OS=android \
    -DTARGET_BOARD=None \
    -DENABLE_LTO=OFF \
    -DENABLE_SNAPSHOT=ON \
    -DENABLE_NAPI=ON \
    -DENABLE_JERRYX=ON \
    -DBUILD_LIB_ONLY=OFF \
    -DFEATURE_MEM_STATS=OFF \
    -DFEATURE_HEAP_PROFILER=ON \
    -DEXTERNAL_MODULES='' \
    -DFEATURE_PROFILE='./deps/jerry/jerry-core/profiles/es5.1.profile' \
    -DMEM_HEAP_SIZE_KB=2048 \
    -DEXTERNAL_LIBS='' \
    -DEXTERNAL_COMPILE_FLAGS='' \
    -DEXTERNAL_LINKER_FLAGS='' \
    -DEXTERNAL_INCLUDE_DIR='' \
    -DBUILD_STATIC='ON'
# make -C ./build-android_aarch64