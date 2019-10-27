
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
	AVFormatContext	*pFormatCtx;   //解码上下文
	AVCodecContext	*pCodecCtx;    //解码器上下文
	AVCodec			*pCodec;       //加码器
	AVFrame			*pFrame;       //解码后的数据结构体
	AVFrame			*pFrameYUV;    //解码后的数据再处理的结构体
	AVPacket		*packet;       //解码前的数据结构体
	uint8_t			*out_buffer;   //数据缓存
	int				 v_index;      //视频流的轨道下标
	int				 v_size;       //一帧数据的大小
	CMFCFFmpegDlg *dlg;   //
	int				 dely_time;    //需要暂停的毫秒数
								   ///////////////////////先这么解释，结合代码再看/////////////////////////////
								   //先把刚刚传递进来的this指针，转换成可调用的对象
	dlg = (CMFCFFmpegDlg *)lpParam;
	//获取指定的视频路径
	char filepath[250] = { 0 };
	GetWindowTextA(dlg->mVideoEdit, (LPSTR)filepath, 250); //mVideoEdit是文件路径对象Edit Control控件关联的Control关联的对象
	pFormatCtx = avformat_alloc_context(); //获取解码上下文
										   //解码上下文关联文件
	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0) {
		cout << "视频文件打开失败" << endl;
		return;
	}
	//打开文件输入输出流
	if (avformat_find_stream_info(pFormatCtx, NULL)<0) {
		cout << "视频文件不可读" << endl;
		return;
	}
	//打印
	av_dump_format(pFormatCtx, -1, filepath, NULL);
	//寻找视频帧的下标（视频文件存在视频、音频、字幕等轨道）
	v_index = -1;
	for (int i = 0; i<pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			v_index = i;
			break;
		}
	}
	//判断是否存在视频帧
	if (v_index < 0) {
		cout << "目标不是视频文件" << endl;
		return;
	}
	//计算需要delay的毫秒数
	int fps = pFormatCtx->streams[v_index]->avg_frame_rate.num / pFormatCtx->streams[v_index]->avg_frame_rate.den;//每秒多少帧
	dely_time = 1000 / fps;
	//获取解码器上下文对象
	pCodecCtx = avcodec_alloc_context3(NULL);
	//根据解码上下文初始化解码器上下文
	if (avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[v_index]->codecpar) < 0)
	{
		cout << "拷贝解码器数据失败" << endl;
		return;
	}
	//获取解码器对象
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	//打开解码器
	if (avcodec_open2(pCodecCtx, pCodec, NULL) != 0) {
		cout << "解码器打开失败" << endl;
		return;
	}
	//确认上下文和解码器都没有问题后，申请解码所需要的结构体空间
	pFrame = av_frame_alloc();
	pFrameYUV = av_frame_alloc();
	packet = av_packet_alloc();
	//根据YUV数据格式，计算解码后图片的大小
	v_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
	//申请缓存对象
	out_buffer = (uint8_t *)av_malloc(v_size);
	//将自定义缓存空间绑定到输出的AVFrame中
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
	while (av_read_frame(pFormatCtx, packet) >= 0) { //读取一个帧数据
		if (packet->stream_index == v_index) {  //判断是否是视频帧
			if (avcodec_send_packet(pCodecCtx, packet) != 0) {  //发送数据进行解码
				cout << "发送解码数据出错" << endl;
				return;
			}
			if (avcodec_receive_frame(pCodecCtx, pFrame) != 0)
			{
				cout << "接受解码数据出错，解码时发生错误";
				return;
			}
			//解码完成，pFrame为解码后的数据
		}
	}
}
UINT Thread_Play(LPVOID lpParam) {
	Play_H264_File(lpParam);
	//cout << "播放文件线程结束" << endl;
	return 0;
}
void CMFCFFmpegDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	if (mVideoPath.IsEmpty())
	{
		MessageBox(TEXT("请选择视频文件路径"));
		return;
	}
	AfxBeginThread(Thread_Play, this);
	//CDialogEx::OnOK();
}

