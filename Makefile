
SRC_VER = 2.0.0
SHELL=/bin/csh
DBG=-g
SOURCES = VRAY_clusterThis.C
CFLAGS := $(CFLAGS) -ftree-vectorize -ftree-vectorizer-verbose=2

DSONAME = VRAY_clusterThis.so

include $(HT)/makefiles/Makefile.gnu

install_dso:
	cp ./VRAY_clusterThis.so $(DCA_COMMON)/lib/houdini/dso_x86_64/mantra/VRAY_clusterThis.so

#TAGINFO = $(shell (echo -n "Compiled on:" `date`"\n  by:" `whoami`@`hostname`"\n$(SESI_TAGINFO)") | /opt/hfs/bin/sesitag -m)
#CFLAGS := $(CFLAGS) ${TAGINFO} -ftree-vectorize -ftree-vectorizer-verbose=2

src = VRAY_clusterThis.C VRAY_clusterThis.h \
VRAY_clusterThisChild.C  VRAY_clusterThisChild.h  \
VRAY_clusterCVEXUtil.C VRAY_clusterThisInstance.C \
VRAY_clusterThisRunCVEX.C VRAY_clusterThisAttributeUtils.C \
VRAY_clusterThisRender.C

# all: clusterThis
# Debug: clusterThis install
# Release: clusterThis install

# clusterThis: $(src)



#ifeq ($(OSTYPE),linux)
#	$(CXX) $(DBG) $(CFLAGS) -DVERSION=\"${VERSION}\" -DDLLEXPORT=  -D_GNU_SOURCE -DLINUX -DAMD64 -m64 -fPIC -DSIZEOF_VOID_P=8 -DSESI_LITTLE_ENDIAN -DENABLE_THREADS \
#	-DUSE_PTHREADS -D_REENTRANT -D_FILE_OFFSET_BITS=64 -c  -DGCC4 -DGCC3 -Wno-deprecated -I/opt/hfs${HOUDINI_VERSION}/toolkit/include \
#	-I/opt/hfs${HOUDINI_VERSION}/toolkit/include/htools -Wall -W -Wno-parentheses -Wno-sign-compare -Wno-reorder -Wno-uninitialized -Wunused -Wno-unused-parameter \
#	-O2 -DMAKING_DSO -o VRAY_clusterThis.o VRAY_clusterThis.C
#	$(CXX) -shared VRAY_clusterThis.o -L/usr/X11R6/lib64 -L/usr/X11R6/lib -lGLU -lGL -lX11 -lXext -lXi -ldl -o ./VRAY_clusterThis.so
##	cp ./VRAY_clusterThis.so $(DCA_COMMON)/lib/houdini/dso_x86_64/mantra/VRAY_clusterThis.so
#endif
#
#
#ifeq ($(OSTYPE),darwin)
#	$(CXX) $(DBG) $(CFLAGS) -DVERSION=\"${VERSION}\" -DDLLEXPORT=  -D_GNU_SOURCE -DMBSD -DMBSD_COCOA -DMBSD_INTEL -arch x86_64 -DAMD64 -fPIC -DSIZEOF_VOID_P=8 \
#	-DSESI_LITTLE_ENDIAN -DENABLE_THREADS -DUSE_PTHREADS -D_REENTRANT -D_FILE_OFFSET_BITS=64 -fobjc-gc-only -c  -DGCC4 -DGCC3 -Wno-deprecated \
#	-I/opt/hfs/toolkit/include -I/opt/hfs/toolkit/include/htools -Wall -W -Wno-parentheses -Wno-sign-compare -Wno-reorder -Wno-uninitialized \
#	-Wunused -Wno-unused-parameter -O2 -DMAKING_DSO -o VRAY_clusterThis.o VRAY_clusterThis.C
#	$(CXX) -shared VRAY_clusterThis.o -L/usr/X11R6/lib64 -L/usr/X11R6/lib -lGLU -lGL -lX11 -lXext -lXi -ldl -o ./VRAY_clusterThis.dylib
#endif
#



#install: ${src}
#ifeq ($(OSTYPE),linux)
##	strip ${DSONAME}
#	cp ${DSONAME}.so $(DCA_COMMON)/lib/houdini/dso_x86_64/mantra/VRAY_clusterThis.so
##	cp ${DSONAME} ${HOME}/houdini${VERSION}/dso/mantra/
#endif
#ifeq ($(OSTYPE),darwin)
#	cp ${DSONAME}.dylib ${HOME}/houdini${VERSION}/dso/mantra/
#endif

#archive_src:
#	rm -fr docs
#	doxygen Doxyfile
#	tar zcvf mantra_clusterThis_v${SRC_VER}.src.tar.gz \
#		${src} \
#		docs/	\
#		Makefile Doxyfile \
#		README.txt \
#      help.html


#clean:
#	rm -f *.o *.so *.dll *.exp *.lib *.dylib




#       DSONAME = SOP_MySOP.so
#                (or SOP_MySOP.dylib for Mac OSX)
#                (or SOP_MySOP.dll for Windows)
#       SOURCES = SOP_MySOP.C
#       include $(HFS)/toolkit/makefiles/Makefile.gnu
# Then you just need to invoke make from the same directory.
#
# Complete list of variables used by this file:
#   OPTIMIZER   Override the optimization level (optional, defaults to -O2)
#   INCDIRS     Specify any additional include directories.
#   LIBDIRS     Specify any addition library include directories
#   SOURCES     List all .C files required to build the DSO or App
#   DSONAME     Name of the desired output DSO file (if applicable)
#   APPNAME     Name of the desires output application (if applicable)
#   INSTDIR     Directory to be installed. If not specified, this will
#               default to the the HOME houdini directory.
#   ICONS       Name of the icon files to install (optionial)
