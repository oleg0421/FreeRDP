# FreeRDP: A Remote Desktop Protocol Implementation
# FreeRDP cmake build script
#
# Copyright 2016 Jiang Zihao <zihao.jiang@yahoo.com>
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

define_channel_server("rdpgfx")

set(${MODULE_PREFIX}_SRCS rdpgfx_main.c rdpgfx_main.h ../rdpgfx_common.c ../rdpgfx_common.h)

set(${MODULE_PREFIX}_LIBS freerdp)

include_directories(..)

add_channel_server_library(${MODULE_PREFIX} ${MODULE_NAME} ${CHANNEL_NAME} FALSE "DVCPluginEntry")
