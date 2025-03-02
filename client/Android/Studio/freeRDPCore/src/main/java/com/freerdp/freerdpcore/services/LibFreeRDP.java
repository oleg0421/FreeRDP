/*
   Android FreeRDP JNI Wrapper

   Copyright 2013 Thincast Technologies GmbH, Author: Martin Fleisz

   This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
   If a copy of the MPL was not distributed with this file, You can obtain one at
   http://mozilla.org/MPL/2.0/.
*/

package com.freerdp.freerdpcore.services;

import android.content.Context;
import android.graphics.Bitmap;
import android.net.Uri;
import android.util.Log;

import androidx.collection.LongSparseArray;

import com.freerdp.freerdpcore.application.GlobalApp;
import com.freerdp.freerdpcore.application.SessionState;
import com.freerdp.freerdpcore.domain.BookmarkBase;
import com.freerdp.freerdpcore.domain.ManualBookmark;
import com.freerdp.freerdpcore.presentation.ApplicationSettingsActivity;

import java.util.ArrayList;
import java.util.Objects;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class LibFreeRDP
{
	private static final String TAG = "LibFreeRDP";
	private static EventListener listener;
	private static boolean mHasH264 = false;

	private static final LongSparseArray<Boolean> mInstanceState = new LongSparseArray<>();

	public static final long VERIFY_CERT_FLAG_NONE = 0x00;
	public static final long VERIFY_CERT_FLAG_LEGACY = 0x02;
	public static final long VERIFY_CERT_FLAG_REDIRECT = 0x10;
	public static final long VERIFY_CERT_FLAG_GATEWAY = 0x20;
	public static final long VERIFY_CERT_FLAG_CHANGED = 0x40;
	public static final long VERIFY_CERT_FLAG_MISMATCH = 0x80;
	public static final long VERIFY_CERT_FLAG_MATCH_LEGACY_SHA1 = 0x100;
	public static final long VERIFY_CERT_FLAG_FP_IS_PEM = 0x200;

	private static boolean tryLoad(String[] libraries)
	{
		boolean success = false;
		final String LD_PATH = System.getProperty("java.library.path");
		for (String lib : libraries)
		{
			try
			{
				Log.v(TAG, "Trying to load library " + lib + " from LD_PATH: " + LD_PATH);
				System.loadLibrary(lib);
				success = true;
			}
			catch (UnsatisfiedLinkError e)
			{
				Log.e(TAG, "Failed to load library " + lib + ": " + e);
				success = false;
				break;
			}
		}

		return success;
	}

	private static boolean tryLoad(String library)
	{
		return tryLoad(new String[] { library });
	}

	static
	{
		try
		{
			System.loadLibrary("freerdp-android");

			/* Load dependent libraries too to trigger JNI_OnLoad calls */
			String version = freerdp_get_jni_version();
			String[] versions = version.split("[\\.-]");
			if (versions.length > 0)
			{
				System.loadLibrary("freerdp-client" + versions[0]);
				System.loadLibrary("freerdp" + versions[0]);
				System.loadLibrary("winpr" + versions[0]);
			}
			Pattern pattern = Pattern.compile("^(\\d+)\\.(\\d+)\\.(\\d+).*");
			Matcher matcher = pattern.matcher(version);
			if (!matcher.matches() || (matcher.groupCount() < 3))
				throw new RuntimeException("APK broken: native library version " + version +
				                           " does not meet requirements!");
			int major = Integer.parseInt(Objects.requireNonNull(matcher.group(1)));
			int minor = Integer.parseInt(Objects.requireNonNull(matcher.group(2)));
			int patch = Integer.parseInt(Objects.requireNonNull(matcher.group(3)));

			if (major > 2)
				mHasH264 = freerdp_has_h264();
			else if (minor > 5)
				mHasH264 = freerdp_has_h264();
			else if ((minor == 5) && (patch >= 1))
				mHasH264 = freerdp_has_h264();
			else
				throw new RuntimeException("APK broken: native library version " + version +
				                           " does not meet requirements!");
			Log.i(TAG, "Successfully loaded native library. H264 is " +
			               (mHasH264 ? "supported" : "not available"));
		}
		catch (UnsatisfiedLinkError e)
		{
			Log.e(TAG, "Failed to load library: " + e);
			throw e;
		}
	}

	public static boolean hasH264Support()
	{
		return mHasH264;
	}

	private static native boolean freerdp_has_h264();

	private static native String freerdp_get_jni_version();

	private static native String freerdp_get_version();

	private static native String freerdp_get_build_revision();

	private static native String freerdp_get_build_config();

	private static native long freerdp_new(Context context);

	private static native void freerdp_free(long inst);

	private static native boolean freerdp_parse_arguments(long inst, String[] args);

	private static native boolean freerdp_connect(long inst);

	private static native boolean freerdp_disconnect(long inst);

	private static native boolean freerdp_update_graphics(long inst, Bitmap bitmap, int x, int y,
	                                                      int width, int height);

	private static native boolean freerdp_send_cursor_event(long inst, int x, int y, int flags);

	private static native boolean freerdp_send_key_event(long inst, int keycode, boolean down);

	private static native boolean freerdp_send_unicodekey_event(long inst, int keycode,
	                                                            boolean down);

	private static native boolean freerdp_send_clipboard_data(long inst, String data);

	private static native String freerdp_get_last_error_string(long inst);

	public static void setEventListener(EventListener l)
	{
		listener = l;
	}

	public static long newInstance(Context context)
	{
		return freerdp_new(context);
	}

	public static void freeInstance(long inst)
	{
		synchronized (mInstanceState)
		{
			if (mInstanceState.get(inst, false))
			{
				freerdp_disconnect(inst);
			}
			while (mInstanceState.get(inst, false))
			{
				try
				{
					mInstanceState.wait();
				}
				catch (InterruptedException e)
				{
					throw new RuntimeException();
				}
			}
		}
		freerdp_free(inst);
	}

	public static boolean connect(long inst)
	{
		synchronized (mInstanceState)
		{
			if (mInstanceState.get(inst, false))
			{
				throw new RuntimeException("instance already connected");
			}
		}
		return freerdp_connect(inst);
	}

	public static boolean disconnect(long inst)
	{
		synchronized (mInstanceState)
		{
			if (mInstanceState.get(inst, false))
			{
				return freerdp_disconnect(inst);
			}
			return true;
		}
	}

	public static boolean cancelConnection(long inst)
	{
		synchronized (mInstanceState)
		{
			if (mInstanceState.get(inst, false))
			{
				return freerdp_disconnect(inst);
			}
			return true;
		}
	}

	private static String addFlag(String name, boolean enabled)
	{
		if (enabled)
		{
			return "+" + name;
		}
		return "-" + name;
	}

	public static boolean setConnectionInfo(Context context, long inst, BookmarkBase bookmark)
	{
		BookmarkBase.ScreenSettings screenSettings = bookmark.getActiveScreenSettings();
		BookmarkBase.AdvancedSettings advanced = bookmark.getAdvancedSettings();
		BookmarkBase.DebugSettings debug = bookmark.getDebugSettings();

		String arg;
		ArrayList<String> args = new ArrayList<>();

		args.add(TAG);
		args.add("/gdi:sw");

		final String clientName = ApplicationSettingsActivity.getClientName(context);
		if (!clientName.isEmpty())
		{
			args.add("/client-hostname:" + clientName);
		}
		String certName = "";
		if (bookmark.getType() != BookmarkBase.TYPE_MANUAL)
		{
			return false;
		}

		int port = bookmark.<ManualBookmark>get().getPort();
		String hostname = bookmark.<ManualBookmark>get().getHostname();

		args.add("/v:" + hostname);
		args.add("/port:" + port);

		arg = bookmark.getUsername();
		if (!arg.isEmpty())
		{
			args.add("/u:" + arg);
		}
		arg = bookmark.getDomain();
		if (!arg.isEmpty())
		{
			args.add("/d:" + arg);
		}
		arg = bookmark.getPassword();
		if (!arg.isEmpty())
		{
			args.add("/p:" + arg);
		}

		args.add(
		    String.format("/size:%dx%d", screenSettings.getWidth(), screenSettings.getHeight()));
		args.add("/bpp:" + screenSettings.getColors());

		if (advanced.getConsoleMode())
		{
			args.add("/admin");
		}

		switch (advanced.getSecurity())
		{
			case 3: // NLA
				args.add("/sec:nla");
				break;
			case 2: // TLS
				args.add("/sec:tls");
				break;
			case 1: // RDP
				args.add("/sec:rdp");
				break;
			default:
				break;
		}

		if (!certName.isEmpty())
		{
			args.add("/cert-name:" + certName);
		}

		BookmarkBase.PerformanceFlags flags = bookmark.getActivePerformanceFlags();
		if (flags.getRemoteFX())
		{
			args.add("/rfx");
		}

		if (flags.getGfx())
		{
			args.add("/gfx");
		}

		if (flags.getH264() && mHasH264)
		{
			args.add("/gfx:AVC444");
		}

		args.add(addFlag("wallpaper", flags.getWallpaper()));
		args.add(addFlag("window-drag", flags.getFullWindowDrag()));
		args.add(addFlag("menu-anims", flags.getMenuAnimations()));
		args.add(addFlag("themes", flags.getTheming()));
		args.add(addFlag("fonts", flags.getFontSmoothing()));
		args.add(addFlag("aero", flags.getDesktopComposition()));

		if (!advanced.getRemoteProgram().isEmpty())
		{
			args.add("/shell:" + advanced.getRemoteProgram());
		}

		if (!advanced.getWorkDir().isEmpty())
		{
			args.add("/shell-dir:" + advanced.getWorkDir());
		}

		args.add(addFlag("async-channels", debug.getAsyncChannel()));
		args.add(addFlag("async-update", debug.getAsyncUpdate()));

		if (advanced.getRedirectSDCard())
		{
			String path = android.os.Environment.getExternalStorageDirectory().getPath();
			args.add("/drive:sdcard," + path);
		}

		args.add("/clipboard");

		// Gateway enabled?
		if (bookmark.getType() == BookmarkBase.TYPE_MANUAL &&
		    bookmark.<ManualBookmark>get().getEnableGatewaySettings())
		{
			ManualBookmark.GatewaySettings gateway =
			    bookmark.<ManualBookmark>get().getGatewaySettings();

			args.add(String.format("/g:%s:%d", gateway.getHostname(), gateway.getPort()));

			arg = gateway.getUsername();
			if (!arg.isEmpty())
			{
				args.add("/gu:" + arg);
			}
			arg = gateway.getDomain();
			if (!arg.isEmpty())
			{
				args.add("/gd:" + arg);
			}
			arg = gateway.getPassword();
			if (!arg.isEmpty())
			{
				args.add("/gp:" + arg);
			}
		}

		/* 0 ... local
		   1 ... remote
		   2 ... disable */
		args.add("/audio-mode:" + advanced.getRedirectSound());
		if (advanced.getRedirectSound() == 0)
		{
			args.add("/sound");
		}

		if (advanced.getRedirectMicrophone())
		{
			args.add("/microphone");
		}

		args.add("/kbd:unicode:on");
		args.add("/cert:ignore");
		args.add("/log-level:" + debug.getDebugLevel());
		String[] arrayArgs = args.toArray(new String[0]);
		return freerdp_parse_arguments(inst, arrayArgs);
	}

	public static boolean setConnectionInfo(Context context, long inst, Uri openUri)
	{
		ArrayList<String> args = new ArrayList<>();

		// Parse URI from query string. Same key overwrite previous one
		// freerdp://user@ip:port/connect?sound=&rfx=&p=password&clipboard=%2b&themes=-

		// Now we only support Software GDI
		args.add(TAG);
		args.add("/gdi:sw");

		final String clientName = ApplicationSettingsActivity.getClientName(context);
		if (!clientName.isEmpty())
		{
			args.add("/client-hostname:" + clientName);
		}

		// Parse hostname and port. Set to 'v' argument
		String hostname = openUri.getHost();
		int port = openUri.getPort();
		if (hostname != null)
		{
			hostname = hostname + ((port == -1) ? "" : (":" + port));
			args.add("/v:" + hostname);
		}

		String user = openUri.getUserInfo();
		if (user != null)
		{
			args.add("/u:" + user);
		}

		for (String key : openUri.getQueryParameterNames())
		{
			String value = openUri.getQueryParameter(key);

			if (value.isEmpty())
			{
				// Query: key=
				// To freerdp argument: /key
				args.add("/" + key);
			}
			else if (value.equals("-") || value.equals("+"))
			{
				// Query: key=- or key=+
				// To freerdp argument: -key or +key
				args.add(value + key);
			}
			else
			{
				// Query: key=value
				// To freerdp argument: /key:value
				if (key.equals("drive") && value.equals("sdcard"))
				{
					// Special for sdcard redirect
					String path = android.os.Environment.getExternalStorageDirectory().getPath();
					value = "sdcard," + path;
				}

				args.add("/" + key + ":" + value);
			}
		}

		String[] arrayArgs = args.toArray(new String[0]);
		return freerdp_parse_arguments(inst, arrayArgs);
	}

	public static boolean updateGraphics(long inst, Bitmap bitmap, int x, int y, int width,
	                                     int height)
	{
		return freerdp_update_graphics(inst, bitmap, x, y, width, height);
	}

	public static boolean sendCursorEvent(long inst, int x, int y, int flags)
	{
		return freerdp_send_cursor_event(inst, x, y, flags);
	}

	public static boolean sendKeyEvent(long inst, int keycode, boolean down)
	{
		return freerdp_send_key_event(inst, keycode, down);
	}

	public static boolean sendUnicodeKeyEvent(long inst, int keycode, boolean down)
	{
		return freerdp_send_unicodekey_event(inst, keycode, down);
	}

	public static boolean sendClipboardData(long inst, String data)
	{
		return freerdp_send_clipboard_data(inst, data);
	}

	private static void OnConnectionSuccess(long inst)
	{
		if (listener != null)
			listener.OnConnectionSuccess(inst);
		synchronized (mInstanceState)
		{
			mInstanceState.append(inst, true);
			mInstanceState.notifyAll();
		}
	}

	private static void OnConnectionFailure(long inst)
	{
		if (listener != null)
			listener.OnConnectionFailure(inst);
		synchronized (mInstanceState)
		{
			mInstanceState.remove(inst);
			mInstanceState.notifyAll();
		}
	}

	private static void OnPreConnect(long inst)
	{
		if (listener != null)
			listener.OnPreConnect(inst);
	}

	private static void OnDisconnecting(long inst)
	{
		if (listener != null)
			listener.OnDisconnecting(inst);
	}

	private static void OnDisconnected(long inst)
	{
		if (listener != null)
			listener.OnDisconnected(inst);
		synchronized (mInstanceState)
		{
			mInstanceState.remove(inst);
			mInstanceState.notifyAll();
		}
	}

	private static void OnSettingsChanged(long inst, int width, int height, int bpp)
	{
		SessionState s = GlobalApp.getSession(inst);
		if (s == null)
			return;
		UIEventListener uiEventListener = s.getUIEventListener();
		if (uiEventListener != null)
			uiEventListener.OnSettingsChanged(width, height, bpp);
	}

	private static boolean OnAuthenticate(long inst, StringBuilder username, StringBuilder domain,
	                                      StringBuilder password)
	{
		SessionState s = GlobalApp.getSession(inst);
		if (s == null)
			return false;
		UIEventListener uiEventListener = s.getUIEventListener();
		if (uiEventListener != null)
			return uiEventListener.OnAuthenticate(username, domain, password);
		return false;
	}

	private static boolean OnGatewayAuthenticate(long inst, StringBuilder username,
	                                             StringBuilder domain, StringBuilder password)
	{
		SessionState s = GlobalApp.getSession(inst);
		if (s == null)
			return false;
		UIEventListener uiEventListener = s.getUIEventListener();
		if (uiEventListener != null)
			return uiEventListener.OnGatewayAuthenticate(username, domain, password);
		return false;
	}

	private static int OnVerifyCertificateEx(long inst, String host, long port, String commonName,
	                                       String subject, String issuer, String fingerprint,
	                                       long flags)
	{
		SessionState s = GlobalApp.getSession(inst);
		if (s == null)
			return 0;
		UIEventListener uiEventListener = s.getUIEventListener();
		if (uiEventListener != null)
			return uiEventListener.OnVerifiyCertificateEx(host, port, commonName, subject, issuer,
			                                              fingerprint, flags);
		return 0;
	}

	private static int OnVerifyChangedCertificateEx(long inst, String host, long port,
	                                                String commonName, String subject,
	                                                String issuer, String fingerprint,
	                                                String oldSubject, String oldIssuer,
	                                                String oldFingerprint, long flags)
	{
		SessionState s = GlobalApp.getSession(inst);
		if (s == null)
			return 0;
		UIEventListener uiEventListener = s.getUIEventListener();
		if (uiEventListener != null)
			return uiEventListener.OnVerifyChangedCertificateEx(host, port, commonName, subject,
			                                                    issuer, fingerprint, oldSubject,
			                                                    oldIssuer, oldFingerprint, flags);
		return 0;
	}

	private static void OnGraphicsUpdate(long inst, int x, int y, int width, int height)
	{
		SessionState s = GlobalApp.getSession(inst);
		if (s == null)
			return;
		UIEventListener uiEventListener = s.getUIEventListener();
		if (uiEventListener != null)
			uiEventListener.OnGraphicsUpdate(x, y, width, height);
	}

	private static void OnGraphicsResize(long inst, int width, int height, int bpp)
	{
		SessionState s = GlobalApp.getSession(inst);
		if (s == null)
			return;
		UIEventListener uiEventListener = s.getUIEventListener();
		if (uiEventListener != null)
			uiEventListener.OnGraphicsResize(width, height, bpp);
	}

	private static void OnRemoteClipboardChanged(long inst, String data)
	{
		SessionState s = GlobalApp.getSession(inst);
		if (s == null)
			return;
		UIEventListener uiEventListener = s.getUIEventListener();
		if (uiEventListener != null)
			uiEventListener.OnRemoteClipboardChanged(data);
	}

	public static String getVersion()
	{
		return freerdp_get_version();
	}

	public interface EventListener
	{
		void OnPreConnect(long instance);

		void OnConnectionSuccess(long instance);

		void OnConnectionFailure(long instance);

		void OnDisconnecting(long instance);

		void OnDisconnected(long instance);
	}

	public interface UIEventListener
	{
		void OnSettingsChanged(int width, int height, int bpp);

		boolean OnAuthenticate(StringBuilder username, StringBuilder domain,
		                       StringBuilder password);

		boolean OnGatewayAuthenticate(StringBuilder username, StringBuilder domain,
		                              StringBuilder password);

		int OnVerifiyCertificateEx(String host, long port, String commonName, String subject, String issuer,
		                         String fingerprint, long flags);

		int OnVerifyChangedCertificateEx(String host, long port, String commonName, String subject, String issuer,
		                               String fingerprint, String oldSubject, String oldIssuer,
		                               String oldFingerprint, long flags);

		void OnGraphicsUpdate(int x, int y, int width, int height);

		void OnGraphicsResize(int width, int height, int bpp);

		void OnRemoteClipboardChanged(String data);
	}
}
