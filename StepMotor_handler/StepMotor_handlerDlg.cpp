#include "pch.h"
#include "framework.h"
#include "StepMotor_handler.h"
#include "StepMotor_handlerDlg.h"
#include "afxdialogex.h"
#include <cstdint>
#include <vector>
#include <iostream>
#include <iomanip>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

HWND hCommWnd;
const CString run_Alarm_text = _T("[Alarm] 작동 중 설정 변경은 정지 후 재동작 시 적용됩니다.\r\n");

#define TIMER_TIME_RUN		1000
#define TIMER_USER_TEST		2000
#define TIMER_STOP 3000
#define TIMER_PERIOD 4000
int nDelay = 0;


class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX); 

protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{

}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

CStepMotorhandlerDlg::CStepMotorhandlerDlg(CWnd* pParent)
	: CDialogEx(IDD_STEPMOTOR_HANDLER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_iStopBit = 0;
	m_iSerialPort = 7;
	m_iParity = 0;
	m_iDataBit = 3;
	m_iBaudRate = 7;
}

void CStepMotorhandlerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PortCOMBO, m_PotNum);
	DDX_Control(pDX, IDC_PROTOCOL_WRITE_LIST, m_Write_List_Ctrl);


}

BEGIN_MESSAGE_MAP(CStepMotorhandlerDlg, CDialogEx)
	ON_MESSAGE(WM_COMM_READ, OnCommunication) //추가
	ON_WM_HSCROLL()
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OpenBTN, &CStepMotorhandlerDlg::OnBnClickedOpenbtn)
	ON_BN_CLICKED(IDC_CloseBTN, &CStepMotorhandlerDlg::OnBnClickedClosebtn)
	ON_WM_TIMER()

	ON_BN_CLICKED(IDC_RUN_BTN, &CStepMotorhandlerDlg::OnBnClickedRunBtn)
	ON_BN_CLICKED(IDC_STOP_BTN, &CStepMotorhandlerDlg::OnBnClickedStopBtn)
	ON_BN_CLICKED(IDC_PRO_SET_BTN, &CStepMotorhandlerDlg::OnBnClickedProSetBtn)
	ON_BN_CLICKED(IDC_PRO_LIST_RESET_BTN, &CStepMotorhandlerDlg::OnBnClickedProListResetBtn)
	ON_BN_CLICKED(IDC_PRO_LIST_DELETE_BTN, &CStepMotorhandlerDlg::OnBnClickedProListDeleteBtn)
	ON_BN_CLICKED(IDC_LOAD_BTN, &CStepMotorhandlerDlg::OnBnClickedLoadBtn)
	ON_BN_CLICKED(IDC_SAVE_BTN, &CStepMotorhandlerDlg::OnBnClickedSaveBtn)
	ON_BN_CLICKED(IDC_EDIT_CLEAR_BTN, &CStepMotorhandlerDlg::OnBnClickedEditClearBtn)
END_MESSAGE_MAP()


BOOL CStepMotorhandlerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_Timer.AttachListener(*(CMMTimerListener*)this);

	m_PotNum.SetCurSel(7);
	
	bRunning = FALSE;
	nSet_Runtime = 0;
	
	CRect rt;
	m_Write_List_Ctrl.GetWindowRect(&rt);
	m_Write_List_Ctrl.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);


	m_Write_List_Ctrl.InsertColumn(0, TEXT("No"), LVCFMT_CENTER, rt.Width()*0.198); 
	m_Write_List_Ctrl.InsertColumn(1, TEXT("TargetRPM"), LVCFMT_CENTER, rt.Width()*0.198); 
	m_Write_List_Ctrl.InsertColumn(2, TEXT("Period"), LVCFMT_CENTER, rt.Width()*0.198);
	m_Write_List_Ctrl.InsertColumn(3, TEXT("Delta"), LVCFMT_CENTER, rt.Width()*0.198);  
	m_Write_List_Ctrl.InsertColumn(4, TEXT("Delay"), LVCFMT_CENTER, rt.Width()*0.198);     
 
	return TRUE; 
}

void CStepMotorhandlerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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


void CStepMotorhandlerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CStepMotorhandlerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

CString CStepMotorhandlerDlg::byIndexComPort(int xPort)
{
	CString PortName;
	if (xPort < 4)
	{
		PortName.Format(_T("COM%d"), xPort + 1);
	}
	else if (xPort >= 4)
	{
		PortName.Format(_T("\\\\.\\COM%d"), xPort + 1);
	}
	return PortName;
}

