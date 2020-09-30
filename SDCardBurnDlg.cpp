
// SDCardBurnDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SDCardBurn.h"
#include "SDCardBurnDlg.h"
#include "afxdialogex.h"
#include <winioctl.h>
#include <Dbt.h>
#include "MsgBoxTimeout.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//WriteSDCardThread *pThWrite;
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include <winioctl.h>
#include <string.h>
#include <ctype.h>
#include <memory.h>

DISK_GEOMETRY SupportedGeometry[20];
DWORD SupportedGeometryCount;

int GetTotalSector(HANDLE hDisk)
{
	struct _DISK_GEOMETRY_EX 
	{	DISK_GEOMETRY  Geometry;  
		LARGE_INTEGER  DiskSize;  
		UCHAR  Data[1];
	} DiskEX;
	DWORD bRet;
	DWORD bytesReturned;
	wchar_t temp2[20];
	
	DWORD totalSector = 0;	
	bRet = DeviceIoControl(
		  hDisk,              // handle to device
		  IOCTL_DISK_GET_DRIVE_GEOMETRY_EX,    // dwIoControlCode
		  NULL,                          // lpInBuffer
		  0,                             // nInBufferSize
		  (LPVOID) &DiskEX,          // output buffer
		  sizeof(DiskEX),        // size of output buffer
		  (LPDWORD) &bytesReturned ,     // number of bytes returned
		  NULL    // OVERLAPPED structure
	);

	if (bRet == 0)
	{	
		return 0;
	}

	if ( DiskEX.DiskSize.HighPart != 0 )
	{
		totalSector = ((DiskEX.DiskSize.HighPart) << 23);
	}
	totalSector |= (DiskEX.DiskSize.LowPart >> 9);
	return totalSector;
}

BOOL
GetDiskGeometry(
    HANDLE hDisk,
    PDISK_GEOMETRY lpGeometry
    )

{
    DWORD ReturnedByteCount;

    return DeviceIoControl(
                hDisk,
                IOCTL_DISK_GET_DRIVE_GEOMETRY,
                NULL,
                0,
                lpGeometry,
                sizeof(*lpGeometry),
                &ReturnedByteCount,
                NULL
                );
}

DWORD
GetSupportedGeometrys(
    HANDLE hDisk
    )
{
    DWORD ReturnedByteCount;
    BOOL b;
    DWORD NumberSupported;

    b = DeviceIoControl(
                hDisk,
                IOCTL_DISK_GET_MEDIA_TYPES,
                NULL,
                0,
                SupportedGeometry,
                sizeof(SupportedGeometry),
                &ReturnedByteCount,
                NULL
                );
    if ( b ) {
        NumberSupported = ReturnedByteCount / sizeof(DISK_GEOMETRY);
        }
    else {
        NumberSupported = 0;
        }
    SupportedGeometryCount = NumberSupported;

    return NumberSupported;
}

