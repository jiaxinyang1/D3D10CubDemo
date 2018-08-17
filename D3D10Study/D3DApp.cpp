#include "D3DApp.h"
#include "d3dUtil.h"
#include <sstream>
#pragma comment(lib, "legacy_stdio_definitions.lib")
LRESULT CALLBACK 
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)//窗口创建过程中的proc不能指定为成员方法，这里使用一个公共的过程，在内部，我们交给实例过程去处理
{
	static D3DApp *app = 0;
	switch(msg)
	{
	case WM_CREATE:
		CREATESTRUCT * cs = reinterpret_cast<CREATESTRUCT*>(lParam);
		app = static_cast<D3DApp*>(cs->lpCreateParams);
		return 0;
	}
	if (app)
		return app->msgProc(msg, wParam, lParam);
	else
		return DefWindowProc(hwnd, msg, wParam, lParam);
}
D3DApp::D3DApp(HINSTANCE hInstance)
{
	//变量的初始化过程
	mhAppInst = hInstance;
	mhMainWnd = nullptr;
	mAppPaused = false;
	mMinimized = false;
	mMaximized = false;
	mResizing = false;

	mFrameStats = L"";

	md3dDevice = 0;
	mSwapChain = 0;
	mDepthStencilBuffer = 0;
	mRenderTargetView = 0;
	mDepthStencilView = 0;
	mFont = 0;

	mMainWndCaption = L"D3D10 Application";
	md3dDriverType = D3D10_DRIVER_TYPE_HARDWARE;
	mClearColor = D3DXCOLOR(0.0f, 0.0f, 1.0f, 1.0f);
	mClientWidth = 800;//窗口默认大小
	mClientHeight = 600;
}
D3DApp::~D3DApp()
{
	//资源释放
	ReleaseCOM(mRenderTargetView);
	ReleaseCOM(mDepthStencilView);
	ReleaseCOM(mSwapChain);
	ReleaseCOM(mDepthStencilBuffer);
	ReleaseCOM(md3dDevice);
	ReleaseCOM(mFont);
}
HINSTANCE D3DApp::getAppInst() const
{
	return mhAppInst;
}
HWND D3DApp::getMainWnd() const
{
	return mhMainWnd;
}
int D3DApp::run()
{
	MSG msg{ nullptr };
	mTimer.reset();
	while (msg.message!=WM_QUIT)
	{
		if(PeekMessage(&msg,0,0,0,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			mTimer.tick();
			if (!mAppPaused)
				updateScene(mTimer.getDeltaTime());
			else
				Sleep(50);

			drawScene();
		}
	}
	return static_cast<int>(msg.wParam);
}
void D3DApp::initMainWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mhAppInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"D3DWndClassName";
	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass FAILED", 0, 0);
		PostQuitMessage(0);
	}

	RECT R = { 0, 0, mClientWidth, mClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	mhMainWnd = CreateWindow(L"D3DWndClassName", mMainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, mhAppInst, this);
	if (!mhMainWnd)
	{
		MessageBox(0, L"CreateWindow FAILED", 0, 0);
		PostQuitMessage(0);
	}

	ShowWindow(mhMainWnd, SW_SHOW);
	UpdateWindow(mhMainWnd);
}
void D3DApp::initDirect3D()
{
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = mClientWidth;
	sd.BufferDesc.Height = mClientHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;

	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = mhMainWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
#endif

	HR(D3D10CreateDeviceAndSwapChain(
		0,                 //default adapter
		md3dDriverType,
		0,                 // no software device
		createDeviceFlags,
		D3D10_SDK_VERSION,
		&sd,
		&mSwapChain,
		&md3dDevice));

	onResize();
}
LRESULT D3DApp::msgProc(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_ACTIVATE://焦点处理 失去焦点则暂停游戏
		if(LOWORD(lParam)== WA_INACTIVE)
		{
			mAppPaused = true;
			mTimer.stop();
		}
		else
		{
			mAppPaused = false;
			mTimer.start();
		}
		return 0;
	case WM_SIZE:
		//保存新的窗口信息
		mClientWidth = LOWORD(lParam);
		mClientHeight = HIWORD(lParam);
		if(md3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)//最小化
			{
				mAppPaused = true;//游戏暂停
				mMinimized = true;
				mMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)//最大化
			{
				mAppPaused = false;
				mMinimized = false;
				mMaximized = true;
				onResize();
			}
			else if (wParam == SIZE_RESTORED)
			{

				if (mMinimized)
				{
					mAppPaused = false;
					mMinimized = false;
					onResize();
				}

				else if (mMaximized)
				{
					mAppPaused = false;
					mMaximized = false;
					onResize();
				}
				else if (mResizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else 
				{
					onResize();
				}
			}
		}
		return 0;
	case WM_ENTERSIZEMOVE://窗口缩放
		mAppPaused = true;
		mResizing = true;
		mTimer.stop();
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_MENUCHAR://菜单
		return MAKELRESULT(0, MNC_CLOSE);
	case WM_GETMINMAXINFO:
		reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.x = 200;
		reinterpret_cast<MINMAXINFO*>(lParam)->ptMinTrackSize.y = 200;
		return 0;
	}
	return DefWindowProc(mhMainWnd, msg, wParam, lParam);
}

