cmake_minimum_required(VERSION 2.8)
project(mb_driver_ext)

option(BIG_ENDIAN "Target is big-endian system" FALSE)

set (SRCs
  cmd_server.c
  connection.c
  connections_pool.c
  global.c
  high_client.c
  input_queue.c
  low_client.c
  main.c
  message.c
  sockets.c
  std_input.c
  tcp_server.c
  trace.c
  tty.c
  udp_server.c
  )

add_executable(${CMAKE_PROJECT_NAME} ${SRCs})
if (${BIG_ENDIAN})
    target_compile_definitions(${CMAKE_PROJECT_NAME} PUBLIC -DMX_BIG_ENDIAN)
endif()

