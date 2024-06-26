if(MSVC)
  if(DEFINED ENV{MAXTHREADS})
    set(MP_FLAG "/MP$ENV{MAXTHREADS}")
  else()
    set(MP_FLAG "/MP")
  endif()
  set(CMAKE_C_FLAGS "/D_UNICODE /DUNICODE /utf-8 /DRPC_USE_NATIVE_WCHAR ${MP_FLAG} /DWIN32 /D_WINDOWS /W3 /Zi")
  set(CMAKE_C_FLAGS_DEBUG "/D_DEBUG /MDd /Ob0 /Od /RTC1 /D_ITERATOR_DEBUG_LEVEL=0")
  set(CMAKE_C_FLAGS_RELEASE "/MD /Ox /Ob2 /Oi /Ot /Oy /GL /DNDEBUG")
endif()

if("$ENV{Platform}" STREQUAL "x86")
  string(APPEND CMAKE_C_FLAGS " /arch:SSE2")
endif()
