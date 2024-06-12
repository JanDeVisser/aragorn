add_executable(
	TSParser
	TSParser.cpp
)

target_link_libraries(
	TSParser
        PRIVATE
	LibCore
        m
)

install(TARGETS TSParser
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)
