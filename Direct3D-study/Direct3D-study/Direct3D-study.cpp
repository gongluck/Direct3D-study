//Direct3D-study.cpp: 定义应用程序的入口点。
//

#include "stdafx.h"
#include "Direct3D-study.h"

//多媒体
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

//使用D3D
#include "d3dx9.h"
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3d9.lib")

//MATRICES_TEST、LIGHTS_TEST、TEXUTRE_TEST、MESHES_TEST
#define MESHES_TEST

//自定义定点结构
#ifdef MATRICES_TEST

struct CUSTOMVERTEX
{
	FLOAT x, y, z;
	DWORD color;
};
CUSTOMVERTEX g_vertices[] =
{
	{ -1.0f, -1.0f, 0.0f, 0xffff0000, },
	{  1.0f, -1.0f, 0.0f, 0xff0000ff, },
	{  0.0f,  1.0f, 0.0f, 0xffffffff, },
};
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE)

#elif defined LIGHTS_TEST

struct CUSTOMVERTEX
{
	D3DXVECTOR3 position;
	D3DXVECTOR3 normal;//法线
};
CUSTOMVERTEX g_vertices[100];
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL)

#elif defined TEXUTRE_TEST

struct CUSTOMVERTEX
{
	D3DXVECTOR3 position;
	D3DCOLOR    color;
	FLOAT       tu, tv;//纹理坐标
};
CUSTOMVERTEX g_vertices[100];
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX1)

#endif

//D3D
IDirect3D9* g_d3d = nullptr;
IDirect3DDevice9* g_device = nullptr;

IDirect3DVertexBuffer9* g_vb = nullptr;
IDirect3DTexture9* g_tx = nullptr;

ID3DXMesh* g_mesh = nullptr;
D3DMATERIAL9* g_meshmaterial = nullptr;
LPDIRECT3DTEXTURE9* g_txs = nullptr;

HWND g_hwnd = nullptr;

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 在此放置代码。
	MyRegisterClass(hInstance);

	// 执行应用程序初始化: 
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	//创建D3D对象
	g_d3d = Direct3DCreate9(D3D_SDK_VERSION);

	D3DPRESENT_PARAMETERS d3dpp = { 0 };
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.EnableAutoDepthStencil = TRUE;//开启深度缓存和模板缓存
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;//指定深度缓冲及模板缓冲区的格式

	//创建D3D设备对象
	HRESULT res = g_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_device);

	//设置渲染状态
	g_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE); //不剔除任何面
	g_device->SetRenderState(D3DRS_LIGHTING, FALSE); //禁用Direct3D光照
	g_device->SetRenderState(D3DRS_ZENABLE, TRUE); //打开Z缓冲

#ifndef MESHES_TEST
	//创建顶点缓冲区
	g_device->CreateVertexBuffer(sizeof(g_vertices), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_vb, nullptr);
#endif

	void* pVertices = nullptr;
	DWORD materials = 0;//接收材质数量

#ifdef LIGHTS_TEST
	//拷贝数据到顶点缓冲区
	for (int i = 0; i < 50; i++)
	{
		FLOAT theta = 2 * D3DX_PI / 49 * i;
		g_vertices[2 * i + 0].position = D3DXVECTOR3(sinf(theta), -1.0f, cosf(theta));
		g_vertices[2 * i + 0].normal = D3DXVECTOR3(sinf(theta), 0.0f, cosf(theta));
		g_vertices[2 * i + 1].position = D3DXVECTOR3(sinf(theta), 1.0f, cosf(theta));
		g_vertices[2 * i + 1].normal = D3DXVECTOR3(sinf(theta), 0.0f, cosf(theta));
	}
