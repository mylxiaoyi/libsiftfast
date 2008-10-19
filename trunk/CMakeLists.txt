cmake_minimum_required (VERSION 2.6)
project (libsiftfast)
set( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE )

if( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g")
  add_definitions(" -Wall -fPIC ")
endif()

# check for SSE extensions
if( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
  add_definitions( "-msse2 -mfpmath=sse" )
elseif(MSVC)
  add_definitions( "/arch:SSE2 /fp:fast" )
endif()
message(STATUS "Using SSE2 extensions")


add_library(libsiftfast SHARED libsiftfast.cpp)
target_link_libraries(libsiftfast m stdc++)
set_target_properties(libsiftfast PROPERTIES OUTPUT_NAME  siftfast)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "libsiftfast install prefix" FORCE )
endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT) 

message(STATUS "installing to ${CMAKE_INSTALL_PREFIX}")

# compile without depending on libsiftfast
add_executable(siftfast libsiftfast.cpp siftfast.cpp)
#target_link_libraries(siftfast libsiftfast)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# check for OpenMP
if( NOT DEFINED USE_OPENMP OR USE_OPENMP  )
  include(CheckIncludeFile)
  CHECK_INCLUDE_FILE(omp.h HAVE_OMP_H)
  if( HAVE_OMP_H )
    message(STATUS "Using OpenMP")

    # check if can compile and use openmp
    if( WIN32 )
    else()
      if( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
        add_definitions(" -fopenmp ")
        target_link_libraries(siftfast gomp)
        set(OPENMP_LFLAGS "-lgomp")
      endif()
    endif()
  endif()
endif()

#
# generate mex files
#

set(MATLAB MATLAB-NOTFOUND)
FIND_PROGRAM(MATLAB NAME "mex" PATHS )

if( CMAKE_COMPILER_IS_GNUC OR CMAKE_COMPILER_IS_GNUCXX )
  set(MEX_CXXFLAGS "${OPENMP_LFLAGS} -Wl,-rpath,${CMAKE_INSTALL_PREFIX}/lib")
else()
  set(MEX_CXXFLAGS)
endif()

if(MATLAB)

  if( DARWIN OR APPLE )
    set(MEXEXT "mexmac")
  elseif( UNIX )
    set(MEXEXT "mexglx")
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
