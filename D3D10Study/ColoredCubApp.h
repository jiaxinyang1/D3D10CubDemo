#pragma once
#include "D3DApp.h"
#include "Box.h"
class ColoredCubApp:public D3DApp
{
public:
	ColoredCubApp(HINSTANCE hInstance);
	~ColoredCubApp();

	void initApp() override;
	void onResize() override;
	void updateScene(float dt) override;
	void drawScene() override;
private:
	void buildFX();
	void buildVertexLayouts();

private:

	Box mBox;

	ID3D10Effect *mFX;
	ID3D10EffectTechnique *mTech;
	ID3D10InputLayout *mVertexLayout;
	ID3D10EffectMatrixVariable *mfxWVPVar;

	D3DXMATRIX mView;
	D3DXMATRIX mProj;
	D3DXMATRIX mWVP;


	float mTheta;
	float mPhi;
};