VOID
PrintGeometry(
    LPSTR lpDriveName,
    PDISK_GEOMETRY lpGeometry
    )
{
    LPSTR MediaType;

    if (lpDriveName) {
        printf("Geometry for Drive %s\n",lpDriveName);
        }

    switch ( lpGeometry->MediaType ) {
        case F5_1Pt2_512:  MediaType = "5.25, 1.2MB,  512 bytes/sector";break;
        case F3_1Pt44_512: MediaType = "3.5,  1.44MB, 512 bytes/sector";break;
        case F3_2Pt88_512: MediaType = "3.5,  2.88MB, 512 bytes/sector";break;
        case F3_20Pt8_512: MediaType = "3.5,  20.8MB, 512 bytes/sector";break;
        case F3_720_512:   MediaType = "3.5,  720KB,  512 bytes/sector";break;
        case F5_360_512:   MediaType = "5.25, 360KB,  512 bytes/sector";break;
        case F5_320_512:   MediaType = "5.25, 320KB,  512 bytes/sector";break;
        case F5_320_1024:  MediaType = "5.25, 320KB,  1024 bytes/sector";break;
        case F5_180_512:   MediaType = "5.25, 180KB,  512 bytes/sector";break;
        case F5_160_512:   MediaType = "5.25, 160KB,  512 bytes/sector";break;
        case RemovableMedia: MediaType = "Removable media other than floppy";break;
        case FixedMedia:   MediaType = "Fixed hard disk media";break;
        default:           MediaType = "Unknown";break;
    }
    printf("    Media Type %s\n",MediaType);
    printf("    Cylinders %d Tracks/Cylinder %d Sectors/Track %d\n",
        lpGeometry->Cylinders.LowPart,
        lpGeometry->TracksPerCylinder,
        lpGeometry->SectorsPerTrack
        );
	
	ULONGLONG  val = lpGeometry->Cylinders.LowPart*(ULONG)lpGeometry->TracksPerCylinder*(ULONG)lpGeometry->SectorsPerTrack;
	CString str;
	str.Format(_T("%I64d, %d, %d"), lpGeometry->Cylinders, lpGeometry->TracksPerCylinder, lpGeometry->SectorsPerTrack);
	AfxMessageBox(str, MB_OK | MB_ICONERROR);
}

BOOL
LowLevelFormat(
    HANDLE hDisk,
    PDISK_GEOMETRY lpGeometry
    )
{
    FORMAT_PARAMETERS FormatParameters;
    PBAD_TRACK_NUMBER lpBadTrack;
    UINT i;
    BOOL b;
    DWORD ReturnedByteCount;

    FormatParameters.MediaType = lpGeometry->MediaType;
    FormatParameters.StartHeadNumber = 0;
    FormatParameters.EndHeadNumber = lpGeometry->TracksPerCylinder - 1;
    lpBadTrack = (PBAD_TRACK_NUMBER) LocalAlloc(LMEM_ZEROINIT,lpGeometry->TracksPerCylinder*sizeof(*lpBadTrack));

    for (i = 0; i < lpGeometry->Cylinders.LowPart; i++) {

        FormatParameters.StartCylinderNumber = i;
        FormatParameters.EndCylinderNumber = i;

        b = DeviceIoControl(
                hDisk,
                IOCTL_DISK_FORMAT_TRACKS,
                &FormatParameters,
                sizeof(FormatParameters),
                lpBadTrack,
                lpGeometry->TracksPerCylinder*sizeof(*lpBadTrack),
                &ReturnedByteCount,
                NULL
                );

        if (!b ) {
            LocalFree(lpBadTrack);
            return b;
            }
        }

    LocalFree(lpBadTrack);

    return TRUE;
}

BOOL
LockVolume(
    HANDLE hDisk
    )
{
    DWORD ReturnedByteCount;

    return DeviceIoControl(
                hDisk,
                FSCTL_LOCK_VOLUME,
                NULL,
                0,
                NULL,
                0,
                &ReturnedByteCount,
                NULL
                );
}

BOOL
UnlockVolume(
    HANDLE hDisk
    )
{
    DWORD ReturnedByteCount;

    return DeviceIoControl(
                hDisk,
                FSCTL_UNLOCK_VOLUME,
                NULL,
                0,
                NULL,
                0,
                &ReturnedByteCount,
                NULL
                );
}

BOOL
DismountVolume(
    HANDLE hDisk
    )
{
    DWORD ReturnedByteCount;

    return DeviceIoControl(
                hDisk,
                FSCTL_DISMOUNT_VOLUME,
                NULL,
                0,
                NULL,
                0,
                &ReturnedByteCount,
                NULL
                );
}



CFile m_CFSrcFile;
HANDLE m_HdSDCard;


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

														// ʵ��
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


// CSDCardBurnDlg �Ի���



