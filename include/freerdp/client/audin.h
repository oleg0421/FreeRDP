/**
 * FreeRDP: A Remote Desktop Protocol Implementation
 * Audio Input Redirection Virtual Channel
 *
 * Copyright 2010-2011 Vic Lee
 * Copyright 2015 Thincast Technologies GmbH
 * Copyright 2015 DI (FH) Martin Haimberger <martin.haimberger@thincast.com>
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

#ifndef FREERDP_CHANNEL_AUDIN_CLIENT_AUDIN_H
#define FREERDP_CHANNEL_AUDIN_CLIENT_AUDIN_H

#include <freerdp/api.h>
#include <freerdp/types.h>

#include <freerdp/channels/audin.h>
#include <freerdp/codec/audio.h>

#ifdef __cplusplus
extern "C"
{
#endif

	/**
	 * Subsystem Interface
	 */

	typedef UINT (*AudinReceive)(const AUDIO_FORMAT* format, const BYTE* data, size_t size,
	                             void* userData);

	typedef struct s_IAudinDevice IAudinDevice;
	struct s_IAudinDevice
	{
		UINT (*Open)(IAudinDevice* devplugin, AudinReceive receive, void* userData);
		BOOL (*FormatSupported)(IAudinDevice* devplugin, const AUDIO_FORMAT* format);
		UINT(*SetFormat)
		(IAudinDevice* devplugin, const AUDIO_FORMAT* format, UINT32 FramesPerPacket);
		UINT (*Close)(IAudinDevice* devplugin);
		UINT (*Free)(IAudinDevice* devplugin);
	};

#define AUDIN_DEVICE_EXPORT_FUNC_NAME "freerdp_audin_client_subsystem_entry"

typedef UINT (*PREGISTERAUDINDEVICE)(IWTSPlugin* plugin, IAudinDevice* device);

typedef struct
{
	IWTSPlugin* plugin;
	PREGISTERAUDINDEVICE pRegisterAudinDevice;
	const ADDIN_ARGV* args;
	rdpContext* rdpcontext;
} FREERDP_AUDIN_DEVICE_ENTRY_POINTS;
typedef FREERDP_AUDIN_DEVICE_ENTRY_POINTS* PFREERDP_AUDIN_DEVICE_ENTRY_POINTS;

typedef UINT(VCAPITYPE* PFREERDP_AUDIN_DEVICE_ENTRY)(
	PFREERDP_AUDIN_DEVICE_ENTRY_POINTS pEntryPoints);

#ifdef __cplusplus
}
#endif

#endif /* FREERDP_CHANNEL_AUDIN_CLIENT_AUDIN_H */
