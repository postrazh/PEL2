
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#include "Exp.h"

CExpire::CExpire(const TCHAR*  ProgName,const TCHAR* KeyName,UINT Num,UINT ExpireType)
{	
	m_first=true;
	m_ExpireType=ExpireType;
	m_count=Num;
	CreateProgKey(ProgName,KeyName);
	UINT rcount;	
	GetSystemDirectory(m_systemfolder,MAX_PATH);
	lstrcpy(m_filename,m_systemfolder);
	if(m_filename[lstrlen(m_filename)-1]!= _T('\\'))
		lstrcat(m_filename, _T("\\"));	
	lstrcat(m_filename,KeyName);
	lstrcat(m_filename, _T(".dll"));
	m_filefound=LocateFile();
	switch (m_ExpireType)
	{
	case TYPERUNS:		
		if(GetRunCount(rcount))
		{
			if(!m_filefound)
			{			
				SetRunCount();
				m_first=false;
				CreateProtectFile();
			}
		}
			
		else
		{		
			if(rcount>0 && rcount<=m_count)							
			{
				SetRunCount(rcount-1);
				m_first=false;
			}		
		}
		break;
	case TYPEDAYS:
		
		if(GetDayCount(rcount))
		{
			if(!m_filefound)
			{			
				SetDayCount();
				m_first=false;
				CreateProtectFile();
			}
		}
			
		else
		{	
			
			if(rcount>0 && rcount<=m_count)							
			{				
				m_first=false;
			}		
		}

		break;
	}


}

CExpire::~CExpire()
{
	RegCloseKey(m_hKey);
}

bool CExpire::CreateProgKey(const TCHAR*  ProgName, const TCHAR* KeyName)
{
	TCHAR *SubKey = new TCHAR [lstrlen(ProgName) +lstrlen(KeyName)+300];
	lstrcpy(SubKey, _T("CLSID\\"));
	lstrcat(SubKey,ProgName);
	lstrcat(SubKey, _T("\\"));
	lstrcat(SubKey,KeyName);
	HKEY hKey;
	DWORD dwDisposition;
	LONG ret;
	ret=RegCreateKeyEx(HKEY_CLASSES_ROOT, SubKey, 0, (LPWSTR)_T(""), REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
	m_hKey=hKey;
	delete SubKey;
	if(ret==ERROR_SUCCESS)
		return false;
	else
		return true;
}

bool CExpire::GetRunCount(UINT& count)
{
	LONG ret;
	TCHAR data[128];
	DWORD size=sizeof(data);
	ret=RegQueryValueEx(m_hKey, _T("windll"), 0, NULL, (BYTE*)data,&size);
	if(ret==ERROR_SUCCESS)
	{
		DeObfuscate(data,count);
		
		return false;
	}
	else
		return true;
}

bool CExpire::SetRunCount()
{
	LONG ret;
	TCHAR str[128];	
	Obfuscate(m_count-1,str);
	DWORD size=lstrlen(str) * sizeof(TCHAR)+1;
	ret=RegSetValueEx(m_hKey, _T("windll"), 0, REG_SZ, (BYTE*)str,size);
	if(ret==ERROR_SUCCESS)
		return false;
	else
		return true;
}

bool CExpire::SetRunCount(UINT count)
{
	LONG ret;
	TCHAR str[128];	
	Obfuscate(count,str);
	DWORD size=lstrlen(str) * sizeof(TCHAR) +1;
	ret=RegSetValueEx(m_hKey, _T("windll"), 0, REG_SZ, (BYTE*)str,size);
	if(ret==ERROR_SUCCESS)
		return false;
	else
		return true;
}


bool CExpire::Obfuscate(UINT num, TCHAR *str)
{	
	TCHAR tmpstr[128];
	srand((unsigned int)time(NULL));
	int seed1,seed2,seed3;
	seed1=rand();
	seed2=rand();
	seed3=abs(seed1-seed2);
	wsprintf(tmpstr, _T("%d.%d.%d"), seed1, num+seed3, seed2);
	lstrcpy(str,tmpstr);
	return false;
}

bool CExpire::DeObfuscate(TCHAR *str, UINT &num)
{
	TCHAR tmpstr[128];
	lstrcpy(tmpstr,str);
	int seed1,seed2,seed3;
	TCHAR *p1=wcschr(tmpstr, _T('.'));	
	if(!p1)
	{
		return true;
	}
	*p1=0;
	seed1=_wtoi(tmpstr);
	p1++;
	TCHAR *p2=wcschr(p1, _T('.'));
	if(!p2)
	{
		return true;
	}
	*p2=0;
	int tmpNum=_wtoi(p1);
	p2++;
	seed2= _wtoi(p2);
	seed3=abs(seed1-seed2);
	num=tmpNum-seed3;
	return false;
}

bool CExpire::SetDayCount()
{
	LONG ret;
	TCHAR str[128];	
	UINT TodaySecs=(UINT)time(NULL);
	TodaySecs+=(m_count)*24*3600;	
	Obfuscate(TodaySecs,str);
	DWORD size=lstrlen(str) * sizeof(TCHAR)+1;
	ret=RegSetValueEx(m_hKey, _T("windll"), 0, REG_SZ, (BYTE*)str,size);
	if(ret==ERROR_SUCCESS)
		return false;
	else
		return true;
}


bool CExpire::GetDayCount(UINT& count)
{
	LONG ret;
	TCHAR data[128];
	DWORD size=sizeof data;
	ret=RegQueryValueEx(m_hKey, _T("windll"), 0, NULL, (BYTE*)data,&size);
	if(ret==ERROR_SUCCESS)
	{
		DeObfuscate(data,count);
		UINT TodaySecs=(UINT)time(NULL);		
		UINT tmpcount=count;
		count=(count-TodaySecs)/(24*3600);
		if((tmpcount-TodaySecs)%(24*3600))
			count++;
		return false;
	}
	else
		return true;
}

bool CExpire::HasExpired()
{
	if(m_first)
		RegDeleteValue(m_hKey, _T("windll"));
	return m_first;
}

bool CExpire::LocateFile()
{

	WIN32_FIND_DATA tmpWIN32_FIND_DATA;
	HANDLE hFile=FindFirstFile(m_filename,&tmpWIN32_FIND_DATA);
	if (hFile==INVALID_HANDLE_VALUE)
		return false;
	else
	{
		
		FindClose(hFile);
		return true;
	}
}

bool CExpire::CreateProtectFile()
{
	TCHAR buff[32];
	buff[31]=0;
	bool ret=false;
	FILE* fp=_wfsopen(m_filename, _T("w"), _SH_DENYNO);
	fwprintf(fp,buff);
	fclose(fp);
	return ret;
}

UINT CExpire::GetRunsLeft()
{
	UINT n=0;
	GetRunCount(n);
	return n;
}

UINT CExpire::GetDaysLeft()
{
	UINT n=0;
	GetDayCount(n);
	return n;	
}
