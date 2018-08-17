#pragma once
#include "D3DApp.h"
class D3DDemo :public D3DApp
{
public:
	D3DDemo(HINSTANCE hInstance);
	~D3DDemo();
	void onResize() override;
	void initApp() override;
	void updateScene(float dt) override;
	void drawScene() override;
};

