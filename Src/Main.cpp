#include "Test.h"
#include <stdio.h>
#include "imgui/imgui.h"
#include "imgui_impl_dx9.h"

static D3DPRESENT_PARAMETERS    g_d3dpp;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern LRESULT ImGui_ImplDX9_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
//函数开始;

const int width = 800;
const int height = 600;
IDirect3DDevice9* Device = NULL;
D3DDEVTYPE deviceType = D3DDEVTYPE_HAL;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
#ifdef _DEBUG
	AllocConsole();
	freopen("conout$", "w", stdout);
#endif

	Test test;

	WNDCLASS wc = { 0 };
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = "Direct3D9App";

	if (!RegisterClass(&wc))
	{
		::MessageBox(0, TEXT("RegisterClass() - FAILED"), 0, 0);
		return 0;
	}

	HWND hwnd = 0;
	hwnd = CreateWindow("Direct3D9App",
		"Low Poly Convertor",
		WS_MAXIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		0,
		0,
		hInstance,
		0);
	if (!hwnd)
	{
		::MessageBox(0, TEXT("CreateWindow() - FAILED"), 0, 0);
		return 0;
	}

	::ShowWindow(hwnd, SW_SHOW);
	::UpdateWindow(hwnd);

	HRESULT hr = 0;
	IDirect3D9* d3d9 = 0;
	d3d9 = Direct3DCreate9(D3D_SDK_VERSION);
	if (!d3d9)
	{
		::MessageBox(0, TEXT("Direct3DCreate9() - FAILED"), 0, 0);
		return false;
	}

	D3DCAPS9 caps;
	d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, deviceType, &caps);
	int vp = 0;
	if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
		vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
	else
		vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.BackBufferFormat = D3DFMT_A8R8G8B8;
	g_d3dpp.BackBufferCount = 1;
	g_d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
	g_d3dpp.MultiSampleQuality = 0;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.hDeviceWindow = hwnd;
	g_d3dpp.Windowed = true;
	g_d3dpp.EnableAutoDepthStencil = true;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
	g_d3dpp.Flags = 0;
	g_d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	hr = d3d9->CreateDevice(
		D3DADAPTER_DEFAULT,
		deviceType,
		hwnd,
		vp,
		&g_d3dpp,
		&Device);
	if (FAILED(hr))
	{
		// try again using a 16-bit depth buffer
		g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

		hr = d3d9->CreateDevice(
			D3DADAPTER_DEFAULT,
			deviceType,
			hwnd,
			vp,
			&g_d3dpp,
			&Device);

		if (FAILED(hr))
		{
			d3d9->Release(); // done with d3d9 object
			::MessageBox(0, TEXT("CreateDevice() - FAILED"), 0, 0);
			return 0;
		}
	}
	else
	{
		//开始设置参数;
		test.OnInit(hwnd, Device);

		ImGui_ImplDX9_Init(hwnd, Device);
	}

	d3d9->Release(); // done with d3d9 object

	//消息循环;
	MSG msg = { 0 };
	float lastTime = (float)timeGetTime();
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			ImGui_ImplDX9_NewFrame();
			test.OnGUI();

			float curTime = (float)timeGetTime();
			float timeDelta = (curTime - lastTime)*0.001f;
			if (Device)
			{
				Device->BeginScene();
				Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0x00666666, 1.0f, 0);
				//开始显示;
				ImGui::Render();
				test.OnUpdate();
				Device->EndScene();

				Device->Present(0, 0, 0, 0);
			}
			lastTime = curTime;
		}
	}

	test.OnQuit();
	ImGui_ImplDX9_Shutdown();
	UnregisterClass("Direct3D9App", wc.hInstance);
	FreeConsole();
	return 0;
}

LRESULT CALLBACK WndProc(
	HWND hwnd,
	UINT msg,
	WPARAM wParam,
	LPARAM lParam)
{
	if (ImGui_ImplDX9_WndProcHandler(hwnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (Device != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			g_d3dpp.BackBufferWidth = LOWORD(lParam);
			g_d3dpp.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = Device->Reset(&g_d3dpp);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		return 0;
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			::DestroyWindow(hwnd);
		break;
	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);
}