CSDCardBurnDlg::CSDCardBurnDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SDCARDBURN_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSDCardBurnDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_CBDisk);
	DDX_Control(pDX, IDC_EDIT_PATH, m_CEFilePath);
	DDX_Control(pDX, IDC_BUTTON_SELFILE, m_BuSelFile);
	DDX_Control(pDX, IDC_BUTTON_BURN, m_BuBurn);
	DDX_Control(pDX, IDC_PROGRESS, m_CProgress);
	DDX_Control(pDX, IDC_STATIC_STATE, m_CSState);
}

BEGIN_MESSAGE_MAP(CSDCardBurnDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_SELFILE, &CSDCardBurnDlg::OnBnClickedButtonSelfile)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON_BURN, &CSDCardBurnDlg::OnBnClickedButtonBurn)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CSDCardBurnDlg ��Ϣ�������

BOOL CSDCardBurnDlg::OnInitDialog()
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

									// TODO: �ڴ���Ӷ���ĳ�ʼ������

									//���ڱ���
	this->SetWindowTextW(L"S3C2416 SD����д����V0.1");

	WCHAR	curDire[MAX_PATH];
	CString strPath;
	GetCurrentDirectory(MAX_PATH, curDire);
	strPath.Format(L"%s", curDire);
	//strPath = strPath.Left(strPath.ReverseFind(L'\\'));
	strPath += L"\\u-boot_movi.bin";
	//strPath.Format(L"%s\\ACC3.bin", curDire);
	m_CEFilePath.SetWindowTextW(strPath);
	
	m_curDir.Format(L"%s", curDire); 
	m_ImgSrcDir.Format(L"%s", curDire); 
	m_ImgSrcDir += L"\\images";
	//pThWrite = (WriteSDCardThread *)AfxBeginThread(RUNTIME_CLASS(WriteSDCardThread), 0, 1024 * 1024 * 1, CREATE_SUSPENDED);
	ScanAllDisk(2);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CSDCardBurnDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSDCardBurnDlg::OnPaint()
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
HCURSOR CSDCardBurnDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

UINT CSDCardBurnDlg::MainRun(LPVOID lParam)
{


	return 0;
}
#define _CRT_SECURE_NO_WARNINGS
int CSDCardBurnDlg::CopyFolder(LPCTSTR lpszFromPath,LPCTSTR lpszToPath)
{
	//AfxMessageBox(L"1", MB_OK | MB_ICONERROR);
	//return CopyFile(lpszFromPath, lpszToPath, TRUE);
	size_t nLengthFrm = _tcslen(lpszFromPath);
    TCHAR *NewPathFrm = new TCHAR[nLengthFrm+2];
	//AfxMessageBox(L"2", MB_OK | MB_ICONERROR);
    //_tcscpy_s(NewPathFrm, nLengthFrm, lpszFromPath);
	_tcscpy(NewPathFrm,lpszFromPath);
	//AfxMessageBox(L"3", MB_OK | MB_ICONERROR);
    NewPathFrm[nLengthFrm] = '\0';
    NewPathFrm[nLengthFrm+1] = '\0';
	//AfxMessageBox(L"4", MB_OK | MB_ICONERROR);
    SHFILEOPSTRUCT FileOp;
    ZeroMemory((void*)&FileOp,sizeof(SHFILEOPSTRUCT));
    FileOp.fFlags = FOF_NOCONFIRMATION|FOF_NOCONFIRMMKDIR|FOF_NOERRORUI|FOF_FILESONLY|FOF_NOCOPYSECURITYATTRIBS ;
    FileOp.hNameMappings = NULL;
    FileOp.hwnd = NULL;
    FileOp.lpszProgressTitle = NULL;
    FileOp.pFrom = NewPathFrm;
    FileOp.pTo = lpszToPath;
    FileOp.wFunc = FO_COPY;
	//AfxMessageBox(L"5", MB_OK | MB_ICONERROR);
	return SHFileOperation(&FileOp);
}




#define RDBUFF_SIZE	(1024*1024*2)
CHAR	rdBuffer[RDBUFF_SIZE], verBuffer[RDBUFF_SIZE];