DWORD CStepMotorhandlerDlg::byIndexBaud(int xBaud)
{
	DWORD dwBaud;
	switch (xBaud)
	{
	case 0:		dwBaud = CBR_4800;		break;

	case 1:		dwBaud = CBR_9600;		break;

	case 2:		dwBaud = CBR_14400;		break;

	case 3:		dwBaud = CBR_19200;		break;

	case 4:		dwBaud = CBR_38400;		break;

	case 5:		dwBaud = CBR_56000;		break;

	case 6:		dwBaud = CBR_57600;		break;

	case 7:		dwBaud = CBR_115200;	break;
	}

	return dwBaud;
}

BYTE CStepMotorhandlerDlg::byIndexData(int xData)
{
	BYTE byData;
	switch (xData)
	{
	case 0:	byData = 5;			break;

	case 1:	byData = 6;			break;

	case 2:	byData = 7;			break;

	case 3:	byData = 8;			break;
	}
	return byData;
}

BYTE CStepMotorhandlerDlg::byIndexStop(int xStop)
{
	BYTE byStop;
	if (xStop == 0)
	{
		byStop = ONESTOPBIT;
	}
	else
	{
		byStop = TWOSTOPBITS;
	}
	return byStop;
}

BYTE CStepMotorhandlerDlg::byIndexParity(int xParity)
{
	BYTE byParity;
	switch (xParity)
	{
	case 0:	byParity = NOPARITY;	break;

	case 1:	byParity = ODDPARITY;	break;

	case 2:	byParity = EVENPARITY;	break;
	}

	return byParity;
}

void CStepMotorhandlerDlg::OnBnClickedOpenbtn()
{
	if (m_ComuPort.m_bConnected == FALSE)//포트가 닫혀 있을 경우에만 포트를 열기 위해
	{
		m_iSerialPort = ((CComboBox*)GetDlgItem(IDC_PortCOMBO))->GetCurSel();
		if (m_ComuPort.OpenPort(byIndexComPort(m_iSerialPort), byIndexBaud(m_iBaudRate), byIndexData(m_iDataBit), byIndexStop(m_iStopBit), byIndexParity(m_iParity)))
		{
			//m_edit_log.ReplaceSel(_T("접속 되었습니다.\r\n"));
			SetDlgItemText(IDC_STATIC_CONNECT, _T("Connected"));
			Sleep(100);
		}
		else
		{
			//m_edit_log.ReplaceSel(_T("접속 실패했습니다.\r\n"));
		}
	}
	else
	{
		//m_edit_log.ReplaceSel(_T("이미 포트가 사용 중 입니다.\r\n"));
	}
}


void CStepMotorhandlerDlg::OnBnClickedClosebtn()
{
	m_ComuPort.ClosePort();
	//m_edit_log.ReplaceSel(_T("접속을 해제했습니다.\r\n"));
	SetDlgItemText(IDC_STATIC_CONNECT, _T("Not Connected"));
	SetDlgItemText(IDC_STATIC_COMM_MODE, _T(""));
}

DWORD CStepMotorhandlerDlg::send_data(unsigned char* data, int size)
{
	return m_ComuPort.WriteComm(data, size);
}

LRESULT CStepMotorhandlerDlg::OnCommunication(WPARAM wParam, LPARAM lParam)
{
	UpdateData(TRUE);//받는 데이터 타입을 알기 위해
	CString str = _T("");
	CString result;

	unsigned char* data = new unsigned char[1024];
	BYTE aByte; //데이터를 저장할 변수 
	int iSize = (m_ComuPort.m_QueueRead).GetSize(); //포트로 들어온 데이터 갯수
	if (iSize != 0) {
		ComRead = true;
		OutputDebugString(_T("comread = true \r\n"));
	}
	delete[]data;
	return 1;
}

unsigned short CStepMotorhandlerDlg::modbus_crc16(const unsigned char* data, size_t length)
{
	uint16_t crc = 0xFFFF;

	for (size_t pos = 0; pos < length; pos++) {
		crc ^= data[pos];

		for (int i = 0; i < 8; i++) {
			if (crc & 0x0001) {
				crc >>= 1;
				crc ^= 0xA001;
			}
			else {
				crc >>= 1;
			}
		}
	}
	unsigned char recon_CRC[2];
	recon_CRC[0] = crc >> 8;
	recon_CRC[1] = crc;

	crc = recon_CRC[1] << 8;
	crc += recon_CRC[0];
	return crc; // LSB가 먼저 (low byte first)
}

