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

//自定义定点结构
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

//D3D
IDirect3D9* g_d3d = nullptr;
IDirect3DDevice9* g_device = nullptr;
IDirect3DVertexBuffer9* g_vb = nullptr;

HWND g_hwnd = nullptr;

// 此代码模块中包含的函数的前向声明: 
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此放置代码。
    MyRegisterClass(hInstance);

    // 执行应用程序初始化: 
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DIRECT3DSTUDY));

	//创建D3D对象
	g_d3d = Direct3DCreate9(D3D_SDK_VERSION);

	D3DPRESENT_PARAMETERS d3dpp = { 0 };
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	//创建D3D设备对象
	HRESULT res = g_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, g_hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &g_device);

	//设置渲染状态
	g_device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE); //不剔除任何面
	g_device->SetRenderState(D3DRS_LIGHTING, FALSE); //禁用Direct3D光照

	//创建顶点缓冲区
	g_device->CreateVertexBuffer(sizeof(g_vertices), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &g_vb, nullptr);

	//拷贝数据到顶点缓冲区
	void* pVertices;
	g_vb->Lock(0, sizeof(g_vertices), &pVertices, 0);
	memcpy(pVertices, g_vertices, sizeof(g_vertices));
	g_vb->Unlock();

    MSG msg;
    // 主消息循环: 
	ZeroMemory(&msg, sizeof(msg));
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

			//世界坐标系
			D3DXMATRIX matWorld, *pmatWorld = nullptr;
			pmatWorld = D3DXMatrixRotationY(&matWorld, timeGetTime() / 300.0f);//绕Y轴旋转
			HRESULT res = g_device->SetTransform(D3DTS_WORLD, &matWorld);

			//观察坐标系
			D3DXVECTOR3 vEyePt(0.0f, 0.0f, -1.0f);
			D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f);
			D3DXVECTOR3 vUpVec(0.0f, 1.0f, 0.0f);
			D3DXMATRIX matView, *pmatView = nullptr;
			pmatView = D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookatPt, &vUpVec);//创建观察矩阵
			res = g_device->SetTransform(D3DTS_VIEW, &matView);

			//投影坐标系
			D3DXMATRIX matProj, *pmatProj = nullptr;
			pmatProj = D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI/2/*在y方向上的视野，用弧度表示*/, 1/*宽高比*/, 0/*近视图平面的Z值*/, 0/*远视图平面的Z值*/);//创建投影矩阵
			res = g_device->SetTransform(D3DTS_PROJECTION, &matProj);

			//清空背景
			res = g_device->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 255, 0), 0, 0);

			//开始“画图”
			res = g_device->BeginScene();

			//设置渲染源数据
			res = g_device->SetStreamSource(0, g_vb, 0, sizeof(CUSTOMVERTEX));//内存位置
			res = g_device->SetFVF(D3DFVF_CUSTOMVERTEX);//格式
			res = g_device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);//绘制控制

			//结束“画图”
			res = g_device->EndScene();

			//渲染
			res = g_device->Present(nullptr, nullptr, nullptr, nullptr);
		}
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

    return (int) msg.wParam;
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

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DIRECT3DSTUDY));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = TEXT("Direct3D-study");
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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