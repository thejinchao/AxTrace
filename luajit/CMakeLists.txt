#cmake_minimum_required(VERSION 3.20)

#project(LuaJIT VERSION 2.1 LANGUAGES C)

if(NOT WIN32 OR NOT MSVC)
  message(FATAL_ERROR "This CMake build is the MSVC/Windows equivalent of src/msvcbuild.bat.")
endif()

#set(LUAJIT_BUILD_MODE "dynamic" CACHE STRING
#    "Library mode: dynamic, static, or mixed (static library built with DLL ABI/CRT)")
#set_property(CACHE LUAJIT_BUILD_MODE PROPERTY STRINGS dynamic static mixed)
set(LUAJIT_BUILD_MODE "static")
#option(LUAJIT_AMALGAMATED "Build the amalgamated ljamalg.c source" OFF)
set(LUAJIT_AMALGAMATED OFF)
#option(LUAJIT_DISABLE_GC64 "Disable GC64 on x64" OFF)
set(LUAJIT_DISABLE_GC64 OFF)
#option(LUAJIT_ENABLE_LUA52COMPAT "Enable the extra Lua 5.2 compatibility features" OFF)
set(LUAJIT_ENABLE_LUA52COMPAT OFF)

#if(NOT LUAJIT_BUILD_MODE MATCHES "^(dynamic|static|mixed)$")
#  message(FATAL_ERROR "LUAJIT_BUILD_MODE must be dynamic, static, or mixed.")
#endif()

