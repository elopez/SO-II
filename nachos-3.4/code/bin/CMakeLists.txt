cmake_minimum_required(VERSION 3.4)
project(tooling)

include_directories(../threads)

# converts a COFF file to Nachos object format
add_executable(coff2noff coff2noff.c)

# converts a COFF file to a flat address space (for Nachos version 2)
# currently unbuildable
# add_executable(coff2flat coff2flat.c)

# dis-assembles a COFF file
# currently unbuildable
# add_executable(disassemble out.c opstrings.c)