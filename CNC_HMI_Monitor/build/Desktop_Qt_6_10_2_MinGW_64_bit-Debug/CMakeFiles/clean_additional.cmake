# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\CNC_HMI_Client_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\CNC_HMI_Client_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\CNC_Mock_Server_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\CNC_Mock_Server_autogen.dir\\ParseCache.txt"
  "CNC_HMI_Client_autogen"
  "CNC_Mock_Server_autogen"
  )
endif()
