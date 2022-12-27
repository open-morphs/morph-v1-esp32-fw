file (GLOB_RECURSE SOURCES defines.h webserver/* *.cpp *.cfonts/* images/*)
set (srcs ${SOURCES})

idf_component_register(SRCS ${srcs})

target_add_binary_data(${COMPONENT_TARGET} "webserver/root.html" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "webserver/settings.html" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "webserver/stylesheet.css" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "webserver/script.js" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "webserver/save.html" TEXT)
target_add_binary_data(${COMPONENT_TARGET} "webserver/ota.html" TEXT)
