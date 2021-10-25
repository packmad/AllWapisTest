#include "MainHeader.h"

using namespace std;

#include <tchar.h>
#include <stdio.h>
#include <windows.h>
#include <wincrypt.h>
#include <conio.h>
#include <string>

// Link with the Advapi32.lib file.
#pragma comment (lib, "advapi32")

#define KEYLENGTH  0x00800000
#define ENCRYPT_ALGORITHM CALG_RC4 
#define ENCRYPT_BLOCK_SIZE 8 

bool MyEncryptFile(
	LPTSTR szSource,
	LPTSTR szDestination,
	LPTSTR szPassword);

void MyHandleError(
	LPTSTR psz,
	int nErrorNumber);

int crypto()
{
	LPTSTR pszSource = LPTSTR(L"input");
	LPTSTR pszDestination = LPTSTR(L"output");
	std::cout << pszSource << std::endl;
	LPTSTR pszPassword = NULL;

	//---------------------------------------------------------------
	// Call EncryptFile to do the actual encryption.
	if (MyEncryptFile(pszSource, pszDestination, pszPassword))
	{
		printf(
			"Encryption of the file %s was successful. \n",
			pszSource);
		printf(
			"The encrypted data is in file %s.\n",
			pszDestination);
	}
	else
	{
		MyHandleError(
			LPTSTR("Error encrypting file!\n"),
			GetLastError());
	}
	return 0;
}