UINT CSDCardBurnDlg::WriteData(LPVOID lParam)
{
	BOOL	bRtn;
	INT		verDiff;
	DWORD	outSize;
	UINT	rdLen;
	ULONGLONG	fsSize, dealSize;
	LARGE_INTEGER 	sdPoint;
	LARGE_INTEGER 	sd2g;
	
	UINT	progValue;
	CString staticStr;
	
	CSDCardBurnDlg *myDlg = (CSDCardBurnDlg *)lParam;
	CProgressCtrl *progress = &myDlg->m_CProgress;
	CStatic *myStatic = &myDlg->m_CSState;

	CButton *myBuFilePath = &myDlg->m_BuSelFile;
	CButton *myBuBurn = &myDlg->m_BuBurn;
	CEdit *myEditPath = &myDlg->m_CEFilePath;

	myEditPath->EnableWindow(false);
	myBuBurn->EnableWindow(false);
	myBuFilePath->EnableWindow(false);
	CString str1;
	int ss = GetTotalSector(m_HdSDCard);
	str1.Format(_T("%d"), ss);
	//AfxMessageBox(str1, MB_OK | MB_ICONERROR);
	
	
	GET_LENGTH_INFORMATION infoStruct;	
	DWORD ReturnedByteCount;
	memset(&infoStruct,0,sizeof(infoStruct));
	DeviceIoControl(m_HdSDCard,IOCTL_DISK_GET_LENGTH_INFO,NULL,0,&infoStruct,sizeof(infoStruct),&ReturnedByteCount,NULL);
	//str1.Format(_T("%I64d"), infoStruct.Length);
	//AfxMessageBox(str1, MB_OK | MB_ICONERROR);
	
	PARTITION_INFORMATION pinfo;
    DeviceIoControl(
        (HANDLE) m_HdSDCard,               
        IOCTL_DISK_GET_PARTITION_INFO, 
        NULL,                            
        0,                              
        (LPVOID) &pinfo,            
        (DWORD) sizeof(pinfo),        
        (LPDWORD) &ReturnedByteCount,       
        (LPOVERLAPPED) NULL     
        );
	

	int ss_reserve = pinfo.StartingOffset.QuadPart / 512;
	
	str1.Format(_T("%d"), ss_reserve);
	//AfxMessageBox(str1, MB_OK | MB_ICONERROR);

	//ɾ������������Ϣ(����0��һ��ƫ����Ϣ�������������ᵼ��д��ĵ�ַ����Ҫ�Ĳ�һ��)
	/*bRtn = DeviceIoControl(m_HdSDCard, IOCTL_DISK_DELETE_DRIVE_LAYOUT, NULL, 0, NULL, 0, &outSize, NULL);
	if (bRtn == FALSE)
	{
		myDlg->ViewLastError(L"ɾ��������Ϣʧ��:");
		goto __LAB_ERROR;
	}
	//�������0
	memset(rdBuffer, 0, 512);
	SetFilePointer(m_HdSDCard, 0, 0, FILE_BEGIN);
	bRtn = WriteFile(m_HdSDCard, rdBuffer, 512, &outSize, NULL);
	if (bRtn == FALSE)
	{
		myDlg->ViewLastError(L"�������0ʧ��:");
		goto __LAB_ERROR;
	}*/
	//����SD����ƫ�Ƶ�
	//sdPoint = 1024 * 1024 * 1;

	//��Ϊ��ʼдλ�ö����ss_reserve������
	sdPoint.QuadPart = ss-ss_reserve;
	sd2g.QuadPart = 2147483648;
	if(infoStruct.Length.QuadPart > sd2g.QuadPart)
	{
		//sdPoint.QuadPart = infoStruct.Length.QuadPart-1024 * 513 - 520*1024;
		
		sdPoint.QuadPart = sdPoint.QuadPart*512 -1024 * 513 - 520*1024;
	}
	else
	{
		//sdPoint.QuadPart = infoStruct.Length.QuadPart-1024 * 1 - 520*1024;
		sdPoint.QuadPart = sdPoint.QuadPart*512-1024 * 513 - 520*1024;
	}

	str1.Format(_T("%I64d"), sdPoint);
	//AfxMessageBox(str1, MB_OK | MB_ICONERROR);
	//Դ�ļ�����
	fsSize = m_CFSrcFile.GetLength();
	dealSize = 0;
	//��ʼд�ļ�
	
	LockVolume(m_HdSDCard);
	while (dealSize < fsSize)
	{
		rdLen = m_CFSrcFile.Read(rdBuffer, RDBUFF_SIZE);
		//memset(rdBuffer, 0, RDBUFF_SIZE);
		if (rdLen)
		{
			//д��SD��
			SetFilePointer(m_HdSDCard, sdPoint.LowPart, &sdPoint.HighPart, FILE_BEGIN);
			bRtn = WriteFile(m_HdSDCard, rdBuffer, rdLen, &outSize, NULL);
			if (bRtn == FALSE)
			{
				myDlg->ViewLastError(L"д��SD��ʧ��:");
				goto __LAB_ERROR;
			}
			//��SD������
			SetFilePointer(m_HdSDCard, sdPoint.LowPart, &sdPoint.HighPart, FILE_BEGIN);
			bRtn = ReadFile(m_HdSDCard, verBuffer, rdLen, &outSize, NULL);
			if (bRtn == FALSE)
			{
				myDlg->ViewLastError(L"��SD����������ʧ��:");
				goto __LAB_ERROR;
			}
			//У��
			verDiff = memcmp(rdBuffer, verBuffer, rdLen);
			if (verDiff != 0)
			{
				AfxMessageBox(L"У�����ݳ���", MB_OK | MB_ICONERROR);
				goto __LAB_ERROR;
			}
			sdPoint.QuadPart += rdLen;
			dealSize += rdLen;

			progValue = (INT)(((float)dealSize / (float)fsSize) * 100);
			progress->SetPos(progValue);
			staticStr.Format(L"%u%%", progValue);
			myStatic->SetWindowTextW(staticStr);
		}
		//Sleep(10);
	}
	UnlockVolume(m_HdSDCard);
	progress->SetPos(100);	
	//AfxMessageBox(myDlg->m_ImgSrcDir, MB_OK | MB_ICONMASK);
	//AfxMessageBox(myDlg->m_ImgDstDir, MB_OK | MB_ICONMASK);
	int ret = myDlg->CopyFolder((LPCTSTR)myDlg->m_ImgSrcDir, (LPCTSTR)myDlg->m_ImgDstDir);
	if (ret)
	{
		myDlg->ViewLastError(L"�����ļ�ʧ��:");
	}
	//AfxMessageBox(L"д���ݳɹ�", MB_OK | MB_ICONMASK);

__LAB_ERROR:
	myEditPath->EnableWindow(true);
	myBuBurn->EnableWindow(true);
	myBuFilePath->EnableWindow(true);
	CloseHandle(m_HdSDCard);
	m_CFSrcFile.Close();
	AfxEndThread(0);
	return 0;
}



