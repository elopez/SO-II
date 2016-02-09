cmake_minimum_required(VERSION 3.4)
project(nachos ASM CXX)

# Only Linux supported as host
add_definitions(-DHOST_LINUX)

# Decide on bitness
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    add_definitions(-DHOST_x86_64)
else()
    add_definitions(-DHOST_i386)
endif()

set(SOURCE_FILES_THREAD
    threads/main.cc
    threads/scheduler.cc
    threads/synch.cc
    threads/port.cc
    threads/system.cc
    threads/thread.cc
    threads/utility.cc
    threads/threadtest.cc
    threads/locktest.cc
    threads/conditiontest.cc
    threads/inversiontest.cc
    threads/porttest.cc
    threads/jointest.cc
    machine/interrupt.cc
    machine/sysdep.cc
    machine/stats.cc
    machine/timer.cc
    threads/preemptive.cc
    threads/switch.S)

set(SOURCE_FILES_USERPROG
    userprog/addrspace.cc
    userprog/bitmap.cc
    userprog/synchconsole.cc
    userprog/processtable.cc
    userprog/exception.cc
    userprog/progtest.cc
    machine/console.cc
    machine/machine.cc
    machine/mipssim.cc
    machine/translate.cc)

set(SOURCE_FILES_VM
    vm/memaccess.cc
    vm/coremap.cc)

set(SOURCE_FILES_FILESYS
    filesys/directory.cc
    filesys/filehdr.cc
    filesys/filesys.cc
    filesys/fstest.cc
    filesys/openfile.cc
    filesys/synchdisk.cc
    machine/disk.cc)

set(SOURCE_FILES_NETWORK
    network/nettest.cc
    network/post.cc
    machine/network.cc)

# nachos threading task
add_executable(nachos-threads ${SOURCE_FILES_THREAD})
target_include_directories(nachos-threads PUBLIC threads machine)
set_target_properties(nachos-threads PROPERTIES
    OUTPUT_NAME "nachos"
    RUNTIME_OUTPUT_DIRECTORY "threads/"
    COMPILE_FLAGS "-DTHREADS -DDFS_TICKS_FIX"
)

# nachos userprog task
add_executable(nachos-userprog ${SOURCE_FILES_THREAD} ${SOURCE_FILES_USERPROG} ${SOURCE_FILES_VM})
target_include_directories(nachos-userprog PUBLIC bin filesys userprog threads machine vm)
set_target_properties(nachos-userprog PROPERTIES
    OUTPUT_NAME "nachos"
    RUNTIME_OUTPUT_DIRECTORY "userprog/"
    COMPILE_FLAGS "-DUSER_PROGRAM -DFILESYS_NEEDED -DFILESYS_STUB  -DDFS_TICKS_FIX -DMP_TIMESLICING"
)

# nachos vm task
add_executable(nachos-vm ${SOURCE_FILES_THREAD} ${SOURCE_FILES_USERPROG} ${SOURCE_FILES_VM})
target_include_directories(nachos-vm PUBLIC filesys bin vm userprog threads machine)
set_target_properties(nachos-vm PROPERTIES
    OUTPUT_NAME "nachos"
    RUNTIME_OUTPUT_DIRECTORY "vm/"
    COMPILE_FLAGS "-DUSER_PROGRAM  -DFILESYS_NEEDED -DFILESYS_STUB -DVM -DUSE_TLB  -DDFS_TICKS_FIX -DCLOCK_EVICT -DDEMAND_LOADING"
)

# nachos filesys task
add_executable(nachos-filesys ${SOURCE_FILES_THREAD} ${SOURCE_FILES_USERPROG} ${SOURCE_FILES_VM} ${SOURCE_FILES_FILESYS})
target_include_directories(nachos-filesys PUBLIC filesys bin vm userprog threads machine)
set_target_properties(nachos-filesys PROPERTIES
    OUTPUT_NAME "nachos"
    RUNTIME_OUTPUT_DIRECTORY "filesys/"
    COMPILE_FLAGS "-DTHREADS -DUSER_PROGRAM -DVM -DFILESYS_NEEDED -DFILESYS"
)

# nachos filesys task
add_executable(nachos-network ${SOURCE_FILES_THREAD} ${SOURCE_FILES_USERPROG} ${SOURCE_FILES_VM} ${SOURCE_FILES_FILESYS} ${SOURCE_FILES_NETWORK})
target_include_directories(nachos-network PUBLIC network bin filesys vm userprog threads machine)
set_target_properties(nachos-network PROPERTIES
    OUTPUT_NAME "nachos"
    RUNTIME_OUTPUT_DIRECTORY "network/"
    COMPILE_FLAGS "-DUSER_PROGRAM -DVM -DFILESYS_NEEDED -DFILESYS -DNETWORK"
)

# misc tools
add_subdirectory(bin)
