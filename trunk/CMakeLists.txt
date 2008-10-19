cmake_minimum_required (VERSION 2.6)
project (libsiftfast)
set( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE )

# check for OpenMP
include(CheckIncludeFiles)
#CHECK_INCLUDE_FILES(omp.h HAVE_OMP_H)
# Output test results to config.h
#CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

# check for SSE extensions
# SET(CMAKE_CXX_FLAGS
# if(LINUX OR DARWIN)
#   set_source_files_properties(
#       libsiftfast.cpp
#       PROPERTIES COMPILE_FLAGS "-msse2 -mfpmath=sse"
#       )
# endif(LINUX OR DARWIN)

add_library(libsiftfast SHARED libsiftfast.cpp)
target_link_libraries(libsiftfast m stdc++)
set_target_properties(libsiftfast PROPERTIES OUTPUT_NAME  siftfast)

if( UNIX OR LINUX OR DARWIN )
    add_definitions(" -Wall -fPIC ")
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "libsiftfast install prefix" FORCE )
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT) 

message(STATUS "CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}")

add_executable(siftfast siftfast.cpp)
target_link_libraries(siftfast libsiftfast)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#
# generate mex files
#

set(MATLAB MATLAB-NOTFOUND)
FIND_PROGRAM(MATLAB NAME "mex" PATHS )

if(UNIX)
  set(MEX_CXXFLAGS "-Wl,-rpath,${CMAKE_INSTALL_PREFIX}/lib")
else()
  set(MEX_CXXFLAGS)
endif(UNIX)

if(MATLAB)

  if( UNIX OR LINUX )
    set(MEXEXT "mexglx")
  elseif( DARWIN OR APPLE )
    set(MEXEXT "mexmac")
  elseif( WIN64 )
    set(MEXEXT "mexw64")
  elseif( WIN32 OR CYGWIN OR WINDOWS )
    set(MEXEXT "mexw32")
  else()
    set(MEXEXT "mex")
  endif()

  set(MATLAB_MEX "siftfast.${MEXEXT}")

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${MATLAB_MEX}
    COMMAND mex
    ARGS ${CMAKE_CURRENT_SOURCE_DIR}/siftmex.cpp -llapack -L${CMAKE_CURRENT_BINARY_DIR} -lsiftfast ${MEX_CXXFLAGS} -outdir ${CMAKE_CURRENT_BINARY_DIR} -o ${MATLAB_MEX}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/siftmex.cpp libsiftfast
  )
  add_custom_target(siftfast_matlab ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${MATLAB_MEX})
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${MATLAB_MEX} siftfast.m DESTINATION share/siftfast/matlab)

else()
message(STATUS "MATLAB installation not found")
endif(MATLAB)

set(OCTAVE OCTAVE-NOTFOUND)
FIND_PROGRAM(OCTAVE NAME "mkoctfile" PATHS)

if(OCTAVE)

  set(OCTAVE_MEX ${CMAKE_CURRENT_BINARY_DIR}/siftfast.mex)
  message(STATUS "${OCTAVE_MEX}")

  add_custom_command(
    OUTPUT ${OCTAVE_MEX}
    COMMAND mkoctfile
    ARGS --mex ${CMAKE_CURRENT_SOURCE_DIR}/siftmex.cpp -llapack -L${CMAKE_CURRENT_BINARY_DIR} -lsiftfast ${MEX_CXXFLAGS} -o ${OCTAVE_MEX}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/siftmex.cpp libsiftfast
  )
  add_custom_target(siftfast_octave ALL DEPENDS ${OCTAVE_MEX})
  install(FILES ${OCTAVE_MEX} siftfast.m DESTINATION share/siftfast/octave)

else()
message(STATUS "Octave installation not found")
endif(OCTAVE)

install(FILES siftfast.h DESTINATION include/siftfast)
install(TARGETS siftfast DESTINATION bin)
install(TARGETS libsiftfast DESTINATION lib PERMISSIONS
                OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE)

# add make uninstall capability
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
