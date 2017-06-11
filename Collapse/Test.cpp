#include "Test.h"
#include "ProgressiveMeshRenderer.h"
#include "BoneRenderer.h"
#include "imgui/imgui.h"

Test::Test()
{
	mLastTime = 0;
	mShowMesh = true;
	mShowBone = false;
	mRotSpeed = 0.0f;
}

Test::~Test()
{
	mLastTime = 0;
}

void Test::OnInit(HWND hwnd, IDirect3DDevice9* device)
{
	mDevice = device;
	mHwnd = hwnd;
	RECT rc;
	::GetClientRect(hwnd, &rc);
	mWidth = (int)(rc.right - rc.left);
	mHeight = (int)(rc.bottom - rc.top);

	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("msyh.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesChinese());

	mLastTime = timeGetTime();

	FBXHelper::BeginFBXHelper("humanoid.fbx");
	FBXHelper::FbxModelList* models = FBXHelper::GetModelList();
	mDisireVtxNums.Clear();
	mMaxDisireVtxNums.Clear();
	for (int i = 0; i < models->mMeshes.Count(); ++i)
	{
		FBXHelper::FbxModel* model = models->mMeshes[i];
		mDisireVtxNums.Add(model->nVertexCount);
		mMaxDisireVtxNums.Add(model->nVertexCount);
	}

	D3DXMATRIX matView, matProj;
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI / 4.0f, 800.0f / 600.0f, 0.1f, 10000.0f);
	D3DXMatrixLookAtLH(&matView, &D3DXVECTOR3(0.0f, 0.0f, -400.0f),
		&D3DXVECTOR3(0.0f, 0.0f, 400.0f), &D3DXVECTOR3(0.0f, 1.0f, 0.0f));
	D3DXMatrixScaling(&mMatWorld, -1.0f, -1.0f, -1.0f);

	device->SetTransform(D3DTS_PROJECTION, &matProj);
	device->SetTransform(D3DTS_VIEW, &matView);
	device->SetTransform(D3DTS_WORLD, &mMatWorld);
	device->SetRenderState(D3DRS_LIGHTING, TRUE);
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	D3DLIGHT9 light;
	light.Ambient = D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f);
	light.Diffuse = D3DXCOLOR(0.8f, 0.8f, 0.8f, 1.0f);
	light.Direction = D3DXVECTOR3(1.0f, -1.0f, 1.0f);
	light.Type = D3DLIGHTTYPE::D3DLIGHT_DIRECTIONAL;
	device->SetLight(0, &light);
	device->LightEnable(0, TRUE);
	material.Ambient = D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f);
	material.Diffuse = D3DXCOLOR(1.0f, 0.1f, 1.0f, 1.0f);
	device->SetMaterial(&material);
	mRotSpeed = 0.0f;

	mMeshRenderer = new ProgressiveMeshRenderer(mDevice);
	mMeshRenderer->Collapse(NULL, 1);

	mBoneRenderer = new BoneRenderer(mDevice);
	mBoneRenderer->BuildMesh();

	FBXHelper::SetCurrentAnimation("run");
}

