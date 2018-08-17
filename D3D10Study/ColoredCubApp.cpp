#include "ColoredCubApp.h"
ColoredCubApp::ColoredCubApp(HINSTANCE hInstance): D3DApp(hInstance),mFX(0),mTech(0),mVertexLayout(0),mfxWVPVar(0),mTheta(0.0f),mPhi(PI*0.25f)
{
	D3DXMatrixIdentity(&mView);
	D3DXMatrixIdentity(&mProj);
	D3DXMatrixIdentity(&mWVP);
}

ColoredCubApp::~ColoredCubApp()
{
	if (md3dDevice)
		md3dDevice->ClearState();

	ReleaseCOM(mFX);
	ReleaseCOM(mVertexLayout);

}

void ColoredCubApp::initApp()
{
	D3DApp::initApp();
	mBox.init(md3dDevice,1.0f);

	buildFX();
	buildVertexLayouts();
}

void ColoredCubApp::onResize()
{
	D3DApp::onResize();
	const float aspect = static_cast<float>(mClientWidth) / mClientHeight;
	/*
	 * 透视投影矩阵
	 */
	D3DXMatrixPerspectiveFovLH(&mProj, 0.25f*PI, aspect, 1.0f, 1000.0f);

}

void ColoredCubApp::updateScene(float dt)
{
	D3DApp::updateScene(dt);

	//更新角度基于输入
	if (GetAsyncKeyState('A') & 0x8000) mTheta -= 2.0f*dt;
	if (GetAsyncKeyState('D') & 0x8000) mTheta += 2.0f*dt;
	if (GetAsyncKeyState('W') & 0x8000) mPhi -= 2.0f*dt;
	if (GetAsyncKeyState('S') & 0x8000) mPhi -= 2.0f*dt;

	//约束角度
/*	if (mPhi < 0.1f) mPhi = 0.1f;
	if (mPhi > PI - 0.1f)mPhi = PI - 0.1f;*/

	//球坐标转换到笛卡尔坐标
	float x = 5.0f*sinf(mPhi)*sinf(mTheta);
	float y = -5.0f*sinf(mPhi)*cosf(mTheta);
	float z = 5.0f*cosf(mPhi);

	//构建视图矩阵

	D3DXVECTOR3 pos(x, y, z);
	D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);

	D3DXMatrixLookAtLH(&mView, &pos, &target, &up);

}

void ColoredCubApp::drawScene()
{
	D3DApp::drawScene();

	md3dDevice->OMSetDepthStencilState(0, 0);
	float blendFactors[] = { 0.0f,0.0f,0.0f,0.0f };
	md3dDevice->OMSetBlendState(0, blendFactors, 0xffffffff);
	md3dDevice->IASetInputLayout(mVertexLayout);//输入布局绑定到设备驱动上
	md3dDevice->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);//设置图元拓扑来告诉如何组织图形

	//设置常量

	mWVP = mView * mProj;
	mfxWVPVar->SetMatrix(reinterpret_cast<float *>(&mWVP));

	D3D10_TECHNIQUE_DESC techDesc;
	mTech->GetDesc(&techDesc);
	for(UINT p=0;p<techDesc.Passes;++p)
	{
		mTech->GetPassByIndex(p)->Apply(0);
		mBox.draw();
	}

	RECT R = { 5,5,0,0 };
	mFont->DrawText(nullptr, mFrameStats.c_str(), -1, &R, DT_NOCLIP, BLACK);

	mSwapChain->Present(0, 0);

}

void ColoredCubApp::buildFX()
{
	DWORD shadeFlags = D3D10_SHADER_ENABLE_STRICTNESS;
#if defined(DEBUG)||defined(_DEBUG)
	shadeFlags |= D3D10_SHADER_DEBUG;
	shadeFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

	ID3D10Blob * compilationErrors = 0;
	HRESULT hr = 0;
	hr = D3DX10CreateEffectFromFile(L"color.fx", nullptr, nullptr, "fx_4_0", shadeFlags, 0, md3dDevice, 0, 0, &mFX, &compilationErrors, nullptr);

	if(FAILED(hr))
	{
		if(compilationErrors)
		{
			MessageBoxA(nullptr, static_cast<char *>(compilationErrors->GetBufferPointer()), nullptr, 0);
			ReleaseCOM(compilationErrors);
		}
	}

	mTech = mFX->GetTechniqueByName("ColorTech");
	mfxWVPVar = mFX->GetVariableByName("gWVP")->AsMatrix();

}

void ColoredCubApp::buildVertexLayouts()
{
	//输入布局结构信息
	/*
	 * D3D10_INPUT_ELEMENT_DESC
	 * 1. SemanticName :一个名称，可以是任何有效的语义名
	 * 2.SemanticIndex ：附加在语义上的索引值，当定点	结构包含多组纹理坐标时候，我们不是添加一个新的语义名称，而是添加一个索引值
	 * 3.Format :指定元素格式的枚举成员 
	 * 4.InputSlot：指定元素来自于那个input slot （direct3D 支持16个slot）
	 * 5.AlignedByteOffset：对于单个slot 来说，该参数表示定点结构体的起始位置到定点元素的起始位置之间的偏移量	
	 * 6.InputSlotClass
	 * 7. InstanceDataStepRate 
	 */
	D3D10_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D10_INPUT_PER_VERTEX_DATA,0},
		{ "COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D10_INPUT_PER_VERTEX_DATA,0 },
	};

	//创建输入布局
	/*
	 *CreateInputLayout；创建布局信息，获得一个表示输入布局的接口的指针
	 *1.pInputElementDescs ：用于描述定点结构体D3D10_INPUT_ELEMENT_DESC的数组
	 *2.NumElements ：D3D10_INPUT_ELEMENT_DESC数组元素的数量
	 *3.pShaderBytecodeWithInputSignature ：顶点着色器参数的字节码指针
	 *4.BytecodeLength：顶点着色器参数的字节码长度 单位为字节
	 *5.ppInputLayout ：返回创建的指针
	 */
	D3D10_PASS_DESC PassDesc;
	mTech->GetPassByIndex(0)->GetDesc(&PassDesc);
	HR(md3dDevice->CreateInputLayout(vertexDesc, 2, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &mVertexLayout));

}
int main()
{
	HINSTANCE hInst = GetModuleHandle(0);

	ColoredCubApp theApp(hInst);
	theApp.initApp();
	return theApp.run();




}
