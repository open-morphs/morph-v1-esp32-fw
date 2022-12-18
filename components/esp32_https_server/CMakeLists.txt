file(GLOB_RECURSE SOURCES src/*.c src/*.cpp)
set(srcs ${SOURCES})

set(includedirs
src/
)

set(requires arduino openssl)

idf_component_register(INCLUDE_DIRS ${includedirs} SRCS ${srcs} REQUIRES ${requires})