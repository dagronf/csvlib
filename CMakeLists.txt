
# cmake .. -DCMAKE_INSTALL_PREFIX=build -DCMAKE_BUILD_TYPE=Release
# make install

cmake_minimum_required (VERSION 3.2)
set(CMAKE_CXX_STANDARD 11)
project (libcsv-app)

include_directories("csvlib")

if(APPLE)
  include_directories("/usr/local/opt/include")
  link_directories("/usr/local/opt/lib")
endif(APPLE)

include_directories(ICU_INCLUDE_DIRS)
link_directories(ICU_LIBRARIES)

add_subdirectory(csvlib)
add_subdirectory(tools/csv_command_line)