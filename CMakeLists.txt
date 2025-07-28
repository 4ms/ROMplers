 cmake_minimum_required(VERSION 3.19)

if(NOT "${METAMODULE_SDK_DIR}" STREQUAL "")
	message("METAMODULE_SDK_DIR set by CMake variable ${METAMODULE_SDK_DIR}")
elseif (DEFINED ENV{METAMODULE_SDK_DIR})
    set(METAMODULE_SDK_DIR "$ENV{METAMODULE_SDK_DIR}")
	message("METAMODULE_SDK_DIR set by environment variable ${METAMODULE_SDK_DIR}")
else()
    set(METAMODULE_SDK_DIR "${CMAKE_CURRENT_LIST_DIR}/../metamodule-plugin-sdk")
	message("METAMODULE_SDK_DIR set to default: ${METAMODULE_SDK_DIR}")
endif()

include(${METAMODULE_SDK_DIR}/plugin.cmake)

project(4ms-ROMplers VERSION 0.1 DESCRIPTION "4ms-ROMplers for MetaModule" LANGUAGES C CXX ASM)

add_library(4ms-ROMplers STATIC)

# Add sources from the Rack source repo
set(SOURCE_DIR .)

target_sources(4ms-ROMplers PRIVATE
    ${SOURCE_DIR}/src/KayOne.cpp                    # Note: this file contains init(rack::Plugin*
    ${SOURCE_DIR}/src/KayArr.cpp       
    ${SOURCE_DIR}/src/AyysKing.cpp        
    ${SOURCE_DIR}/src/SinSahnix.cpp        
    ${SOURCE_DIR}/src/SicksOh.cpp        
    ${SOURCE_DIR}/src/DeeArr.cpp        
    ${SOURCE_DIR}/src/SeaArr.cpp        
    ${SOURCE_DIR}/src/SehvenToo.cpp     
    ${SOURCE_DIR}/src/SehvenTooSamples.hpp                                      
    ${SOURCE_DIR}/src/SeaArrSamples.hpp                                   
    ${SOURCE_DIR}/src/DeeArrSamples.hpp                                   
    ${SOURCE_DIR}/src/SicksOhSamples.hpp                                   
    ${SOURCE_DIR}/src/SinSahnixSamples.hpp                                   
    ${SOURCE_DIR}/src/AyysKingSamples.hpp                 
    ${SOURCE_DIR}/src/KayOneSamples.hpp                 
    ${SOURCE_DIR}/src/KayArrSamples.hpp        
    ${SOURCE_DIR}/src/plugin.cpp        
    ${SOURCE_DIR}/src/plugin.hpp        
    ) 

# Add includes and compile options for source repo
target_include_directories(4ms-ROMplers PRIVATE ${SOURCE_DIR}/src)

# Call this to link and create the plugin file
create_plugin(
    SOURCE_LIB      4ms-ROMplers                                          # The cmake target name (defined in add_target)
    PLUGIN_NAME     4ms-ROMplers                                          # This must match the brand "slug" used in VCV Rack
    PLUGIN_JSON     ${CMAKE_CURRENT_LIST_DIR}/plugin.json              # Path to the plugin.json file used by VCV Rack
    SOURCE_ASSETS   ${CMAKE_CURRENT_LIST_DIR}/assets                 # Path to the assets/ dir containing the PNGs
    DESTINATION     ${CMAKE_CURRENT_LIST_DIR}/metamodule-plugins   # Path to where you want the plugin file output
)