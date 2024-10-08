/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * RDP Proxy Utils
 *
 * Copyright 2016 Armin Novak <armin.novak@gmail.com>
 * Copyright 2022 Adrian Vollmer <adrian.vollmer@syss.de>
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

#ifndef FREERDP_PROXY_UTILS_H
#define FREERDP_PROXY_UTILS_H

#include <freerdp/api.h>
#include <freerdp/settings.h>

#ifdef __cplusplus
extern "C"
{
#endif

	/** @brief parse a proxy environment variable string and populate settings from it
	 *
	 *  @param settings the settings to populate, must not be \b NULL
	 *  @param uri_in the proxy string to parse, must not be \b NULL
	 *
	 *  @return \b TRUE if parsed successfully
	 */
	FREERDP_API BOOL proxy_parse_uri(rdpSettings* settings, const char* uri_in);

#ifdef __cplusplus
}
#endif

#endif /* FREERDP_PROXY_UTILS_H */