void CSDCardBurnDlg::OnBnClickedButtonSelfile()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������

	CString filePath;
	BOOL bRes;
	WCHAR	curDire[MAX_PATH];
	CFileDialog dlgFile(true, NULL, L"", OFN_HIDEREADONLY, L"bin�ļ�(*.bin)|*.bin|", NULL);
	OPENFILENAME& ofn = dlgFile.GetOFN();
	GetCurrentDirectory(MAX_PATH, curDire);

	ofn.lpstrInitialDir = curDire;
	dlgFile.ApplyOFNToShellDialog();
	if (dlgFile.DoModal() == IDOK)
	{
		filePath = dlgFile.GetPathName();
		m_CEFilePath.SetWindowTextW(filePath);
	}
	else
	{
		//MessageBox(L"ѡ���ļ�����", L"����", MB_OK | MB_ICONERROR);
	}
}


void CSDCardBurnDlg::OnClose()
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ

	CDialogEx::OnClose();
}




void CSDCardBurnDlg::OnBnClickedButtonBurn()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	WCHAR textBuff[1024];
	CString str;
	BOOL bRtn;

	//��ȡԴ�ļ�·��
	m_CEFilePath.GetWindowTextW(textBuff, m_CEFilePath.GetWindowTextLengthW() + 1);
	str = textBuff;
	if (str.IsEmpty())
	{
		MessageBox(L"·������Ϊ��", L"����", MB_OK | MB_ICONERROR);
		return;
	}
	//��Դ�ļ�
	bRtn = m_CFSrcFile.Open(str, CFile::modeRead | CFile::typeBinary | CFile::shareDenyRead);
	if (!bRtn)
	{
		MessageBox(L"��Դ�ļ�ʧ��", L"����", MB_OK | MB_ICONERROR);
		return;
	}

	//�������̾��
	//m_CBDisk.GetWindowTextW(str);
	if (m_CBDisk.GetCount() == 0)
	{
		m_CFSrcFile.Close();
		MessageBox(L"��ѡ�����", L"����", MB_OK | MB_ICONWARNING);
		return;
	}
	m_CBDisk.GetLBText(m_CBDisk.GetCurSel(), str);
	
	if (str.IsEmpty())
	{
		MessageBox(L"��ѡ�����", L"����", MB_OK | MB_ICONWARNING);
		m_CFSrcFile.Close();
		return;
	}
	
	m_ImgDstDir = str;
	m_ImgDstDir += L"\\";
	
	str = L"\\\\.\\" + str;
	m_HdSDCard = CreateFile(str, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS|FILE_FLAG_NO_BUFFERING, NULL);
	if (m_HdSDCard == INVALID_HANDLE_VALUE)
	{
		m_CFSrcFile.Close();
		AfxMessageBox(L"��SD������", MB_OK | MB_ICONERROR);
		return;
	}
	
	/*DISK_GEOMETRY_EX geoStruct;
	DWORD ReturnedByteCount;
	memset(&geoStruct,0,sizeof(geoStruct));
	DeviceIoControl(m_HdSDCard,IOCTL_DISK_GET_DRIVE_GEOMETRY_EX ,NULL,0,&geoStruct,sizeof(geoStruct),&ReturnedByteCount,NULL);
	CString str1;
	str1.Format(_T("%I64d"), geoStruct.DiskSize);
	AfxMessageBox(str1, MB_OK | MB_ICONERROR);
	
	GET_LENGTH_INFORMATION infoStruct;

	memset(&infoStruct,0,sizeof(infoStruct));
	DeviceIoControl(m_HdSDCard,IOCTL_DISK_GET_LENGTH_INFO,NULL,0,&infoStruct,sizeof(infoStruct),&ReturnedByteCount,NULL);
	str1.Format(_T("%I64d"), infoStruct.Length);
	AfxMessageBox(str1, MB_OK | MB_ICONERROR);
	
	DISK_GEOMETRY geo;
	GetDiskGeometry(m_HdSDCard, &geo);
	PrintGeometry(NULL, &geo);
	//AfxMessageBox(str, MB_OK | MB_ICONERROR);*/

	//m_TimerID = SetTimer(0, 10, 0);
	m_CSState.SetWindowTextW(L"0%");
	m_CProgress.SetPos(0);
	m_TFunWriteData = AfxBeginThread((AFX_THREADPROC)WriteData, this, THREAD_PRIORITY_NORMAL, 1024 * 1024 * 1, CREATE_SUSPENDED, NULL);
	m_TFunWriteData->ResumeThread();
}

