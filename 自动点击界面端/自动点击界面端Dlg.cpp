
// �Զ���������Dlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "�Զ���������.h"
#include "�Զ���������Dlg.h"
#include "afxdialogex.h"

#include <tlhelp32.h>			////ö�ٽ���ͷ�ļ�


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// C�Զ���������Dlg �Ի���



C�Զ���������Dlg::C�Զ���������Dlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(C�Զ���������Dlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void C�Զ���������Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(C�Զ���������Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
ON_BN_CLICKED(IDOK, &C�Զ���������Dlg::OnBnClickedOk)
END_MESSAGE_MAP()


// C�Զ���������Dlg ��Ϣ�������

BOOL C�Զ���������Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
	int i = this->EnableDebugPriv(SE_DEBUG_NAME);
	i = this->enumProcess();






	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void C�Զ���������Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void C�Զ���������Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR C�Զ���������Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void C�Զ���������Dlg::OnBnClickedOk()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	//CDialogEx::OnOK();
	//this->enumProcess();		//ˢ�½����б�
	CListBox *m_CListBox = ((CListBox*)GetDlgItem(IDC_LIST1));
	CString str;
	if (LB_ERR == m_CListBox->GetCurSel() || 0 == m_CListBox->GetCurSel())
	{
		//ѡ���һ��"��ѡ".  ��������û��ѡ, ��Ĭ�� ��� �ܼ�
		for (int i = m_CListBox->GetCount(); i; i--)
		{
			m_CListBox->GetText(i-1, str);	
			if (str.Find(_T("baiduyunguanjia.exe")) >= 0)
			{
				//��ƥ�䵽 �ܼ� �Ľ���
				//��ʼ��ȡpid
				str = str.Right(str.GetLength() - str.Find(_T("PID")) - 4);
				str.Format(_T("%d"), _ttol(str.GetBuffer()));
				break;
			}
		}
	}
	else
	{
		//��ѡ������������,   �����  ѡ�еĽ���
		m_CListBox->GetText((m_CListBox->GetCurSel()), str);
		//��ȡpid
		str = str.Right(str.GetLength() - str.Find(_T("PID")) - 4);
		str.Format(_T("%d"), _ttol(str.GetBuffer()));
	}

	//����OpenProcess()��þ��
	HANDLE hRemoteProcess;
	if ((hRemoteProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, _ttol(str.GetBuffer()))) == NULL)
	{
		AfxMessageBox(_T("OpenProcess()�������̳�����"));
		return;
	}


}


int C�Զ���������Dlg::enumProcess()			//ö�ٽ���
{
	//��ȡϵͳ����
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //��Ҫд��CreateToolhelp32Snapshot()
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolHelp32Snapshot error!\n");
		return -1;
	}

	//�����������̿��սṹ�壬��ʼ����С
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(PROCESSENTRY32);  //�����ǰ��ʼ��������Ĭ�ϵĴ�С��һ������Ҫ��

	//��ʼ��������
	WCHAR buff[1024] = { 0 }; //PROCESSENTRY32�е�szExeFileΪWCHAR�������飬�˴�Ӧһ�£�ʹ��Unicode��

	CListBox *m_CListBox = ((CListBox*)GetDlgItem(IDC_LIST1));
	m_CListBox->ResetContent();
	int nIndex  = 0, line = 0, res = 0;
	nIndex = m_CListBox->InsertString(line++, _T("------��----ѡ-------"));
	//ö��ϵͳ���������еĵ�һ��������Ŀ
	BOOL bProcess = Process32First(hProcessSnap, &pe32);
	while (bProcess)
	{
		//��ʽ���������ͽ���ID������Ҫʹ��printf�Ŀ��ַ���
		//��ʽ�ַ�����������Ҫ��Lת��Ϊ���ַ���ʽ
		wsprintf(buff, L"%s          PID:%d", pe32.szExeFile, pe32.th32ProcessID);
		nIndex = m_CListBox->InsertString(line++, buff);

		//����ö����һ������
		bProcess = Process32Next(hProcessSnap, &pe32);
	}

	CloseHandle(hProcessSnap);

	return TRUE;
}


int C�Զ���������Dlg::EnableDebugPriv(LPCTSTR name)		//����������Ȩ��
{
	HANDLE hToken;			//�������ƾ��
	TOKEN_PRIVILEGES tp;	//TOKEN_PRIVILEGES�ṹ�壬���а���һ��������+��������Ȩ������
	LUID luid;				//�����ṹ���е�����ֵ

	//�򿪽������ƻ�
	//GetCurrentProcess()��ȡ��ǰ���̵�α�����ֻ��ָ��ǰ���̻����߳̾������ʱ�仯
	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		AfxMessageBox(_T("�򿪱�����ʧ��"));
		return -7;
	}

	//��ñ��ؽ���name�������Ȩ�����͵ľֲ�ΨһID
	if (!LookupPrivilegeValue(NULL, name, &luid))
	{
		AfxMessageBox(_T("LookupPrivilegeValue() ��ѯ���̵�Ȩ�� ʧ��"));
		return -8;
	}

	tp.PrivilegeCount = 1;								//Ȩ��������ֻ��һ����Ԫ�ء�
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;  //Ȩ�޲���
	tp.Privileges[0].Luid = luid;						 //Ȩ������

	//��������Ȩ��
	if (!AdjustTokenPrivileges(hToken, 0, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL))
	{
		AfxMessageBox(_T("AdjustTokenPrivileges()��������Ȩ�� ʧ��"));
		return -9;
	}

	return TRUE;
}