void CStepMotorhandlerDlg::OnTimer(UINT_PTR nIDEvent)
{
	CDialogEx::OnTimer(nIDEvent);


	if (nIDEvent == TIMER_TIME_RUN)
	{
		if (Delay == nTime_cnt) {

			CString str;
			str.Format(_T("Run 끝\r\n"), nTime_cnt);
			OutputDebugString(str);
			KillTimer(TIMER_TIME_RUN);
			OnBnClickedStopBtn();

		}
		else {
			CString str;
			str.Format(_T("nTime_cnt : %d\r\n"), nTime_cnt + 1);
			OutputDebugString(str);
		}
		nTime_cnt++;
	}
	if (nIDEvent == TIMER_PERIOD) {

		if (nProtocol_step == 0) {
			SetNextCurrentRPM(nProtocol_step);
		}
		
		KillTimer(TIMER_USER_TEST);

		if (target_RPM > current_RPM) {

			target_RPM -= delta_RPM;

			if (target_RPM < current_RPM) {
				target_RPM = current_RPM;
			}
			CString str;
			str.Format(_T("1.  감속 설정 중 target_RPM : %d, current_RPM : %d\r\n"), target_RPM, current_RPM);
			OutputDebugString(str);
			str.Format(_T("감속 설정 중, RPM : %d\r\n"), target_RPM);
			SetDlgItemText(IDC_INFO_STATIC, str);
			SetDlgItemInt(IDC_TARGET_EDIT, target_RPM);

			OnBnClickedRunBtn2();


		}
		else if (target_RPM < current_RPM) {

			target_RPM += delta_RPM;

			if (target_RPM > current_RPM){
				target_RPM = current_RPM;
				}
			CString str;
			str.Format(_T("2. 가속 설정 중 target_RPM : %d, current_RPM : %d\r\n"), target_RPM, current_RPM);
			OutputDebugString(str);
			str.Format(_T("가속 설정 중, RPM : %d\r\n"), target_RPM);
			SetDlgItemText(IDC_INFO_STATIC, str);
			SetDlgItemInt(IDC_TARGET_EDIT, target_RPM);

			OnBnClickedRunBtn2();


		}
		else {

			CString str;
			if (cProtocol_Info.nProtocol_len == nProtocol_cnt) {
				str.Format(_T("Protocol 끝\r\n"));
				SetDlgItemText(IDC_INFO_STATIC, str);

				str.Format(_T("타이머 종료. \r\n"), nTime_cnt + 1);
				SetDlgItemText(IDC_TIME_STATIC, str);

				OnBnClickedStopBtn();

			}
			else{
				str.Format(_T("target_RPM : %d -> current_RPM : %d까지 설정 완료, %d번 프로토콜 진행 \r\n"), target_RPM, current_RPM, nProtocol_step);
				OutputDebugString(str);
				str.Format(_T("List No : %d번 프로토콜 진행 \r\n"), nProtocol_step);
				SetDlgItemText(IDC_INFO_STATIC, str);
				KillTimer(TIMER_PERIOD);
				SetTimer(TIMER_USER_TEST, 1000, NULL);
			}

		}

		if (target_RPM < 10000) {
			OnBnClickedStopBtn();
			KillTimer(TIMER_PERIOD);
			KillTimer(TIMER_USER_TEST);
		}
	}
	if (nIDEvent == TIMER_USER_TEST) {	

		if (Delay == nTime_cnt) {
			nProtocol_step++;
			nProtocol_cnt++;

			CString str;
			str.Format(_T("타이머 대기. \r\n"), nTime_cnt + 1);

			SetDlgItemText(IDC_TIME_STATIC, str);
			if (nProtocol_step < 0) {
				nProtocol_step = 0;
			}
			SetNextTargetRPM(nProtocol_step - 1);
			SetNextControlPeriod(nProtocol_step);
			SetNextDeltaRPM(nProtocol_step);
			SetNextDelay(nProtocol_step);

			OnBnClickedRunBtn2();

		}
		else {
			CString str;
			str.Format(_T("nTime_cnt : %d\r\n"), nTime_cnt + 1);
			OutputDebugString(str);
			str.Format(_T("%d초 경과.. \r\n"), nTime_cnt + 1);
			SetDlgItemText(IDC_TIME_STATIC, str);

			str.Format(_T("nProtocol_cnt : %d\r\n"), nProtocol_cnt);
			OutputDebugString(str);
			SetNextCurrentRPM(nProtocol_step + 1);
		
		}
		if (cProtocol_Info.nProtocol_len == nProtocol_cnt) {
			CString str;
			str.Format(_T("Protocol 끝\r\n"));
			SetDlgItemText(IDC_INFO_STATIC, str);
			OutputDebugString(str);
			OnBnClickedStopBtn();
			KillTimer(TIMER_PERIOD);
			KillTimer(TIMER_USER_TEST);
		}
		//else {
		//	OnBnClickedRunBtn();
		//}

		nTime_cnt++;
	}

}


