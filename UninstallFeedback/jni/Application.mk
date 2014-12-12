# Build both ARMv5TE and ARMv7-A machine code.  armeabi-v7a
APP_ABI := armeabi armeabi-v7a mips x86
APP_PLATFORM := android-8
APP_CPPFLAGS += -frtti -fexceptions
APP_STL := gnustl_static