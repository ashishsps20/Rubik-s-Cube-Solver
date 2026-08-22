cmake_minimum_required(VERSION 3.26)
project(RUBIKS)

set(CMAKE_CXX_STANDARD 14)

# Disable SSL verification for FetchContent to avoid Windows certificate errors
set(CMAKE_TLS_VERIFY OFF)

include(FetchContent)
FetchContent_Declare(
  raylib
  URL https://github.com/raysan5/raylib/archive/master.tar.gz
)
FetchContent_MakeAvailable(raylib)

add_executable(RUBIKS main.cpp
        Model/GenericRubiksCube.cpp
        Model/GenericRubiksCube.h
        Model/RubiksCube3dArray.cpp
        Solver/BFSSolver.h
        Solver/DFSSolver.h
        Model/PatternDatabase/PatternDatabase.h
        Model/RubiksCube1dArray.cpp
        Model/RubiksCubeBitboard.cpp
        Solver/IDAstarSolver.h
        Solver/IDDFSSolver.h
        PatternDatabases/CornerDBMaker.cpp
        PatternDatabases/CornerPatternDatabase.cpp
        PatternDatabases/math.cpp
        PatternDatabases/NibbleArray.cpp
        PatternDatabases/PatternDatabase.cpp
        GUI/RubiksGUI.cpp
)

target_link_libraries(RUBIKS PRIVATE raylib)