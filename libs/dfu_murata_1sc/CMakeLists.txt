# List all available source files in the directory
#set(DFU_MURATA_1SC_FILES
#    "${CMAKE_CURRENT_SOURCE_DIR}/src/dfu_murata_1sc.c"
#)

# Set the variable with the source files list to the parent scope
#set(DFU_MURATA_1SC_FILES ${DFU_MURATA_1SC_FILES} PARENT_SCOPE)
target_sources(app PRIVATE dfu_murata_1sc.c)

# Set a variable with the include directory path
#(DFU_MURATA_1SC_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})