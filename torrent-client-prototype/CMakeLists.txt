cmake_minimum_required(VERSION 3.16)
project(torrent-client-prototype CXX)

# Add your build configuration here

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-fsanitize=thread)
add_link_options(-fsanitize=thread)

find_package(OpenSSL REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIR})

include(FetchContent)

set(CPR_USE_SYSTEM_CURL ON)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG 3b15fa82ea74739b574d705fea44959b58142eb8) # The commit hash for 1.10.x. Replace with the latest from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)

# add_executable(
#         ${PROJECT_NAME}
#         main.cpp
#         peer.h
#         torrent_file.h
#         peer_connect.cpp
#         peer_connect.h
#         tcp_connect.cpp
#         tcp_connect.h
#         torrent_tracker.cpp
#         torrent_tracker.h
#         torrent_file.cpp
#         bencode.cpp
#         bencode.h
#         message.cpp
#         message.h
#         byte_tools.h
#         byte_tools.cpp
#         piece_storage.cpp
#         piece_storage.h
#         piece.cpp
#         piece.h
# )

set(SOURCES
    main.cpp
    parsing/bencode.cpp
    parsing/torrent_file.cpp
    piece_storage/piece_storage.cpp
    piece_storage/piece.cpp
    network/peer_connect.cpp
    network/tcp_connect.cpp
    network/torrent_tracker.cpp
    network/message.cpp
    utils/byte_tools.cpp
)

add_executable(torrent-client-prototype ${SOURCES})


target_link_libraries(${PROJECT_NAME} PUBLIC ${OPENSSL_LIBRARIES} cpr::cpr)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
