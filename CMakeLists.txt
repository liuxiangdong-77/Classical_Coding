# =============================================================================
# CMakeLists.txt —— 最简单的 CUDA C++ 工程：一个 .cu 文件 → 一个 exe
#
# 日常三条命令（在工程根目录执行）：
#   cmake --preset vs2022          # 配置，生成 build/
#   cmake --build --preset debug   # 编译（Release 就换成 release）
#   build\bin\vector_add.exe       # 运行
#
# 说明：本工程故意不附带 src/main.cu，需要你自己把示例代码敲进去。
#       源码不存在时，下面的 message(FATAL_ERROR) 会提醒你，不会报一堆看不懂的错。
# =============================================================================
cmake_minimum_required(VERSION 3.24)

# -----------------------------------------------------------------------------
# 让 CMake / MSBuild 稳定地找到 CUDA。
#
# 坑在这里：VS 的 CUDA 构建目标（BuildCustomizations\CUDA 13.5.Version.props）
# 只认环境变量 CUDA_PATH_V13_5，不看注册表。如果 VS Code / 终端是在安装 CUDA
# 之前打开的，环境里就没有这两个变量，于是会报
#   "The CUDA Toolkit directory '' does not exist"
# 而 nvcc 明明装得好好的。下面先自己找一遍，必要时补上环境变量。
# -----------------------------------------------------------------------------
set(_cuda_root "")

# 1) 先看现有的环境变量
foreach(_var IN ITEMS CUDA_PATH_V13_5 CUDA_PATH)
  if(NOT "$ENV{${_var}}" STREQUAL "" AND EXISTS "$ENV{${_var}}/bin/nvcc.exe")
    set(_cuda_root "$ENV{${_var}}")
    break()
  endif()
endforeach()

# 2) 环境变量没有，就按本机常见安装位置找
if(NOT _cuda_root)
  foreach(_cand IN ITEMS
      "D:/SOFTWARE/CUDA"
      "$ENV{ProgramFiles}/NVIDIA GPU Computing Toolkit/CUDA/v13.5"
      "$ENV{ProgramFiles}/NVIDIA GPU Computing Toolkit/CUDA/v12.9")
    if(EXISTS "${_cand}/bin/nvcc.exe")
      set(_cuda_root "${_cand}")
      break()
    endif()
  endforeach()
endif()

if(NOT _cuda_root)
  message(FATAL_ERROR
    "没有找到 CUDA Toolkit（nvcc.exe）。\n"
    "  请确认已安装 CUDA Toolkit，或设置环境变量 CUDA_PATH 指向安装目录。")
endif()

# 3) 补上 CUDA_PATH 和带版本号的 CUDA_PATH_V<主>_<次>（版本号从 nvcc 输出里读）
execute_process(COMMAND "${_cuda_root}/bin/nvcc.exe" --version
                OUTPUT_VARIABLE _nvcc_out ERROR_QUIET)
if(_nvcc_out MATCHES "release ([0-9]+)\\.([0-9]+)")
  set(_cuda_ver_major "${CMAKE_MATCH_1}")
  set(_cuda_ver_minor "${CMAKE_MATCH_2}")
  set(ENV{CUDA_PATH} "${_cuda_root}")
  set(ENV{CUDA_PATH_V${_cuda_ver_major}_${_cuda_ver_minor}} "${_cuda_root}")
  message(STATUS "CUDA 安装目录: ${_cuda_root}  (CUDA_PATH_V${_cuda_ver_major}_${_cuda_ver_minor})")
endif()

# LANGUAGES 里加上 CUDA，CMake 才会去找 nvcc、才会编译 .cu 文件。
project(vector_add VERSION 1.0.0 LANGUAGES CXX CUDA)

# 单配置生成器（Ninja / NMake）没有默认构建类型，这里兜底为 Release。
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# 生成哪个架构的机器码：native = 按本机显卡自动选择（RTX 3060 是 86）。
# 想固定成某个架构时，配置时加 -DCUDA_ARCH=86 即可。
set(CUDA_ARCH "native" CACHE STRING "CUDA 架构：native / 86 / 89 / 120 ...")
set(CMAKE_CUDA_ARCHITECTURES "${CUDA_ARCH}" CACHE STRING "CUDA 架构" FORCE)

