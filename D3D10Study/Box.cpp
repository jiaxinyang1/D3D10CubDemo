#include "Box.h"



Box::Box() :mNumVertices(0), mNumFaces(0), md3dDevice(0), mVB(0), mIB(0)
{
}


Box::~Box()
{
	ReleaseCOM(mVB);
	ReleaseCOM(mIB);

}

void Box::init(ID3D10Device * device, float scale)
{
	md3dDevice = device;
	mNumVertices = 8;
	mNumFaces = 12;
	Vertex  vertices[]={
		{D3DXVECTOR3(-1.0f,-1.0f,-1.0f),WHITE},
	{ D3DXVECTOR3(-1.0f,+1.0f,-1.0f),BLACK },
	{ D3DXVECTOR3(+1.0f,+1.0f,-1.0f),RED },
	{ D3DXVECTOR3(+1.0f,-1.0f,-1.0f),GREEN },	
	{ D3DXVECTOR3(-1.0f,-1.0f,+1.0f),BLUE },
	{ D3DXVECTOR3(-1.0f,+1.0f,+1.0f),YELLOW },
	{ D3DXVECTOR3(+1.0f,+1.0f,+1.0f),CYAN },
	{ D3DXVECTOR3(+1.0f,-1.0f,+1.0f),MAGENTA },
	};

	for (DWORD i = 0; i < mNumVertices; ++i)
		vertices[i].pos *= scale;

	/*
	 *D3D10_BUFFER_DESC： 顶点缓冲区，为了让GPU访问顶点数组。这是必要的
	 */
	D3D10_BUFFER_DESC vbd;
	vbd.Usage = D3D10_USAGE_IMMUTABLE;//指定缓冲区用途的枚举 此值表示创建完毕后内容不会发生变换GPU以只读方式访问提高效率
	vbd.ByteWidth = sizeof(Vertex)*mNumVertices; //顶点缓冲区的大小
	vbd.BindFlags = D3D10_BIND_VERTEX_BUFFER;//对于定点缓冲区，这个值应该设为D3D10_BIND_VERTEX_BUFFER
	vbd.CPUAccessFlags = 0;//指定GPU的访问权限
	vbd.MiscFlags = 0;//杂项标志，设为0

	/*
	 * D3D10_SUBRESOURCE_DATA 为缓冲区指定初始化数据
	 */
	D3D10_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = vertices;
	HR(md3dDevice->CreateBuffer(&vbd, &vinitData, &mVB));//创建缓冲区
	/*
	 *
	 *索引缓冲区
	 *
	 */
	DWORD indices[] = {
		0,1,2,
		0,2,3,

		4,6,5,
		4,7,6,

		4,5,1,
		4,1,0,

		3,2,6,
		3,6,7,

		1,5,6,
		1,6,2,

		4,0,3,
		4,3,7
	};

	D3D10_BUFFER_DESC ibd;
	ibd.Usage = D3D10_USAGE_IMMUTABLE;
	ibd.ByteWidth = sizeof(DWORD)*mNumFaces * 3;
	ibd.BindFlags = D3D10_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	D3D10_SUBRESOURCE_DATA iinitData;
	iinitData.pSysMem = indices;
	HR(md3dDevice->CreateBuffer(&ibd, &iinitData, &mIB));


}

void Box::draw()
{
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dDevice->IASetVertexBuffers(0, 1, &mVB, &stride, &offset);//缓冲区绑定到设备上，
	md3dDevice->IASetIndexBuffer(mIB, DXGI_FORMAT_R32_UINT, 0);//索引缓冲区绑定到设备上
	md3dDevice->DrawIndexed(mNumFaces * 3, 0, 0);//使用索引

}
