# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/ExtMemLoader"
  "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/ExtMemLoader/build"
  "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/ExtMemLoader"
  "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/ExtMemLoader/tmp"
  "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/ExtMemLoader/src/hg-h7s-som_ExtMemLoader-stamp"
  "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/ExtMemLoader/src"
  "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/ExtMemLoader/src/hg-h7s-som_ExtMemLoader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/ExtMemLoader/src/hg-h7s-som_ExtMemLoader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/ExtMemLoader/src/hg-h7s-som_ExtMemLoader-stamp${cfgdir}") # cfgdir has leading slash
endif()
