######################################################################
# Nernst Potential Simulator
######################################################################

TEMPLATE = app
TARGET = nernst
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

unix:!macx {
   message( "Generating makefile for Linux systems." )
   DEFINES += BLR_USELINUX HAVE_SSE2
   QMAKE_CFLAGS += -msse2
}

macx {
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
   DEFINES += BLR_USEWIN
}

# Input
HEADERS += atom.h ctrl.h gui.h options.h paint.h sim.h util.h world.h
SOURCES += atom.c ctrl.cpp gui.cpp main.cpp options.c paint.cpp sim.cpp world.c ../SFMT/SFMT.c

