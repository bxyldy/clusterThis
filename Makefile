SHELL=/bin/csh
DBG=-g
src = VRAY_clusterThis.cpp VRAY_clusterThis.h \
   VRAY_clusterThisCVEXUtil.cpp VRAY_clusterThisInstance.cpp \
   VRAY_clusterThisRunCVEX.cpp VRAY_clusterThisAttributeUtils.cpp \
   VRAY_clusterThisPreProcess.cpp VRAY_clusterThisPostProcess.cpp VRAY_clusterThisParms.cpp \
   VRAY_clusterThisUtil.cpp VRAY_clusterThisRender.cpp

SOURCES = VRAY_clusterThis.cpp
H_CFLAGS =  $(shell hcustom --cflags)
# -DVERSION=\"12.1.33\" -D_GNU_SOURCE -DLINUX -DAMD64 -m64 -fPIC -DSIZEOF_VOID_P=8 -DSESI_LITTLE_ENDIAN
# -DENABLE_THREADS -DUSE_PTHREADS -D_REENTRANT -D_FILE_OFFSET_BITS=64 -c -DGCC4 -DGCC3 -Wno-deprecated
# -I/opt/hfs/toolkit/include -Wall -W -Wno-parentheses -Wno-sign-compare -Wno-reorder -Wno-uninitialized
# -Wunused -Wno-unused-parameter -O2 -fno-strict-aliasing

today = $(shell date +%j)
MAJOR_VER = "2"
MINOR_VER = "5"
BUILD_VER = "$(today)"
SRC_VER = $(MAJOR_VER).$(MINOR_VER).$(BUILD_VER)
SRC_VER_FLAGS = -DMAJOR_VER=$(MAJOR_VER) -DMINOR_VER=$(MINOR_VER) -DBUILD_VER=$(BUILD_VER)

BUILD_VER = `grep BUILD_VERSION version.h | cut -f 3 -d ' '`

TAGINFO = $(shell (echo -n "Compiled on:" `date`"\n  by:" `whoami`@`hostname`"\n$(SESI_TAGINFO)") | /opt/hfs/bin/sesitag -m)
CFLAGS := $(CFLAGS) $(H_CFLAGS) -I/usr/local/include/ $(SRC_VER_FLAGS) $(DBG) ${TAGINFO} -ftree-vectorize -ftree-vectorizer-verbose=2

INSTDIR = $(DCA_COMMON)/lib/houdini/dso_x86_64/mantra/
DSONAME = VRAY_clusterThis.so

all: clusterThis

Debug: clusterThis install
Release: clusterThis install
clusterThis: $(src)
ifeq ($(OSTYPE),linux)
	$(CXX) $(DBG) $(CFLAGS) -I/usr/local/include -I/usr/local/include/openvdb_houdini -o VRAY_clusterThis.o VRAY_clusterThis.cpp
	$(CXX) -shared VRAY_clusterThis.o -L/usr/X11R6/lib64 -L/usr/local/ -lopenvdb -L/usr/X11R6/lib -lGLU -lGL -lX11 -lXext -lXi -ldl -o ./VRAY_clusterThis.so
endif


install_dso: VRAY_clusterThis.so
	cp ./VRAY_clusterThis.so $(DCA_COMMON)/lib/houdini/dso_x86_64/mantra/VRAY_clusterThis.so

install: ${src} VRAY_clusterThis.so
ifeq ($(OSTYPE),linux)
	-@echo $(SRC_VER)
#	strip ${DSONAME}
	cp ${DSONAME} $(INSTDIR)
endif
ifeq ($(OSTYPE),darwin)
	cp ${DSONAME}.dylib ${HOME}/houdini${VERSION}/dso/mantra/
endif

get_OTL:
	cp $(DCA_COMMON)/lib/houdini/otls/VM_GEO_clusterThis.otl .

archive_src:
	rm -fr docs
	doxygen Doxyfile
	tar zcvf mantra_clusterThis_v${SRC_VER}.src.tar.gz \
		${src} \
		docs/	\
		Makefile Doxyfile

clean:
	rm -f *.o *.so *.dll *.exp *.lib *.dylib


