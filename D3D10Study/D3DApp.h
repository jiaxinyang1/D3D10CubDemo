#pragma once

#include <Windows.h>
#include "GameTimer.h"
#include <string>
#include<D3D10.h>
#include<D3DX10.h>

class D3DApp
{
public:
	D3DApp(HINSTANCE hInstance);
	virtual ~D3DApp();
	HINSTANCE getAppInst() const; //返回hinstance;
	HWND getMainWnd() const; //返回窗口句柄

	int run();//执行过程
	virtual  void initApp();//窗口初始化过程
	virtual  void onResize();//重新设置属性
	virtual  void updateScene(float dt);//刷新画面
	virtual  void drawScene();//绘制画面
	virtual  LRESULT msgProc(UINT msg, WPARAM wParam, LPARAM lParam);//消息过程

protected:
	void initMainWindow();//窗口创建过程
	void initDirect3D();//Direct初始化过程
protected:
	HINSTANCE mhAppInst;//应用程序inst句柄
	HWND			mhMainWnd;//主窗口句柄
	bool				mAppPaused;//窗口是否暂停
	bool				mMinimized;//窗口是否最小化
	bool				mMaximized;//窗口是否最大化
	bool				mResizing;//

	
	GameTimer  mTimer;//Timer类
	std::wstring  mFrameStats;


	//d3d初始化所需要的一些句柄
	ID3D10Device * md3dDevice;
	IDXGISwapChain * mSwapChain;
	ID3D10Texture2D * mDepthStencilBuffer;
	ID3D10RenderTargetView * mRenderTargetView;
	ID3D10DepthStencilView * mDepthStencilView;
	ID3DX10Font * mFont;

	//一些值用来在初始化工作上
	D3D10_DRIVER_TYPE md3dDriverType;
	std::wstring mMainWndCaption;//标题
	D3DXCOLOR mClearColor;//背景颜色

	//窗口大小
	int mClientWidth;
	int mClientHeight;


};

