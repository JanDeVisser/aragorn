cmake_minimum_required(VERSION 3.27)
project(aragorn CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

message("System: ${CMAKE_SYSTEM_NAME}. Processor: ${CMAKE_SYSTEM_PROCESSOR}. Version: ${CMAKE_SYSTEM_VERSION}. Compiler: ${CMAKE_CXX_COMPILER_ID}.")

# if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
# message(FATAL_ERROR "Aragorn requires Clang because it used C++ language extensions not supported in GCC")
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

set(aragorn_NAME Aragorn)
set(aragorn_VERSION_MAJOR 1)
set(aragorn_VERSION_MINOR 0)
set(aragorn_DIR ${CMAKE_INSTALL_PREFIX})
set(aragorn_DATADIR ${CMAKE_INSTALL_PREFIX}/share)

find_package(Freetype)
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

add_library(
        LibCore
        STATIC
        LibCore/Checked.h
        LibCore/Error.cpp
#       LibCore/Integer.h
        LibCore/IO.cpp
        LibCore/JSON.cpp
        LibCore/Lexer.cpp
        LibCore/Logging.cpp
        LibCore/Options.cpp
        LibCore/Pipe.h
        LibCore/Process.h
        LibCore/Result.h
        LibCore/ScopeGuard.h
        LibCore/StringUtil.cpp
        LibCore/Token.cpp
        LibCore/Utf8.cpp
)

target_link_libraries(
        LibCore
        iconv
        dl
)

include_directories(.)
target_include_directories(LibCore PRIVATE LibCore)

#add_subdirectory(LSP/TSParser)
#add_subdirectory(LSP/Schema)

add_executable(
        Aragorn
        MACOSX_BUNDLE
        App/App.cpp
        App/Buffer.cpp
        App/Colour.cpp
        App/BufferView.cpp
        App/Aragorn.cpp
        App/Editor.cpp
        App/FileSelector.h
        App/Gutter.cpp
        App/Layout.cpp
        App/LexerMode.h
        App/MiniBuffer.h
        App/Modal.h
        App/Mode.cpp
        App/StatusBar.cpp
        App/Theme.cpp
        App/Widget.cpp
        App/Project.cpp
)

target_include_directories(Aragorn PRIVATE ${raylib_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS})

target_link_libraries(
        Aragorn
        PRIVATE
        LibCore
#       LSPSchema
        ${raylib_LIBRARIES}
        ${FREETYPE_LIBRARIES}
        m
)

add_executable(
        json
        Util/json.cpp
)

target_link_libraries(
        json
        PRIVATE
        LibCore
)

include_directories(.)

#add_compile_options("-fno-inline-functions")

install(TARGETS LibCore Aragorn json
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        BUNDLE DESTINATION bundle)
install(DIRECTORY share DESTINATION .)
