/**
 * WinPR: Windows Portable Runtime
 * Test for NCrypt library
 *
 * Copyright 2021 David Fort <contact@hardening-consulting.com>
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
#include <winpr/error.h>
#include <winpr/ncrypt.h>
#include <winpr/string.h>
#include <winpr/wlog.h>
#include <winpr/smartcard.h>

#include <openssl/bio.h>
#include <openssl/x509.h>

#define TAG "testNCrypt"

static void crypto_print_name(const BYTE* b, DWORD sz)
{
	if (sz > INT32_MAX)
		return;
	BIO* bio = BIO_new_mem_buf(b, (int)sz);
	if (!bio)
		return;

	X509* x509 = d2i_X509_bio(bio, NULL);
	if (!x509)
		goto bio_release;

	X509_NAME* name = X509_get_subject_name(x509);
	if (!name)
		goto x509_release;

	char* ret = calloc(1024, sizeof(char));
	if (!ret)
		goto bio_release;

	char* ret2 = X509_NAME_oneline(name, ret, 1024);

	printf("\t%s\n", ret2);
	free(ret);

x509_release:
	X509_free(x509);
bio_release:
	BIO_free(bio);
}

int TestNCryptSmartcard(int argc, char* argv[])
{
	int rc = -1;
	DWORD providerCount = 0;
	NCryptProviderName* names = NULL;

	WINPR_UNUSED(argc);
	WINPR_UNUSED(argv);

	SECURITY_STATUS status = NCryptEnumStorageProviders(&providerCount, &names, NCRYPT_SILENT_FLAG);
	if (status != ERROR_SUCCESS)
		return -1;

	for (size_t j = 0; j < providerCount; j++)
	{
		const NCryptProviderName* name = &names[j];
		NCRYPT_PROV_HANDLE provider = 0;
		char providerNameStr[256] = { 0 };
		PVOID enumState = NULL;
		size_t i = 0;
		NCryptKeyName* keyName = NULL;

		if (ConvertWCharToUtf8(name->pszName, providerNameStr, ARRAYSIZE(providerNameStr)) < 0)
			continue;
		printf("provider %" PRIuz ": %s\n", j, providerNameStr);

		status = NCryptOpenStorageProvider(&provider, name->pszName, 0);
		if (status != ERROR_SUCCESS)
			continue;

		while ((status = NCryptEnumKeys(provider, NULL, &keyName, &enumState,
		                                NCRYPT_SILENT_FLAG)) == ERROR_SUCCESS)
		{
			NCRYPT_KEY_HANDLE phKey = 0;
			DWORD dwFlags = 0;
			DWORD cbOutput = 0;
			char keyNameStr[256] = { 0 };
			WCHAR reader[1024] = { 0 };
			PBYTE certBytes = NULL;

			if (ConvertWCharToUtf8(keyName->pszName, keyNameStr, ARRAYSIZE(keyNameStr)) < 0)
				continue;

			printf("\tkey %" PRIuz ": %s\n", i, keyNameStr);
			status = NCryptOpenKey(provider, &phKey, keyName->pszName, keyName->dwLegacyKeySpec,
			                       dwFlags);
			if (status != ERROR_SUCCESS)
			{
				WLog_ERR(TAG, "unable to open key %s", keyNameStr);
				continue;
			}

			status = NCryptGetProperty(phKey, NCRYPT_READER_PROPERTY, (PBYTE)reader, sizeof(reader),
			                           &cbOutput, dwFlags);
			if (status == ERROR_SUCCESS)
			{
				char readerStr[1024] = { 0 };

				(void)ConvertWCharNToUtf8(reader, cbOutput, readerStr, ARRAYSIZE(readerStr));
				printf("\treader: %s\n", readerStr);
			}

			cbOutput = 0;
			status =
			    NCryptGetProperty(phKey, NCRYPT_CERTIFICATE_PROPERTY, NULL, 0, &cbOutput, dwFlags);
			if (status != ERROR_SUCCESS)
			{
				WLog_ERR(TAG, "unable to retrieve certificate len for key '%s'", keyNameStr);
				goto endofloop;
			}

			certBytes = calloc(1, cbOutput);
			status = NCryptGetProperty(phKey, NCRYPT_CERTIFICATE_PROPERTY, certBytes, cbOutput,
			                           &cbOutput, dwFlags);
			if (status != ERROR_SUCCESS)
			{
				WLog_ERR(TAG, "unable to retrieve certificate for key %s", keyNameStr);
				goto endofloop;
			}

			crypto_print_name(certBytes, cbOutput);
			free(certBytes);

		endofloop:
			NCryptFreeBuffer(keyName);
			NCryptFreeObject((NCRYPT_HANDLE)phKey);
			i++;
		}

		NCryptFreeBuffer(enumState);
		NCryptFreeObject((NCRYPT_HANDLE)provider);

		if (status != NTE_NO_MORE_ITEMS)
		{
			(void)fprintf(stderr, "NCryptEnumKeys returned %s [0x%08" PRIx32 "]\n",
			              Win32ErrorCode2Tag(status), status);
		}
	}

	rc = 0;
fail:
	NCryptFreeBuffer(names);
	return rc;
}
