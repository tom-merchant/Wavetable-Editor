cmake_minimum_required(VERSION 3.15)
project(wavetable_editor)

set(CMAKE_CXX_STANDARD 14)


find_package(PkgConfig REQUIRED)

pkg_check_modules(GTKMM REQUIRED IMPORTED_TARGET gtkmm-3.0)

include_directories(../AudioUtils)
include_directories(../Oscillator)
include_directories(../envelope)
include_directories(../portaudio-headers)
link_directories(../AudioUtils/cmake-build-debug)
link_directories(../Oscillator/cmake-build-debug)
link_directories(../envelope)

add_library(mujs mujs/main.c mujs/utf.c mujs/utftype.c mujs/jsarray.c mujs/regexp.c mujs/opnames.h mujs/jsvalue.c mujs/jsarray.c mujs/jsboolean.c mujs/jsboolean.c mujs/jsbuiltin.c mujs/jscompile.c mujs/jsdate.c mujs/jsdtoa.c mujs/jsdump.c mujs/jserror.c mujs/jsfunction.c mujs/jsgc.c mujs/jsintern.c mujs/jslex.c mujs/jsmath.c mujs/jsnumber.c mujs/json.c mujs/jsobject.c mujs/jsparse.c mujs/jsproperty.c mujs/jsregexp.c mujs/jsrepr.c mujs/jsrun.c mujs/jsstate.c mujs/jsstring.c mujs/pp.c)

add_executable(wavetable_editor main.cpp opengl_utils.cpp morph_preview.cpp oscilloscope_control.cpp harmonics_control.cpp EnvelopeEditor.cpp)

target_link_libraries(mujs readline)
target_link_libraries(wavetable_editor Oscillator AudioUtils envelope PkgConfig::GTKMM fftw3 mujs m GL portaudio)