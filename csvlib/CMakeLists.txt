
# cmake .. -DCMAKE_INSTALL_PREFIX=_install
# make install

cmake_minimum_required (VERSION 3.2)
set(CMAKE_CXX_STANDARD 11)
project (libcsv)

include_directories("csvlib")

if(APPLE)
  include_directories("/usr/local/Cellar/icu4c/63.1/include")
  link_directories("/usr/local/Cellar/icu4c/63.1/lib")
endif(APPLE)

include_directories(ICU_INCLUDE_DIRS)
link_directories(ICU_LIBRARIES)

add_library(csvicu STATIC 
  csv/parser.cpp
  csv/datasource/utf8/DataSource.cpp
  csv/datasource/icu/DataSource.cpp
)
target_compile_definitions(csvicu PUBLIC ALLOW_ICU_EXTENSIONS)

add_library(csv STATIC 
  csv/parser.cpp
  csv/datasource/utf8/DataSource.cpp
)

install(TARGETS csv DESTINATION libcsv/lib)
install(TARGETS csvicu DESTINATION libcsv/lib)
install(FILES csv/parser.hpp DESTINATION libcsv/include/csv/)
install(FILES csv/datasource/IDataSource.hpp DESTINATION libcsv/include/csv/datasource/)
install(FILES csv/datasource/utf8/DataSource.hpp DESTINATION libcsv/include/csv/datasource/utf8/)
install(FILES csv/datasource/icu/DataSource.hpp DESTINATION libcsv/include/csv/datasource/icu/)
install(FILES csv/datasource/icu/Encoding.hpp DESTINATION libcsv/include/csv/datasource/icu/)
