cmake_minimum_required(VERSION 2.8)

#set(MX_BIG_ENDIAN "0" CACHE BOOL "Target is big-endian system")
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


add_executable(mb_driver_ext ${SRCs})
