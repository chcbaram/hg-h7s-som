# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/Appli"
  "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/Appli/build"
  "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/Appli"
  "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/Appli/tmp"
  "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/Appli/src/hg-h7s-som_Appli-stamp"
  "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/Appli/src"
  "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/Appli/src/hg-h7s-som_Appli-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/Appli/src/hg-h7s-som_Appli-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/hancheol/hdd/git/hg-h7s-som/firmware/cube/Appli/src/hg-h7s-som_Appli-stamp${cfgdir}") # cfgdir has leading slash
endif()
