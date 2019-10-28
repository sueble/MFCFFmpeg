
// MFCFFmpegDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MFCFFmpeg.h"
#include "MFCFFmpegDlg.h"
#include "afxdialogex.h"
#include"iostream"
using namespace std;

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

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>

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
	mVideoPath = "C:\\Users\\Administrator\\Videos\\1.mp4";
}

void CMFCFFmpegDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDITVideoPath, mVideoPath);
	DDX_Control(pDX, IDC_EDITVideoPath, mVideoEdit);
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


void Play_H264_File(LPVOID lpParam) {

	////////////////////FFmpeg/////////////////////
	AVFormatContext	*pFormatCtx;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;
	AVFrame			*pFrame;
	AVFrame			*pFrameYUV;
	AVPacket		*packet;
	uint8_t			*out_buffer;
	int				 v_index;
	int				 v_size;
	CMFCFFmpegDlg *dlg;
	int				 dely_time;
	///////////////////////////SDL////////////////////////
	SDL_Window         *mSDL_Window;
	SDL_Renderer       *mSDL_Renderer;
	SDL_Texture        *mSDL_Texture;
	struct SwsContext  *mSwsContext;
	int screenW;
	int screenH;
	SDL_Rect mSDL_Rect;
	dlg = (CMFCFFmpegDlg *)lpParam;
	///////////////////////////SDL////////////////////////
	if (SDL_Init(SDL_INIT_VIDEO)) {
		cout << "SDL初始化失败" << endl;
		return;
	}
	/////////////////////////FFmpeg///////////////////////////
	char filepath[250] = { 0 };
	GetWindowTextA(dlg->mVideoEdit, (LPSTR)filepath, 250);
	pFormatCtx = avformat_alloc_context();
	if (avformat_open_input(&pFormatCtx, filepath, NULL, NULL) != 0) {
		cout << "视频文件打开失败" << endl;
		return;
	}
	if (avformat_find_stream_info(pFormatCtx, NULL)<0) {
		cout << "视频文件不可读" << endl;
		return;
	}
	av_dump_format(pFormatCtx, -1, filepath, NULL);
	v_index = -1;

	for (int i = 0; i<pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
			v_index = i;
			break;
		}
	}
	if (v_index < 0) {
		cout << "目标不是视频文件" << endl;
		return;
	}
	int fps = pFormatCtx->streams[v_index]->avg_frame_rate.num / pFormatCtx->streams[v_index]->avg_frame_rate.den;//每秒多少帧
	dely_time = 1000 / fps;
	cout << "视频FPS = " << fps << endl;
	cout << "dely_time = " << dely_time << endl;
	pCodecCtx = avcodec_alloc_context3(NULL);
	//pCodecCtx = pFormatCtx->streams[videoindex]->codec;

	if (avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[v_index]->codecpar) < 0)
	{
		cout << "拷贝解码器失败" << endl;
		return;
	}
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (avcodec_open2(pCodecCtx, pCodec, NULL) != 0) {
		cout << "解码器打开失败" << endl;
		return;
	}
	//sw = SDL_CreateWindow("video", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 680, 540, SDL_WINDOW_OPENGL);

	mSDL_Rect.w = pCodecCtx->width;
	mSDL_Rect.h = pCodecCtx->height;
	mSDL_Window = SDL_CreateWindowFrom(dlg->GetDlgItem(IDC_VIDEO_SURFACE)->GetSafeHwnd());
	mSDL_Renderer = SDL_CreateRenderer(mSDL_Window, -1, 0);
	mSDL_Texture = SDL_CreateTexture(mSDL_Renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);
	mSwsContext = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,

		SWS_BICUBIC, NULL, NULL, NULL);



	pFrame = av_frame_alloc();

	pFrameYUV = av_frame_alloc();

	packet = av_packet_alloc();



	v_size = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);



	out_buffer = (uint8_t *)av_malloc(v_size);



	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);



	while (av_read_frame(pFormatCtx, packet) >= 0) {

		if (packet->stream_index == v_index) {



			if (avcodec_send_packet(pCodecCtx, packet) != 0) {

				cout << "发送解码数据出错" << endl;

				return;

			}

			if (avcodec_receive_frame(pCodecCtx, pFrame) != 0)

			{

				cout << "接受解码数据出错";

				return;

			}

			sws_scale(mSwsContext, pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);



			SDL_UpdateTexture(mSDL_Texture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);

			SDL_RenderClear(mSDL_Renderer);

			SDL_RenderCopy(mSDL_Renderer, mSDL_Texture, NULL, NULL);

			SDL_RenderPresent(mSDL_Renderer);

			Sleep(dely_time);

		}



	}



	av_free(out_buffer);

	av_frame_free(&pFrameYUV);

	av_frame_free(&pFrame);

	av_packet_free(&packet);

	sws_freeContext(mSwsContext);

	SDL_DestroyTexture(mSDL_Texture);

	SDL_DestroyRenderer(mSDL_Renderer);

	SDL_DestroyWindow(mSDL_Window);

	SDL_Quit();

	avcodec_free_context(&pCodecCtx);

	avformat_close_input(&pFormatCtx);

	avformat_free_context(pFormatCtx);

	cout << "视频播放完成" << endl;

	return;

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

