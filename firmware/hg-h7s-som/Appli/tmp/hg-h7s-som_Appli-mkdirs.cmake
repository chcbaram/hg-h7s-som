# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/Users/hancheol/hdd/develop/STM32/hg-h7s-som/Appli"
  "/Users/hancheol/hdd/develop/STM32/hg-h7s-som/Appli/build"
  "/Users/hancheol/hdd/develop/STM32/hg-h7s-som/Appli"
  "/Users/hancheol/hdd/develop/STM32/hg-h7s-som/Appli/tmp"
  "/Users/hancheol/hdd/develop/STM32/hg-h7s-som/Appli/src/hg-h7s-som_Appli-stamp"
  "/Users/hancheol/hdd/develop/STM32/hg-h7s-som/Appli/src"
  "/Users/hancheol/hdd/develop/STM32/hg-h7s-som/Appli/src/hg-h7s-som_Appli-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/hancheol/hdd/develop/STM32/hg-h7s-som/Appli/src/hg-h7s-som_Appli-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/hancheol/hdd/develop/STM32/hg-h7s-som/Appli/src/hg-h7s-som_Appli-stamp${cfgdir}") # cfgdir has leading slash
endif()