bool MyEncryptFile(
	LPTSTR pszSourceFile,
	LPTSTR pszDestinationFile,
	LPTSTR pszPassword)
{
	//---------------------------------------------------------------
	// Declare and initialize local variables.
	bool fReturn = false;
	HANDLE hSourceFile = INVALID_HANDLE_VALUE;
	HANDLE hDestinationFile = INVALID_HANDLE_VALUE;

	HCRYPTPROV hCryptProv = NULL;
	HCRYPTKEY hKey = NULL;
	HCRYPTKEY hXchgKey = NULL;
	HCRYPTHASH hHash = NULL;

	PBYTE pbKeyBlob = NULL;
	DWORD dwKeyBlobLen;

	PBYTE pbBuffer = NULL;
	DWORD dwBlockLen;
	DWORD dwBufferLen;
	DWORD dwCount;

	//---------------------------------------------------------------
	// Open the source file. 
	hSourceFile = CreateFile(
		pszSourceFile,
		FILE_READ_DATA,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (INVALID_HANDLE_VALUE != hSourceFile)
	{
		printf(
			"The source plaintext file, %s, is open. \n",
			pszSourceFile);
	}
	else
	{
		MyHandleError(
			LPTSTR("Error opening source plaintext file!\n"),
			GetLastError());
		return -1;
	}

	//---------------------------------------------------------------
	// Open the destination file. 
	hDestinationFile = CreateFile(
		pszDestinationFile,
		FILE_WRITE_DATA,
		FILE_SHARE_READ,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	if (INVALID_HANDLE_VALUE != hDestinationFile)
	{
		printf(
			"The destination file, %s, is open. \n",
			pszDestinationFile);
	}
	else
	{
		MyHandleError(
			LPTSTR("Error opening destination file!\n"),
			GetLastError());
		return -1;
	}

	//---------------------------------------------------------------
	// Get the handle to the default provider. 
	if (CryptAcquireContext(
		&hCryptProv,
		NULL,
		MS_ENHANCED_PROV,
		PROV_RSA_FULL,
		0))
	{
		printf(
			"A cryptographic provider has been acquired. \n");
	}
	else
	{
		MyHandleError(
			LPTSTR("Error during CryptAcquireContext!\n"),
			GetLastError());
		return -1;
	}

	//---------------------------------------------------------------
	// Create the session key.
	if (!pszPassword || !pszPassword[0])
	{
		//-----------------------------------------------------------
		// No password was passed.
		// Encrypt the file with a random session key, and write the 
		// key to a file. 

		//-----------------------------------------------------------
		// Create a random session key. 
		if (CryptGenKey(
			hCryptProv,
			ENCRYPT_ALGORITHM,
			KEYLENGTH | CRYPT_EXPORTABLE,
			&hKey))
		{
			printf("A session key has been created. \n");
		}
		else
		{
			MyHandleError(
				LPTSTR("Error during CryptGenKey. \n"),
				GetLastError());
			return -1;
		}

		//-----------------------------------------------------------
		// Get the handle to the exchange public key. 
		if (CryptGetUserKey(
			hCryptProv,
			AT_KEYEXCHANGE,
			&hXchgKey))
		{
			printf(
				"The user public key has been retrieved. \n");
		}
		else
		{
			if (NTE_NO_KEY == GetLastError())
			{
				// No exchange key exists. Try to create one.
				if (!CryptGenKey(
					hCryptProv,
					AT_KEYEXCHANGE,
					CRYPT_EXPORTABLE,
					&hXchgKey))
				{
					MyHandleError(
						LPTSTR("Could not create "
							"a user public key.\n"),
						GetLastError());
					return -1;
				}
			}
			else
			{
				MyHandleError(
					LPTSTR("User public key is not available and may not exist.\n"),
					GetLastError());
				return -1;
			}
		}

		//-----------------------------------------------------------
		// Determine size of the key BLOB, and allocate memory. 
		if (CryptExportKey(
			hKey,
			hXchgKey,
			SIMPLEBLOB,
			0,
			NULL,
			&dwKeyBlobLen))
		{
			printf(
				"The key BLOB is %d bytes long. \n",
				dwKeyBlobLen);
		}
		else
		{
			MyHandleError(
				LPTSTR("Error computing BLOB length! \n"),
				GetLastError());
			return -1;
		}

		if (pbKeyBlob = (BYTE*)malloc(dwKeyBlobLen))
		{
			printf(
				"Memory is allocated for the key BLOB. \n");
		}
		else
		{
			MyHandleError(LPTSTR("Out of memory. \n"), E_OUTOFMEMORY);
			return -1;
		}

		//-----------------------------------------------------------
		// Encrypt and export the session key into a simple key 
		// BLOB. 
		if (CryptExportKey(
			hKey,
			hXchgKey,
			SIMPLEBLOB,
			0,
			pbKeyBlob,
			&dwKeyBlobLen))
		{
			printf("The key has been exported. \n");
		}
		else
		{
			MyHandleError(
				LPTSTR("Error during CryptExportKey!\n"),
				GetLastError());
			return -1;
		}

		//-----------------------------------------------------------
		// Release the key exchange key handle. 
		if (hXchgKey)
		{
			if (!(CryptDestroyKey(hXchgKey)))
			{
				MyHandleError(
					LPTSTR("Error during CryptDestroyKey.\n"),
					GetLastError());
				return -1;
			}

			hXchgKey = 0;
		}

		//-----------------------------------------------------------
		// Write the size of the key BLOB to the destination file. 
		if (!WriteFile(
			hDestinationFile,
			&dwKeyBlobLen,
			sizeof(DWORD),
			&dwCount,
			NULL))
		{
			MyHandleError(
				LPTSTR("Error writing header.\n"),
				GetLastError());
			return -1;
		}
		else
		{
			printf("A file header has been written. \n");
		}

		//-----------------------------------------------------------
		// Write the key BLOB to the destination file. 
		if (!WriteFile(
			hDestinationFile,
			pbKeyBlob,
			dwKeyBlobLen,
			&dwCount,
			NULL))
		{
			MyHandleError(
				LPTSTR("Error writing header.\n"),
				GetLastError());
			return -1;
		}
		else
		{
			printf(
				"The key BLOB has been written to the file. \n");
		}

		// Free memory.
		free(pbKeyBlob);
	}
	else
	{

		//-----------------------------------------------------------
		// The file will be encrypted with a session key derived 
		// from a password.
		// The session key will be recreated when the file is 
		// decrypted only if the password used to create the key is 
		// available. 

		//-----------------------------------------------------------
		// Create a hash object. 
		if (CryptCreateHash(
			hCryptProv,
			CALG_MD5,
			0,
			0,
			&hHash))
		{
			printf("A hash object has been created. \n");
		}
		else
		{
			MyHandleError(
				LPTSTR("Error during CryptCreateHash!\n"),
				GetLastError());
			return -1;
		}

		//-----------------------------------------------------------
		// Hash the password. 
		if (CryptHashData(
			hHash,
			(BYTE*)pszPassword,
			lstrlen(pszPassword),
			0))
		{
			printf(
				"The password has been added to the hash. \n");
		}
		else
		{
			MyHandleError(
				LPTSTR("Error during CryptHashData. \n"),
				GetLastError());
			return -1;
		}

		//-----------------------------------------------------------
		// Derive a session key from the hash object. 
		if (CryptDeriveKey(
			hCryptProv,
			ENCRYPT_ALGORITHM,
			hHash,
			KEYLENGTH,
			&hKey))
		{
			printf(
				"An encryption key is derived from the password hash. \n");
		}
		else
		{
			MyHandleError(
				LPTSTR("Error during CryptDeriveKey!\n"),
				GetLastError());
			return -1;
		}
	}

	//---------------------------------------------------------------
	// The session key is now ready. If it is not a key derived from 
	// a  password, the session key encrypted with the private key 
	// has been written to the destination file.

	//---------------------------------------------------------------
	// Determine the number of bytes to encrypt at a time. 
	// This must be a multiple of ENCRYPT_BLOCK_SIZE.
	// ENCRYPT_BLOCK_SIZE is set by a #define statement.
	dwBlockLen = 1000 - 1000 % ENCRYPT_BLOCK_SIZE;

	//---------------------------------------------------------------
	// Determine the block size. If a block cipher is used, 
	// it must have room for an extra block. 
	if (ENCRYPT_BLOCK_SIZE > 1)
	{
		dwBufferLen = dwBlockLen + ENCRYPT_BLOCK_SIZE;
	}
	else
	{
		dwBufferLen = dwBlockLen;
	}

	//---------------------------------------------------------------
	// Allocate memory. 
	if (pbBuffer = (BYTE*)malloc(dwBufferLen))
	{
		printf(
			"Memory has been allocated for the buffer. \n");
	}
	else
	{
		MyHandleError(LPTSTR("Out of memory. \n"), E_OUTOFMEMORY);
		return -1;
	}

	//---------------------------------------------------------------
	// In a do loop, encrypt the source file, 
	// and write to the source file. 
	bool fEOF = FALSE;
	do
	{
		//-----------------------------------------------------------
		// Read up to dwBlockLen bytes from the source file. 
		if (!ReadFile(
			hSourceFile,
			pbBuffer,
			dwBlockLen,
			&dwCount,
			NULL))
		{
			MyHandleError(
				LPTSTR("Error reading plaintext!\n"),
				GetLastError());
			return -1;
		}

		if (dwCount < dwBlockLen)
		{
			fEOF = TRUE;
		}

		//-----------------------------------------------------------
		// Encrypt data. 
		if (!CryptEncrypt(
			hKey,
			NULL,
			fEOF,
			0,
			pbBuffer,
			&dwCount,
			dwBufferLen))
		{
			MyHandleError(
				LPTSTR("Error during CryptEncrypt. \n"),
				GetLastError());
			return -1;
		}

		//-----------------------------------------------------------
		// Write the encrypted data to the destination file. 
		if (!WriteFile(
			hDestinationFile,
			pbBuffer,
			dwCount,
			&dwCount,
			NULL))
		{
			MyHandleError(
				LPTSTR("Error writing ciphertext.\n"),
				GetLastError());
			return -1;
		}

		//-----------------------------------------------------------
		// End the do loop when the last block of the source file 
		// has been read, encrypted, and written to the destination 
		// file.
	} while (!fEOF);

	fReturn = true;

Exit_MyEncryptFile:
	//---------------------------------------------------------------
	// Close files.
	if (hSourceFile)
	{
		CloseHandle(hSourceFile);
	}

	if (hDestinationFile)
	{
		CloseHandle(hDestinationFile);
	}

	//---------------------------------------------------------------
	// Free memory. 
	if (pbBuffer)
	{
		free(pbBuffer);
	}


	//-----------------------------------------------------------
	// Release the hash object. 
	if (hHash)
	{
		if (!(CryptDestroyHash(hHash)))
		{
			MyHandleError(
				LPTSTR("Error during CryptDestroyHash.\n"),
				GetLastError());
		}

		hHash = NULL;
	}

	//---------------------------------------------------------------
	// Release the session key. 
	if (hKey)
	{
		if (!(CryptDestroyKey(hKey)))
		{
			MyHandleError(
				LPTSTR("Error during CryptDestroyKey!\n"),
				GetLastError());
		}
	}

	//---------------------------------------------------------------
	// Release the provider handle. 
	if (hCryptProv)
	{
		if (!(CryptReleaseContext(hCryptProv, 0)))
		{
			MyHandleError(
				LPTSTR("Error during CryptReleaseContext!\n"),
				GetLastError());
		}
	}

	return fReturn;
} // End Encryptfile.


//-------------------------------------------------------------------
//  This example uses the function MyHandleError, a simple error
//  handling function, to print an error message to the  
//  standard error (stderr) file and exit the program. 
//  For most applications, replace this function with one 
//  that does more extensive error reporting.

void MyHandleError(LPTSTR psz, int nErrorNumber)
{
	_ftprintf(stderr, LPTSTR("An error occurred in the program. \n"));
	_ftprintf(stderr, LPTSTR("%s\n"), psz);
	_ftprintf(stderr, LPTSTR("Error number %x.\n"), nErrorNumber);
}