#elif defined TEXUTRE_TEST
	res = D3DXCreateTextureFromFile(g_device, L"banana.bmp", &g_tx);
	for (int i = 0; i < 50; i++)
	{
		FLOAT theta = 2 * D3DX_PI / 49 * i;
		g_vertices[2 * i + 0].position = D3DXVECTOR3(sinf(theta), -1.0f, -cosf(theta));
		g_vertices[2 * i + 0].color = 0xffffffff;
		g_vertices[2 * i + 0].tu = ((FLOAT)i) / 49;
		g_vertices[2 * i + 0].tv = 1.0f;

		g_vertices[2 * i + 1].position = D3DXVECTOR3(sinf(theta), 1.0f, -cosf(theta));
		g_vertices[2 * i + 1].color = 0xff808080;
		g_vertices[2 * i + 1].tu = ((FLOAT)i) / 49;
		g_vertices[2 * i + 1].tv = 0.0f;
	}
#elif defined MESHES_TEST
	ID3DXBuffer* pmb = nullptr;//材质缓冲

	//加载网格模型图片，获取材质
	res = D3DXLoadMeshFromX(L"Tiger.x", D3DXMESH_SYSTEMMEM, g_device, NULL, &pmb, NULL, &materials, &g_mesh);

	//提取材质和纹理
	D3DXMATERIAL* pmtrls = (D3DXMATERIAL*)pmb->GetBufferPointer();
	g_meshmaterial = new D3DMATERIAL9[materials];
	g_txs = new LPDIRECT3DTEXTURE9[materials];
	for (int i = 0; i < materials; ++i)
	{
		g_meshmaterial[i] = pmtrls[i].MatD3D; //拷贝材质
		g_meshmaterial[i].Ambient = g_meshmaterial[i].Diffuse;
		g_txs[i] = nullptr;
		//用获取到的纹理名字创建纹理
		res = D3DXCreateTextureFromFileA(g_device, pmtrls[i].pTextureFilename, &g_txs[i]);
	}

	//释放材质缓冲
	pmb->Release();
#endif

#ifndef MESHES_TEST
	g_vb->Lock(0, 0, &pVertices, 0);
	memcpy(pVertices, g_vertices, sizeof(g_vertices));
	g_vb->Unlock();
#endif

	MSG msg = { 0 };
	// 主消息循环: 
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (g_device == nullptr)
				break;

			//清空背景
			res = g_device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 255, 0), 1.0/*设置Z缓冲的Z初始值*/, 0/*设置模板缓冲的初始值*/);

			//开始“画图”
			res = g_device->BeginScene();

#ifdef LIGHTS_TEST
			//应用材质
			D3DMATERIAL9 mtrl = { 0 };
			mtrl.Diffuse.r = 1.0f;
			mtrl.Diffuse.a = 1.0f;
			mtrl.Ambient.b = 1.0f;
			mtrl.Ambient.a = 1.0f;
			g_device->SetMaterial(&mtrl);//应用材质

			//光源
			D3DLIGHT9 light;
			ZeroMemory(&light, sizeof(D3DLIGHT9));
			light.Type = D3DLIGHT_DIRECTIONAL;
			light.Diffuse.r = 1.0f;
			light.Diffuse.g = 1.0f;
			light.Diffuse.b = 1.0f;
			D3DXVECTOR3 vecDir = D3DXVECTOR3(cosf(timeGetTime() / 500.0f), 0.0f, sinf(timeGetTime() / 500.0f));
			D3DXVec3Normalize((D3DXVECTOR3*)&light.Direction, &vecDir);//设置光源方向
			light.Range = 1000.0f;
			//添加使用光源
			g_device->SetLight(0, &light);
			g_device->LightEnable(0, TRUE);
			g_device->SetRenderState(D3DRS_LIGHTING, TRUE);
			//为整个场景设置环境光(背景光)
			g_device->SetRenderState(D3DRS_AMBIENT, 0x00202020);
#elif defined TEXUTRE_TEST
			res = g_device->SetTexture(0, g_tx);
			res = g_device->SetTextureStageState(0, D3DTSS_COLOROP/*颜色混合方式*/, D3DTOP_MODULATE/*将两个混合参数相乘后输出*/);
			res = g_device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			res = g_device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
			res = g_device->SetTextureStageState(0, D3DTSS_ALPHAOP/*Alpha值混合方法*/, D3DTOP_DISABLE);
