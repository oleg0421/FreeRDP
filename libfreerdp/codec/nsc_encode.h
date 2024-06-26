/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * NSCodec Encoder
 *
 * Copyright 2012 Vic Lee
 * Copyright 2016 Armin Novak <armin.novak@thincast.com>
 * Copyright 2016 Thincast Technologies GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FREERDP_LIB_CODEC_NSC_ENCODE_H
#define FREERDP_LIB_CODEC_NSC_ENCODE_H

#include <freerdp/api.h>

FREERDP_LOCAL BOOL nsc_encode(NSC_CONTEXT* WINPR_RESTRICT context,
                              const BYTE* WINPR_RESTRICT bmpdata, UINT32 rowstride);

#endif /* FREERDP_LIB_CODEC_NSC_ENCODE_H */
