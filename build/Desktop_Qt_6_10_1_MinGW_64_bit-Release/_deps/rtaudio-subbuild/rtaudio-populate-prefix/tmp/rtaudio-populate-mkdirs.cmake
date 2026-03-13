# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "C:/c++code/Kala/build/Desktop_Qt_6_10_1_MinGW_64_bit-Release/_deps/rtaudio-src")
  file(MAKE_DIRECTORY "C:/c++code/Kala/build/Desktop_Qt_6_10_1_MinGW_64_bit-Release/_deps/rtaudio-src")
endif()
file(MAKE_DIRECTORY
  "C:/c++code/Kala/build/Desktop_Qt_6_10_1_MinGW_64_bit-Release/_deps/rtaudio-build"
  "C:/c++code/Kala/build/Desktop_Qt_6_10_1_MinGW_64_bit-Release/_deps/rtaudio-subbuild/rtaudio-populate-prefix"
  "C:/c++code/Kala/build/Desktop_Qt_6_10_1_MinGW_64_bit-Release/_deps/rtaudio-subbuild/rtaudio-populate-prefix/tmp"
  "C:/c++code/Kala/build/Desktop_Qt_6_10_1_MinGW_64_bit-Release/_deps/rtaudio-subbuild/rtaudio-populate-prefix/src/rtaudio-populate-stamp"
  "C:/c++code/Kala/build/Desktop_Qt_6_10_1_MinGW_64_bit-Release/_deps/rtaudio-subbuild/rtaudio-populate-prefix/src"
  "C:/c++code/Kala/build/Desktop_Qt_6_10_1_MinGW_64_bit-Release/_deps/rtaudio-subbuild/rtaudio-populate-prefix/src/rtaudio-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/c++code/Kala/build/Desktop_Qt_6_10_1_MinGW_64_bit-Release/_deps/rtaudio-subbuild/rtaudio-populate-prefix/src/rtaudio-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/c++code/Kala/build/Desktop_Qt_6_10_1_MinGW_64_bit-Release/_deps/rtaudio-subbuild/rtaudio-populate-prefix/src/rtaudio-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
