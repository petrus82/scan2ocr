cmake_minimum_required(VERSION 3.5)
project(scan2ocr VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_DEBUG  "-g")

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets PdfWidgets LinguistTools)
find_package(ImageMagick COMPONENTS Magick++)
find_package(libssh)

set(Leptonica_DIR "/usr/lib/cmake/Leptonica")
set(Tesseract_INCLUDE_DIRS "/usr/include/tesseract")

if (Qt6_VERSION VERSION_GREATER_EQUAL 6.3)
    qt_standard_project_setup()
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Set the debug flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Add CodeQL options
# option(ENABLE_CODEQL "Enable CodeQL analysis" OFF)
# if(ENABLE_CODEQL)
#  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# endif()

set(SOURCE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SOURCE_FILES
  CMakeLists.txt
  scan2ocr.qrc
  src/cfopen.cpp
  src/cfmain.cpp
  src/mainwindow.cpp
  src/pdffile.cpp
  src/scan2ocr.cpp
  src/ftpconnection.cpp
  src/parseurl.cpp
  src/mainwindow.h
  src/cfopen.h
  src/cfmain.h
  src/pdffile.h
  src/ftpconnection.h
  src/parseurl.h
  src/scan2ocr.h
)

qt_add_executable(scan2ocr  
    scan2ocr.qrc
    ${SOURCE_FILES}
)

# Add translation
qt6_add_translations(scan2ocr TS_FILES german.ts LUPDATE_OPTIONS -no-obsolete)

add_dependencies(scan2ocr scan2ocr_lupdate)
add_dependencies(scan2ocr update_translations)

  #QM_FILES_OUTPUT_VARIABLE qm_files)

 # install(FILES ${qm_files} DESTINATION "translations")

target_include_directories(scan2ocr PRIVATE
    ${CMAKE_PROJECT_INCLUDE_DIR}
    ${libssh_INCLUDE_DIRS}
    ${ImageMagick_INCLUDE_DIRS}
    ${Leptonica_INCLUDE_DIRS}
    ${Tesseract_INCLUDE_DIRS}
    /usr/include/poppler/cpp
)

target_compile_options(scan2ocr PRIVATE
    -I/usr/include/ImageMagick-7
    -I/usr/include/poppler/cpp
    -fopenmp
    -DMAGICKCORE_HDRI_ENABLE=1
    -DMAGICKCORE_CHANNEL_MASK_DEPTH=32
    -DMAGICKCORE_QUANTUM_DEPTH=16
)

# Conditionally add flags based on build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  # Add debug flags
  target_compile_options(scan2ocr PRIVATE -ggdb3)
else()
  # Add flags for clean and stripped executable
  target_compile_options(scan2ocr PRIVATE -s -O3 -DNDEBUG)
endif()

target_link_libraries(scan2ocr PRIVATE
    ${ImageMagick_LIBRARIES}
    ${Leptonica_LIBRARIES}
    ${Tesseract_LIBRARIES}
    ${libssh_LIBRARIES}
    ${Poppler_LIBRARIES}
    /usr/lib/libpoppler-cpp.so
    /usr/lib/libtesseract.so
    /usr/lib/libssh.so
    /usr/lib/libMagick++-7.Q16HDRI.so
    /usr/lib/libMagickCore-7.Q16HDRI.so
    /usr/lib/libMagickWand-7.Q16HDRI.so
    /usr/lib/libleptonica.so
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::PdfWidgets
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::PdfWidgets)
install(TARGETS scan2ocr DESTINATION bin)

set(CPACK_PACKAGE_NAME "scan2ocr")
set(CPACK_PACKAGE_VERSION "0.0.1")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Convert scanned files to black/white PDFs with text using OCR.")
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}-${PROJECT_VERSION}")
set(CPACK_GENERATOR "TGZ")

set(CPACK_SOURCE_PACKAGE_FILE_NAME "${PROJECT_NAME}-src-${PROJECT_VERSION}")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CMAKE_INSTALL_PREFIX /usr)
set(CPACK_SOURCE_GENERATOR_DISABLED)
set(CPACK_SOURCE_GENERATOR_TGZ ON)

#install(FILES ${qm_files} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_PROJECT_NAME})

include(CPack)

# Add CodeQL analysis step
#if(ENABLE_CODEQL)
#  find_program(CODEQL_CLI_EXECUTABLE "codeql")
#  if(CODEQL_CLI_EXECUTABLE)
#    add_custom_target(codeql
#      COMMAND ${CODEQL_CLI_EXECUTABLE} database analyze --format=sarif-latest
#        --output=scan2ocr.sarif -- code-analysis .
#      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#      COMMENT "Running CodeQL analysis"
#    )
#  else()
#    message(WARNING "CodeQL CLI not found. Skipping CodeQL analysis.")
#  endif()
#endif()

# Debug: cmake -DCMAKE_BUILD_TYPE=Debug .
# Release: cmake -DCMAKE_BUILD_TYPE=Release .
# With CodeQL:
# cmake -DENABLE_CODEQL=ON .
# cmake --build .
# cmake --build . --target codeql
# codeql database create code-analysis --language=cpp --command='cmake --build . --target all --config Release -- -j4'