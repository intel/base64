add_library(base64
    base64_cpuid.c
    base64_decode.c
    base64_decode_alloc.c
    base64_encode.c
    base64_encode_alloc.c
    base64_encode_alphabet.c
    base64_encode_plain.c

    base64_encode_p.h
    base64_byteorder_p.h
    )
set_property(TARGET base64 PROPERTY PUBLIC_HEADER
    base64.h
    )

set_target_properties(base64 PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED 99
    C_EXTENSIONS OFF
    C_VISIBILITY_PRESET hidden
    FRAMEWORK OFF
    SOVERSION 0
    VERSION 0.1.0)

if(IFUNC_SUPPORTED)
    add_definitions(-DHAVE_IFUNC)
endif()
if(ATOMICS_SUPPORTED)
    add_definitions(-DHAVE_ATOMICS)
endif()
if(SSSE3_SUPPORTED)
    add_definitions(-DHAVE_SSSE3)
    set_source_files_properties(base64_encode_ssse3.c PROPERTIES COMPILE_FLAGS ${SSSE3_CFLAG})
    target_sources(base64 PRIVATE base64_encode_ssse3.c)
endif()
if(AVX2_SUPPORTED)
    add_definitions(-DHAVE_AVX2)
    set_source_files_properties(base64_encode_avx2.c PROPERTIES COMPILE_FLAGS ${AVX2_CFLAG})
    target_sources(base64 PRIVATE base64_encode_avx2.c)
endif()

if(BUILD_SHARED_LIBS)
    if(LFLAG_NO_UNDEFINED_SUPPORTED)
        set_property(TARGET base64 APPEND_STRING PROPERTY LINK_FLAGS "-Wl,-z,defs ")
    endif()
    if(LFLAG_O1)
        set_property(TARGET base64 APPEND_STRING PROPERTY LINK_FLAGS "-Wl,-O1 ")
    endif()
else()
    add_definitions(-DLIBBASE64_API=)
endif()

install(TARGETS base64 RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
    INCLUDES DESTINATION include)
