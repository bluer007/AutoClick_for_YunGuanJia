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
#include <tlhelp32.h>			////ö�ٽ���ͷ�ļ�


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
BOOL isHookGuanJia = FALSE;			//�������Ƿ�hook�˹ܼ�
LPCTSTR  showGuanJiaClass = _T("BaiduBaoheFrameWorkWndClassName");		//�ٶ��ƹܼ�  ���������� �������Ĵ�����
LPCTSTR  GuanJiaProcessName = _T("baiduyunguanjia.exe");			//�ٶ��ƹܼ� �Ľ�����

struct MY_POINT					//��¼Ŀ����������������ı���ֵ�ṹ
{
	double x_percent;			//Ŀ���������X�ı���ֵ
	double y_percent;			//Ŀ���������Y�ı���ֵ
};

typedef BOOL(WINAPI *playsoundW)(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);
playsoundW oldPlaySoundW = NULL;		//���ڱ���ԭ������ַ
FARPROC pfoldPlaySoundW = NULL;			//ָ��ԭ������ַ��Զָ��
BYTE OldCodeW[5];					//�ϵ�ϵͳAPI��ڴ���
BYTE NewCodeW[5];					//Ҫ��ת��API���� (jmp xxxx)
MY_POINT pos = { 0 };				//Ŀ����������������ı���ֵ(����ڴ���)
bool isPaintLine = false;			//�Ƿ������贰��߿�
BOOL  WINAPI MyplaysoundW(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);
int inject();
int HookOn();
int HookOff();
int SetClickPos();
INT GetProcessNameByPID(DWORD pid, LPTSTR processName);		//VC���ݽ�������ý���ID		
int SendClick(HWND wnd, MY_POINT cxPos);					//ģ��������								
unsigned int __stdcall AutoClick(PVOID wnd);						//�Զ����, ���� SendClick(HWND wnd, POINT cxPos)����
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
		//��������ڴ����е��������ı���ֵ
		pos.x_percent = (point.x - rect.left) / double(rect.right - rect.left);
		pos.y_percent = (point.y - rect.top) / double(rect.bottom - rect.top);

		TCHAR name[MAX_PATH], confirm[MAX_PATH];
		GetWindowText(g_hWnd, name, MAX_PATH);
		_stprintf_s(confirm, MAX_PATH, _T("���ڱ���:  %s\n�������:  �����еĺ��\n\nһ��ȷ��, ֻ����������Ŀ������������ Ŀ������\n\n�Ƿ�ȷ�� Ŀ�괰�� �� Ŀ�����ꣿ"), name);
		
		isPaintLine = TRUE;
		//������߿��߳�
		assert(CloseHandle((HANDLE)_beginthreadex(NULL, 0, PaintLine, &pos, 0, NULL)) > 0);
		
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
		HPEN hpen = CreatePen(PS_SOLID, 8, RGB(255, 0, 0));
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

		point.x = double(rect.right - rect.left) * pos.x_percent + double(rect.left);
		point.y = double(rect.bottom - rect.top) * pos.y_percent + double(rect.top);
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
		Sleep(200);
	}
	return TRUE;
}

//���õ������
int SetClickPos()
{
	//�����߳�
	assert(CloseHandle((HANDLE)_beginthreadex(NULL, 0, SetPosThreadFun, NULL, 0, NULL)) > 0);
	return TRUE;
}

BOOL  WINAPI MyplaysoundW(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound)
{
	//�ָ�ԭ���̵�oldPlaySoundW()
	HookOff();
	INT res = oldPlaySoundW( pszSound,  hmod, fdwSound);
	//���� �ܼ� �ϴ��쳣, ��ʼģ���� ����
	assert(CloseHandle((HANDLE)_beginthreadex(NULL, 0, AutoClick, &g_hWnd, 0, NULL)) > 0);
	//�ָ�hook.dll�е�MyplaysoundW()
	HookOn();
	return res;
}

