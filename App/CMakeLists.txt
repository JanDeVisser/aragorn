find_package(Freetype)

add_executable(
        Aragorn
        MACOSX_BUNDLE
        App.cpp
        Buffer.cpp
        Colour.cpp
        BufferView.cpp
        Aragorn.cpp
        Editor.cpp
        FileSelector.h
        Gutter.cpp
        Layout.cpp
        LexerMode.h
        MiniBuffer.h
        Modal.h
        Mode.cpp
        StatusBar.cpp
        Theme.cpp
        Widget.cpp
        Project.cpp
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

install(TARGETS Aragorn
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        BUNDLE DESTINATION bundle)
