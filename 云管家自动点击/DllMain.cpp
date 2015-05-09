//////////////////////////////////////////////////////////////////
//
//作用: 本工程是hook.dll的实现. 用于给 自动点击界面端 调用并进行相关hook
//作者: bluer
//时间: 2015.5.8
//
/////////////////////////////////////////////////////////////////


#include <windows.h>


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
BOOL bIsInjected = FALSE;			//是否已经hook

typedef BOOL(WINAPI *playsoundW)(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);
playsoundW oldMsgBoxW = NULL;		//用于保存原函数地址
FARPROC pfMsgBoxW = NULL;			//指向原函数地址的远指针
BYTE OldCodeW[5];					//老的系统API入口代码
BYTE NewCodeW[5];					//要跳转的API代码 (jmp xxxx)
BOOL  WINAPI MyplaysoundW(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);


//声明本dll用到的函数
int inject()
{


	return TRUE;
}



//dll的主函数, 任何加载dll和释放dll都要调用本函数
BOOL WINAPI DllMain(
	HINSTANCE hinstDLL,  // handle to DLL module
	DWORD fdwReason,     // reason for calling function
	LPVOID lpReserved)  // reserved
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
	

		break;

	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		break;

	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:
		// Perform any necessary cleanup.


		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.		
}