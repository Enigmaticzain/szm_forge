# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/mnt/D/projects/szm_forge/build2/_deps/cpp_httplib-src"
  "/mnt/D/projects/szm_forge/build2/_deps/cpp_httplib-build"
  "/mnt/D/projects/szm_forge/build2/_deps/cpp_httplib-subbuild/cpp_httplib-populate-prefix"
  "/mnt/D/projects/szm_forge/build2/_deps/cpp_httplib-subbuild/cpp_httplib-populate-prefix/tmp"
  "/mnt/D/projects/szm_forge/build2/_deps/cpp_httplib-subbuild/cpp_httplib-populate-prefix/src/cpp_httplib-populate-stamp"
  "/mnt/D/projects/szm_forge/build2/_deps/cpp_httplib-subbuild/cpp_httplib-populate-prefix/src"
  "/mnt/D/projects/szm_forge/build2/_deps/cpp_httplib-subbuild/cpp_httplib-populate-prefix/src/cpp_httplib-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/mnt/D/projects/szm_forge/build2/_deps/cpp_httplib-subbuild/cpp_httplib-populate-prefix/src/cpp_httplib-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/mnt/D/projects/szm_forge/build2/_deps/cpp_httplib-subbuild/cpp_httplib-populate-prefix/src/cpp_httplib-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
