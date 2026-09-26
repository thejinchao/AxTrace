
if(WIN32 AND NOT MSVC)
  message(FATAL_ERROR "Windows builds require the MSVC toolchain (see src/msvcbuild.bat).")
endif()

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------


set(LUAJIT_BUILD_MODE "static")
set(LUAJIT_AMALGAMATED OFF)
set(LUAJIT_DISABLE_GC64  OFF)
set(LUAJIT_ENABLE_LUA52COMPAT OFF)

set(LUAJIT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(LUAJIT_SRC "${LUAJIT_ROOT}/src")
set(DYNASM_DIR "${LUAJIT_ROOT}/dynasm")
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(GENERATED_HOST_DIR "${GENERATED_DIR}/host")
set(GENERATED_JIT_DIR "${GENERATED_DIR}/jit")
file(MAKE_DIRECTORY "${GENERATED_HOST_DIR}" "${GENERATED_JIT_DIR}")

# ---------------------------------------------------------------------------
# Target architecture (aligned with Makefile + msvcbuild.bat)
# ---------------------------------------------------------------------------
string(TOLOWER
  "${CMAKE_GENERATOR_PLATFORM};${CMAKE_OSX_ARCHITECTURES};${CMAKE_SYSTEM_PROCESSOR};${MSVC_C_ARCHITECTURE_ID}"
  _luajit_platform)

if(_luajit_platform MATCHES "arm64|aarch64")
  set(LUAJIT_ARCH arm64)
elseif(_luajit_platform MATCHES "x86_64|amd64|x64" OR
       (CMAKE_SIZEOF_VOID_P EQUAL 8 AND NOT _luajit_platform MATCHES "arm"))
  set(LUAJIT_ARCH x64)
elseif(_luajit_platform MATCHES "i.?86|x86|win32")
  set(LUAJIT_ARCH x86)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(LUAJIT_ARCH x64)
else()
  set(LUAJIT_ARCH x86)
endif()

if(LUAJIT_DISABLE_GC64 AND NOT LUAJIT_ARCH STREQUAL "x64")
  message(FATAL_ERROR "LUAJIT_DISABLE_GC64 is only valid for an x64 target.")
endif()
if(APPLE AND LUAJIT_ARCH STREQUAL "x64" AND LUAJIT_DISABLE_GC64)
  message(FATAL_ERROR "macOS requires GC64; do not enable LUAJIT_DISABLE_GC64.")
endif()

set(LUAJIT_TARGET_DEFINITIONS "LUAJIT_TARGET=LUAJIT_ARCH_${LUAJIT_ARCH}")
if(WIN32)
  list(APPEND LUAJIT_TARGET_DEFINITIONS _CRT_SECURE_NO_DEPRECATE)
endif()
if(LUAJIT_DISABLE_GC64)
  list(APPEND LUAJIT_TARGET_DEFINITIONS LUAJIT_DISABLE_GC64)
endif()
if(LUAJIT_ENABLE_LUA52COMPAT)
  list(APPEND LUAJIT_TARGET_DEFINITIONS LUAJIT_ENABLE_LUA52COMPAT)
endif()
if(APPLE)
  # Makefile always sets these for Darwin.
  list(APPEND LUAJIT_TARGET_DEFINITIONS
    LUAJIT_UNWIND_EXTERNAL
    _FILE_OFFSET_BITS=64
    _LARGEFILE_SOURCE)
  add_compile_options(-U_FORTIFY_SOURCE)
  if(NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    message(FATAL_ERROR
      "macOS builds require CMAKE_OSX_DEPLOYMENT_TARGET (Makefile: MACOSX_DEPLOYMENT_TARGET).")
  endif()
endif()

# ---------------------------------------------------------------------------
# DynASM flags / VM backend (Makefile DASM_AFLAGS + DASM_ARCH)
# ---------------------------------------------------------------------------
set(DASM_FLAGS -LN)
if(WIN32)
  list(APPEND DASM_FLAGS -D WIN)
endif()
list(APPEND DASM_FLAGS -D JIT -D FFI)

# Host/target Windows & macOS are little-endian hard-float.
list(APPEND DASM_FLAGS -D ENDIAN_LE -D FPU -D HFABI)

if(LUAJIT_ARCH STREQUAL "x64")
  list(APPEND DASM_FLAGS -D P64)
  if(LUAJIT_DISABLE_GC64)
    # Without GC64, LJ_FR2=0 ¡ú Makefile switches DASM_ARCH to x86.
    set(DASM_ARCH x86)
  else()
    set(DASM_ARCH x64)
  endif()
elseif(LUAJIT_ARCH STREQUAL "arm64")
  list(APPEND DASM_FLAGS -D P64 -D DUALNUM -D VER=80)
  set(DASM_ARCH arm64)
else()
  set(DASM_ARCH x86)
endif()

set(DASM_SOURCE "${LUAJIT_SRC}/vm_${DASM_ARCH}.dasc")

# ---------------------------------------------------------------------------
# Rolling release version from .relver only
# ---------------------------------------------------------------------------
set(LUAJIT_RELVER_FILE "${GENERATED_DIR}/luajit_relver.txt")
set(_luajit_relver "")
if(EXISTS "${LUAJIT_ROOT}/.relver")
  file(STRINGS "${LUAJIT_ROOT}/.relver" _luajit_relver LIMIT_COUNT 1)
endif()
if(NOT _luajit_relver MATCHES "^[0-9]+$")
  set(_luajit_relver ROLLING)
endif()
file(WRITE "${LUAJIT_RELVER_FILE}" "${_luajit_relver}\n")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${LUAJIT_ROOT}/.relver")

# ---------------------------------------------------------------------------
# Host toolchain helpers used only in Generate/build custom commands
# ---------------------------------------------------------------------------
if(WIN32 AND LUAJIT_ARCH STREQUAL "arm64" AND NOT
   "${CMAKE_HOST_SYSTEM_PROCESSOR}" MATCHES "^(ARM64|arm64|aarch64|AARCH64)$")
  # Counterpart of msvcbuild.bat :SETHOSTVARS for x64¡úARM64 cross builds.
  if(CMAKE_GENERATOR_INSTANCE)
    set(_vs_root "${CMAKE_GENERATOR_INSTANCE}")
  elseif(DEFINED ENV{VSINSTALLDIR})
    set(_vs_root "$ENV{VSINSTALLDIR}")
  endif()
  set(_vsdevcmd "${_vs_root}/Common7/Tools/VsDevCmd.bat")
  if(NOT EXISTS "${_vsdevcmd}")
    message(FATAL_ERROR
      "ARM64 cross-build requires VsDevCmd.bat. Configure from a Visual Studio "
      "developer prompt or use a Visual Studio generator.")
  endif()
  set(HOST_ENV_CMD "${CMAKE_CURRENT_BINARY_DIR}/run_host_msvc.cmd")
  file(WRITE "${HOST_ENV_CMD}"
    "@echo off\r\n"
    "call \"${_vsdevcmd}\" -arch=x64 -host_arch=x64 -no_logo >nul\r\n"
    "if errorlevel 1 exit /b %errorlevel%\r\n"
    "%*\r\n")
  set(HOST_CC_COMMAND "${HOST_ENV_CMD}" cl)
else()
  set(HOST_CC_COMMAND "${CMAKE_C_COMPILER}")
endif()

if(WIN32)
  set(MINILUA_EXE "${GENERATED_HOST_DIR}/minilua.exe")
  set(BUILDVM_EXE "${GENERATED_HOST_DIR}/buildvm.exe")
  set(HOST_EXE_SUFFIX ".exe")
  set(HOST_OBJ_EXT "obj")
else()
  set(MINILUA_EXE "${GENERATED_HOST_DIR}/minilua")
  set(BUILDVM_EXE "${GENERATED_HOST_DIR}/buildvm")
  set(HOST_EXE_SUFFIX "")
  set(HOST_OBJ_EXT "o")
endif()

# minilua
if(MSVC)
  add_custom_command(
    OUTPUT "${MINILUA_EXE}"
    COMMAND ${HOST_CC_COMMAND} /nologo /O2 /W3 /MT
      /D_CRT_SECURE_NO_DEPRECATE
      "/D_CRT_STDIO_INLINE=__declspec(dllexport)__inline"
      "${LUAJIT_SRC}/host/minilua.c" "/Fe${MINILUA_EXE}"
      "/Fo${GENERATED_HOST_DIR}/minilua.${HOST_OBJ_EXT}"
      /link /nologo /INCREMENTAL:NO
      "/IMPLIB:${GENERATED_HOST_DIR}/minilua.lib"
    DEPENDS "${LUAJIT_SRC}/host/minilua.c"
    BYPRODUCTS
      "${GENERATED_HOST_DIR}/minilua.${HOST_OBJ_EXT}"
      "${GENERATED_HOST_DIR}/minilua.lib"
      "${GENERATED_HOST_DIR}/minilua.exp"
    COMMENT "Generating host tool minilua"
    VERBATIM)
else()
  add_custom_command(
    OUTPUT "${MINILUA_EXE}"
    COMMAND ${HOST_CC_COMMAND} -O2 -Wall
      -o "${MINILUA_EXE}" "${LUAJIT_SRC}/host/minilua.c" -lm
    DEPENDS "${LUAJIT_SRC}/host/minilua.c"
    COMMENT "Generating host tool minilua"
    VERBATIM)
endif()

# luajit.h (generated into the build tree; sources include it via GENERATED_DIR)
set(GENERATED_LUAJIT_H "${GENERATED_DIR}/luajit.h")
set(GENERATED_VERSION_STAMP "${GENERATED_DIR}/luajit_version.stamp")
add_custom_command(
  OUTPUT "${GENERATED_VERSION_STAMP}"
  COMMAND "${MINILUA_EXE}" "${LUAJIT_SRC}/host/genversion.lua"
    "${LUAJIT_SRC}/luajit_rolling.h"
    "${LUAJIT_RELVER_FILE}"
    "${GENERATED_LUAJIT_H}"
  COMMAND "${CMAKE_COMMAND}" -E touch "${GENERATED_VERSION_STAMP}"
  DEPENDS "${MINILUA_EXE}" "${LUAJIT_SRC}/host/genversion.lua"
    "${LUAJIT_SRC}/luajit_rolling.h" "${LUAJIT_RELVER_FILE}"
  COMMENT "Generating LuaJIT version header"
  VERBATIM)

# DynASM ¡ú host/buildvm_arch.h
set(GENERATED_ARCH_H "${GENERATED_HOST_DIR}/buildvm_arch.h")
add_custom_command(
  OUTPUT "${GENERATED_ARCH_H}"
  COMMAND "${MINILUA_EXE}" "${DYNASM_DIR}/dynasm.lua" ${DASM_FLAGS}
    -o "${GENERATED_ARCH_H}" "${DASM_SOURCE}"
  DEPENDS "${MINILUA_EXE}" "${DYNASM_DIR}/dynasm.lua" "${DASM_SOURCE}"
    "${LUAJIT_SRC}/lj_arch.h" "${LUAJIT_SRC}/lua.h" "${LUAJIT_SRC}/luaconf.h"
  COMMENT "Generating DynASM VM backend"
  VERBATIM)

set(BUILDVM_SOURCES
  "${LUAJIT_SRC}/host/buildvm.c"
  "${LUAJIT_SRC}/host/buildvm_asm.c"
  "${LUAJIT_SRC}/host/buildvm_peobj.c"
  "${LUAJIT_SRC}/host/buildvm_lib.c"
  "${LUAJIT_SRC}/host/buildvm_fold.c")
file(GLOB BUILDVM_DYNASM_HEADERS CONFIGURE_DEPENDS "${DYNASM_DIR}/dasm_*.h")
set(BUILDVM_DEPENDENCIES
  "${LUAJIT_SRC}/host/buildvm_libbc.h"
  ${BUILDVM_DYNASM_HEADERS})

set(BUILDVM_DEFINITION_FLAGS)
foreach(_definition IN LISTS LUAJIT_TARGET_DEFINITIONS)
  if(MSVC)
    list(APPEND BUILDVM_DEFINITION_FLAGS "/D${_definition}")
  else()
    list(APPEND BUILDVM_DEFINITION_FLAGS "-D${_definition}")
  endif()
endforeach()

set(BUILDVM_OBJECTS
  "${GENERATED_HOST_DIR}/buildvm.${HOST_OBJ_EXT}"
  "${GENERATED_HOST_DIR}/buildvm_asm.${HOST_OBJ_EXT}"
  "${GENERATED_HOST_DIR}/buildvm_peobj.${HOST_OBJ_EXT}"
  "${GENERATED_HOST_DIR}/buildvm_lib.${HOST_OBJ_EXT}"
  "${GENERATED_HOST_DIR}/buildvm_fold.${HOST_OBJ_EXT}")

if(MSVC)
  add_custom_command(
    OUTPUT "${BUILDVM_EXE}"
    COMMAND ${HOST_CC_COMMAND} /nologo /O2 /W3 /MT
      /D_CRT_SECURE_NO_DEPRECATE
      "/D_CRT_STDIO_INLINE=__declspec(dllexport)__inline"
      ${BUILDVM_DEFINITION_FLAGS}
      "/I${LUAJIT_SRC}" "/I${DYNASM_DIR}" "/I${GENERATED_HOST_DIR}" "/I${GENERATED_DIR}"
      ${BUILDVM_SOURCES} "/Fe${BUILDVM_EXE}" "/Fo${GENERATED_HOST_DIR}/"
      /link /nologo /INCREMENTAL:NO
      "/IMPLIB:${GENERATED_HOST_DIR}/buildvm.lib"
    DEPENDS ${BUILDVM_SOURCES} ${BUILDVM_DEPENDENCIES}
      "${GENERATED_ARCH_H}" "${GENERATED_VERSION_STAMP}"
    BYPRODUCTS
      ${BUILDVM_OBJECTS}
      "${GENERATED_HOST_DIR}/buildvm.lib"
      "${GENERATED_HOST_DIR}/buildvm.exp"
    COMMENT "Generating host tool buildvm"
    VERBATIM)
else()
  add_custom_command(
    OUTPUT "${BUILDVM_EXE}"
    COMMAND ${HOST_CC_COMMAND} -O2 -Wall
      ${BUILDVM_DEFINITION_FLAGS}
      "-I${LUAJIT_SRC}" "-I${DYNASM_DIR}" "-I${GENERATED_HOST_DIR}" "-I${GENERATED_DIR}"
      -o "${BUILDVM_EXE}" ${BUILDVM_SOURCES} -lm
    DEPENDS ${BUILDVM_SOURCES} ${BUILDVM_DEPENDENCIES}
      "${GENERATED_ARCH_H}" "${GENERATED_VERSION_STAMP}"
    COMMENT "Generating host tool buildvm"
    VERBATIM)
endif()

# ---------------------------------------------------------------------------
# buildvm-generated artifacts
# ---------------------------------------------------------------------------
set(LUAJIT_LIB_SOURCES
  lib_base.c lib_math.c lib_bit.c lib_string.c lib_table.c lib_io.c
  lib_os.c lib_package.c lib_debug.c lib_jit.c lib_ffi.c lib_buffer.c)
set(_luajit_lib_paths)
foreach(_source IN LISTS LUAJIT_LIB_SOURCES)
  list(APPEND _luajit_lib_paths "${LUAJIT_SRC}/${_source}")
endforeach()

set(GENERATED_BCDEF "${GENERATED_DIR}/lj_bcdef.h")
set(GENERATED_FFDEF "${GENERATED_DIR}/lj_ffdef.h")
set(GENERATED_LIBDEF "${GENERATED_DIR}/lj_libdef.h")
set(GENERATED_RECDEF "${GENERATED_DIR}/lj_recdef.h")
set(GENERATED_FOLDDEF "${GENERATED_DIR}/lj_folddef.h")
set(GENERATED_VMDEF "${GENERATED_JIT_DIR}/vmdef.lua")

if(WIN32)
  # Makefile / msvcbuild: peobj ¡ú prebuilt COFF object
  set(LJVM_MODE peobj)
  set(GENERATED_VM_OBJ "${GENERATED_DIR}/lj_vm.obj")
  set(LUAJIT_VM_LINK_FILE "${GENERATED_VM_OBJ}")
  set(LUAJIT_GENERATED_FILES
    "${GENERATED_VM_OBJ}" "${GENERATED_BCDEF}" "${GENERATED_FFDEF}"
    "${GENERATED_LIBDEF}" "${GENERATED_RECDEF}" "${GENERATED_FOLDDEF}"
    "${GENERATED_VMDEF}")
  add_custom_command(
    OUTPUT ${LUAJIT_GENERATED_FILES}
    COMMAND "${BUILDVM_EXE}" -m peobj -o "${GENERATED_VM_OBJ}"
    COMMAND "${BUILDVM_EXE}" -m bcdef -o "${GENERATED_BCDEF}" ${LUAJIT_LIB_SOURCES}
    COMMAND "${BUILDVM_EXE}" -m ffdef -o "${GENERATED_FFDEF}" ${LUAJIT_LIB_SOURCES}
    COMMAND "${BUILDVM_EXE}" -m libdef -o "${GENERATED_LIBDEF}" ${LUAJIT_LIB_SOURCES}
    COMMAND "${BUILDVM_EXE}" -m recdef -o "${GENERATED_RECDEF}" ${LUAJIT_LIB_SOURCES}
    COMMAND "${BUILDVM_EXE}" -m vmdef -o "${GENERATED_VMDEF}" ${LUAJIT_LIB_SOURCES}
    COMMAND "${BUILDVM_EXE}" -m folddef -o "${GENERATED_FOLDDEF}" lj_opt_fold.c
    WORKING_DIRECTORY "${LUAJIT_SRC}"
    DEPENDS "${BUILDVM_EXE}" ${_luajit_lib_paths} "${LUAJIT_SRC}/lj_opt_fold.c"
    COMMENT "Generating LuaJIT VM object and definitions"
    VERBATIM)
  set_source_files_properties("${GENERATED_VM_OBJ}" PROPERTIES
    GENERATED TRUE EXTERNAL_OBJECT TRUE)
else()
  # Makefile Darwin: machasm ¡ú lj_vm.S, then assemble with the C compiler
  # (clang -c). Do not add the .S as a LANGUAGE C/ASM source: Clang would
  # mis-parse assembler directives such as `.file` as C.
  set(LJVM_MODE machasm)
  set(GENERATED_VM_S "${GENERATED_DIR}/lj_vm.S")
  set(GENERATED_VM_O "${GENERATED_DIR}/lj_vm.o")
  set(LUAJIT_VM_LINK_FILE "${GENERATED_VM_O}")

  set(_luajit_asm_flags -O2 -fomit-frame-pointer)
  if(CMAKE_OSX_DEPLOYMENT_TARGET)
    list(APPEND _luajit_asm_flags
      "-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
  endif()
  if(CMAKE_OSX_ARCHITECTURES)
    foreach(_arch IN LISTS CMAKE_OSX_ARCHITECTURES)
      list(APPEND _luajit_asm_flags -arch "${_arch}")
    endforeach()
  elseif(LUAJIT_ARCH STREQUAL "arm64")
    list(APPEND _luajit_asm_flags -arch arm64)
  elseif(LUAJIT_ARCH STREQUAL "x64")
    list(APPEND _luajit_asm_flags -arch x86_64)
  endif()

  set(LUAJIT_GENERATED_HEADERS
    "${GENERATED_BCDEF}" "${GENERATED_FFDEF}"
    "${GENERATED_LIBDEF}" "${GENERATED_RECDEF}" "${GENERATED_FOLDDEF}"
    "${GENERATED_VMDEF}")
  add_custom_command(
    OUTPUT "${GENERATED_VM_S}" ${LUAJIT_GENERATED_HEADERS}
    COMMAND "${BUILDVM_EXE}" -m machasm -o "${GENERATED_VM_S}"
    COMMAND "${BUILDVM_EXE}" -m bcdef -o "${GENERATED_BCDEF}" ${LUAJIT_LIB_SOURCES}
    COMMAND "${BUILDVM_EXE}" -m ffdef -o "${GENERATED_FFDEF}" ${LUAJIT_LIB_SOURCES}
    COMMAND "${BUILDVM_EXE}" -m libdef -o "${GENERATED_LIBDEF}" ${LUAJIT_LIB_SOURCES}
    COMMAND "${BUILDVM_EXE}" -m recdef -o "${GENERATED_RECDEF}" ${LUAJIT_LIB_SOURCES}
    COMMAND "${BUILDVM_EXE}" -m vmdef -o "${GENERATED_VMDEF}" ${LUAJIT_LIB_SOURCES}
    COMMAND "${BUILDVM_EXE}" -m folddef -o "${GENERATED_FOLDDEF}" lj_opt_fold.c
    WORKING_DIRECTORY "${LUAJIT_SRC}"
    DEPENDS "${BUILDVM_EXE}" ${_luajit_lib_paths} "${LUAJIT_SRC}/lj_opt_fold.c"
    COMMENT "Generating LuaJIT VM assembly and definitions"
    VERBATIM)
  add_custom_command(
    OUTPUT "${GENERATED_VM_O}"
    COMMAND "${CMAKE_C_COMPILER}" ${_luajit_asm_flags}
      -c -o "${GENERATED_VM_O}" "${GENERATED_VM_S}"
    DEPENDS "${GENERATED_VM_S}"
    COMMENT "Assembling LuaJIT VM"
    VERBATIM)
  set_source_files_properties("${GENERATED_VM_O}" PROPERTIES
    GENERATED TRUE EXTERNAL_OBJECT TRUE)
endif()

# ---------------------------------------------------------------------------
# Core sources
# ---------------------------------------------------------------------------
set(LUAJIT_CORE_SOURCES
  lj_assert.c lj_gc.c lj_err.c lj_char.c lj_bc.c lj_obj.c lj_buf.c
  lj_str.c lj_tab.c lj_func.c lj_udata.c lj_meta.c lj_debug.c lj_prng.c
  lj_state.c lj_dispatch.c lj_vmevent.c lj_vmmath.c lj_strscan.c
  lj_strfmt.c lj_strfmt_num.c lj_serialize.c lj_api.c lj_profile.c
  lj_lex.c lj_parse.c lj_bcread.c lj_bcwrite.c lj_load.c lj_ir.c
  lj_opt_mem.c lj_opt_fold.c lj_opt_narrow.c lj_opt_dce.c lj_opt_loop.c
  lj_opt_split.c lj_opt_sink.c lj_mcode.c lj_snap.c lj_record.c
  lj_crecord.c lj_ffrecord.c lj_asm.c lj_trace.c lj_gdbjit.c lj_ctype.c
  lj_cdata.c lj_cconv.c lj_ccall.c lj_ccallback.c lj_carith.c lj_clib.c
  lj_cparse.c lj_lib.c lj_alloc.c lib_aux.c lib_base.c lib_math.c
  lib_bit.c lib_string.c lib_table.c lib_io.c lib_os.c lib_package.c
  lib_debug.c lib_jit.c lib_ffi.c lib_buffer.c lib_init.c)
list(TRANSFORM LUAJIT_CORE_SOURCES PREPEND "${LUAJIT_SRC}/")

if(LUAJIT_AMALGAMATED)
  set(LUAJIT_BUILD_SOURCES "${LUAJIT_SRC}/ljamalg.c")
else()
  set(LUAJIT_BUILD_SOURCES ${LUAJIT_CORE_SOURCES})
endif()

# ---------------------------------------------------------------------------
# Library target(s)
# ---------------------------------------------------------------------------
# Prefer no stack protector when the toolchain supports it (Makefile).
if(NOT MSVC)
  include(CheckCCompilerFlag)
  check_c_compiler_flag(-fno-stack-protector LUAJIT_HAVE_NO_STACK_PROTECTOR)
endif()

function(luajit_configure_lib_target target)
  target_sources(${target} PRIVATE
    "${GENERATED_BCDEF}" "${GENERATED_FFDEF}" "${GENERATED_LIBDEF}"
    "${GENERATED_RECDEF}" "${GENERATED_FOLDDEF}")
  target_include_directories(${target}
    PUBLIC
      "$<BUILD_INTERFACE:${GENERATED_DIR}>"
      "$<BUILD_INTERFACE:${LUAJIT_SRC}>"
      "$<INSTALL_INTERFACE:include>")
  target_compile_definitions(${target} PRIVATE ${LUAJIT_TARGET_DEFINITIONS})

  if(MSVC)
    set_property(TARGET ${target} PROPERTY MSVC_RUNTIME_CHECKS "")
    target_compile_options(${target} PRIVATE
      /nologo /O2 /W3 /Zi
      "/D_CRT_STDIO_INLINE=__declspec(dllexport)__inline")
    if(LUAJIT_ARCH STREQUAL "x86")
      target_compile_options(${target} PRIVATE /arch:SSE2)
    endif()
  else()
    target_compile_options(${target} PRIVATE -O2 -fomit-frame-pointer -Wall)
    if(LUAJIT_ARCH STREQUAL "x86")
      target_compile_options(${target} PRIVATE -march=i686 -msse -msse2 -mfpmath=sse)
    endif()
    if(LUAJIT_HAVE_NO_STACK_PROTECTOR)
      target_compile_options(${target} PRIVATE -fno-stack-protector)
    endif()
  endif()
endfunction()

if(WIN32)
  if(LUAJIT_BUILD_MODE STREQUAL "dynamic")
    add_library(lua51 SHARED ${LUAJIT_BUILD_SOURCES} "${LUAJIT_VM_LINK_FILE}")
  else()
    add_library(lua51 STATIC ${LUAJIT_BUILD_SOURCES} "${LUAJIT_VM_LINK_FILE}")
  endif()
  luajit_configure_lib_target(lua51)

  if(NOT LUAJIT_BUILD_MODE STREQUAL "static")
    target_compile_definitions(lua51 PRIVATE LUA_BUILD_AS_DLL)
  else()
    set_property(TARGET lua51 APPEND PROPERTY STATIC_LIBRARY_OPTIONS /NODEFAULTLIB)
  endif()
  set_target_properties(lua51 PROPERTIES PREFIX "" OUTPUT_NAME lua51)
  target_link_options(lua51 PRIVATE
    $<$<STREQUAL:${LUAJIT_BUILD_MODE},dynamic>:/DEBUG>
    $<$<STREQUAL:${LUAJIT_BUILD_MODE},dynamic>:/OPT:REF>
    $<$<STREQUAL:${LUAJIT_BUILD_MODE},dynamic>:/OPT:ICF>
    $<$<STREQUAL:${LUAJIT_BUILD_MODE},dynamic>:/INCREMENTAL:NO>
    $<$<AND:$<STREQUAL:${LUAJIT_BUILD_MODE},dynamic>,$<NOT:$<CONFIG:Debug>>>:/RELEASE>)

  set(LUAJIT_LINK_LIB lua51)
  set(LUAJIT_INSTALL_LIBS lua51)
else()
  # Use the same lua51 basename as Windows (liblua51.a / liblua51.dylib).
  if(NOT CMAKE_INSTALL_PREFIX STREQUAL "/usr/local" AND NOT CMAKE_INSTALL_PREFIX STREQUAL "/usr")
    list(APPEND LUAJIT_TARGET_DEFINITIONS
      "LUA_ROOT=\"${CMAKE_INSTALL_PREFIX}\""
      "LUA_LJDIR=\"${CMAKE_INSTALL_PREFIX}/share/luajit-2.1\"")
  elseif(NOT CMAKE_INSTALL_PREFIX STREQUAL "/usr/local")
    list(APPEND LUAJIT_TARGET_DEFINITIONS "LUA_ROOT=\"${CMAKE_INSTALL_PREFIX}\"")
  endif()

  if(LUAJIT_BUILD_MODE STREQUAL "dynamic")
    add_library(lua51 SHARED ${LUAJIT_BUILD_SOURCES} "${LUAJIT_VM_LINK_FILE}")
    set(LUAJIT_LINK_LIB lua51)
    set(LUAJIT_INSTALL_LIBS lua51)
  elseif(LUAJIT_BUILD_MODE STREQUAL "static")
    add_library(lua51 STATIC ${LUAJIT_BUILD_SOURCES} "${LUAJIT_VM_LINK_FILE}")
    set(LUAJIT_LINK_LIB lua51)
    set(LUAJIT_INSTALL_LIBS lua51)
  else()
    # mixed: static archive + dylib; executable links the static archive
    add_library(lua51 STATIC ${LUAJIT_BUILD_SOURCES} "${LUAJIT_VM_LINK_FILE}")
    add_library(libluajit_shared SHARED ${LUAJIT_BUILD_SOURCES} "${LUAJIT_VM_LINK_FILE}")
    set_target_properties(libluajit_shared PROPERTIES OUTPUT_NAME lua51)
    luajit_configure_lib_target(libluajit_shared)
    target_link_libraries(libluajit_shared PRIVATE m)
    target_link_options(libluajit_shared PRIVATE
      -undefined dynamic_lookup
      "LINKER:-install_name,${CMAKE_INSTALL_PREFIX}/lib/liblua51.dylib"
      "LINKER:-compatibility_version,2.1"
      "LINKER:-current_version,2.1.255")
    set_target_properties(libluajit_shared PROPERTIES MACOSX_RPATH ON)
    set(LUAJIT_LINK_LIB lua51)
    set(LUAJIT_INSTALL_LIBS lua51 libluajit_shared)
  endif()

  luajit_configure_lib_target(lua51)
  target_link_libraries(lua51 PRIVATE m)
  set_target_properties(lua51 PROPERTIES OUTPUT_NAME lua51)

  if(LUAJIT_BUILD_MODE STREQUAL "dynamic")
    target_link_options(lua51 PRIVATE
      -undefined dynamic_lookup
      "LINKER:-install_name,${CMAKE_INSTALL_PREFIX}/lib/liblua51.dylib"
      "LINKER:-compatibility_version,2.1"
      "LINKER:-current_version,2.1.255")
    set_target_properties(lua51 PROPERTIES MACOSX_RPATH ON)
  endif()
endif()

# ---------------------------------------------------------------------------
# luajit executable
# ---------------------------------------------------------------------------
add_executable(luajit "${LUAJIT_SRC}/luajit.c")
set_source_files_properties("${LUAJIT_SRC}/luajit.c" PROPERTIES
  OBJECT_DEPENDS "${GENERATED_VERSION_STAMP}")
target_link_libraries(luajit PRIVATE ${LUAJIT_LINK_LIB})
target_include_directories(luajit PRIVATE "${GENERATED_DIR}" "${LUAJIT_SRC}")
target_compile_definitions(luajit PRIVATE ${LUAJIT_TARGET_DEFINITIONS})

if(MSVC)
  set_property(TARGET luajit PROPERTY MSVC_RUNTIME_CHECKS "")
  target_compile_options(luajit PRIVATE
    /nologo /O2 /W3 /Zi
    "/D_CRT_STDIO_INLINE=__declspec(dllexport)__inline")
  if(LUAJIT_ARCH STREQUAL "x86")
    target_compile_options(luajit PRIVATE /arch:SSE2)
  endif()
  target_link_options(luajit PRIVATE
    /DEBUG /OPT:REF /OPT:ICF /INCREMENTAL:NO
    $<$<NOT:$<CONFIG:Debug>>:/RELEASE>)
else()
  target_compile_options(luajit PRIVATE -O2 -fomit-frame-pointer -Wall)
  target_link_libraries(luajit PRIVATE m)
endif()

# Build-tree layout: jit/*.lua next to the executable (Windows-style and
# convenient for local runs; install uses platform-appropriate paths).
add_custom_command(TARGET luajit POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy_directory
    "${LUAJIT_SRC}/jit" "$<TARGET_FILE_DIR:luajit>/jit"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different
    "${GENERATED_VMDEF}" "$<TARGET_FILE_DIR:luajit>/jit/vmdef.lua"
  VERBATIM)