//�Զ����, ���� SendClick(HWND wnd, MY_POINT cxPos)����
unsigned int __stdcall AutoClick(PVOID wnd)
{
	Sleep(800);
	if (TRUE == isHookGuanJia)
	{
		//���hook�˹ܼ�, Ҫ����ļ����´������ģ����
		HWND hWnd2 = ::GetTopWindow(0);
		TCHAR name2[100] = {0};
		while (hWnd2)
		{
			GetClassName(hWnd2, name2, 100);
			if (0 == _tcscmp(name2, showGuanJiaClass))
			{
				//�������������showGuanJiaClass, ��"BaiduBaoheFrameWorkWndClassName", 
				//����ٶȹܼ�
				//�������� ��spy���߼���ƹܼ�������
				SendMessage(hWnd2, WM_USER + 1025, 0x200, 0);
				Sleep(100);
				SendMessage(hWnd2, WM_USER + 1025, 0x202, 0);
				Sleep(100);
				//����ǹܼ�, �����ش���, ���ص�������
				::SetForegroundWindow((*(HWND*)wnd));
				ShowWindow((*(HWND*)wnd), SW_HIDE);
				Sleep(500);
				break;
			}
			hWnd2 = ::GetNextWindow(hWnd2, GW_HWNDNEXT);
		}
	}
	else
	{
		//�������������, ����С��
		::SetForegroundWindow((*(HWND*)wnd));
		ShowWindow((*(HWND*)wnd), SW_MINIMIZE);
		Sleep(500);
	}
	//�����Ƿ�hook�˹ܼ�, ��ʼģ����

	SendClick((*(HWND*)wnd), pos);
	return TRUE;
}

//ģ��������
int SendClick(HWND wnd, MY_POINT cxPos)
{
	//cxPos������ڴ�����������ı���ֵ		wnd��Ŀ�Ĵ���
	DWORD click = 0;
	RECT rect = {0};
	GetWindowRect(wnd, &rect);
	//���ݱ���ֵ ���� �������
	DWORD cx = DWORD(double(rect.right - rect.left) * cxPos.x_percent);
	DWORD cy = DWORD(double(rect.bottom - rect.top) * cxPos.y_percent);

	click = ((DWORD(cy)) << 16) | DWORD(cx);
	//��ʼģ����
	SendMessage(g_hWnd, WM_LBUTTONDOWN, MK_LBUTTON, click);
	Sleep(100);
	SendMessage(g_hWnd, WM_LBUTTONUP, 0, click);
	Sleep(100);
	return TRUE;
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

//VC���ݽ�������ý���ID
INT GetProcessNameByPID(DWORD pid, LPTSTR processName)
{
	HANDLE snapshot;
	PROCESSENTRY32 processinfo;
	processinfo.dwSize = sizeof(processinfo);
	snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == NULL)
		return FALSE;

	BOOL status = Process32First(snapshot, &processinfo);
	while (status)
	{
		if (processinfo.th32ProcessID == pid)
		{
			_tcscpy_s(processName, MAX_PATH, processinfo.szExeFile);
			return TRUE;
		}
		status = Process32Next(snapshot, &processinfo);
	}
	return FALSE;
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
		hInst = hinstDLL;
		//���dll ʵ�������̾��
		hInst = ::GetModuleHandle(NULL);
		dwPid = ::GetCurrentProcessId();
		hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE, 0, dwPid);		//		XP�²�����PROCESS_ALL_ACCESS
		TCHAR name[MAX_PATH];
		if (GetProcessNameByPID(dwPid, name))
		{
			//�ѽ�������ת��Сд
			_tcslwr_s(name, MAX_PATH);
			//����Ƿ�hook�˹ܼ�
			if (0 == _tcscmp(name, GuanJiaProcessName))
			{
				isHookGuanJia = TRUE;
			}
				
		}

		//����ע�亯��
		inject();
		//���õ������ʹ���
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
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.		
}