set(LUAJIT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(LUAJIT_SRC "${LUAJIT_ROOT}/src")
set(DYNASM_DIR "${LUAJIT_ROOT}/dynasm")
set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(GENERATED_HOST_DIR "${GENERATED_DIR}/host")
set(GENERATED_JIT_DIR "${GENERATED_DIR}/jit")
file(MAKE_DIRECTORY "${GENERATED_HOST_DIR}" "${GENERATED_JIT_DIR}")

# Match msvcbuild.bat's target selection. CMAKE_GENERATOR_PLATFORM is decisive
# for Visual Studio cross-builds; the remaining checks cover Ninja/NMake.
string(TOLOWER "${CMAKE_GENERATOR_PLATFORM};${CMAKE_SYSTEM_PROCESSOR};${MSVC_C_ARCHITECTURE_ID}"
       _luajit_platform)
if(_luajit_platform MATCHES "arm64|aarch64")
  set(LUAJIT_ARCH arm64)
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(LUAJIT_ARCH x64)
else()
  set(LUAJIT_ARCH x86)
endif()

if(LUAJIT_DISABLE_GC64 AND NOT LUAJIT_ARCH STREQUAL "x64")
  message(FATAL_ERROR "LUAJIT_DISABLE_GC64 is only valid for an x64 target.")
endif()

set(LUAJIT_TARGET_DEFINITIONS
    _CRT_SECURE_NO_DEPRECATE
    "LUAJIT_TARGET=LUAJIT_ARCH_${LUAJIT_ARCH}")
if(LUAJIT_DISABLE_GC64)
  list(APPEND LUAJIT_TARGET_DEFINITIONS LUAJIT_DISABLE_GC64)
endif()
if(LUAJIT_ENABLE_LUA52COMPAT)
  list(APPEND LUAJIT_TARGET_DEFINITIONS LUAJIT_ENABLE_LUA52COMPAT)
endif()

set(DASM_FLAGS -LN -D WIN -D JIT -D FFI -D ENDIAN_LE -D FPU)
if(LUAJIT_ARCH STREQUAL "x64")
  list(APPEND DASM_FLAGS -D P64)
  if(LUAJIT_DISABLE_GC64)
    set(DASM_SOURCE "${LUAJIT_SRC}/vm_x86.dasc")
  else()
    set(DASM_SOURCE "${LUAJIT_SRC}/vm_x64.dasc")
  endif()
elseif(LUAJIT_ARCH STREQUAL "arm64")
  list(APPEND DASM_FLAGS -D P64)
  set(DASM_SOURCE "${LUAJIT_SRC}/vm_arm64.dasc")
else()
  set(DASM_SOURCE "${LUAJIT_SRC}/vm_x86.dasc")
endif()

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

# ARM64 cross-builds cannot execute ARM64 minilua/buildvm on the x64 build
# machine. This is the CMake counterpart of :SETHOSTVARS in msvcbuild.bat.
if(LUAJIT_ARCH STREQUAL "arm64" AND NOT
   "${CMAKE_HOST_SYSTEM_PROCESSOR}" MATCHES "^(ARM64|arm64|aarch64|AARCH64)$")
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

set(MINILUA_EXE "${GENERATED_HOST_DIR}/minilua.exe")
add_custom_command(
  OUTPUT "${MINILUA_EXE}"
  COMMAND ${HOST_CC_COMMAND} /nologo /O2 /W3 /MT
    /D_CRT_SECURE_NO_DEPRECATE
    "/D_CRT_STDIO_INLINE=__declspec(dllexport)__inline"
    "${LUAJIT_SRC}/host/minilua.c" "/Fe${MINILUA_EXE}"
    "/Fo${GENERATED_HOST_DIR}/minilua.obj"
    /link /nologo /INCREMENTAL:NO
    "/IMPLIB:${GENERATED_HOST_DIR}/minilua.lib"
  DEPENDS "${LUAJIT_SRC}/host/minilua.c"
  BYPRODUCTS
    "${GENERATED_HOST_DIR}/minilua.obj"
    "${GENERATED_HOST_DIR}/minilua.lib"
    "${GENERATED_HOST_DIR}/minilua.exp"
  COMMENT "Generating host tool minilua"
  VERBATIM)

# genversion.lua must update src/luajit.h in place, just like msvcbuild.bat.
# Many source files include it with quotes after defining per-file macros, so
# forcing a binary-tree copy with /FI would change LUA_API linkage semantics.
set(GENERATED_LUAJIT_H "${LUAJIT_SRC}/luajit.h")
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
  list(APPEND BUILDVM_DEFINITION_FLAGS "/D${_definition}")
endforeach()

set(BUILDVM_EXE "${GENERATED_HOST_DIR}/buildvm.exe")
set(BUILDVM_OBJECTS
  "${GENERATED_HOST_DIR}/buildvm.obj"
  "${GENERATED_HOST_DIR}/buildvm_asm.obj"
  "${GENERATED_HOST_DIR}/buildvm_peobj.obj"
  "${GENERATED_HOST_DIR}/buildvm_lib.obj"
  "${GENERATED_HOST_DIR}/buildvm_fold.obj")
add_custom_command(
  OUTPUT "${BUILDVM_EXE}"
  COMMAND ${HOST_CC_COMMAND} /nologo /O2 /W3 /MT
    /D_CRT_SECURE_NO_DEPRECATE
    "/D_CRT_STDIO_INLINE=__declspec(dllexport)__inline"
    ${BUILDVM_DEFINITION_FLAGS}
    "/I${LUAJIT_SRC}" "/I${DYNASM_DIR}" "/I${GENERATED_HOST_DIR}"
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

set(LUAJIT_LIB_SOURCES
  lib_base.c lib_math.c lib_bit.c lib_string.c lib_table.c lib_io.c
  lib_os.c lib_package.c lib_debug.c lib_jit.c lib_ffi.c lib_buffer.c)
set(_luajit_lib_paths)
foreach(_source IN LISTS LUAJIT_LIB_SOURCES)
  list(APPEND _luajit_lib_paths "${LUAJIT_SRC}/${_source}")
endforeach()

set(GENERATED_VM_OBJ "${GENERATED_DIR}/lj_vm.obj")
set(GENERATED_BCDEF "${GENERATED_DIR}/lj_bcdef.h")
set(GENERATED_FFDEF "${GENERATED_DIR}/lj_ffdef.h")
set(GENERATED_LIBDEF "${GENERATED_DIR}/lj_libdef.h")
set(GENERATED_RECDEF "${GENERATED_DIR}/lj_recdef.h")
set(GENERATED_FOLDDEF "${GENERATED_DIR}/lj_folddef.h")
set(GENERATED_VMDEF "${GENERATED_JIT_DIR}/vmdef.lua")
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

set_source_files_properties("${GENERATED_VM_OBJ}" PROPERTIES
  GENERATED TRUE EXTERNAL_OBJECT TRUE)
if(LUAJIT_BUILD_MODE STREQUAL "dynamic")
  add_library(lua51 SHARED ${LUAJIT_BUILD_SOURCES} "${GENERATED_VM_OBJ}")
else()
  add_library(lua51 STATIC ${LUAJIT_BUILD_SOURCES} "${GENERATED_VM_OBJ}")
endif()
set_property(TARGET lua51 PROPERTY MSVC_RUNTIME_CHECKS "")

target_sources(lua51 PRIVATE
  "${GENERATED_BCDEF}" "${GENERATED_FFDEF}" "${GENERATED_LIBDEF}"
  "${GENERATED_RECDEF}" "${GENERATED_FOLDDEF}")
target_include_directories(lua51
  PUBLIC
    "$<BUILD_INTERFACE:${GENERATED_DIR}>"
    "$<BUILD_INTERFACE:${LUAJIT_SRC}>"
    "$<INSTALL_INTERFACE:include>")
target_compile_definitions(lua51 PRIVATE ${LUAJIT_TARGET_DEFINITIONS})
target_compile_options(lua51 PRIVATE
  /nologo /O2 /W3 /Zi
  "/D_CRT_STDIO_INLINE=__declspec(dllexport)__inline")
if(LUAJIT_ARCH STREQUAL "x86")
  target_compile_options(lua51 PRIVATE /arch:SSE2)
endif()
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

