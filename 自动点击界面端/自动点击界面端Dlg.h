
// �Զ���������Dlg.h : ͷ�ļ�
//

#pragma once


// C�Զ���������Dlg �Ի���
class C�Զ���������Dlg : public CDialogEx
{
// ����
public:
	C�Զ���������Dlg(CWnd* pParent = NULL);	// ��׼���캯��

	int enumProcess();						//ö�ٽ���
	int EnableDebugPriv(LPCTSTR name);		//����������Ȩ��



// �Ի�������
	enum { IDD = IDD_MY_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
afx_msg void OnBnClickedButton1();
};
