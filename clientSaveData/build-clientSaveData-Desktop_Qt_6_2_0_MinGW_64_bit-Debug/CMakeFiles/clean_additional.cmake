# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\clientSaveData_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\clientSaveData_autogen.dir\\ParseCache.txt"
  "clientSaveData_autogen"
  )
endif()
