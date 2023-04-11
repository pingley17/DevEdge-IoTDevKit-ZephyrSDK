# libs/CMakeLists.txt

# Include the dfu_modem_1sc CMakeLists.txt
#include(${CMAKE_CURRENT_SOURCE_DIR}/dfu_modem_1sc/CMakeLists.txt)

# Export the source and include variables
#set(DFU_MURATA_1SC_FILES ${DFU_MURATA_1SC_FILES} PARENT_SCOPE)
#set(DFU_MURATA_1SC_INCLUDE ${DFU_MURATA_1SC_INCLUDE} PARENT_SCOPE)

# target_sources(app PRIVATE tmo_dfu_download.c)
target_include_directories(app PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dfu_gecko)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dfu_murata_1sc)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/dfu_rs9116w)