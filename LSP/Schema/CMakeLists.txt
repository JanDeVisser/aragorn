#
# Copyright (c) 2024, Jan de Visser <jan@finiandarcy.com>
#
# SPDX-License-Identifier: MIT
#
# THIS IS GENERATED CODE. DO NOT MODIFY.
#

include_directories(.)

add_library(
  LSPSchema
  LSPBase.h
)

include(LSPSchemaSources.cmake)
      
set_target_properties(LSPSchema PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(LSPSchema LibCore)

install(TARGETS LSPSchema
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)
