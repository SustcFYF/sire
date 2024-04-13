# SIRE 使用介绍

环境的编译与配置请参照Dockerfile的描述

开发环境配置文档待补充，目前主要使用vscode + cmake + vcpkg.

## 使用方法

注：和ARIS一致

1. 代码文件需要的地方使用 `#include <sire.hpp>`
2. CMakeLists.txt需要使用 `find_package()`指令进行查找，查找方法与aris一致

   示例：

   ```cmake
   # find Assimp
   set(TARGET_ASSIMP_PATH "" CACHE PATH "Assimp install path")
   if(EXISTS ${TARGET_ASSIMP_PATH})
   	message(STATUS "Directory to search Assimp at ${TARGET_ASSIMP_PATH}")
   	list(APPEND CMAKE_PREFIX_PATH ${TARGET_ASSIMP_PATH})
   else()
   	message(WARNING "File/Directory at variable TARGET_ASSIMP_PATH not exists!")
   endif()
   find_package(assimp REQUIRED)

   # find Hpp-fcl
   set(TARGET_HPP_FCL_PATH "" CACHE PATH "Hpp-fcl install path")
   if(EXISTS ${TARGET_HPP_FCL_PATH})
   	message(STATUS "Directory to search Assimp at ${TARGET_HPP_FCL_PATH}")
   	list(APPEND CMAKE_PREFIX_PATH ${TARGET_HPP_FCL_PATH})
   else()
   	message(WARNING "File/Directory at variable TARGET_HPP_FCL_PATH not exists!")
   endif()
   find_package(hpp-fcl REQUIRED)

   set(SIRE_INSTALL_PATH C:/sire CACHE PATH "Sire install path") # 设置默认查找位置 C:\sire
   if(EXISTS ${SIRE_INSTALL_PATH})
   	message(STATUS "Directory to search sire at ${SIRE_INSTALL_PATH}")
   	list(APPEND CMAKE_PREFIX_PATH ${SIRE_INSTALL_PATH})
   else()
   	message(WARNING "File/Directory at variable SIRE_INSTALL_PATH not exists!")
   endif()
   find_package(sire REQUIRED)

   include_directories(${SIRE_INSTALL_PATH})
   include_directories(${hpp-fcl_INCLUDE_DIRS})
   include_directories(${assimp_INCLUDE_DIRS})

   target_link_libraries(${PROJECT_NAME} ${sire_LIBRARIES})
   target_link_libraries(${PROJECT_NAME} assimp::assimp)
   target_link_libraries(${PROJECT_NAME} ${hpp-fcl_LIBRARIES})
   ```
