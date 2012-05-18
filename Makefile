SRC_VER = 1.5.3
SHELL=/bin/csh

src_files = VRAY_clusterThis.C VRAY_clusterThis.h \
VRAY_clusterThisChild.C  VRAY_clusterThisVolume.C \
VRAY_clusterThisChild.h  VRAY_clusterThisVolume.h \
VRAY_clusterThisChildVolume.h  VRAY_clusterThisChildVolume.C \
VRAY_clusterCVEXUtil.C VRAY_clusterThisInstance.C VRAY_clusterThisRunCVEX.C \
VRAY_clusterThisAttributeUtils.C VRAY_clusterThisRender.C

TAGINFO = $(shell (echo -n "Compiled on:" `date`"\n         by:" `whoami`@`hostname`"\n$(SESI_TAGINFO)") | sesitag -m)
CFLAGS := $(CFLAGS) ${TAGINFO} -DVERSION=\"${VERSION}\" -DHOUDINI_MAJOR_RELEASE=${HOUDINI_MAJOR_RELEASE} -DHOUDINI_MINOR_RELEASE=${HOUDINI_MINOR_RELEASE} -DHOUDINI_BUILD_VERSION=${HOUDINI_BUILD_VERSION}

all:
ifeq ($(OSTYPE),linux)
	g++ -g $(CFLAGS) -DVERSION=\"${VERSION}\" -DDLLEXPORT=  -D_GNU_SOURCE -DLINUX -DAMD64 -m64 -fPIC -DSIZEOF_VOID_P=8 -DSESI_LITTLE_ENDIAN -DENABLE_THREADS -DUSE_PTHREADS -D_REENTRANT -D_FILE_OFFSET_BITS=64 -c  -DGCC4 -DGCC3 -Wno-deprecated -I/opt/hfs${HOUDINI_VERSION}/toolkit/include -I/opt/hfs${HOUDINI_VERSION}/toolkit/include/htools -Wall -W -Wno-parentheses -Wno-sign-compare -Wno-reorder -Wno-uninitialized -Wunused -Wno-unused-parameter -O2 -DMAKING_DSO -o VRAY_clusterThis.o VRAY_clusterThis.C
	g++ -shared VRAY_clusterThis.o -L/usr/X11R6/lib64 -L/usr/X11R6/lib -lGLU -lGL -lX11 -lXext -lXi -ldl -o ./VRAY_clusterThis.so
endif

ifeq ($(OSTYPE),darwin)
	g++ -g $(CFLAGS) -DVERSION=\"${VERSION}\" -DDLLEXPORT=  -D_GNU_SOURCE -DMBSD -DMBSD_COCOA -DMBSD_INTEL -arch x86_64 -DAMD64 -fPIC -DSIZEOF_VOID_P=8 -DSESI_LITTLE_ENDIAN -DENABLE_THREADS -DUSE_PTHREADS -D_REENTRANT -D_FILE_OFFSET_BITS=64 -fobjc-gc-only -c  -DGCC4 -DGCC3 -Wno-deprecated -I/opt/hfs/toolkit/include -I/opt/hfs/toolkit/include/htools -Wall -W -Wno-parentheses -Wno-sign-compare -Wno-reorder -Wno-uninitialized -Wunused -Wno-unused-parameter -O2 -DMAKING_DSO -o VRAY_clusterThis.o VRAY_clusterThis.C
	g++ -shared VRAY_clusterThis.o -L/usr/X11R6/lib64 -L/usr/X11R6/lib -lGLU -lGL -lX11 -lXext -lXi -ldl -o ./VRAY_clusterThis.dylib
endif

install: ${src_files} 
ifeq ($(OSTYPE),linux)
	strip ${DSONAME}
	cp ${DSONAME} ${HOME}/houdini${VERSION}/dso/mantra/
endif
ifeq ($(OSTYPE),darwin)
	cp ${DSONAME} ${HOME}/houdini${VERSION}/dso/mantra/
endif

archive_src:
	rm -fr docs
	doxygen Doxy_config
	tar zcvf mantra_clusterThis_v${SRC_VER}.src.tar.gz \
		${src_files} \
		docs/	\
		Makefile Doxy_config \
		README.txt \
      help.html
      
clean: 
	rm -f *.o *.so *.dll *.exp *.lib *.dylib

