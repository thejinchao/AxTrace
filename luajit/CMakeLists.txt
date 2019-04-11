cmake_minimum_required (VERSION 3.0)

set(TARGET_NAME lua51)

set(CMAKE_DEBUG_POSTFIX d)
set(TEMP_DIR _build)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-DLUA_BUILD_AS_DLL)
    add_definitions(-D_CRT_STDIO_INLINE=__declspec\(dllexport\)__inline)
endif()

set(LUAJIT_MINILUA_SRC
    host/minilua.c
)

set(LUAJIT_BUILDVM_SRC
    host/buildvm.c
    host/buildvm_asm.c
    host/buildvm_asm.c
    host/buildvm_fold.c
    host/buildvm_lib.c
    host/buildvm_peobj.c
)

set(LUAJIT_LJ_C_FILES
	lj_alloc.c
	lj_api.c
	lj_asm.c
	lj_bc.c
	lj_bcread.c
	lj_bcwrite.c
	lj_carith.c
	lj_ccall.c
	lj_ccallback.c
	lj_cconv.c
	lj_cdata.c
	lj_char.c
	lj_clib.c
	lj_cparse.c
	lj_crecord.c
	lj_ctype.c
	lj_debug.c
	lj_dispatch.c
	lj_err.c
	lj_ffrecord.c
	lj_func.c
	lj_gc.c
	lj_gdbjit.c
	lj_ir.c
	lj_lex.c
	lj_lib.c
	lj_load.c
	lj_mcode.c
	lj_meta.c
	lj_obj.c
	lj_opt_dce.c
	lj_opt_fold.c
	lj_opt_loop.c
	lj_opt_mem.c
	lj_opt_narrow.c
	lj_opt_sink.c
	lj_opt_split.c
	lj_parse.c
	lj_record.c
	lj_snap.c
	lj_state.c
	lj_str.c
	lj_strscan.c
	lj_tab.c
	lj_trace.c
	lj_udata.c
	lj_vmevent.c
	lj_vmmath.c
)

set(LUAJIT_LIB_C_FILES
	lib_aux.c
	lib_base.c
	lib_bit.c
	lib_debug.c
	lib_ffi.c
	lib_init.c
	lib_io.c
	lib_jit.c
	lib_math.c
	lib_os.c
	lib_package.c
	lib_string.c
	lib_table.c
)

set(LUAJIT_H_FILES
	lauxlib.h
	lj_alloc.h
	lj_arch.h
	lj_asm.h
	lj_asm_arm.h
	lj_asm_mips.h
	lj_asm_ppc.h
	lj_asm_x86.h
	lj_bc.h
	lj_bcdef.h
	lj_bcdump.h
	lj_carith.h
	lj_ccall.h
	lj_ccallback.h
	lj_cconv.h
	lj_cdata.h
	lj_char.h
	lj_clib.h
	lj_cparse.h
	lj_crecord.h
	lj_ctype.h
	lj_debug.h
	lj_def.h
	lj_dispatch.h
	lj_emit_arm.h
	lj_emit_mips.h
	lj_emit_ppc.h
	lj_emit_x86.h
	lj_err.h
	lj_errmsg.h
	lj_ff.h
	lj_ffdef.h
	lj_ffrecord.h
	lj_folddef.h
	lj_frame.h
	lj_func.h
	lj_gc.h
	lj_gdbjit.h
	lj_ir.h
	lj_ircall.h
	lj_iropt.h
	lj_jit.h
	lj_lex.h
	lj_lib.h
	lj_libdef.h
	lj_mcode.h
	lj_meta.h
	lj_obj.h
	lj_parse.h
	lj_recdef.h
	lj_record.h
	lj_snap.h
	lj_state.h
	lj_str.h
	lj_strscan.h
	lj_tab.h
	lj_target.h
	lj_target_arm.h
	lj_target_mips.h
	lj_target_ppc.h
	lj_target_x86.h
	lj_trace.h
	lj_traceerr.h
	lj_udata.h
	lj_vm.h
	lj_vmevent.h
	lua.h
	luaconf.h
	luajit.h
	lualib.h
)

set(LUAJIT_ALL_LIBS
	lib_base.c lib_math.c lib_bit.c lib_string.c lib_table.c lib_io.c lib_os.c lib_package.c lib_debug.c lib_jit.c lib_ffi.c
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/${TEMP_DIR}
    src
    dynasm
)

add_executable(minilua
    ${LUAJIT_MINILUA_SRC}
)
set_property(TARGET minilua PROPERTY FOLDER "luajit")


add_custom_command(TARGET minilua
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${TEMP_DIR}
	COMMAND $<TARGET_FILE:minilua>
	ARGS ${CMAKE_CURRENT_SOURCE_DIR}/dynasm/dynasm.lua -LN -D WIN -D JIT -D FFI -D P64 -o ${CMAKE_CURRENT_BINARY_DIR}/${TEMP_DIR}/buildvm_arch.h  ${CMAKE_CURRENT_SOURCE_DIR}/vm_x86.dasc
)

add_executable(buildvm
    ${LUAJIT_BUILDVM_SRC}
)
set_property(TARGET buildvm PROPERTY FOLDER "luajit")

add_dependencies(buildvm minilua)

add_custom_command(TARGET buildvm
	POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/${TEMP_DIR}

	COMMAND $<TARGET_FILE:buildvm>
	ARGS -m peobj -o ${CMAKE_CURRENT_BINARY_DIR}/${TEMP_DIR}/lj_vm.obj
	
	COMMAND $<TARGET_FILE:buildvm>
	ARGS -m bcdef -o ${CMAKE_CURRENT_BINARY_DIR}/${TEMP_DIR}/lj_bcdef.h  ${LUAJIT_ALL_LIBS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}

	COMMAND $<TARGET_FILE:buildvm>
	ARGS -m ffdef -o ${CMAKE_CURRENT_BINARY_DIR}/${TEMP_DIR}/lj_ffdef.h  ${LUAJIT_ALL_LIBS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}

	COMMAND $<TARGET_FILE:buildvm>
	ARGS -m libdef -o ${CMAKE_CURRENT_BINARY_DIR}/${TEMP_DIR}/lj_libdef.h  ${LUAJIT_ALL_LIBS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}

	COMMAND $<TARGET_FILE:buildvm>
	ARGS -m recdef -o ${CMAKE_CURRENT_BINARY_DIR}/${TEMP_DIR}/lj_recdef.h  ${LUAJIT_ALL_LIBS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}

	COMMAND $<TARGET_FILE:buildvm>
	ARGS -m vmdef -o ${CMAKE_CURRENT_BINARY_DIR}/${TEMP_DIR}/lj_vmdef.lua  ${LUAJIT_ALL_LIBS}
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}

	COMMAND $<TARGET_FILE:buildvm>
	ARGS -m folddef -o ${CMAKE_CURRENT_BINARY_DIR}/${TEMP_DIR}/lj_folddef.h lj_opt_fold.c
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(${TARGET_NAME}
	${LUAJIT_LJ_C_FILES} 
	${LUAJIT_LIB_C_FILES}
)
set_property(TARGET ${TARGET_NAME} PROPERTY FOLDER "luajit")
add_dependencies(${TARGET_NAME} buildvm minilua)
