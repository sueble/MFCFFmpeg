
// MFCFFmpegDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFCFFmpeg.h"
#include "MFCFFmpegDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define __STDC_CONSTANT_MACROS
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
}


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
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


// CMFCFFmpegDlg dialog



CMFCFFmpegDlg::CMFCFFmpegDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_MFCFFMPEG_DIALOG, pParent)
	, mVideoPath(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMFCFFmpegDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDITVideoPath, mVideoPath);
}

BEGIN_MESSAGE_MAP(CMFCFFmpegDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CMFCFFmpegDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CMFCFFmpegDlg message handlers

BOOL CMFCFFmpegDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMFCFFmpegDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMFCFFmpegDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMFCFFmpegDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void Play_H264_File(LPVOID lpParam)
{
	AVFormatContext	*pFormatCtx;   //����������
	AVCodecContext	*pCodecCtx;    //������������
	AVCodec			*pCodec;       //������
	AVFrame			*pFrame;       //���������ݽṹ��
	AVFrame			*pFrameYUV;    //�����������ٴ���Ľṹ��
	AVPacket		*packet;       //����ǰ�����ݽṹ��
	uint8_t			*out_buffer;   //���ݻ���
	int				 v_index;      //��Ƶ���Ĺ���±�
	int				 v_size;       //һ֡���ݵĴ�С
	CMFCFFmpegDlg *dlg;   //
	int				 dely_time;    //��Ҫ��ͣ�ĺ�����
								   ///////////////////////����ô���ͣ���ϴ����ٿ�/////////////////////////////
								   //�ȰѸոմ��ݽ�����thisָ�룬ת���ɿɵ��õĶ���
	dlg = (CMFCFFmpegDlg *)lpParam;
	//��ȡָ������Ƶ·��
	char filepath[250] = { 0 };
	GetWindowTextA(dlg->mVideoEdit, (LPSTR)filepath, 250); //mVideoEdit���ļ�·������Edit Control�ؼ�������Control�����Ķ���
	pFormatCtx = avformat_alloc_context(); //��ȡ����������
										   //���������Ĺ����ļ�
	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0) {
		cout << "��Ƶ�ļ���ʧ��" << endl;
		return;
	}
	//���ļ����������
	if (avformat_find_stream_info(pFormatCtx, NULL)<0) {
		cout << "��Ƶ�ļ����ɶ�" << endl;
		return;
	}
	//��ӡ
	av_dump_format(pFormatCtx, -1, filepath, NULL);
	//Ѱ����Ƶ֡���±꣨��Ƶ�ļ�������Ƶ����Ƶ����Ļ�ȹ����
	v_index = -1;
	for (int i = 0; i<pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			v_index = i;
			break;
		}
	}
	//�ж��Ƿ������Ƶ֡
	if (v_index < 0) {
		cout << "Ŀ�겻����Ƶ�ļ�" << endl;
		return;
	}
	//������Ҫdelay�ĺ�����
	int fps = pFormatCtx->streams[v_index]->avg_frame_rate.num / pFormatCtx->streams[v_index]->avg_frame_rate.den;//ÿ�����֡
	dely_time = 1000 / fps;
	//��ȡ�����������Ķ���
	pCodecCtx = avcodec_alloc_context3(NULL);
	//���ݽ��������ĳ�ʼ��������������
	if (avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[v_index]->codecpar) < 0)
	{
		cout << "��������������ʧ��" << endl;
		return;
	}
	//��ȡ����������
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	//�򿪽�����
	if (avcodec_open2(pCodecCtx, pCodec, NULL) != 0) {
		cout << "��������ʧ��" << endl;
		return;
	}
	//ȷ�������ĺͽ�������û������������������Ҫ�Ľṹ��ռ�
	pFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();
	packet = av_packet_alloc();
	//����YUV���ݸ�ʽ����������ͼƬ�Ĵ�С
	v_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
	//���뻺�����
	out_buffer = (uint8_t *)av_malloc(v_size);
	//���Զ��建��ռ�󶨵������AVFrame��
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
	while (av_read_frame(pFormatCtx, packet) >= 0) { //��ȡһ��֡����
		if (packet->stream_index == v_index) {  //�ж��Ƿ�����Ƶ֡
			if (avcodec_send_packet(pCodecCtx, packet) != 0) {  //�������ݽ��н���
				cout << "���ͽ������ݳ���" << endl;
				return;
			}
			if (avcodec_receive_frame(pCodecCtx, pFrame) != 0)
			{
				cout << "���ܽ������ݳ�������ʱ��������";
				return;
			}
			//������ɣ�pFrameΪ����������
		}
	}
}
UINT Thread_Play(LPVOID lpParam) {
	Play_H264_File(lpParam);
	//cout << "�����ļ��߳̽���" << endl;
	return 0;
}
void CMFCFFmpegDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (mVideoPath.IsEmpty())
	{
		MessageBox(TEXT("��ѡ����Ƶ�ļ�·��"));
		return;
	}
	AfxBeginThread(Thread_Play, this);
	//CDialogEx::OnOK();
}

