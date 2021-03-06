######################################################################
# Nernst Potential Simulator
######################################################################

TEMPLATE = app
# TARGET = "Nernst Potential Simulator"
TARGET = "nernst"
RESOURCES = resources.qrc
DEPENDPATH += .
INCLUDEPATH += ../SFMT
DEFINES += MEXP=132049

QMAKE_CFLAGS_RELEASE -= -O2
QMAKE_CFLAGS_RELEASE += -O3
QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -O3
CONFIG += warn_on 
QT += opengl

isEmpty( MACTARGET ) {
   MACTARGET = intel
}

!contains( CONFIG, static ) {
   !contains( CONFIG, shared ) {
      CONFIG += shared
   }
}

unix:!macx {
   message( "Generating makefile for Linux systems." )
   INCLUDEPATH += /usr/include/qwt-qt4
   DEFINES += BLR_USELINUX HAVE_SSE2
   LIBS += -lqwt-qt4
   QMAKE_CFLAGS += -msse2
}

macx {
   INCLUDEPATH += /usr/local/qwt-5.2.1/include
   QMAKE_LIBDIR += /usr/local/qwt-5.2.1/lib
   LIBS += -lqwt
   ICON = img/nernst.icns

   contains( MACTARGET, intel ) {
      message( "Generating makefile for Intel Macs." )
      DEFINES += BLR_USEMAC HAVE_SSE2
      QMAKE_CFLAGS += -msse2
      CONFIG += x86
   }

   contains( MACTARGET, ppc ) {
      message( "Generating makefile for PowerPC Macs." )
      DEFINES += BLR_USEMAC
      CONFIG += ppc

      # if we're building a universal application, disable SSE (because it 
      # won't work on PPC) and set the SDK path to the universal SDK
      contains( MACTARGET, intel ) {
         DEFINES -= HAVE_SSE2
         QMAKE_CFLAGS -= -msse2
         QMAKE_MAC_SDK = /Developer/SDKs/MacOSX10.4u.sdk  
      }
   }
}

win32 {
   message( "Generating makefile for Windows." )
   INCLUDEPATH += "C:\Qwt\static\src"
   QMAKE_LIBDIR += "C:\Qwt\static\lib"
   DEFINES += BLR_USEWIN
   LIBS += -lqwt
   RC_FILE = win32_resources.rc
}

# Input
HEADERS += ctrl.h gui.h options.h paint.h safecalls.h sim.h status.h util.h xsim.h
SOURCES += ctrl.cpp gui.cpp main.cpp options.cpp paint.cpp safecalls.cpp sim.cpp status.cpp xsim.cpp ../SFMT/SFMT.c

