cmake_minimum_required(VERSION 3.28)
project(ECS36BBattleShipHumansOnlySrc) # make sure to update

# add new files to the end of this line
add_library(${SRC_LIB_NAME} STATIC
GameConfig.cpp GameConfig.h
        Game.cpp Game.h
        FiringResult.cpp
        FiringResult.h
        Player.cpp
        Player.h
        Board.cpp
        Board.h
        ExtraStringMethods.cpp
        ExtraStringMethods.h
        ShipPlacement.cpp
        ShipPlacement.h
        Orientation.h
        BoardCell.cpp
        BoardCell.h
        AIPlayer.cpp
        AIPlayer.h
)


target_include_directories(${SRC_LIB_NAME} PUBLIC .)
target_compile_options(${SRC_LIB_NAME}  PRIVATE ${CXX_WARNINGS})

add_executable(${EXE_NAME} main.cpp
        AIPlayer.h
        AIPlayer.cpp)
target_link_libraries(${EXE_NAME} PRIVATE ${SRC_LIB_NAME})
target_compile_options(${EXE_NAME} PRIVATE ${CXX_WARNINGS})
