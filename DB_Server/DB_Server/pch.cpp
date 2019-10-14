#include "pch.h"

void show_error(SQLHANDLE handle, SQLSMALLINT type, RETCODE retcode)
{
	SQLSMALLINT rec = 0;
	SQLINTEGER error;
	WCHAR message[1000];
	WCHAR state[SQL_SQLSTATE_SIZE + 1];
	
	if (retcode == SQL_INVALID_HANDLE)
	{
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	
	while(SQLGetDiagRec(type, handle, ++rec, state, &error, message,
		(SQLSMALLINT)(sizeof(message) / sizeof(WCHAR)), 
		(SQLSMALLINT*)NULL) == SQL_SUCCESS)
	{
		if (wcsncmp(state, L"01004", 5))
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", state, message, error);
	}
}