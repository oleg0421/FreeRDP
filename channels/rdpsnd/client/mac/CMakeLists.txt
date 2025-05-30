# FreeRDP: A Remote Desktop Protocol Implementation
# FreeRDP cmake build script
#
# Copyright 2012 Laxmikant Rashinkar <LK.Rashinkar@gmail.com>
# Copyright 2012 Marc-Andre Moreau <marcandre.moreau@gmail.com>
# Copyright 2013 Corey Clayton <can.of.tuna@gmail.com>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

define_channel_client_subsystem("rdpsnd" "mac" "")

find_library(COCOA_LIBRARY Cocoa REQUIRED)
find_library(CORE_FOUNDATION CoreFoundation)
find_library(CORE_AUDIO CoreAudio REQUIRED)
find_library(AUDIO_TOOL AudioToolbox REQUIRED)
find_library(AV_FOUNDATION AVFoundation REQUIRED)

set(${MODULE_PREFIX}_SRCS rdpsnd_mac.m)

set(${MODULE_PREFIX}_LIBS
    winpr
    freerdp
    ${AUDIO_TOOL}
    ${AV_FOUNDATION}
    ${CORE_AUDIO}
    ${COCOA_LIBRARY}
    ${CORE_FOUNDATION}
)

include_directories(..)
include_directories(SYSTEM ${MACAUDIO_INCLUDE_DIRS})

add_channel_client_subsystem_library(${MODULE_PREFIX} ${MODULE_NAME} ${CHANNEL_NAME} "" TRUE "")
