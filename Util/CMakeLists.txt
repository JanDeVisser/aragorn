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

install(TARGETS json
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)