void CSDCardBurnDlg::ShowMessageBoxTimeout()
{
    //you must load user32.dll before calling the function
    HMODULE hUser32 = LoadLibrary(_T("user32.dll"));

    if (hUser32)
    {
        int iRet = 0;
        UINT uiFlags = MB_OK | MB_SETFOREGROUND | MB_SYSTEMMODAL | MB_ICONINFORMATION;
		CString str;
		
		m_CBDisk.GetLBText(m_CBDisk.GetCurSel(), str);
		str += _T("�̲���");

        /*iRet = MessageBoxTimeout(NULL, _T("Test a timeout of 2 seconds."),
            _T("MessageBoxTimeout Test"), uiFlags, 0, 2000);
        //iRet will = 1*/

        uiFlags = MB_YESNO | MB_SETFOREGROUND | MB_SYSTEMMODAL | MB_ICONINFORMATION;

        iRet = MessageBoxTimeout(NULL, _T("5����Զ���ʼ��д��������"), str, uiFlags, 0, 5000);
            
        //iRet will = MB_TIMEDOUT if no buttons pressed, button values otherwise
		
		if((iRet == IDYES))
		{
			//AfxMessageBox(L"yes pressed", MB_OK | MB_ICONERROR);
			OnBnClickedButtonBurn();
		}
		else if((iRet == MB_TIMEDOUT))
		{
			//AfxMessageBox(L"timeout occured", MB_OK | MB_ICONERROR);
			OnBnClickedButtonBurn();
		}

        //only unload user32.dll when you have no further need 
        //for the MessageBoxTimeout function
        FreeLibrary(hUser32);
    }
}