#endif
			//世界坐标系
			D3DXMATRIX matWorld, *pmatWorld = nullptr;
			//pmatWorld = D3DXMatrixRotationX(&matWorld, timeGetTime() / 500.0f);//绕X轴旋转
			//pmatWorld = D3DXMatrixRotationY(&matWorld, timeGetTime() / 500.0f);//绕Y轴旋转
			pmatWorld = D3DXMatrixRotationAxis(&matWorld, &D3DXVECTOR3(1,1,0), timeGetTime() / 500.0f);//绕自定义轴旋转

			res = g_device->SetTransform(D3DTS_WORLD, &matWorld);

			//观察坐标系
			D3DXVECTOR3 vEyePt(0.0f, 3.0f, -5.0f);//眼睛点
			D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);//观察点
			D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);//向上方向
			D3DXMATRIX matView, *pmatView = nullptr;
			pmatView = D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);//创建观察矩阵
			res = g_device->SetTransform(D3DTS_VIEW, &matView);

			//投影坐标系
			D3DXMATRIX matProj, *pmatProj = nullptr;
			pmatProj = D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 2/*在y方向上的视野，用弧度表示*/,
				1/*宽高比*/, 1/*近视图平面的Z值*/, 100/*远视图平面的Z值*/);//创建投影矩阵
			res = g_device->SetTransform(D3DTS_PROJECTION, &matProj);

#ifndef MESHES_TEST
			//设置渲染源数据
			res = g_device->SetStreamSource(0, g_vb, 0, sizeof(CUSTOMVERTEX));//内存位置
			res = g_device->SetFVF(D3DFVF_CUSTOMVERTEX);//格式
#else
			for (int i = 0; i < materials; ++i)
			{
				g_device->SetMaterial(&g_meshmaterial[i]);//应用材质
				g_device->SetTexture(0, g_txs[i]);//应用纹理
				g_mesh->DrawSubset(0);//绘制网格子集
			}
#endif

#ifdef MATRICES_TEST
			res = g_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);//绘制控制
#elif defined LIGHTS_TEST
			res = g_device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 98);//绘制控制(三角形带)
#elif defined TEXUTRE_TEST
			res = g_device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 98);//绘制控制(三角形带)
#endif		
			//结束“画图”
			res = g_device->EndScene();

			//渲染
			res = g_device->Present(nullptr, nullptr, nullptr, nullptr);
		}
	}

	//清理
	if (g_meshmaterial != nullptr)
	{
		delete[] g_meshmaterial;
		g_meshmaterial = nullptr;
	}
	if (g_txs != nullptr)
	{
		for (int i = 0; i < materials; ++i)
		{
			if (g_txs[i] != nullptr)
			{
				g_txs[i]->Release();
				g_txs[i] = nullptr;
			}
		}
		delete[] g_txs;
		g_txs = nullptr;
	}
	if (g_mesh != nullptr)
	{
		g_mesh->Release();
		g_mesh = nullptr;
	}


	if (g_tx != nullptr)
	{
		g_tx->Release();
		g_tx = nullptr;
	}
	if (g_vb != nullptr)
	{
		g_vb->Release();
		g_vb = nullptr;
	}
	if (g_device != nullptr)
	{
		g_device->Release();
		g_device = nullptr;
	}
	if (g_d3d != nullptr)
	{
		g_d3d->Release();
		g_d3d = nullptr;
	}

	return (int)msg.wParam;
}

//
//  函数: MyRegisterClass()
//
//  目的: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DIRECT3DSTUDY));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = nullptr;
	wcex.lpszClassName = TEXT("Direct3D-study");
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目的: 保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HWND hWnd = CreateWindowW(TEXT("Direct3D-study"), TEXT("Direct3D-study"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	//保存一个窗口句柄，供D3D使用
	g_hwnd = hWnd;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}