# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Mansa/ESP-IDF/v5.2.1/esp-idf/components/bootloader/subproject"
  "C:/Mansa/Classes/ECE_445/ECE-445/src/blink/build/bootloader"
  "C:/Mansa/Classes/ECE_445/ECE-445/src/blink/build/bootloader-prefix"
  "C:/Mansa/Classes/ECE_445/ECE-445/src/blink/build/bootloader-prefix/tmp"
  "C:/Mansa/Classes/ECE_445/ECE-445/src/blink/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Mansa/Classes/ECE_445/ECE-445/src/blink/build/bootloader-prefix/src"
  "C:/Mansa/Classes/ECE_445/ECE-445/src/blink/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Mansa/Classes/ECE_445/ECE-445/src/blink/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Mansa/Classes/ECE_445/ECE-445/src/blink/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
