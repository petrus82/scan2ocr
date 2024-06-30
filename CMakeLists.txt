cmake_minimum_required(VERSION 3.5)
project(scan2ocr VERSION "0.0.4" LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG  "-g -ggdb")

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets PdfWidgets LinguistTools)

find_library(Leptonica_LIBRARIES NAMES libleptonica.so PATHS /usr/lib/)
find_library(libssh_LIBRARIES NAMES libssh.so PATHS /usr/lib/)
find_library(Tesseract_LIBRARIES NAMES libtesseract.so PATHS /usr/lib/)

if (Qt6_VERSION VERSION_GREATER_EQUAL 6.3)
    qt_standard_project_setup()
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SOURCE_FILES
  CMakeLists.txt
  scan2ocr.qrc
  src/mainwindow.cpp
  src/pdffile.cpp
  src/scan2ocr.cpp
  src/ftpconnection.cpp
  src/parseurl.cpp
  src/settings.cpp
  src/mainwindow.h
  src/pdffile.h
  src/BS_thread_pool.hpp
  src/ftpconnection.h
  src/parseurl.h
  src/scan2ocr.h
  src/settings.h
)

qt_add_executable(scan2ocr  
    ${SOURCE_FILES}
)

# Add translation
qt_add_translations(scan2ocr TS_FILES german.ts RESOURCE_PREFIX "translations" LUPDATE_OPTIONS -no-obsolete)

add_dependencies(scan2ocr scan2ocr_lupdate)
add_dependencies(scan2ocr update_translations)

target_include_directories(scan2ocr PRIVATE
    ${CMAKE_PROJECT_INCLUDE_DIR}
    ${libssh_INCLUDE_DIRS}
#    ${ImageMagick_INCLUDE_DIRS}
#    ${ImageMagick_Magick}
    ${Leptonica_INCLUDE_DIRS}
    ${Tesseract_INCLUDE_DIRS}
)

target_compile_options(scan2ocr PRIVATE
    -fopenmp
    -DPROGRAM_VERSION="${PROJECT_VERSION}"
)

# Conditionally add flags based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  # Add debug flags
  target_compile_options(scan2ocr PRIVATE -ggdb3)
  add_compile_definitions(DEBUG)
else()
  # Add flags for clean and stripped executable
  target_compile_options(scan2ocr PRIVATE -s -O3 -DNDEBUG)
endif()

target_link_libraries(scan2ocr PRIVATE
    ${Leptonica_LIBRARIES}
    ${Tesseract_LIBRARIES}
    ${libssh_LIBRARIES}
    ${Leptonica_LIBRARIES}
    ${libssh_LIBRARIES}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::PdfWidgets
)

install(TARGETS scan2ocr DESTINATION bin)

set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Convert scanned files to black/white PDFs with text using OCR.")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")
set(CPACK_GENERATOR "TGZ")

set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-src-${PROJECT_VERSION}")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CMAKE_INSTALL_PREFIX /usr)
set(CPACK_SOURCE_GENERATOR_DISABLED)
set(CPACK_SOURCE_GENERATOR_TGZ ON)

include(CPack)
