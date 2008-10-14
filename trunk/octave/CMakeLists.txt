link_directories (${libsiftfast_BIN_DIR}) 

# # Determine on the flight the system we are running on
# Darwin_ARCH := mac
# Linux_ARCH  := glx
# ARCH := $($(shell uname)_ARCH)
# 
# mac_CFLAGS       := -O -I. -faltivec -pedantic -Wall
# mac_MEX_CFLAGS   := CFLAGS='$(mac_CFLAGS)'
# mac_MEX_SUFFIX   := mexmac
# 
# #glx_CFLAGS_MAT := -O -I. -pedantic -Wall
# glx_MEX_CFLAGS_MAT   := CXXFLAGS='$(CXXFLAGS)' -O -I. -llapack -L$(shell pwd) -Wl,-rpath,$(shell pwd) -lsift_fast 
# glx_MEX_CFLAGS_OCT := -I. -Wall -llapack -L$(shell pwd) -Wl,-rpath,$(shell pwd) -lsift_fast
# glx_MEX_CFLAGS   := $(CFLAGS) $(glx_CFLAGS) 
# glx_MEX_SUFFIX   := mexglx
# 
# MEX_SUFFIX   := $($(ARCH)_MEX_SUFFIX) 
# MEX_CFLAGS_MAT   := $($(ARCH)_MEX_CFLAGS_MAT)
# MEX_CFLAGS_OCT   := $($(ARCH)_MEX_CFLAGS_OCT)
# 
# 
# sift_fast.mex: libsift_fast.so sift_mex.cpp sift_fast.h
# 	if [ `which mkoctfile` ]; then \
# 		mkoctfile  --mex $(MEX_CFLAGS_OCT) sift_mex.cpp -o octave/sift_fast.mex; fi
