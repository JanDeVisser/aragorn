find_package(Freetype)

add_executable(
        Eddy
        App.cpp
        Buffer.cpp
        BufferView.cpp
        Eddy.cpp
        Editor.cpp
        FileSelector.h
        Gutter.cpp
        Layout.cpp
        MiniBuffer.h
        Modal.h
        StatusBar.cpp
        Widget.cpp
        Project.cpp
)

target_include_directories(Eddy PRIVATE ${raylib_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(
        Eddy
        PRIVATE
        LibCore
        ${raylib_LIBRARIES}
        ${FREETYPE_LIBRARIES}
        m
)

include_directories(.)

install(TARGETS Eddy
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)
