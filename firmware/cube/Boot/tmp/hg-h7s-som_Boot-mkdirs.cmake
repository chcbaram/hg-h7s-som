# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/Boot"
  "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/Boot/build"
  "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/Boot"
  "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/Boot/tmp"
  "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/Boot/src/hg-h7s-som_Boot-stamp"
  "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/Boot/src"
  "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/Boot/src/hg-h7s-som_Boot-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/Boot/src/hg-h7s-som_Boot-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/Boot/src/hg-h7s-som_Boot-stamp${cfgdir}") # cfgdir has leading slash
endif()