void CStepMotorhandlerDlg::Finish_test()
{
	bRunning = FALSE;
	KillTimer(TIMER_USER_TEST);
	nTime_cnt = 0;

	target_RPM = 0;

	CStringA cmd;
	cmd.Format("SPD %u\r\n", target_RPM);

	const unsigned char* buf = reinterpret_cast<const unsigned char*>((LPCSTR)cmd);
	const int len = cmd.GetLength();
	send_data(const_cast<unsigned char*>(buf), len);

	CString msg;
	msg.Format(_T("send data : %d %u"), target_RPM);
	//m_edit_log.ReplaceSel(_T("모터를 정지합니다.\r\n"));

}

BOOL CStepMotorhandlerDlg::Verify_protocol()
{
	if (false) {

		return FALSE;
	}
	return TRUE;
}


void CStepMotorhandlerDlg::OnBnClickedRunBtn()
{
	UI_disable();
	cProtocol_Info.nProtocol_len = m_Write_List_Ctrl.GetItemCount();

	for (int i = 0; i < cProtocol_Info.nProtocol_len; i++)
	{
		CString strVal;

		// Time
		strVal = m_Write_List_Ctrl.GetItemText(i, 1);
		cProtocol_Info.target_RPM[i] = _ttoi(strVal);

		// Accle
		strVal = m_Write_List_Ctrl.GetItemText(i, 2);
		cProtocol_Info.control_Period[i] = _ttoi(strVal);

		// RPM
		strVal = m_Write_List_Ctrl.GetItemText(i, 3);
		cProtocol_Info.delta_RPM[i] = _ttoi(strVal);

		// Repeat
		strVal = m_Write_List_Ctrl.GetItemText(i, 4);
		cProtocol_Info.Delay[i] = _ttoi(strVal);


	}
	if (run_Flag == TRUE){
		nProtocol_step = 0;

		SetNextTargetRPM(nProtocol_step);
		SetControlPeriod(0);
		SetDeltaRPM(0);
		SetNextDelay(nProtocol_step);
		SetNextCurrentRPM(nProtocol_step);
	}

	CStringA cmd;
	cmd.Format("SPD %u\r\n", target_RPM);

	const unsigned char* buf = reinterpret_cast<const unsigned char*>((LPCSTR)cmd);
	const int len = cmd.GetLength();
	send_data(const_cast<unsigned char*>(buf), len);

	run_Flag = FALSE;
}


void CStepMotorhandlerDlg::OnBnClickedStopBtn()
{
	UI_enable();
	KillTimer(TIMER_USER_TEST);
	nTime_cnt = -1;
	target_RPM = 0;
	current_RPM = 0;
	nProtocol_cnt = 0;
	run_Flag = TRUE;

	CStringA cmd;
	cmd.Format("SPD %u\r\n", target_RPM);

	Sleep(500);

	const unsigned char* buf = reinterpret_cast<const unsigned char*>((LPCSTR)cmd);
	const int len = cmd.GetLength();
	send_data(const_cast<unsigned char*>(buf), len);

	CString msg;
	msg.Format(_T("send data : %d %u"), target_RPM);
	msg.Format(_T("모터 끝\r\n"), target_RPM);
	OutputDebugString(msg);
	msg.Format(_T("Protocol 끝, 모터 정지\r\n"), target_RPM);
	SetDlgItemText(IDC_INFO_STATIC, msg);

	OnBnClickedEditClearBtn();
	//m_edit_log.ReplaceSel(_T("모터를 정지합니다.\r\n"));
}

