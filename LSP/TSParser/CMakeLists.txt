add_executable(
        TSParser
        CPPOutput.cpp
	Parser.cpp
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
