project(libbase64 C)
cmake_minimum_required(VERSION 3.0)
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0063 NEW)

# Check some features we need from the compiler
include(CheckCCompilerFlag)
include(CheckCSourceCompiles)
include(CMakePushCheckState)

macro(check_simd_support NAME cflag cflag_msvc snippet)
    if("${cflag}" STREQUAL "")
        set(${NAME}_CFLAG_SUPPORTED 1)
        set(${NAME}_CFLAG "")
    else()
        if (MSVC)
            set(${NAME}_CFLAG ${cflag_msvc})
        else()
            set(${NAME}_CFLAG ${cflag})
        endif()
        check_c_compiler_flag(${${NAME}_CFLAG} ${NAME}_CFLAG_SUPPORTED)
    endif()
    if (${NAME}_CFLAG_SUPPORTED)
        cmake_push_check_state(RESET)
        set(CMAKE_REQUIRED_FLAGS ${${NAME}_CFLAG})
        check_c_source_compiles("${snippet}" ${NAME}_SUPPORTED)
        cmake_pop_check_state()
    endif()
endmacro()
check_simd_support(AVX2 "-mavx2" "-arch:AVX2"
    "#include <immintrin.h>\n\
    void f(void *ptr) { _mm256_broadcastsi128_si256(* (const __m128i *)ptr); } int main() {}")
check_simd_support(SSSE3 "-mssse3" ""
    "#include <tmmintrin.h>\n\
    __m128i f(__m128i a, __m128i b) { return _mm_alignr_epi8(a, b, 8); } int main() {}")

check_c_source_compiles([=[
    void f() __attribute__((ifunc("resolve_f")));
    void f1() {}
    void resolve_f() { return f1; }
    int main() { f(); }
    ]=] IFUNC_SUPPORTED)

check_c_source_compiles([=[#include <stdatomic.h>
    typedef void (*fptr)();
    static _Atomic fptr f = ATOMIC_VAR_INIT((fptr)0);
    int main() { f = (fptr)0; }
    ]=] ATOMICS_SUPPORTED)

cmake_push_check_state(RESET)
set(CMAKE_REQUIRED_FLAGS "-shared -Wl,-z,defs")
check_c_source_compiles("void f() {}" LFLAG_NO_UNDEFINED_SUPPORTED)

set(CMAKE_REQUIRED_FLAGS "-shared -Wl,-O1")
check_c_source_compiles("void f() {}" LFLAG_O1)
cmake_pop_check_state()

add_subdirectory(src)
