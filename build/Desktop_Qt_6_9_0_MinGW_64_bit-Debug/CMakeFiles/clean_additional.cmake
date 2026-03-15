# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\appCentral_Ar_Condicionado_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\appCentral_Ar_Condicionado_autogen.dir\\ParseCache.txt"
  "appCentral_Ar_Condicionado_autogen"
  )
endif()
