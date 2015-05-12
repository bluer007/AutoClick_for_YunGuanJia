//////////////////////////////////////////////////////////////////
//
//作用: 本工程是hook.dll的实现. 用于给 自动点击界面端 调用并进行相关hook
//作者: bluer
//时间: 2015.5.8
//
/////////////////////////////////////////////////////////////////


#include <windows.h>
#include <tchar.h>
#include <assert.h>
#include <process.h>
#include <tlhelp32.h>			////枚举进程头文件


//全局共享变量
#pragma data_seg(".Share")
	HWND g_hWnd = NULL;			//主窗口句柄;
	HINSTANCE hInst = NULL;		//本dll实例句柄;
#pragma data_seg()
#pragma comment(linker, "/section:.Share,rws")
//以上的变量在所有被hook程序中共享哦!



//定义本程序的全局变量
HANDLE hProcess = NULL;				//保存被hook程序的进程句柄
BOOL bIsInjected = FALSE;			//是否已经注入了进程
BOOL bHook = FALSE;					//是否Hook了API
BOOL isHookGuanJia = FALSE;			//本工具是否hook了管家
LPCTSTR  showGuanJiaClass = _T("BaiduBaoheFrameWorkWndClassName");		//百度云管家  控制主窗口 弹出与否的窗体类
LPCTSTR  GuanJiaProcessName = _T("baiduyunguanjia.exe");			//百度云管家 的进程名

struct MY_POINT					//记录目标鼠标点击的相对坐标的比例值结构
{
	double x_percent;			//目标相对坐标X的比例值
	double y_percent;			//目标相对坐标Y的比例值
};

typedef BOOL(WINAPI *playsoundW)(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);
playsoundW oldPlaySoundW = NULL;		//用于保存原函数地址
FARPROC pfoldPlaySoundW = NULL;			//指向原函数地址的远指针
BYTE OldCodeW[5];					//老的系统API入口代码
BYTE NewCodeW[5];					//要跳转的API代码 (jmp xxxx)
MY_POINT pos = { 0 };				//目标鼠标点击的相对坐标的比例值(相对于窗体)
bool isPaintLine = false;			//是否正在描窗体边框
BOOL  WINAPI MyplaysoundW(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);
int inject();
int HookOn();
int HookOff();
int SetClickPos();
INT GetProcessNameByPID(DWORD pid, LPTSTR processName);		//VC根据进程名获得进程ID		
int SendClick(HWND wnd, MY_POINT cxPos);					//模拟点击函数								
unsigned int __stdcall AutoClick(PVOID wnd);						//自动点击, 调用 SendClick(HWND wnd, POINT cxPos)函数
unsigned int __stdcall PaintLine(PVOID ppoint);
unsigned int __stdcall SetPosThreadFun(PVOID pM);

//设置点击坐标的子线程
unsigned int __stdcall SetPosThreadFun(PVOID pM)
{
	while (1)
	{
		MessageBox(NULL, _T("开始设置点击坐标\n请在 5s 内移动到 目标窗口 的 目标坐标"), _T("提醒"), MB_OK);
		Sleep(5000);
		POINT point = { 0 };		//鼠标绝对坐标
		//获取鼠标坐标和 所在窗口
		GetCursorPos(&point);
		g_hWnd = WindowFromPoint(point);
		RECT rect = {0};
		//获得窗体大小和坐标
		GetWindowRect(g_hWnd, &rect);
		//计算鼠标在窗体中的相对坐标的比例值
		pos.x_percent = (point.x - rect.left) / double(rect.right - rect.left);
		pos.y_percent = (point.y - rect.top) / double(rect.bottom - rect.top);

		TCHAR name[MAX_PATH], confirm[MAX_PATH];
		GetWindowText(g_hWnd, name, MAX_PATH);
		_stprintf_s(confirm, MAX_PATH, _T("窗口标题:  %s\n点击坐标:  窗口中的红点\n\n一旦确定, 只有重新运行目标程序才能修正 目标坐标\n\n是否确定 目标窗口 和 目标坐标？"), name);
		
		isPaintLine = TRUE;
		//创建描边框线程
		assert(CloseHandle((HANDLE)_beginthreadex(NULL, 0, PaintLine, &pos, 0, NULL)) > 0);
		
		if (MessageBox(NULL, confirm, _T("确认"), MB_YESNO) == IDYES)
		{
			isPaintLine = FALSE;
			::InvalidateRect(NULL, NULL, TRUE);
			break;
		}
		else
		{
			//因为本函数 有5s的睡眠, 所以让子线程退出
			isPaintLine = FALSE;
			::InvalidateRect(NULL, NULL, TRUE);
		}
	}
	return TRUE;
}

