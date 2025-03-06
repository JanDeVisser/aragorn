add_library(
        LibCore
        SHARED
        Checked.h
        Error.cpp
#        Integer.h
        IO.cpp
        JSON.cpp
        Lexer.cpp
        Logging.cpp
        Options.cpp
        Pipe.h
        Process.h
        Result.h
        ScopeGuard.h
        StringUtil.cpp
        Token.cpp
        Utf8.cpp
)

target_link_libraries(
        LibCore
        dl
)

include_directories(.)

install(TARGETS LibCore
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)
