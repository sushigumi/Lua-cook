set(LUAPP_SRCS
    lib/executor.cpp
    lib/execution_items.cpp
)

add_library(lua-cook SHARED ${LUAPP_SRCS})
target_include_directories(lua-cook PUBLIC include)
target_link_libraries(lua-cook PUBLIC lualib)