# -----------------------------------------------------------------------------
# 源码文件
# 现在只有一个文件；以后想拆成多个 .cu/.cpp，在 add_executable 里多写几行就行。
# -----------------------------------------------------------------------------
set(MAIN_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cu")
if(NOT EXISTS "${MAIN_SOURCE}")
  message(FATAL_ERROR
    "没有找到 src/main.cu —— 这一步是故意的，源码需要你自己敲。\n"
    "\n"
    "  1. 在 VS Code 里新建文件 src/main.cu\n"
    "  2. 把示例代码敲进去并保存\n"
    "  3. 再执行：cmake --preset vs2022\n")
endif()

add_executable(vector_add src/main.cu)

# 把 CudaToolkitDir 直接写进生成的 .vcxproj。
# 这样即使以后从一个“没有 CUDA 环境变量”的终端 / 编辑器里执行 cmake --build，
# MSBuild 也能自己找到 CUDA，不用手动配环境。
if(MSVC AND _cuda_root)
  string(REPLACE "/" "\\" _cuda_root_win "${_cuda_root}")
  set_target_properties(vector_add PROPERTIES
    VS_GLOBAL_CudaToolkitDir "${_cuda_root_win}\\"
  )
endif()

# -----------------------------------------------------------------------------
# 运行库：动态 cudart（nvcc 的默认行为，也是官方示例的用法）
# 注意：CUDA 13 起 cudart DLL 放在 bin\x64 下，而且默认不在 PATH 里，
# 不处理的话双击 exe 会报“找不到 cudart64_13.dll”。这里直接把它拷到 exe 旁边。
# -----------------------------------------------------------------------------
set_target_properties(vector_add PROPERTIES CUDA_RUNTIME_LIBRARY "Shared")

file(GLOB _cudart_dlls
  "${_cuda_root}/bin/cudart64_*.dll"
  "${_cuda_root}/bin/x64/cudart64_*.dll")
if(_cudart_dlls)
  add_custom_command(TARGET vector_add POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${_cudart_dlls}
            "$<TARGET_FILE_DIR:vector_add>"
    COMMAND_EXPAND_LISTS
    VERBATIM)
else()
  message(WARNING "没有在 ${_cuda_root} 下找到 cudart64_*.dll，exe 运行时可能需要 CUDA 的 bin 目录在 PATH 里")
endif()

# -----------------------------------------------------------------------------
# 编译选项
#   -lineinfo        给 Nsight / compute-sanitizer 提供源码行号（不影响性能）
#   -Xcompiler=...   把参数透传给主机编译器 cl.exe；
#                    /utf-8 让 MSVC 正确读 UTF-8 源码（中文注释靠它）
# -----------------------------------------------------------------------------
target_compile_definitions(vector_add PRIVATE _CRT_SECURE_NO_WARNINGS)

if(MSVC)
  target_compile_options(vector_add PRIVATE
    $<$<COMPILE_LANGUAGE:CUDA>:-Xcompiler=/utf-8>
    $<$<COMPILE_LANGUAGE:CUDA>:-Xcompiler=/W3>
  )
endif()

target_compile_options(vector_add PRIVATE
  $<$<COMPILE_LANGUAGE:CUDA>:-lineinfo>
)

# 统一输出到 build/bin/，无论 Debug 还是 Release，exe 路径都是
# build\bin\vector_add.exe，launch.json 和 tasks.json 不用跟着改。
set_target_properties(vector_add PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
if(CMAKE_CONFIGURATION_TYPES)
  foreach(config IN LISTS CMAKE_CONFIGURATION_TYPES)
    string(TOUPPER "${config}" CONFIG_UPPER)
    set_target_properties(vector_add PROPERTIES
      "RUNTIME_OUTPUT_DIRECTORY_${CONFIG_UPPER}" "${CMAKE_BINARY_DIR}/bin"
    )
  endforeach()
endif()

message(STATUS "nvcc 版本 : ${CMAKE_CUDA_COMPILER_VERSION}")
message(STATUS "显卡架构  : ${CMAKE_CUDA_ARCHITECTURES}")
message(STATUS "输出目录  : ${CMAKE_BINARY_DIR}/bin")
message(STATUS "下一步    : cmake --build --preset debug")