//描出选中窗口边框
unsigned int __stdcall PaintLine(PVOID ppoint)
{
	Sleep(300);		//等待SetPosThreadFun()中的messagebox()出现
	static RECT rect = { 0 };
	RECT preRect = {0};		//记录上一次窗体的大小和坐标情况
	while (TRUE == isPaintLine)
	{
		HDC hdc = GetWindowDC(GetDesktopWindow());
		HPEN hpen = CreatePen(PS_SOLID, 8, RGB(255, 0, 0));
		HGDIOBJ  holdPen = SelectObject(hdc, hpen);
		POINT point = { 0 };	//鼠标点击的坐标
		POINT oldPoint = {0};
		//记录上一次窗体的大小和坐标情况
		memcpy_s(&preRect, sizeof(preRect), &rect, sizeof(rect));	
		//获得窗体  当前大小和坐标
		GetWindowRect(g_hWnd, &rect);
		if (0 != memcmp(&preRect, &rect, sizeof(RECT)))
		{
			//上次坐标和当前窗体坐标不同, 所以刷新桌面, 让上次描的边框消失
			//RedrawWindow(NULL, NULL, NULL, RDW_UPDATENOW);
			::InvalidateRect(NULL, NULL, FALSE);
		}

		point.x = double(rect.right - rect.left) * pos.x_percent + double(rect.left);
		point.y = double(rect.bottom - rect.top) * pos.y_percent + double(rect.top);
		//上边
		MoveToEx(hdc, rect.left, rect.top, &oldPoint);
		LineTo(hdc, rect.right, rect.top);
		//右边
		LineTo(hdc, rect.right, rect.bottom);
		//下边
		LineTo(hdc, rect.left, rect.bottom);
		//左边
		LineTo(hdc, rect.left, rect.top);
		//点击的坐标
		MoveToEx(hdc, point.x, point.y, &oldPoint);
		LineTo(hdc, point.x, point.y);

		SelectObject(hdc, holdPen);
		DeleteObject(hpen);
		ReleaseDC(GetDesktopWindow(), hdc);
		Sleep(200);
	}
	return TRUE;
}

//设置点击坐标
int SetClickPos()
{
	//创建线程
	assert(CloseHandle((HANDLE)_beginthreadex(NULL, 0, SetPosThreadFun, NULL, 0, NULL)) > 0);
	return TRUE;
}

BOOL  WINAPI MyplaysoundW(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound)
{
	//恢复原进程的oldPlaySoundW()
	HookOff();
	INT res = oldPlaySoundW( pszSound,  hmod, fdwSound);
	//发现 管家 上传异常, 开始模拟点击 继续
	assert(CloseHandle((HANDLE)_beginthreadex(NULL, 0, AutoClick, &g_hWnd, 0, NULL)) > 0);
	//恢复hook.dll中的MyplaysoundW()
	HookOn();
	return res;
}

//自动点击, 调用 SendClick(HWND wnd, MY_POINT cxPos)函数
unsigned int __stdcall AutoClick(PVOID wnd)
{
	Sleep(800);
	if (TRUE == isHookGuanJia)
	{
		//如果hook了管家, 要特殊的激活下窗体才能模拟点击
		HWND hWnd2 = ::GetTopWindow(0);
		TCHAR name2[100] = {0};
		while (hWnd2)
		{
			GetClassName(hWnd2, name2, 100);
			if (0 == _tcscmp(name2, showGuanJiaClass))
			{
				//如果窗体类名是showGuanJiaClass, 即"BaiduBaoheFrameWorkWndClassName", 
				//激活百度管家
				//以下数据 从spy工具监测云管家中所得
				SendMessage(hWnd2, WM_USER + 1025, 0x200, 0);
				Sleep(100);
				SendMessage(hWnd2, WM_USER + 1025, 0x202, 0);
				Sleep(100);
				//如果是管家, 则隐藏窗口, 即回到托盘栏
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
		//如果是其他程序, 则最小化
		::SetForegroundWindow((*(HWND*)wnd));
		ShowWindow((*(HWND*)wnd), SW_MINIMIZE);
		Sleep(500);
	}
	//无论是否hook了管家, 开始模拟点击

	SendClick((*(HWND*)wnd), pos);
	return TRUE;
}

//模拟点击函数
int SendClick(HWND wnd, MY_POINT cxPos)
{
	//cxPos是相对于窗体的相对坐标的比例值		wnd是目的窗体
	DWORD click = 0;
	RECT rect = {0};
	GetWindowRect(wnd, &rect);
	//根据比例值 计算 相对坐标
	DWORD cx = DWORD(double(rect.right - rect.left) * cxPos.x_percent);
	DWORD cy = DWORD(double(rect.bottom - rect.top) * cxPos.y_percent);

	click = ((DWORD(cy)) << 16) | DWORD(cx);
	//开始模拟点击
	SendMessage(g_hWnd, WM_LBUTTONDOWN, MK_LBUTTON, click);
	Sleep(100);
	SendMessage(g_hWnd, WM_LBUTTONUP, 0, click);
	Sleep(100);
	return TRUE;
}


//关闭钩子
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
				MessageBox(NULL, _T("更改进程内存保护模式失败"), _T("VirtualProtectEx() ERROR"), MB_OK);
		}
		else
			MessageBox(NULL, _T("进程内存写入失败"), _T("WriteProcessMemory() ERROR"), MB_OK);
	}
	else
		MessageBox(NULL, _T("更改进程内存保护模式失败"), _T("VirtualProtectEx() ERROR"), MB_OK);
	
	
	bHook = FALSE;
	return TRUE;
}

