cmake_minimum_required (VERSION 2.6)
project (libsiftfast)

# Flags for the C compiler:
#   -Wall for strict gcc warnings (requires prototypes for all functions).
#   -g to produce debug data for gdb
#   -msse2 - SSE2 extensions about 2.5x speedup (if CPU has SSE3, try -msse3 instead)
#   -fopenmp - Use OpenMP extension (about 2.5x speedup for 4 core cpu)
# CFLAGS = -Wall -fPIC -O3 -msse2 -fopenmp
# CXXFLAGS = ${CFLAGS}

# ${CC} -shared -Wl,-soname,$@ -o $@ ${CFLAGS}  ${LIBS} sift_fast.o -lstdc++

add_library(libsiftfast libsiftfast.cpp)

#add_subdirectory (octave)
#add_subdirectory (matlab) 