void CStepMotorhandlerDlg::SetTargetRPM(int target) 
{

	target_RPM = target;

	SetDlgItemInt(IDC_TARGET_EDIT, target_RPM);

	CString str;
	str.Format(_T("Set target_RPM : %d\r\n"), target_RPM);
	OutputDebugString(str);

}
void CStepMotorhandlerDlg::SetCurrentRPM(int rpm)
{

	current_RPM = rpm;
	CString str;

	SetDlgItemInt(IDC_CURRENT_EDIT, current_RPM);
	str.Format(_T("Set current_RPM : %d\r\n"), current_RPM);
	OutputDebugString(str);
}

void CStepMotorhandlerDlg::SetControlPeriod(int period) 
{
	control_Period = period;

	SetDlgItemInt(IDC_PERIOD_EDIT, control_Period);

	SetTimer(TIMER_PERIOD, control_Period, NULL);
}

void CStepMotorhandlerDlg::SetDeltaRPM(int delta) 
{
	delta_RPM = delta;

	SetDlgItemInt(IDC_DELTA_EDIT, delta_RPM);
}

void CStepMotorhandlerDlg::SetDelay(int delay) 
{

	Delay = delay;
	SetDlgItemInt(IDC_DELAY_EDIT, Delay);

}


void CStepMotorhandlerDlg::OnBnClickedProSetBtn()
{
	UpdateData(TRUE);

	if (m_Write_List_Ctrl.GetItemCount() == 49) {

		AfxMessageBox(_T("프로토콜은 50개 이상 설정 불가능합니다."));
		return;
	}

	int n_Target_RPM = GetDlgItemInt(IDC_TARGET_EDIT);
	int n_Control_Period = GetDlgItemInt(IDC_PERIOD_EDIT);
	int n_Delta_RPM = GetDlgItemInt(IDC_DELTA_EDIT);
	int n_Delay = GetDlgItemInt(IDC_DELAY_EDIT);


	if (n_Target_RPM < 10000 || n_Target_RPM > 40000) {

		AfxMessageBox(_T("Target은 10000~40000 사이 값을 입력해주세요."));
		return;
	}
	if (n_Control_Period > 10000) {

		AfxMessageBox(_T("Period는 10000 이하의 값을 입력해주세요."));
		return;
	}	
	if (n_Delta_RPM > 10000) {

		AfxMessageBox(_T("Delta RPM은 10000 이하의 값을 입력해주세요."));
		return;
	}	
	if (n_Delay > 10000) {

		AfxMessageBox(_T("Delay는 10000 이하의 값을 입력해주세요."));
		return;
	}
	CString str_Target_RPM, str_Control_Period, str_Delta_RPM, str_Delay;

	str_Target_RPM.Format(_T("%d"), n_Target_RPM);
	str_Control_Period.Format(_T("%d"), n_Control_Period);
	str_Delta_RPM.Format(_T("%d"), n_Delta_RPM);
	str_Delay.Format(_T("%d"), n_Delay);


	int nSel = m_Write_List_Ctrl.GetNextItem(-1, LVNI_SELECTED);

	int nInsert;
	if (nSel != -1) {
		nInsert = nSel + 1;
	}
	else {
		nInsert = m_Write_List_Ctrl.GetItemCount();
	}
	CString str_Num;
	str_Num.Format(_T("%d"), nInsert);

	m_Write_List_Ctrl.InsertItem(nInsert, str_Num);
	m_Write_List_Ctrl.SetItemText(nInsert, 0, str_Num);
	m_Write_List_Ctrl.SetItemText(nInsert, 1, str_Target_RPM);
	m_Write_List_Ctrl.SetItemText(nInsert, 2, str_Control_Period);
	m_Write_List_Ctrl.SetItemText(nInsert, 3, str_Delta_RPM);
	m_Write_List_Ctrl.SetItemText(nInsert, 4, str_Delay);

	int nCount = m_Write_List_Ctrl.GetItemCount();
	for (int i = 0; i < nCount; i++)
	{
		CString strIdx;
		strIdx.Format(_T("%d"), i);
		m_Write_List_Ctrl.SetItemText(i, 0, strIdx);
	}

	ListNum++;
}

