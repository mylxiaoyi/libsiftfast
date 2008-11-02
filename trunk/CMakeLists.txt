cmake_minimum_required (VERSION 2.4.7)
project (libsiftfast)
set( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE )

if( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g")
  add_definitions(" -Wall -fPIC ")
endif()

include(CheckIncludeFile)
include(CheckLibraryExists)
include(CheckCXXSourceRuns)
include(CheckCXXCompilerFlag)

# check for SSE extensions
if( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
  set(SSE_FLAGS)

  set(CMAKE_REQUIRED_FLAGS "-msse2")
  check_cxx_source_runs("
    #include <emmintrin.h>

    int main()
    {
        __m128d a, b;
        double vals[2] = {0};
        a = _mm_loadu_pd(vals);
        b = _mm_add_pd(a,a);
        _mm_storeu_pd(vals,b);
        return 0;
     }"
     HAS_SSE2_EXTENSIONS)

  set(CMAKE_REQUIRED_FLAGS "-msse")
  check_cxx_source_runs("
    #include <xmmintrin.h>
    int main()
    {
        __m128 a, b;
        float vals[4] = {0};
        a = _mm_loadu_ps(vals);
        b = a;
        b = _mm_add_ps(a,b);
        _mm_storeu_ps(vals,b);
        return 0;
    }"
    HAS_SSE_EXTENSIONS)

  set(CMAKE_REQUIRED_FLAGS)

  if(HAS_SSE2_EXTENSIONS)
    message(STATUS "Using SSE2 extensions")
    set(SSE_FLAGS "-msse2 -mfpmath=sse")
  elseif(HAS_SSE_EXTENSIONS)
    message(STATUS "Using SSE extensions")
    set(SSE_FLAGS "-msse -mfpmath=sse")
  endif()

  add_definitions(${SSE_FLAGS})
elseif(MSVC)
  check_cxx_source_runs("
    #include <emmintrin.h>

    int main()
    {
        __m128d a, b;
        double vals[2] = {0};
        a = _mm_loadu_pd(vals);
        b = _mm_add_pd(a,a);
        _mm_storeu_pd(vals,b);
        return 0;
     }"
     HAS_SSE2_EXTENSIONS)
  if( HAS_SSE2_EXTENSIONS )
    message(STATUS "Using SSE2 extensions")
    add_definitions( "/arch:SSE2 /fp:fast -D__SSE__ -D__SSE2__" )
  endif()
endif()

add_library(libsiftfast SHARED libsiftfast.cpp)
add_library(libsiftfast-static STATIC libsiftfast.cpp)

if( CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX )
  target_link_libraries(libsiftfast m stdc++)
endif()

set_target_properties(libsiftfast PROPERTIES OUTPUT_NAME  siftfast CLEAN_DIRECT_OUTPUT 1)
set_target_properties(libsiftfast-static PROPERTIES OUTPUT_NAME  siftfast CLEAN_DIRECT_OUTPUT 1)

#if(UNIX AND CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
#  set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "libsiftfast install prefix" FORCE )
#endif()

message(STATUS "installing to ${CMAKE_INSTALL_PREFIX}")

# compile without depending on libsiftfast
add_executable(siftfast libsiftfast.cpp siftfast.cpp)
#target_link_libraries(siftfast libsiftfast)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# check for OpenMP
if( NOT DEFINED USE_OPENMP OR USE_OPENMP  )

  if( WIN32 )
    CHECK_INCLUDE_FILE(omp.h HAVE_OMP_H)
    if( HAVE_OMP_H )
      message(STATUS "Using OpenMP")
      check_cxx_compiler_flag(/openmp HAVE_OPENMP)

      if( HAVE_OPENMP )
        add_definitions("/openmp")
      endif()
    endif()
  elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)

    # check if compilers supports -fopenmp
    INCLUDE(CheckCCompilerFlag)
    check_cxx_compiler_flag(-fopenmp HAVE_OPENMP)
    check_library_exists(gomp omp_get_num_threads "" HAS_GOMP_LIB)

    if( HAVE_OPENMP AND HAS_GOMP_LIB )
      add_definitions("-fopenmp")
      target_link_libraries(siftfast gomp)
      target_link_libraries(libsiftfast gomp)
      target_link_libraries(libsiftfast-static gomp)
      set(OPENMP_LFLAGS "-lgomp")
    endif()
  endif()
endif()

#
# generate mex files
#

set(MATLAB MATLAB-NOTFOUND)
if( WIN32 )
  FIND_PROGRAM(MATLAB NAME "mex.bat" PATHS )
else()
  FIND_PROGRAM(MATLAB NAME "mex" PATHS )
endif()

if( CMAKE_COMPILER_IS_GNUC OR CMAKE_COMPILER_IS_GNUCXX )
  if( WIN32 )
    set(MEX_CXXFLAGS "${OPENMP_LFLAGS} ")
  else()
    # check for lapack
    check_library_exists(lapack _init "" HAS_LAPACK_LIB)
    set(MEX_LIBS)
    if( HAS_LAPACK_LIB )
      set(MEX_LIBS "-llapack")
    endif()
    set(MEX_CXXFLAGS ${OPENMP_LFLAGS} ${MEX_LIBS} -Wl,-rpath,${CMAKE_INSTALL_PREFIX}/lib)
  endif()
elseif(MSVC)
  set(MEX_CXXFLAGS "-L\"${libsiftfast_BINARY_DIR}/Release\" -L\"${libsiftfast_BINARY_DIR}/Debug\"")
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

  if( MSVC )
    set(MATLAB_MEX_OUT "${CMAKE_CURRENT_BINARY_DIR}/matlab")
  else()
    set(MATLAB_MEX_OUT ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  add_custom_command(
    OUTPUT ${MATLAB_MEX_OUT}/${MATLAB_MEX}
    COMMAND mex
    ARGS -I\"${CMAKE_SOURCE_DIR}\" -L\"${libsiftfast_BINARY_DIR}\" ${MEX_CXXFLAGS} -lsiftfast -outdir \"${MATLAB_MEX_OUT}\" -output \"${MATLAB_MEX}\" \"${CMAKE_CURRENT_SOURCE_DIR}/siftmex.cpp\"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/siftmex.cpp libsiftfast libsiftfast-static
  )
  add_custom_target(siftfast_matlab ALL DEPENDS ${MATLAB_MEX_OUT}/${MATLAB_MEX})
  install(FILES ${MATLAB_MEX_OUT}/${MATLAB_MEX} siftfast.m DESTINATION share/siftfast/matlab)
else()
message(STATUS "MATLAB installation not found")
endif(MATLAB)

set(OCTAVE OCTAVE-NOTFOUND)
FIND_PROGRAM(OCTAVE NAME "mkoctfile" PATHS)

if(OCTAVE)

  if( MSVC )
    # need _mex since mkoctfile generates a siftfast.lib which messes with windows stuff
    set(OCTAVE_MEX ${CMAKE_CURRENT_BINARY_DIR}/octave/siftfast_mex.mex)
  else()
    set(OCTAVE_MEX ${CMAKE_CURRENT_BINARY_DIR}/siftfast.mex)
  endif()

  add_custom_command(
    OUTPUT ${OCTAVE_MEX}
    COMMAND mkoctfile
    ARGS --mex -I${CMAKE_SOURCE_DIR} -L\"${libsiftfast_BINARY_DIR}\" ${MEX_CXXFLAGS} -lsiftfast -o \"${OCTAVE_MEX}\" \"${CMAKE_CURRENT_SOURCE_DIR}/siftmex.cpp\"
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/siftmex.cpp libsiftfast libsiftfast-static
  )

  add_custom_target(siftfast_octave ALL DEPENDS ${OCTAVE_MEX})
  if( MSVC )
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/octave/siftfast.mex
      COMMAND ${CMAKE_COMMAND} -E copy
      ARGS \"${OCTAVE_MEX}\" \"${CMAKE_CURRENT_BINARY_DIR}/octave/siftfast.mex\"
      DEPENDS ${OCTAVE_MEX}
    )
    add_custom_target(siftfast_octave2 ALL DEPENDS DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/octave/siftfast.mex)
  endif()

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
