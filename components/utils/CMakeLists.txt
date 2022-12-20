# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES
  "console"
  "fatfs"
  "json"
  "mdns"
  "nvs_flash"
  "bt"
)
#set(COMPONENT_PRIV_REQUIRES "arduino")

file(GLOB COMPONENT_SRCS
  LIST_DIRECTORIES false
  "BLE/*.cp"
  "BLE/*.cpp"
  "BLE/*.h"
  "BLE/*.hpp"
  "*.cpp"
  "*.c"
  "*.S"
)

set(COMPONENT_ADD_INCLUDEDIRS
  BLE
  .)

register_component()

target_compile_options(${COMPONENT_TARGET} PUBLIC
# COMPONENTS BUILD FLAGS
  -DGET_SERVICES_TIMEOUT_MS=10000UL
)