void Test::OnGUI()
{
	FBXHelper::FbxModelList* models = FBXHelper::GetModelList();
	mImGuiID = 0;
	
	char szTxt[256];
	std::string infoTxt = "";
	memset(szTxt, 0, 256);
	sprintf_s(szTxt, "模型数: %d\n", models->mMeshes.Count());
	infoTxt += szTxt;
	for (int i = 0; i < models->mMeshes.Count(); ++i)
	{
		FBXHelper::FbxModel* model = models->mMeshes[i];
		memset(szTxt, 0, 256);
		sprintf_s(szTxt, "\t模型 [%d]\n", i + 1);
		infoTxt += szTxt;
		memset(szTxt, 0, 256);
		sprintf_s(szTxt, "\t\t顶点数：%d\n", model->nVertexCount);
		infoTxt += szTxt;
		memset(szTxt, 0, 256);
		sprintf_s(szTxt, "\t\t面数：%d\n", model->nIndexCount / 3);
		infoTxt += szTxt;
	}
	memset(szTxt, 0, 256);
	sprintf_s(szTxt, "蒙皮数：%d\n", models->mSkins.Count());
	infoTxt += szTxt;

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(256, (float)mHeight));
	ImGui::Begin(STU("转换器").c_str());

	if (mMeshRenderer->mIsSkinnedMesh)
	{
		if (ImGui::CollapsingHeader(STU("蒙皮模型减面").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Indent(15.0f);
			int* collapseParam = new int[models->mMeshes.Count()];
			for (int i = 0; i < models->mMeshes.Count(); ++i)
			{
				ImGui::Text(STU("模型[%d]预期顶点数").c_str(), i + 1);

				ImGui::SameLine();
				ImGui::PushID(mImGuiID++);
				ImGui::PushItemWidth(50);
				ImGui::DragInt(STU("个").c_str(), &mDisireVtxNums[i], 1.0f, 3, mMaxDisireVtxNums[i]);
				ImGui::PopItemWidth();
				ImGui::PopID();

				collapseParam[i] = mDisireVtxNums[i];
			}

			ImGui::PushID(mImGuiID++);
			if (ImGui::Button(STU("坍塌").c_str(), ImVec2(150, 30)))
			{
				mMeshRenderer->Collapse(collapseParam, models->mMeshes.Count());
			}
			ImGui::PopID();
			delete[] collapseParam;
			ImGui::Unindent(15.0f);
		}
	}

	if (ImGui::CollapsingHeader(STU("渲染选项").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Indent(15.0f);
		ImGui::Text(STU("旋转速度").c_str());
		ImGui::SameLine();
		ImGui::SliderFloat("##rotSpeed", &mRotSpeed, 0.0f, 10.0f);

		ImGui::Checkbox(STU("显示模型").c_str(), &mShowMesh);
		ImGui::Checkbox(STU("显示骨骼").c_str(), &mShowBone);
		if (mShowBone && mBoneRenderer)
		{
			ImGui::Indent(10.0f);
			float thick = mBoneRenderer->GetBoneThick();
			ImGui::Text(STU("骨骼粗细").c_str());
			ImGui::SameLine();
			ImGui::DragFloat("##thick", &thick, 0.05f);
			mBoneRenderer->SetBoneThick(thick);

			bool bindpos = mBoneRenderer->GetShowAnimated();
			ImGui::Checkbox(STU("显示BindPose").c_str(), &bindpos);
			mBoneRenderer->SetShowAnimated(bindpos);

			ImGui::Unindent(10.0f);
		}
		ImGui::Unindent(15.0f);
	}

	if (ImGui::CollapsingHeader(STU("模型信息").c_str(), ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Indent(15.0f);
		ImGui::Text(STU(infoTxt).c_str());
		ImGui::Unindent(15.0f);
	}
	ImGui::End();
}

void Test::OnUpdate()
{
	DWORD curTime = timeGetTime();
	DWORD timeDelta = curTime - mLastTime;
	float dt = (float)timeDelta * 0.001f;

	D3DXMATRIX matRot;
	D3DXMatrixRotationY(&matRot, dt * mRotSpeed);
	D3DXMatrixMultiply(&mMatWorld, &mMatWorld, &matRot);
	mDevice->SetTransform(D3DTS_WORLD, &mMatWorld);

	FBXHelper::UpdateSkeleton();

	if (mShowMesh)
		mMeshRenderer->Render();
	if (mShowBone)
		mBoneRenderer->Render();

	mLastTime = curTime;
}

void Test::OnQuit()
{
	if (mMeshRenderer)
		delete mMeshRenderer;
	SAFE_DELETE(mBoneRenderer);
	FBXHelper::EndFBXHelper();
}