# Copyright (C) 2015-2018, Gepard Graphics
# Copyright (C) 2015-2018, Dániel Bátyai <dbatyai@inf.u-szeged.hu>
# Copyright (C) 2016, Kristof Kosztyo <kkristof@inf.u-szeged.hu>
# Copyright (C) 2016, Szilard Ledan <szledan@gmail.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

SET(GEPARD_DEP_LIBS "")
SET(GEPARD_DEP_INCLUDES "")

if (BACKEND STREQUAL "GLES2")
  find_package(GLESv2 REQUIRED)
  find_package(EGL REQUIRED)

  list(APPEND GEPARD_DEP_LIBS ${GLESv2_LIBRARY} ${EGL_LIBRARY})
  list(APPEND GEPARD_DEP_INCLUDES ${GLESv2_INCLUDE_DIR} ${EGL_INCLUDE_DIR})
elseif (BACKEND STREQUAL "VULKAN")
  find_package(Vulkan)
  if (NOT VULKAN_FOUND)
    message(STATUS "Vulkan headers will be downloaded in a build step")
    add_custom_command (OUTPUT ${PROJECT_BINARY_DIR}/thirdparty/include/vulkan/vulkan.h
                        COMMAND mkdir -p "${PROJECT_BINARY_DIR}/thirdparty/include/vulkan"
                        COMMAND wget -O "${PROJECT_BINARY_DIR}/thirdparty/include/vulkan/vulkan.h"
                                        "https://raw.githubusercontent.com/KhronosGroup/Vulkan-Docs/1.0/src/vulkan/vulkan.h")

    add_custom_command (OUTPUT ${PROJECT_BINARY_DIR}/thirdparty/include/vulkan/vk_platform.h
                        COMMAND mkdir -p "${PROJECT_BINARY_DIR}/thirdparty/include/vulkan"
                        COMMAND wget -O "${PROJECT_BINARY_DIR}/thirdparty/include/vulkan/vk_platform.h"
                                        "https://raw.githubusercontent.com/KhronosGroup/Vulkan-Docs/1.0/src/vulkan/vk_platform.h")

    add_custom_target(vulkan_headers
                      DEPENDS ${PROJECT_BINARY_DIR}/thirdparty/include/vulkan/vulkan.h
                              ${PROJECT_BINARY_DIR}/thirdparty/include/vulkan/vk_platform.h)

    set(VULKAN_INCLUDE_DIR ${PROJECT_BINARY_DIR}/thirdparty/include)
  endif()

  find_program(GLSLANG_VALIDATOR
               NAMES glslangValidator
               HINTS ${PROJECT_SOURCE_DIR}/thirdparty/bin)

  # TODO(kkristof) remove this once XSync has been removed from GepardVulkan::createSwapChain
  find_package(X11)
  list(APPEND GEPARD_DEP_LIBS ${X11_LIBRARIES})

  list(APPEND GEPARD_DEP_INCLUDES ${VULKAN_INCLUDE_DIR})
  list(APPEND GEPARD_DEP_LIBS ${CMAKE_DL_LIBS})
endif ()
