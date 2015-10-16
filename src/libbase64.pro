QT       -= core gui
CONFIG += use_c_linker

TARGET = base64
TEMPLATE = lib

SOURCES += \
    base64_encode_alphabet.c \
    base64_cpuid.c \
    base64_decode.c \
    base64_decode_alloc.c \
    base64_encode.c \
    base64_encode_alloc.c \
    base64_encode_plain.c

ssse3 {
    DEFINES += HAVE_SSSE3
    SOURCES += base64_encode_ssse3.c
}
avx2 {
    DEFINES += HAVE_AVX2
    SOURCES += base64_encode_avx2.c
}

HEADERS += base64.h base64_encode_p.h base64_byteorder_p.h
QMAKE_CFLAGS += -std=c11

unix {
    target.path = /usr/lib
    INSTALLS += target
    gcc {
        DEFINES += HAVE_IFUNC
        !mac: QMAKE_LFLAGS += -Wl,--no-undefined -Wl,-nostdlib
        QMAKE_CFLAGS += -fno-asynchronous-unwind-tables -fno-dwarf2-cfi-asm
        CONFIG -= force_debug_info
    }
}