void CStepMotorhandlerDlg::OnBnClickedEditClearBtn()
{
	SetDlgItemText(IDC_TARGET_EDIT, NULL);
	SetDlgItemText(IDC_PERIOD_EDIT, NULL);
	SetDlgItemText(IDC_DELTA_EDIT, NULL);
	SetDlgItemText(IDC_DELAY_EDIT, NULL);
}


void CStepMotorhandlerDlg::OnBnClickedProListResetBtn()
{
	UpdateData(TRUE);
	m_Write_List_Ctrl.DeleteAllItems();
	ListNum = 0;
}


void CStepMotorhandlerDlg::OnBnClickedProListDeleteBtn()
{
	UpdateData(TRUE);
	int click_Index = m_Write_List_Ctrl.GetSelectionMark();

	m_Write_List_Ctrl.DeleteItem(click_Index);
}


void CStepMotorhandlerDlg::OnBnClickedLoadBtn()
{
	CFileDialog dlg(TRUE, _T("ini"), NULL,
		OFN_FILEMUSTEXIST | OFN_HIDEREADONLY,
		_T("INI Files (*.ini)|*.ini||"));

	if (dlg.DoModal() != IDOK)
		return;

	CString strPath;
	strPath.Format(_T(".\\%s"), dlg.GetFileName());

	m_Write_List_Ctrl.DeleteAllItems();

	for (int i = 0;; i++)
	{
		CString section;
		section.Format(_T("%d Step"), i);

		TCHAR buf[256] = { 0 };
		GetPrivateProfileString(section, _T("Index"), _T(""), buf, 256, strPath);
		CString strIndex = buf;

		if (strIndex.IsEmpty())
			break; // 더 이상 데이터 없음

		CString strTargetRPM, strControlPeriod, strDeltaRPM, strDelay;

		GetPrivateProfileString(section, _T("TargetRPM"), _T(""), buf, 256, strPath); strTargetRPM = buf;
		GetPrivateProfileString(section, _T("Period"), _T(""), buf, 256, strPath); strControlPeriod = buf;
		GetPrivateProfileString(section, _T("DeltaRPM"), _T(""), buf, 256, strPath); strDeltaRPM = buf;
		GetPrivateProfileString(section, _T("Delay"), _T(""), buf, 256, strPath); strDelay = buf;

		int nInsert = m_Write_List_Ctrl.InsertItem(i, strIndex);
		m_Write_List_Ctrl.SetItemText(nInsert, 1, strTargetRPM);
		m_Write_List_Ctrl.SetItemText(nInsert, 2, strControlPeriod);
		m_Write_List_Ctrl.SetItemText(nInsert, 3, strDeltaRPM);
		m_Write_List_Ctrl.SetItemText(nInsert, 4, strDelay);
	}

	CString msg;
	msg.Format(_T("%s Load Complete"), dlg.GetFileName());
	AfxMessageBox(msg);
}


void CStepMotorhandlerDlg::OnBnClickedSaveBtn()
{
	if (m_Write_List_Ctrl.GetItemCount() == 0) {

		AfxMessageBox(_T("프로토콜이 없어 저장되지 않습니다."));
		return;
	}
	// 파일명만 선택 (기본 확장자는 ini)
	CFileDialog dlg(FALSE, _T("ini"), _T("data.ini"),
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("INI Files (*.ini)|*.ini||"));

	if (dlg.DoModal() != IDOK)
		return;

	// 실행폴더 + 파일명
	CString strPath;
	strPath.Format(_T(".\\%s"), dlg.GetFileName());

	int nCount = m_Write_List_Ctrl.GetItemCount();
	int nColCount = 5;

	for (int i = 0; i < nCount; i++)
	{
		CString section;
		section.Format(_T("%d Step"), i);

		for (int j = 0; j < nColCount; j++)
		{
			CString key;
			switch (j)
			{
			case 0: key = _T("Index"); break;
			case 1: key = _T("TargetRPM");  break;
			case 2: key = _T("Period");   break;
			case 3: key = _T("DeltaRPM");   break;
			case 4: key = _T("Delay"); break;
			}

			CString value = m_Write_List_Ctrl.GetItemText(i, j);
			WritePrivateProfileString(section, key, value, strPath);
		}
	}

	CString msg;
	msg.Format(_T("%s Save Complete"), dlg.GetFileName());
	AfxMessageBox(msg);
}

