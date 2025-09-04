
#include "CommThread.h"
#include "MMTimer.h"
// StepMotor_handlerDlg.h: 헤더 파일
//

#pragma once

struct Protocol_Info
{
	int nProtocol_len = 0;
	int	target_RPM[50] = { 0 , };
	int	control_Period[50] = { 0 , };
	int	delta_RPM[50] = { 0 , };
	int	Delay[50] = { 0 , };
	//int	Current_RPM[50] = { 0 , };
	//int Repeat[50] = { 1 , };

	void Init()
	{
		target_RPM[50] = { 5500 , };
		control_Period[50] = { 0 , };
		delta_RPM[50] = { 0 , };
		Delay[50] = { 0 , };
		//Current_RPM[50] = { 0 , };
		//Repeat[50] = { 1 , };
	}
};

// CStepMotorhandlerDlg 대화 상자
class CStepMotorhandlerDlg : public CDialogEx
{

	Protocol_Info cProtocol_Info;

// 생성입니다.
public:
	CStepMotorhandlerDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_STEPMOTOR_HANDLER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public :
	CCommThread m_ComuPort;
	int		m_iStopBit;
	int		m_iSerialPort;
	int		m_iParity;
	int		m_iDataBit;
	int		m_iBaudRate;

	CString byIndexComPort(int xPort);// 포트이름 받기 
	DWORD byIndexBaud(int xBaud);// 전송률 받기
	BYTE byIndexData(int xData);//데이터 비트 받기
	BYTE byIndexStop(int xStop);// 스톱비트 받기 
	BYTE byIndexParity(int xParity);// 펠리티 받기	
	DWORD send_data(unsigned char* data, int size);
	CComboBox m_PotNum;
	unsigned short modbus_crc16(const unsigned char* data, size_t length);

	afx_msg LRESULT OnCommunication(WPARAM wParam, LPARAM lParam);// 추가 시킨 내용
	afx_msg void OnBnClickedOpenbtn();
	afx_msg void OnBnClickedClosebtn();

	CListCtrl m_Write_List_Ctrl;
	int ListNum;


	afx_msg void OnTimer(UINT_PTR nIDEvent);

	BOOL g_bFirstStepDone = FALSE;

private :
	BOOL bRunning;
	int nProtocol_step;
	int nSet_Runtime;
	int nTime_cnt;

	unsigned int before_RPM, target_RPM, control_Period, delta_RPM, Delay, current_RPM; // 현재 변수 이름을 거꾸로 써놨음, 원래 target_RPM 자리에 current_RPM, current_RPM 자리에 target_RPM으로 쓰임.
	CString str_edit_log;

	BOOL ComRead = FALSE;

	int nProtocol_cnt;
	void Finish_test();

	
public:
	BOOL Verify_protocol();

public:
	void CALLBACK Update_RPM(HWND hWnd, UINT, UINT_PTR, DWORD);
	CMMTimer m_Timer;
	UINT m_wTimerRes;
	BOOL run_Flag = TRUE;
	afx_msg void OnBnClickedRunBtn();
	afx_msg void OnBnClickedRunBtn2();
	afx_msg void OnBnClickedStopBtn();

	void SetTargetRPM(int target); 
	void SetControlPeriod(int period);
	void SetDeltaRPM(int delta);
	void SetDelay(int second);
	void SetCurrentRPM(int rpm);

	void SetNextTargetRPM(int count);
	void SetNextControlPeriod(int count);
	void SetNextDeltaRPM(int count);
	void SetNextDelay(int count);
	void SetNextCurrentRPM(int count);

	void UI_enable();
	void UI_disable();

	afx_msg void OnBnClickedProSetBtn();
	afx_msg void OnBnClickedProListResetBtn();
	afx_msg void OnBnClickedProListDeleteBtn();
	afx_msg void OnBnClickedLoadBtn();
	afx_msg void OnBnClickedSaveBtn();
	afx_msg void OnBnClickedEditClearBtn();



};
