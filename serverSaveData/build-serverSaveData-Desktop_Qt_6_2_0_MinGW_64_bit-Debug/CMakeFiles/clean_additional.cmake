# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\serverSaveData_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\serverSaveData_autogen.dir\\ParseCache.txt"
  "serverSaveData_autogen"
  )
endif()
