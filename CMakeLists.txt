cmake_minimum_required(VERSION 3.25)
project(GBemu C)

set(CMAKE_C_STANDARD 11)

add_compile_definitions(SDL_MAIN_HANDLED)

# Set path to SDL2 library
set(SDL2_LIBRARY_DIR "./SDL2-2.26.4/x86_64-w64-mingw32/lib")
set(SDL2_INCLUDE_DIR "./SDL2-2.26.4/x86_64-w64-mingw32/include/SDL2")

set(EXTERNAL_LIB_INCLUDE_DIR "./Libs/include")

# Include SDL2 headers
include_directories(${SDL2_INCLUDE_DIR} ${EXTERNAL_LIB_INCLUDE_DIR})

# Add the library directories to the linker's path
link_directories(${SDL2_LIBRARY_DIR})

add_executable(GBemu main.c Emulator.c Emulator.h utils.h MemoryBus.c MemoryBus.h Cartridge.c Cartridge.h CPU.c CPU.h GPU.c GPU.h Timer.c Timer.h Instructions.h Instructions.c CPU_utils.c CPU_Processing.c DirectMemoryAccess.c DirectMemoryAccess.h Interrupts.c Interrupts.h IO.c IO.h LCD.c LCD.h GPU_StateMachine.c GPU_StateMachine.h GPU_Pipeline.c RAM.c RAM.h Stack.c Stack.h GUI.c GUI.h CPU_Fetching.c Input.c Input.h utils.c)

# Link SDL2 library
target_link_libraries(GBemu mingw32 SDL2main SDL2 SDL2_ttf)