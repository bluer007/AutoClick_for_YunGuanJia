//////////////////////////////////////////////////////////////////
//
//����: ��������hook.dll��ʵ��. ���ڸ� �Զ��������� ���ò��������hook
//����: bluer
//ʱ��: 2015.5.8
//
/////////////////////////////////////////////////////////////////


#include <windows.h>
#include <tchar.h>
#include <assert.h>

//ȫ�ֹ������
#pragma data_seg(".Share")
	HWND g_hWnd = NULL;			//�����ھ��;
	HHOOK hhk = NULL;			//��깳�Ӿ��;
	HINSTANCE hInst = NULL;		//��dllʵ�����;
#pragma data_seg()
#pragma comment(linker, "/section:.Share,rws")
//���ϵı��������б�hook�����й���Ŷ!



//���屾�����ȫ�ֱ���
HANDLE hProcess = NULL;				//���汻hook����Ľ��̾��
BOOL bIsInjected = FALSE;			//�Ƿ��Ѿ�ע���˽���
BOOL bHook = FALSE;					//�Ƿ�Hook��API

typedef BOOL(WINAPI *playsoundW)(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);
playsoundW oldPlaySoundW = NULL;		//���ڱ���ԭ������ַ
FARPROC pfoldPlaySoundW = NULL;			//ָ��ԭ������ַ��Զָ��
BYTE OldCodeW[5];					//�ϵ�ϵͳAPI��ڴ���
BYTE NewCodeW[5];					//Ҫ��ת��API���� (jmp xxxx)
BOOL  WINAPI MyplaysoundW(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);
int inject();
int HookOn();
int HookOff();


BOOL  WINAPI MyplaysoundW(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound)
{
	HookOff();
	MessageBox(NULL, _T("����, �Ѿ���hook��"), _T("hook success"), MB_OK);
	INT res = oldPlaySoundW( pszSound,  hmod, fdwSound);
	HookOn();
	return res;
}


//�رչ���
int HookOff()
{
	assert(hProcess != NULL);

	DWORD dwTemp = 0;
	DWORD dwOldProtect;

	if (VirtualProtectEx(hProcess, pfoldPlaySoundW, 5, PAGE_READWRITE, &dwOldProtect))
	{
		if (WriteProcessMemory(hProcess, pfoldPlaySoundW, OldCodeW, 5, 0))
		{
			if (VirtualProtectEx(hProcess, pfoldPlaySoundW, 5, dwOldProtect, &dwTemp))
			{
				;
			}
			else
				MessageBox(NULL, _T("���Ľ����ڴ汣��ģʽʧ��"), _T("VirtualProtectEx() ERROR"), MB_OK);
		}
		else
			MessageBox(NULL, _T("�����ڴ�д��ʧ��"), _T("WriteProcessMemory() ERROR"), MB_OK);
	}
	else
		MessageBox(NULL, _T("���Ľ����ڴ汣��ģʽʧ��"), _T("VirtualProtectEx() ERROR"), MB_OK);
	
	
	bHook = FALSE;
	return TRUE;
}

//��������
int HookOn()
{
	assert(hProcess != NULL);

	DWORD dwTemp = 0;
	DWORD dwOldProtect;

	//���ڴ汣��ģʽ��Ϊ��д,��ǰ���ڴ汣��ģʽ������dwOldProtect
	if (VirtualProtectEx(hProcess, pfoldPlaySoundW, 5, PAGE_READWRITE, &dwOldProtect))
	{
		//������������add()��ǰ5���ֽڸ�ΪJmp Myadd 
		if (WriteProcessMemory(hProcess, pfoldPlaySoundW, NewCodeW, 5, 0))
		{
			//���ڴ汣��ģʽ�Ļ�ΪdwOldProtect
			if (VirtualProtectEx(hProcess, pfoldPlaySoundW, 5, dwOldProtect, &dwTemp))
			{
				;
			}
			else
				MessageBox(NULL, _T("���Ľ����ڴ汣��ģʽʧ��"), _T("VirtualProtectEx() ERROR"), MB_OK);
		}
		else
			MessageBox(NULL, _T("�����ڴ�д��ʧ��"), _T("WriteProcessMemory() ERROR"), MB_OK);
	}
	else
		MessageBox(NULL, _T("���Ľ����ڴ汣��ģʽʧ��"), _T("VirtualProtectEx() ERROR"), MB_OK);
	bHook = TRUE;
	return TRUE;
}

//������dll�õ��ĺ���
int inject()
{
	if (bIsInjected == FALSE)
	{ 
		//��ֻ֤����1��
		bIsInjected = TRUE;

		//��ȡ����
		HMODULE hmod = ::LoadLibrary(_T("WINMM.DLL"));
		oldPlaySoundW = (playsoundW)::GetProcAddress(hmod, "PlaySoundW");
		pfoldPlaySoundW = (FARPROC)oldPlaySoundW;
		if (pfoldPlaySoundW == NULL)
		{
			MessageBox(NULL, _T("�����ҵ�������ԭPlaySoundW()����"), _T("GetProcAddress() error"), 0);
			return -1;
		}

		// ��add()�е���ڴ��뱣����OldCode[]
		_asm
		{
			lea edi, OldCodeW
				mov esi, pfoldPlaySoundW
				cld
				movsd
				movsb
		}

		NewCodeW[0] = 0xe9;//ʵ����0xe9���൱��jmpָ��
						  //��ȡMyplaysoundW()����Ե�ַ
		_asm
		{
			lea eax, MyplaysoundW
				mov ebx, pfoldPlaySoundW
				sub eax, ebx
				sub eax, 5
				mov dword ptr[NewCodeW + 1], eax
		}
		//�����ϣ�����NewCode[]���ָ���൱��Jmp MyplaysoundW
		HookOn(); //���Կ���������
	}

	return TRUE;
}



//dll��������, �κμ���dll���ͷ�dll��Ҫ���ñ�����
BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpReserved)  // reserved
{
	DWORD dwPid;
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
		hProcess = hinstDLL;
		MessageBox(NULL, TEXT("���� �� ��ʼ��\nDLL_PROCESS_ATTACH"), TEXT(""), MB_OK);
		//���dll ʵ�������̾��
		hInst = ::GetModuleHandle(NULL);
		dwPid = ::GetCurrentProcessId();
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, dwPid);
		//����ע�亯��
		inject();
		break;

	//case DLL_THREAD_ATTACH:
	//	// Do thread-specific initialization.
	//	MessageBox(NULL, TEXT("DLL_THREAD_ATTACH"), TEXT(""), MB_OK);
	//	break;

	//case DLL_THREAD_DETACH:
	//	// Do thread-specific cleanup.
	//	MessageBox(NULL, TEXT("DLL_THREAD_DETACH"), TEXT(""), MB_OK);
	//	break;

	case DLL_PROCESS_DETACH:
		// Perform any necessary cleanup.
		HookOff();
		MessageBox(NULL, TEXT("���� �� ������\nDLL_PROCESS_DETACH"),TEXT(""), MB_OK);
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.		
}