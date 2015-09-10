# Just to show qml files in QtCreator
# Files are ignored by CMake when passed to add_executable()
# See: https://bugreports.qt.io/browse/QTCREATORBUG-3166
file(GLOB_RECURSE QMLSRCS *.qml *.js)
melp_add_sources(SRCS ${QMLSRCS})

melp_add_sources(SRCS sevensegmentdisplay.cpp)