void CStepMotorhandlerDlg::SetNextTargetRPM(int count)
{
	int nTemp_target = cProtocol_Info.target_RPM[count];
	SetTargetRPM(nTemp_target);
}

void CStepMotorhandlerDlg::SetNextControlPeriod(int count)
{
	int nTemp_period = cProtocol_Info.control_Period[count];
	SetControlPeriod(nTemp_period);
}
void CStepMotorhandlerDlg::SetNextDeltaRPM(int count)
{
	int nTemp_delta = cProtocol_Info.delta_RPM[count];
	SetDeltaRPM(nTemp_delta);
}
void CStepMotorhandlerDlg::SetNextDelay(int count)
{
	int nTemp_delay = cProtocol_Info.Delay[count];
	SetDelay(nTemp_delay);
}
void CStepMotorhandlerDlg::SetNextCurrentRPM(int count)
{
	int nTemp_CurrentRPM = cProtocol_Info.target_RPM[count];
	SetCurrentRPM(nTemp_CurrentRPM);
}

void CStepMotorhandlerDlg::OnBnClickedRunBtn2()
{
	nTime_cnt = 0;

	CStringA cmd;
	cmd.Format("SPD %u\r\n", target_RPM);

	const unsigned char* buf = reinterpret_cast<const unsigned char*>((LPCSTR)cmd);
	const int len = cmd.GetLength();
	send_data(const_cast<unsigned char*>(buf), len);

	CString msg;
	msg.Format(_T("target_RPM : %d, current_RPM : %d \r\n"),target_RPM , current_RPM);
	OutputDebugString(msg);

	if (target_RPM == 0) {
		KillTimer(TIMER_USER_TEST);
	}
}


void CStepMotorhandlerDlg::UI_enable() {
	GetDlgItem(IDC_PRO_SET_BTN)->EnableWindow(TRUE);
	GetDlgItem(IDC_TARGET_EDIT)->EnableWindow(TRUE);
	GetDlgItem(IDC_PERIOD_EDIT)->EnableWindow(TRUE);
	GetDlgItem(IDC_DELTA_EDIT)->EnableWindow(TRUE);
	GetDlgItem(IDC_DELAY_EDIT)->EnableWindow(TRUE);
	GetDlgItem(IDC_PRO_LIST_RESET_BTN)->EnableWindow(TRUE);
	GetDlgItem(IDC_PRO_LIST_DELETE_BTN)->EnableWindow(TRUE);
	GetDlgItem(IDC_LOAD_BTN)->EnableWindow(TRUE);
	GetDlgItem(IDC_SAVE_BTN)->EnableWindow(TRUE);
	GetDlgItem(IDC_RUN_BTN)->EnableWindow(TRUE);
	GetDlgItem(IDC_OpenBTN)->EnableWindow(TRUE);
	GetDlgItem(IDC_CloseBTN)->EnableWindow(TRUE);
	GetDlgItem(IDC_EDIT_CLEAR_BTN)->EnableWindow(TRUE);
	GetDlgItem(IDC_PortCOMBO)->EnableWindow(TRUE);
}
void CStepMotorhandlerDlg::UI_disable() {
	GetDlgItem(IDC_PRO_SET_BTN)->EnableWindow(FALSE);
	GetDlgItem(IDC_TARGET_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_PERIOD_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_DELTA_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_DELAY_EDIT)->EnableWindow(FALSE);
	GetDlgItem(IDC_PRO_LIST_RESET_BTN)->EnableWindow(FALSE);
	GetDlgItem(IDC_PRO_LIST_DELETE_BTN)->EnableWindow(FALSE);
	GetDlgItem(IDC_LOAD_BTN)->EnableWindow(FALSE);
	GetDlgItem(IDC_SAVE_BTN)->EnableWindow(FALSE);
	GetDlgItem(IDC_RUN_BTN)->EnableWindow(FALSE);
	GetDlgItem(IDC_OpenBTN)->EnableWindow(FALSE);
	GetDlgItem(IDC_CloseBTN)->EnableWindow(FALSE);
	GetDlgItem(IDC_EDIT_CLEAR_BTN)->EnableWindow(FALSE);
	GetDlgItem(IDC_PortCOMBO)->EnableWindow(FALSE);
}
