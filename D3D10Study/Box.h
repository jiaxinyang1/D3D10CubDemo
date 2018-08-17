#pragma once
#include "d3dUtil.h"
class Box
{
public:
	Box();
	~Box();
	void init(ID3D10Device *device, float scale);
	void draw();

private:
	DWORD mNumVertices;
	DWORD mNumFaces;

	ID3D10Device *md3dDevice;
	ID3D10Buffer *mVB;
	ID3D10Buffer *mIB;

};

