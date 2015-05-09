
// 自动点击界面端Dlg.cpp : 实现文件
//

#include "stdafx.h"
#include "自动点击界面端.h"
#include "自动点击界面端Dlg.h"
#include "afxdialogex.h"

#include <tlhelp32.h>			////枚举进程头文件


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// C自动点击界面端Dlg 对话框



C自动点击界面端Dlg::C自动点击界面端Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(C自动点击界面端Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void C自动点击界面端Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(C自动点击界面端Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
ON_BN_CLICKED(IDOK, &C自动点击界面端Dlg::OnBnClickedOk)
END_MESSAGE_MAP()


// C自动点击界面端Dlg 消息处理程序

BOOL C自动点击界面端Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
	int i = this->EnableDebugPriv(SE_DEBUG_NAME);
	i = this->enumProcess();






	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void C自动点击界面端Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void C自动点击界面端Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR C自动点击界面端Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void C自动点击界面端Dlg::OnBnClickedOk()
{
	// TODO:  在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
	//this->enumProcess();		//刷新进程列表
	CListBox *m_CListBox = ((CListBox*)GetDlgItem(IDC_LIST1));
	CString str;
	if (LB_ERR == m_CListBox->GetCurSel() || 0 == m_CListBox->GetCurSel())
	{
		//选择第一项"不选".  或者真正没有选, 就默认 针对 管家
		for (int i = m_CListBox->GetCount(); i; i--)
		{
			m_CListBox->GetText(i-1, str);	
			if (str.Find(_T("baiduyunguanjia.exe")) >= 0)
			{
				//即匹配到 管家 的进程
				//开始提取pid
				str = str.Right(str.GetLength() - str.Find(_T("PID")) - 4);
				str.Format(_T("%d"), _ttol(str.GetBuffer()));
				break;
			}
		}
	}
	else
	{
		//即选择了其他进程,   就针对  选中的进程
		m_CListBox->GetText((m_CListBox->GetCurSel()), str);
		//提取pid
		str = str.Right(str.GetLength() - str.Find(_T("PID")) - 4);
		str.Format(_T("%d"), _ttol(str.GetBuffer()));
	}

	//调用OpenProcess()获得句柄
	HANDLE hRemoteProcess;
	if ((hRemoteProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, _ttol(str.GetBuffer()))) == NULL)
	{
		AfxMessageBox(_T("OpenProcess()其他进程出错了"));
		return;
	}


}


int C自动点击界面端Dlg::enumProcess()			//枚举进程
{
	//获取系统快照
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //不要写错CreateToolhelp32Snapshot()
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolHelp32Snapshot error!\n");
		return -1;
	}

	//创建单个进程快照结构体，初始化大小
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);  //务必提前初始化，否则默认的大小不一定满足要求

	//初始化缓冲区
	WCHAR buff[1024] = { 0 }; //PROCESSENTRY32中的szExeFile为WCHAR类型数组，此处应一致，使用Unicode码

	CListBox *m_CListBox = ((CListBox*)GetDlgItem(IDC_LIST1));
	m_CListBox->ResetContent();
	int nIndex  = 0, line = 0, res = 0;
	nIndex = m_CListBox->InsertString(line++, _T("------不----选-------"));
	//枚举系统快照链表中的第一个进程项目
	BOOL bProcess = Process32First(hProcessSnap, &pe32);
	while (bProcess)
	{
		//格式化进程名和进程ID，这里要使用printf的宽字符版
		//格式字符串“”都需要用L转换为宽字符形式
		wsprintf(buff, L"%s          PID:%d", pe32.szExeFile, pe32.th32ProcessID);
		nIndex = m_CListBox->InsertString(line++, buff);

		//继续枚举下一个进程
		bProcess = Process32Next(hProcessSnap, &pe32);
	}

	CloseHandle(hProcessSnap);

	return TRUE;
}


int C自动点击界面端Dlg::EnableDebugPriv(LPCTSTR name)		//提升本程序权限
{
	HANDLE hToken;			//进程令牌句柄
	TOKEN_PRIVILEGES tp;	//TOKEN_PRIVILEGES结构体，其中包含一个【类型+操作】的权限数组
	LUID luid;				//上述结构体中的类型值

	//打开进程令牌环
	//GetCurrentProcess()获取当前进程的伪句柄，只会指向当前进程或者线程句柄，随时变化
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		AfxMessageBox(_T("打开本进程失败"));
		return -7;
	}

	//获得本地进程name所代表的权限类型的局部唯一ID
	if (!LookupPrivilegeValue(NULL, name, &luid))
	{
		AfxMessageBox(_T("LookupPrivilegeValue() 查询进程的权限 失败"));
		return -8;
	}

	tp.PrivilegeCount = 1;								//权限数组中只有一个“元素”
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;  //权限操作
	tp.Privileges[0].Luid = luid;						 //权限类型

	//调整进程权限
	if (!AdjustTokenPrivileges(hToken, 0, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
	{
		AfxMessageBox(_T("AdjustTokenPrivileges()调整进程权限 失败"));
		return -9;
	}

	return TRUE;
}