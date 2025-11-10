cmake_minimum_required(VERSION 3.10)
project(NL2SQL_Engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. Header path
set(MARIADB_INCLUDE_DIR "C:/msys64/mingw64/include/mysql")

# 2. Library file
set(MARIADB_LIBRARY "C:/msys64/mingw64/lib/libmariadb.dll.a")

add_executable(nl2sql_engine src/nl2sql_engine.cpp) #<-- Make sure this filename is correct

# 3. Add header path
target_include_directories(nl2sql_engine
    PRIVATE
    ${MARIADB_INCLUDE_DIR}
)

# 4. Link ALL dependencies
target_link_libraries(nl2sql_engine
    PRIVATE
    ${MARIADB_LIBRARY}
    z         # Zlib for compression
    ws2_32    # Windows Sockets
    crypt32   # Windows Crypto API
    secur32   # Windows Security API
)