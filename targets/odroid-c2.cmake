# NOTE
# Пока у меня не получилось автоматическое добавление флагов -> их следует добавить вручную
# Вот доп. флаги для сборки модуля на UC-8410
# Compiler Flags: -g -mbig-endian -mcpu=xscale -D__STDC_LIMIT_MACROS -D__STDC_FORMAT_MACROS -DCLOCK_REALTIME_STUB -DSLOWDEV -Wall  -Xlinker "-EB"
# Linker Flags: -lrt -lpthread

# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

# specify the cross compiler
SET(CMAKE_C_COMPILER   /usr/bin/aarch64-linux-gnu-gcc)
SET(CMAKE_CXX_COMPILER /usr/bin/aarch64-linux-gnu-g++)

# where is the target environment 
SET(CMAKE_FIND_ROOT_PATH  /usr/aarch64-linux-gnu)

# search for programs in the build host directories
#SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
