
// 自动点击界面端Dlg.h : 头文件
//

#pragma once


// C自动点击界面端Dlg 对话框
class C自动点击界面端Dlg : public CDialogEx
{
// 构造
public:
	C自动点击界面端Dlg(CWnd* pParent = NULL);	// 标准构造函数

	int enumProcess();						//枚举进程
	int EnableDebugPriv(LPCTSTR name);		//提升本程序权限



// 对话框数据
	enum { IDD = IDD_MY_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
afx_msg void OnBnClickedButton1();
};
