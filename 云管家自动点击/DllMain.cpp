//////////////////////////////////////////////////////////////////
//
//����: ��������hook.dll��ʵ��. ���ڸ� �Զ��������� ���ò��������hook
//����: bluer
//ʱ��: 2015.5.8
//
/////////////////////////////////////////////////////////////////


#include <windows.h>


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
BOOL bIsInjected = FALSE;			//�Ƿ��Ѿ�hook

typedef BOOL(WINAPI *playsoundW)(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);
playsoundW oldMsgBoxW = NULL;		//���ڱ���ԭ������ַ
FARPROC pfMsgBoxW = NULL;			//ָ��ԭ������ַ��Զָ��
BYTE OldCodeW[5];					//�ϵ�ϵͳAPI��ڴ���
BYTE NewCodeW[5];					//Ҫ��ת��API���� (jmp xxxx)
BOOL  WINAPI MyplaysoundW(LPCSTR pszSound, HMODULE hmod, DWORD fdwSound);


//������dll�õ��ĺ���
int inject()
{


	return TRUE;
}



//dll��������, �κμ���dll���ͷ�dll��Ҫ���ñ�����
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