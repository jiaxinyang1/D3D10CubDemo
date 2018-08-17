#include "D3DDemo.h"
#include "d3dUtil.h"


D3DDemo::D3DDemo(HINSTANCE hInstance):D3DApp(hInstance)
{
}

D3DDemo::~D3DDemo()
{
	if(md3dDevice)
	{
		md3dDevice->ClearState();
	}
}
void D3DDemo::initApp()
{
	D3DApp::initApp();
}
void D3DDemo::drawScene()
{
	D3DApp::drawScene();
	RECT R = { 5,5,0,0 };
	mFont->DrawText(0, mFrameStats.c_str(), -1, &R, DT_NOCLIP, BLACK);
	mSwapChain->Present(0,0);
}
void D3DDemo::updateScene(float dt)
{
	D3DApp::updateScene(dt);
}
void D3DDemo::onResize()
{
	D3DApp::onResize();
}
