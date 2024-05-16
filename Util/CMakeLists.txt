add_executable(
        json
        json.cpp
)

target_link_libraries(
        json
        PRIVATE
        LibCore
)

include_directories(.)

add_compile_options("-fno-inline-functions")

install(TARGETS json
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)