void CSDCardBurnDlg::ScanAllDisk(UINT flag)
{
	TCHAR szBuf[100];
	DWORD len, i;
	UINT type;
	CString str;
	memset(szBuf, 0, 100);

	len = GetLogicalDriveStrings(sizeof(szBuf) / sizeof(TCHAR), szBuf);
	m_CBDisk.ResetContent();

	for (i = 0; i<len; i++)
	{
		str = &szBuf[i];
		type = GetDriveType(str);
		if (type == DRIVE_REMOVABLE)
		{
			str.Delete(2, 1);
			m_CBDisk.AddString(str);
		}
	}
	i = m_CBDisk.GetCount();
	if (i > 0)
	{
		m_CBDisk.SetCurSel(i - 1);
	}
	else
	{
		m_CBDisk.SetCurSel(0);
	}
	
	if(flag == 1)
	{
		ShowMessageBoxTimeout();
	}
}

void CSDCardBurnDlg::ViewLastError(LPTSTR lpMsg)
{

	LPVOID lpMsgBuf;
	DWORD dw = GetLastError();//��ȡ�������

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	CString strErr;
	strErr.Format(L"%s%s", lpMsg, lpMsgBuf);
	MessageBox(strErr, L"����", MB_OK | MB_ICONERROR);
	LocalFree(lpMsgBuf);
}




LRESULT CSDCardBurnDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		//WM_DEVICECHANGE��ϵͳӲ���ı䷢����ϵͳ��Ϣ
	case WM_DEVICECHANGE:
	{
		PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
		switch (wParam)
		{
		case WM_DEVICECHANGE:
			break;
		case DBT_DEVICEARRIVAL://DBT_DEVICEARRIVAL���豸�����������ҿ���ʹ��
		{
			if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)//�߼���
			{
				PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
				switch (lpdbv->dbcv_flags)
				{
				case 0:                //U��
				{
					ScanAllDisk(1);
				}
				break;
				case DBTF_MEDIA:    //����
					break;
				}
			}
		}
		break;
		case DBT_DEVICEREMOVECOMPLETE://DBT_DEVICEREMOVECOMPLETE,�豸ж�ػ��߰γ�
		{
			if (lpdb->dbch_devicetype == DBT_DEVTYP_VOLUME)//�߼���
			{
				PDEV_BROADCAST_VOLUME lpdbv = (PDEV_BROADCAST_VOLUME)lpdb;
				switch (lpdbv->dbcv_flags)
				{
				case 0:                //U��
				{
					ScanAllDisk(0);
				}
				break;
				case DBTF_MEDIA:    //����

					break;
				}
			}
		}
		break;
		}
	}
	break;
	}
	return CDialog::WindowProc(message, wParam, lParam);
}


void CSDCardBurnDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ


	CDialogEx::OnTimer(nIDEvent);
}
