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
#include <process.h>


//ȫ�ֹ������
#pragma data_seg(".Share")
	HWND g_hWnd = NULL;			//�����ھ��;
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
POINT pos = { 0 };				//Ŀ����������������
bool isPaintLine = false;			//�Ƿ������贰��߿�
BOOL  WINAPI MyplaysoundW(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);
int inject();
int HookOn();
int HookOff();
HWND GetHwndByPid(DWORD dwProcessID);
int SetClickPos();
unsigned int __stdcall PaintLine(PVOID ppoint);
unsigned int __stdcall SetPosThreadFun(PVOID pM);

//���õ����������߳�
unsigned int __stdcall SetPosThreadFun(PVOID pM)
{
	while (1)
	{
		MessageBox(NULL, _T("��ʼ���õ������\n���� 5s ���ƶ��� Ŀ�괰�� �� Ŀ������"), _T("����"), MB_OK);
		Sleep(5000);
		POINT point = { 0 };		//����������
		//��ȡ�������� ���ڴ���
		GetCursorPos(&point);
		g_hWnd = WindowFromPoint(point);
		RECT rect = {0};
		//��ô����С������
		GetWindowRect(g_hWnd, &rect);
		//��������ڴ����е��������
		pos.x = point.x - rect.left;
		pos.y = point.y - rect.top;

		TCHAR name[MAX_PATH], confirm[MAX_PATH];
		GetWindowText(g_hWnd, name, MAX_PATH);
		_stprintf_s(confirm, MAX_PATH, _T("���ڱ���:%S\n�������:�����еĺ��\n\n�Ƿ�ȷ�� Ŀ�Ĵ��� �� Ŀ�����ꣿ"), name);
		
		isPaintLine = TRUE;
		//������߿��߳�
		assert(_beginthreadex(NULL, 0, PaintLine, &pos, 0, NULL) >= 0);

		if (MessageBox(NULL, confirm, _T("ȷ��"), MB_YESNO) == IDYES)
		{
			isPaintLine = FALSE;
			::InvalidateRect(NULL, NULL, TRUE);
			break;
		}
		else
		{
			//��Ϊ������ ��5s��˯��, ���������߳��˳�
			isPaintLine = FALSE;
			::InvalidateRect(NULL, NULL, TRUE);
		}
	}
	return TRUE;
}

//���ѡ�д��ڱ߿�
unsigned int __stdcall PaintLine(PVOID ppoint)
{
	Sleep(300);		//�ȴ�SetPosThreadFun()�е�messagebox()����
	static RECT rect = { 0 };
	RECT preRect = {0};		//��¼��һ�δ���Ĵ�С���������
	while (TRUE == isPaintLine)
	{
		HDC hdc = GetWindowDC(GetDesktopWindow());
		HPEN hpen = CreatePen(PS_SOLID, 5, RGB(255, 0, 0));
		HGDIOBJ  holdPen = SelectObject(hdc, hpen);
		POINT point = { 0 };	//�����������
		POINT oldPoint = {0};
		//��¼��һ�δ���Ĵ�С���������
		memcpy_s(&preRect, sizeof(preRect), &rect, sizeof(rect));	
		//��ô���  ��ǰ��С������
		GetWindowRect(g_hWnd, &rect);
		if (0 != memcmp(&preRect, &rect, sizeof(RECT)))
		{
			//�ϴ�����͵�ǰ�������겻ͬ, ����ˢ������, ���ϴ���ı߿���ʧ
			//RedrawWindow(NULL, NULL, NULL, RDW_UPDATENOW);
			::InvalidateRect(NULL, NULL, FALSE);
		}

		point.x = rect.left + pos.x;
		point.y = rect.top + pos.y;
		//�ϱ�
		MoveToEx(hdc, rect.left, rect.top, &oldPoint);
		LineTo(hdc, rect.right, rect.top);
		//�ұ�
		LineTo(hdc, rect.right, rect.bottom);
		//�±�
		LineTo(hdc, rect.left, rect.bottom);
		//���
		LineTo(hdc, rect.left, rect.top);
		//���������
		MoveToEx(hdc, point.x, point.y, &oldPoint);
		LineTo(hdc, point.x, point.y);

		SelectObject(hdc, holdPen);
		DeleteObject(hpen);
		ReleaseDC(GetDesktopWindow(), hdc);
		Sleep(400);
	}
	return TRUE;
}

//���õ������
int SetClickPos()
{
	//�����߳�
	assert(_beginthreadex(NULL, 0, SetPosThreadFun, NULL, 0, NULL) >= 0);
	return TRUE;
}

BOOL  WINAPI MyplaysoundW(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound)
{
	HookOff();
	MessageBox(NULL, _T("����, �Ѿ���hook��"), _T("hook success"), MB_OK);
	INT res = oldPlaySoundW( pszSound,  hmod, fdwSound);
	HookOn();
	return res;
}

int AutoClick()
{
	
}

int SendClick()
{

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

//���ݽ���ID��ȡ���ھ��
HWND GetHwndByPid(DWORD dwProcessID)
{
	//����Z�򶥲��Ĵ��ھ��
	HWND hWnd = ::GetTopWindow(0);

	while (hWnd)
	{
		DWORD pid = 0;
		//���ݴ��ھ����ȡ����ID
		DWORD dwTheardId = ::GetWindowThreadProcessId(hWnd, &pid);

		if (dwTheardId != 0)
		{
			if (pid == dwProcessID)
			{
				return hWnd;
			}
		}
		//����z���е�ǰһ�����һ�����ڵľ��
		hWnd = ::GetNextWindow(hWnd, GW_HWNDNEXT);

	}
	return hWnd;
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
		SetClickPos();
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