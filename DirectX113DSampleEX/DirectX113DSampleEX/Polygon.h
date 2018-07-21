#pragma once
#include <D3D11.h>			// DirectX11SDKを使うために必要なもの
#include <d3dcompiler.h>	// シェーダーファイルをコンパイルするために必要
#include <string>

#pragma comment(lib, "d3d11.lib")		// DirectX11ライブラリを利用するために必要
#pragma comment(lib, "d3dCompiler.lib")	// シェーダーコードをコンパイルするために必要

// 頂点データ構造体
struct vertex
{
	float pos[3];		// 頂点座標	
	float col[4];		// 頂点ごとの色
	float uv[2];		// テクスチャ座標
};

class CPolygon
{
private:
	ID3D11InputLayout*	pVertexLayout=nullptr;		// 頂点データの構造を定義
	ID3D11Buffer*		pVertexBuffer = nullptr;		// 頂点データを保存
	ID3D11VertexShader* pVertexShader = nullptr;		// 頂点シェーダ
	ID3D11PixelShader*	pPixelShader = nullptr;		// ピクセルシェーダ
	char*				pCsoBin = nullptr;
	int					CsoSize;

	HRESULT CompileShaderFromFile(LPCWSTR szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	// const wchar_tからstringにしたよ
	HRESULT CreateVS(ID3D11Device* device, std::string vsStr);	// 頂点シェーダ作成
	HRESULT CreatePS(ID3D11Device* device, std::string psStr);	// ピクセルシェーダ作成
	void    ReadCSOFile(LPCWSTR szFileName );

	vertex vertexList[4];
public:
	void Init(ID3D11Device* device, std::string vsStr, std::string psStr);// const wchar_tからstringにしたよ
	void Render(ID3D11DeviceContext* context, ID3D11Buffer* buffer);
	void Render(ID3D11DeviceContext* context, ID3D11Buffer* buffer, ID3D11ShaderResourceView* texture, ID3D11SamplerState* sampler);
	void Release();
	void SetColor(float r, float g, float b, float a, int v);
};