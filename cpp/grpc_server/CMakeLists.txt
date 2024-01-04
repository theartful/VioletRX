find_package(Protobuf REQUIRED GLOBAL)
find_package(gRPC REQUIRED GLOBAL)

include(FetchContent)
FetchContent_Declare(
  broadcast_queue
  GIT_REPOSITORY    https://github.com/theartful/broadcast_queue
  GIT_TAG           master
)
FetchContent_MakeAvailable(broadcast_queue)

set(protos_path "${CMAKE_SOURCE_DIR}/protos")
set(protos_out_path "${CMAKE_CURRENT_BINARY_DIR}")

file(MAKE_DIRECTORY ${protos_out_path})

set(protos "receiver")

foreach(proto ${protos})
  set(${proto}_src "${protos_out_path}/${proto}.pb.cc")
  set(${proto}_hdr "${protos_out_path}/${proto}.pb.h")
  set(${proto}_grpc_src "${protos_out_path}/${proto}.grpc.pb.cc")
  set(${proto}_grpc_hdr "${protos_out_path}/${proto}.grpc.pb.h")

  add_custom_command(
    OUTPUT "${${proto}_src}" "${${proto}_hdr}" "${${proto}_grpc_src}" "${${proto}_grpc_hdr}"
    COMMAND "$<TARGET_FILE:protobuf::protoc>"
       ARGS --grpc_out "${protos_out_path}"
         --cpp_out "${protos_out_path}"
         -I "${protos_path}"
         --plugin=protoc-gen-grpc="$<TARGET_FILE:gRPC::grpc_cpp_plugin>"
         "${protos_path}/${proto}.proto"
    MAIN_DEPENDENCY "${protos_path}/${proto}.proto"
  )

  list(APPEND proto_srcs "${${proto}_src}")
  list(APPEND proto_hdrs "${${proto}_hdr}")
  list(APPEND grpc_srcs "${${proto}_grpc_src}")
  list(APPEND grpc_hdrs "${${proto}_grpc_hdr}")
endforeach()

add_library(
protos
  "${proto_srcs}"
  "${proto_hdrs}"
  "${grpc_srcs}"
  "${grpc_hdrs}"
)

target_link_libraries(
protos
PUBLIC
  protobuf::libprotobuf
  gRPC::grpc++
  gRPC::grpc++_reflection
)

target_include_directories(
protos
PUBLIC
  ${protos_out_path}
)

add_library(
grpc_server
    server.h
    server.cpp
    transactions_poster.h
    transactions_poster.cpp
)

target_link_libraries(
grpc_server
PRIVATE
    protos
    broadcast_queue
PUBLIC
    async_core
)

add_executable(headless_server main.cpp)
target_link_libraries(headless_server grpc_server)
