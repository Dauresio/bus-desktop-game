# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Release")
  file(REMOVE_RECURSE
  "BusJamDomain_autogen"
  "BusJamPuzzle_autogen"
  "CMakeFiles\\BusJamDomain_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\BusJamDomain_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\BusJamPuzzle_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\BusJamPuzzle_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\tst_BoardEngine_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\tst_BoardEngine_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\tst_LevelLoader_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\tst_LevelLoader_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\tst_MovementSolver_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\tst_MovementSolver_autogen.dir\\ParseCache.txt"
  "tst_BoardEngine_autogen"
  "tst_LevelLoader_autogen"
  "tst_MovementSolver_autogen"
  )
endif()