//开启钩子
int HookOn()
{
	assert(hProcess != NULL);

	DWORD dwTemp = 0;
	DWORD dwOldProtect;

	//将内存保护模式改为可写,以前的内存保护模式保存入dwOldProtect
	if (VirtualProtectEx(hProcess, pfoldPlaySoundW, 5, PAGE_READWRITE, &dwOldProtect))
	{
		//将所属进程中add()的前5个字节改为Jmp Myadd 
		if (WriteProcessMemory(hProcess, pfoldPlaySoundW, NewCodeW, 5, 0))
		{
			//将内存保护模式改回为dwOldProtect
			if (VirtualProtectEx(hProcess, pfoldPlaySoundW, 5, dwOldProtect, &dwTemp))
			{
				;
			}
			else
				MessageBox(NULL, _T("更改进程内存保护模式失败"), _T("VirtualProtectEx() ERROR"), MB_OK);
		}
		else
			MessageBox(NULL, _T("进程内存写入失败"), _T("WriteProcessMemory() ERROR"), MB_OK);
	}
	else
		MessageBox(NULL, _T("更改进程内存保护模式失败"), _T("VirtualProtectEx() ERROR"), MB_OK);
	bHook = TRUE;
	return TRUE;
}

//声明本dll用到的函数
int inject()
{
	if (bIsInjected == FALSE)
	{ 
		//保证只调用1次
		bIsInjected = TRUE;

		//获取函数
		HMODULE hmod = ::LoadLibrary(_T("WINMM.DLL"));
		oldPlaySoundW = (playsoundW)::GetProcAddress(hmod, "PlaySoundW");
		pfoldPlaySoundW = (FARPROC)oldPlaySoundW;
		if (pfoldPlaySoundW == NULL)
		{
			MessageBox(NULL, _T("不能找到进程中原PlaySoundW()函数"), _T("GetProcAddress() error"), 0);
			return -1;
		}

		// 将add()中的入口代码保存入OldCode[]
		_asm
		{
			lea edi, OldCodeW
				mov esi, pfoldPlaySoundW
				cld
				movsd
				movsb
		}

		NewCodeW[0] = 0xe9;//实际上0xe9就相当于jmp指令
						  //获取MyplaysoundW()的相对地址
		_asm
		{
			lea eax, MyplaysoundW
				mov ebx, pfoldPlaySoundW
				sub eax, ebx
				sub eax, 5
				mov dword ptr[NewCodeW + 1], eax
		}
		//填充完毕，现在NewCode[]里的指令相当于Jmp MyplaysoundW
		HookOn(); //可以开启钩子了
	}

	return TRUE;
}

//VC根据进程名获得进程ID
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


//dll的主函数, 任何加载dll和释放dll都要调用本函数
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
		//获得dll 实例，进程句柄
		hInst = ::GetModuleHandle(NULL);
		dwPid = ::GetCurrentProcessId();
		hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE, 0, dwPid);		//		XP下不能用PROCESS_ALL_ACCESS
		TCHAR name[MAX_PATH];
		if (GetProcessNameByPID(dwPid, name))
		{
			//把进程名字转成小写
			_tcslwr_s(name, MAX_PATH);
			//检测是否hook了管家
			if (0 == _tcscmp(name, GuanJiaProcessName))
			{
				isHookGuanJia = TRUE;
			}
				
		}

		//调用注射函数
		inject();
		//设置点击坐标和窗口
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