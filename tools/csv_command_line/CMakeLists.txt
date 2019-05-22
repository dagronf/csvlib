
# cmake .. -DCMAKE_INSTALL_PREFIX=_install
# make

cmake_minimum_required (VERSION 3.2)
set(CMAKE_CXX_STANDARD 11)
project (csv_command_line)

include_directories("${CMAKE_BINARY_DIR}/csvlib/include" "tclap/include")
link_directories("${CMAKE_BINARY_DIR}/csvlib")

if(APPLE)
  include_directories("/usr/local/Cellar/icu4c/63.1/include")
  link_directories("/usr/local/Cellar/icu4c/63.1/lib")
else()
  include_directories(ICU_INCLUDE_DIRS)
  link_directories(ICU_LIBRARIES)
endif(APPLE)

add_executable(convert2tsv main.cpp command_line.cpp)
target_compile_definitions(convert2tsv PUBLIC ALLOW_ICU_EXTENSIONS)

if(APPLE)
  target_link_libraries(convert2tsv libcsvicu.a libicui18n.a libicuio.a libicudata.a libicuuc.a libicutu.a)
else()
  target_link_libraries(convert2tsv libcsvicu.a libicui18n.so libicuio.so libicudata.so libicuuc.so libicutu.so libdl.a libstdc++.so)
endif(APPLE)

install(TARGETS convert2tsv DESTINATION libcsv/bin)