void D3DApp::drawScene()
{
	md3dDevice->ClearRenderTargetView(mRenderTargetView, mClearColor);
	md3dDevice->ClearDepthStencilView(mDepthStencilView, D3D10_CLEAR_DEPTH | D3D10_CLEAR_STENCIL, 1.0f, 0);
}
void D3DApp::updateScene(float dt)
{
	static int frameCnt = 0;
	static float t_base = 0.0f;

	frameCnt++;
	if ((mTimer.getGameTime() - t_base) >= 1.0f)
	{
		float fps = (float)frameCnt; // fps = frameCnt / 1
		float mspf = 1000.0f / fps;

		std::wostringstream outs;
		outs.precision(6);
		outs << L"FPS: " << fps << L"\n"
			<< "Milliseconds: Per Frame: " << mspf;
		mFrameStats = outs.str();

		// Reset for next average.
		frameCnt = 0;
		t_base += 1.0f;
	}
}
void D3DApp::onResize()
{
	ReleaseCOM(mRenderTargetView);
	ReleaseCOM(mDepthStencilView);
	ReleaseCOM(mDepthStencilBuffer);


	// Resize the swap chain and recreate the render target view.

	HR(mSwapChain->ResizeBuffers(1, mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
	ID3D10Texture2D* backBuffer;
	HR(mSwapChain->GetBuffer(0, __uuidof(ID3D10Texture2D), reinterpret_cast<void**>(&backBuffer)));
	HR(md3dDevice->CreateRenderTargetView(backBuffer, 0, &mRenderTargetView));
	ReleaseCOM(backBuffer);


	// Create the depth/stencil buffer and view.

	D3D10_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = mClientWidth;
	depthStencilDesc.Height = mClientHeight;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1; // multisampling must match
	depthStencilDesc.SampleDesc.Quality = 0; // swap chain values.
	depthStencilDesc.Usage = D3D10_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	HR(md3dDevice->CreateTexture2D(&depthStencilDesc, 0, &mDepthStencilBuffer));
	HR(md3dDevice->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView));


	// Bind the render target view and depth/stencil view to the pipeline.

	md3dDevice->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);


	// Set the viewport transform.

	D3D10_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = mClientWidth;
	vp.Height = mClientHeight;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;

	md3dDevice->RSSetViewports(1, &vp);
}
void D3DApp::initApp()
{
	initMainWindow();
	initDirect3D();

	D3DX10_FONT_DESC fontDesc;
	fontDesc.Height = 24;
	fontDesc.Width = 0;
	fontDesc.Weight = 0;
	fontDesc.MipLevels = 1;
	fontDesc.Italic = false;
	fontDesc.CharSet = DEFAULT_CHARSET;
	fontDesc.OutputPrecision = OUT_DEFAULT_PRECIS;
	fontDesc.Quality = DEFAULT_QUALITY;
	fontDesc.PitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	wcscpy_s(fontDesc.FaceName, L"Times New Roman");

	D3DX10CreateFontIndirect(md3dDevice, &fontDesc, &mFont);
}