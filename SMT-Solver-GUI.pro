#-------------------------------------------------
#
# Project created by QtCreator 2015-11-29T20:36:47
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SMT-Solver-GUI
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
        smtsolver.cpp \
        minisat/core/Solver.cc \
        minisat/simp/SimpSolver.cc \
        minisat/utils/Options.cc \
        minisat/utils/System.cc \


HEADERS  += mainwindow.h \
    smtsolver.h \
    minisat/core/Dimacs.h \
    minisat/core/Solver.h \
    minisat/core/SolverTypes.h \
    minisat/mtl/Alg.h \
    minisat/mtl/Alloc.h \
    minisat/mtl/Heap.h \
    minisat/mtl/IntMap.h \
    minisat/mtl/IntTypes.h \
    minisat/mtl/Map.h \
    minisat/mtl/Queue.h \
    minisat/mtl/Rnd.h \
    minisat/mtl/Sort.h \
    minisat/mtl/Vec.h \
    minisat/mtl/XAlloc.h \
    minisat/simp/SimpSolver.h \
    minisat/utils/Options.h \
    minisat/utils/ParseUtils.h \
    minisat/utils/System.h \


FORMS    += mainwindow.ui

macx: LIBS += -L$$PWD/lib/ -lz.1
unix: LIBS += -L$$PWD/lib/ -lz
INCLUDEPATH += /usr/include/
DEPENDPATH += /usr/include/
