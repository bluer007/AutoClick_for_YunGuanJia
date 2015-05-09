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

//全局共享变量
#pragma data_seg(".Share")
	HWND g_hWnd = NULL;			//主窗口句柄;
	HHOOK hhk = NULL;			//鼠标钩子句柄;
	HINSTANCE hInst = NULL;		//本dll实例句柄;
#pragma data_seg()
#pragma comment(linker, "/section:.Share,rws")
//以上的变量在所有被hook程序中共享哦!



//定义本程序的全局变量
HANDLE hProcess = NULL;				//保存被hook程序的进程句柄
BOOL bIsInjected = FALSE;			//是否已经注入了进程
BOOL bHook = FALSE;					//是否Hook了API

typedef BOOL(WINAPI *playsoundW)(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);
playsoundW oldPlaySoundW = NULL;		//用于保存原函数地址
FARPROC pfoldPlaySoundW = NULL;			//指向原函数地址的远指针
BYTE OldCodeW[5];					//老的系统API入口代码
BYTE NewCodeW[5];					//要跳转的API代码 (jmp xxxx)
BOOL  WINAPI MyplaysoundW(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);
int inject();
int HookOn();
int HookOff();


BOOL  WINAPI MyplaysoundW(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound)
{
	HookOff();
	MessageBox(NULL, _T("哈哈, 已经被hook了"), _T("hook success"), MB_OK);
	INT res = oldPlaySoundW( pszSound,  hmod, fdwSound);
	HookOn();
	return res;
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
		hProcess = hinstDLL;
		MessageBox(NULL, TEXT("进程 中 初始化\nDLL_PROCESS_ATTACH"), TEXT(""), MB_OK);
		//获得dll 实例，进程句柄
		hInst = ::GetModuleHandle(NULL);
		dwPid = ::GetCurrentProcessId();
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, 0, dwPid);
		//调用注射函数
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
		MessageBox(NULL, TEXT("进程 中 清理工作\nDLL_PROCESS_DETACH"),TEXT(""), MB_OK);
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.		
}