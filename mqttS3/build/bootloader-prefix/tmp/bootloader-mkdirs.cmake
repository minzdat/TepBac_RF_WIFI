# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Users/tminh/esp/v5.2.2/esp-idf/components/bootloader/subproject"
  "C:/Dev_minz/Tepbac_project/public_github/New folder/Esp32_node/mqttS3/build/bootloader"
  "C:/Dev_minz/Tepbac_project/public_github/New folder/Esp32_node/mqttS3/build/bootloader-prefix"
  "C:/Dev_minz/Tepbac_project/public_github/New folder/Esp32_node/mqttS3/build/bootloader-prefix/tmp"
  "C:/Dev_minz/Tepbac_project/public_github/New folder/Esp32_node/mqttS3/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Dev_minz/Tepbac_project/public_github/New folder/Esp32_node/mqttS3/build/bootloader-prefix/src"
  "C:/Dev_minz/Tepbac_project/public_github/New folder/Esp32_node/mqttS3/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Dev_minz/Tepbac_project/public_github/New folder/Esp32_node/mqttS3/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Dev_minz/Tepbac_project/public_github/New folder/Esp32_node/mqttS3/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
