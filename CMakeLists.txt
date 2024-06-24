cmake_minimum_required(VERSION 3.27)
project(eddy CXX)

message("System: ${CMAKE_SYSTEM_NAME}. Processor: ${CMAKE_SYSTEM_PROCESSOR}. Version: ${CMAKE_SYSTEM_VERSION}. Compiler: ${CMAKE_CXX_COMPILER_ID}.")

# if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
# message(FATAL_ERROR "Eddy requires Clang because it used C++ language extensions not supported in GCC")
# endif()
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 23)

if(APPLE)
  set(IS_APPLE TRUE)
  set(OS macos)
elseif(WIN32)
  set(IS_WINDOWS TRUE)
  set(OS windows)
else()
  set(IS_LINUX TRUE)
  set(OS linux)
endif()

set(CMAKE_INSTALL_PREFIX ${PROJECT_BINARY_DIR} CACHE PATH "..." FORCE)

set(eddy_NAME Eddy)
set(eddy_VERSION_MAJOR 1)
set(eddy_VERSION_MINOR 0)
set(eddy_DIR ${CMAKE_INSTALL_PREFIX})
set(eddy_DATADIR ${CMAKE_INSTALL_PREFIX}/share)

if(DEFINED RAYLIB)
  set(raylib_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}/${RAYLIB}")

  # FIXME abstract out Apple bits
  set(raylib_LIBRARIES "-framework IOKit -framework Cocoa -framework OpenGL -L${PROJECT_SOURCE_DIR}/${RAYLIB} -lraylib -lSDL2 -lglfw3")
else()
  set(raylib_VERBOSE ON)
  include(cmake/raylib-config.cmake)
endif()

include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)

check_cxx_source_compiles("
   [[noreturn]] void testFunc()
   {}

   int main(void)
   {
     return 0;
   }
" HAVE_C23_ATTRIBUTE_NORETURN
)

check_cxx_source_compiles("
   __attribute__((noreturn)) void testFunc()
   {}

   int main(void)
   {
     return 0;
   }
" HAVE_LEGACY_ATTRIBUTE_NORETURN
)

check_cxx_source_compiles("
   __attribute__((format(printf,1,2))) void testFunc(char *fmt, ...)
   {
   }

   int main(void)
   {
     return 0;
   }
" HAVE_ATTRIBUTE_FORMAT
)

include(CheckStructHasMember)
check_struct_has_member("struct dirent" d_namlen dirent.h HAVE_DIRENT_D_NAMLEN LANGUAGE CXX)

configure_file(
  "config.h.in"
  "config.h"
)

include_directories("${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}")

add_subdirectory(LibCore)
add_subdirectory(LSP/TSParser)
#add_subdirectory(LSP/Schema)
add_subdirectory(Util)
add_subdirectory(App)

install(DIRECTORY share DESTINATION .)
