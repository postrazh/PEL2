
#include <Windows.h>
#include <tchar.h>

#define TYPERUNS 100
#define TYPEDAYS 200
class CExpire  
{
public:
	UINT GetDaysLeft();
	UINT GetRunsLeft();
	bool HasExpired();

	CExpire(const TCHAR*  ProgName,const TCHAR* KeyName, UINT Num, UINT ExpireType);
	virtual ~CExpire();
private:
	bool CreateProtectFile();
	bool m_filefound;
	bool LocateFile();
	TCHAR m_systemfolder[MAX_PATH];
	TCHAR m_filename[MAX_PATH];
	bool DeObfuscate(TCHAR *str, UINT& num);
	bool Obfuscate(UINT num, TCHAR *str);	
	bool GetRunCount(UINT& count);
	bool SetRunCount();
	bool SetRunCount(UINT count);
	bool GetDayCount(UINT& count);
	bool SetDayCount();
	bool CreateProgKey(const TCHAR*  ProgName, const TCHAR* KeyName);
	UINT m_ExpireType;
	UINT m_count;
	HKEY m_hKey;
	bool m_first;
};
