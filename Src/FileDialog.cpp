#include "FileDialog.h"

FileDialog::FileDialog()
{
	open = false;
	ext = "*";
	memset(cur_directory, 0, MAX_PATH);
	memset(directory, 0, MAX_PATH);
	memset(cur_filename, 0, MAX_PATH);
	selFileIndex = -1;
	fileName = "";
	dlgName = "";
	mUsage = eFileDialogUsage_OpenFile;
}

FileDialog::FileDialog(const char* name)
{
	open = false;
	ext = "*";
	memset(cur_directory, 0, MAX_PATH);
	memset(directory, 0, MAX_PATH);
	memset(cur_filename, 0, MAX_PATH);
	selFileIndex = -1;
	selFolderIndex = -1;
	fileName = "";
	dlgName = name;
	mUsage = eFileDialogUsage_OpenFile;
}

void FileDialog::Open()
{
	ImGui::OpenPopup(STU(dlgName).c_str());
	selFileIndex = -1;
	selFolderIndex = -1;
	fileName = "";
	memset(cur_filename, 0, MAX_PATH);
	std::string dir = directory;
	files.clear();
	folders.clear();
	if (dir.size() > 0)
	{
		GetFiles(directory);
	}
}

bool FileDialog::IsExt(const std::string& filename, const std::string& ext)
{
	int lastExt = filename.rfind(ext);
	int lastAct = filename.length() - ext.length();
	return lastExt == lastAct;
}

void FileDialog::GetFiles(const std::string& path)
{
	selFileIndex = -1;
	selFolderIndex = -1;
	fileName = "";
	memset(cur_filename, 0, MAX_PATH);
	//文件句柄;
	long   hFile = 0;;
	//文件信息;
	struct _finddata_t fileinfo;
	std::string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,迭代之;
			//如果不是,加入列表;
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
				{
					folders.push_back(fileinfo.name);
				}
			}
			else
			{
				if (IsExt(fileinfo.name, ext))
				{
					files.push_back(fileinfo.name);
				}
			}
		}
		while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}

void FileDialog::SetDefaultDirectory(const std::string& szDir)
{
	memset(cur_directory, 0, MAX_PATH);
	memset(directory, 0, MAX_PATH);
	memcpy(cur_directory, szDir.c_str(), szDir.length());
	memcpy(directory, szDir.c_str(), szDir.length());
}

bool FileDialog::DoModal()
{
	bool rst = false;

	if (ImGui::BeginPopupModal(STU(dlgName).c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text(STU("路径").c_str());
		ImGui::SameLine();
		std::string curdir = STU(cur_directory);
		char szTemp[MAX_PATH];
		memset(szTemp, 0, MAX_PATH);
		memcpy(szTemp, curdir.c_str(), curdir.length());
		ImGui::PushItemWidth(220);
		ImGui::InputText("", szTemp, MAX_PATH);
		ImGui::PopItemWidth();
		curdir = UTS(szTemp);
		memset(cur_directory, 0, MAX_PATH);
		memcpy(cur_directory, curdir.c_str(), curdir.length());

		ImGui::SameLine();
		if (ImGui::Button(STU("转到").c_str()))
		{
			memset(directory, 0, MAX_PATH);
			memcpy(directory, cur_directory, strlen(cur_directory));
		}
		ImGui::SameLine(0, 16);
		if (ImGui::Button(STU("向上").c_str()))
		{
			std::string s = directory;
			if (s.size() > 0)
			{
				StringReplace(s, "/", "\\");
				char last = s[s.length() - 1];
				if (last == '\\')
				{
					s = s.substr(0, s.length() - 1);
				}
				memset(cur_directory, 0, MAX_PATH);
				memset(directory, 0, MAX_PATH);
				int lastPos = s.find_last_of('\\');
				if (lastPos >= 0)
				{
					s = s.substr(0, lastPos);
					s += "\\";
					memcpy(directory, s.c_str(), s.length());
					memcpy(cur_directory, s.c_str(), s.length());
					files.clear();
					folders.clear();
					GetFiles(directory);
				}
			}
		}
		ImGui::SameLine();
		static char szFolder[128];
		if (ImGui::Button(STU("新建").c_str()))
		{
			if (directory[0] != '\0')
			{
				memset(szFolder, 0, 128);
				ImGui::OpenPopup(STU("新建新建文件夹").c_str());
			}
		}
		
		if (ImGui::BeginPopupModal(STU("新建新建文件夹").c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text(STU("文件夹名字").c_str());
			ImGui::SameLine();
			ImGui::InputText(STU("(中文会乱码)").c_str(), szFolder, 128);
			ImGui::Spacing();
			ImGui::Separator();
			if (ImGui::Button(STU("确定").c_str()) && szFolder[0] != '\0')
			{
				std::string newDir = directory;
				newDir += szFolder;
				if (_mkdir(newDir.c_str()) == 0)
				{
					folders.push_back(szFolder);
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::SameLine();
			if (ImGui::Button(STU("取消").c_str()))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ChildWindowRounding, 5.0f);
		ImGui::BeginChild("Sub2", ImVec2(400, 300), true);

		std::string path = directory;
		if (path.size() == 0)
		{
			char diskStrings[256];
			memset(diskStrings, 0, 256);
			GetLogicalDriveStrings(256, diskStrings);

			for (TCHAR *s = diskStrings; *s; s += strlen(s) + 1)
			{
				ImGui::Image(Global::mDiskTexID, ImVec2(32, 32));
				ImGui::SameLine();
				if (ImGui::Selectable(s, false, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, 32)))
				{
					if (ImGui::IsMouseDoubleClicked(0))
					{
						memset(cur_directory, 0, MAX_PATH);
						memcpy(directory, s, strlen(s));
						memcpy(cur_directory, s, strlen(s));

						files.clear();
						folders.clear();
						GetFiles(s);
					}
					else
					{

					}
				}
			}
		}
		else
		{
			for (size_t i = 0; i < folders.size(); ++i)
			{
				std::string& folder = folders[i];
				ImGui::Image(Global::mFolderTexID, ImVec2(32, 32));
				ImGui::SameLine();
				if (ImGui::Selectable(STU(folder).c_str(), selFolderIndex == i, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0, 32)))
				{
					if (ImGui::IsMouseDoubleClicked(0))
					{
						path = path + folder + "\\";
						memcpy(directory, path.c_str(), path.length());
						memcpy(cur_directory, path.c_str(), path.length());

						std::string tmp;
						if (mUsage == eFileDialogUsage_OpenFolder)
						{
							tmp = folder;							
						}						

						files.clear();
						folders.clear();
						GetFiles(directory);

						if (mUsage == eFileDialogUsage_OpenFolder)
						{
							selFileIndex = i;
							fileName = tmp;
							memset(cur_filename, 0, MAX_PATH);
							memcpy(cur_filename, fileName.c_str(), fileName.length());
							fileName = "|" + fileName;
						}
					}
					else if (mUsage == eFileDialogUsage_OpenFolder)
					{
						selFileIndex = i;
						fileName = folder;
						memset(cur_filename, 0, MAX_PATH);
						memcpy(cur_filename, fileName.c_str(), fileName.length());
					}
				}
			}
			for (size_t i = 0; i < files.size() && mUsage != eFileDialogUsage_OpenFolder; ++i)
			{
				std::string& file = files[i];
				ImGui::Image(Global::mFileTexID, ImVec2(32, 32));
				ImGui::SameLine();
				if (ImGui::Selectable(STU(file).c_str(), selFileIndex >= 0 && selFileIndex == i, 0, ImVec2(0, 32)))
				{
					selFileIndex = i;
					fileName = file;
					memset(cur_filename, 0, MAX_PATH);
					memcpy(cur_filename, fileName.c_str(), fileName.length());
				}
			}
		}

		ImGui::EndChild();

		ImGui::Text(STU("文件名").c_str());
		ImGui::SameLine();
		std::string curFileName = STU(cur_filename);
		std::string curFileNameOld = curFileName;
		char tempFilename[MAX_PATH];
		memset(tempFilename, 0, MAX_PATH);
		memcpy(tempFilename, curFileName.c_str(), curFileName.size());
		ImGui::InputText("##cur_filename", tempFilename, MAX_PATH);
		curFileName = UTS(tempFilename);
		memset(cur_filename, 0, MAX_PATH);
		memcpy(cur_filename, curFileName.c_str(), curFileName.size());

		ImGui::Separator();

		if (mUsage == eFileDialogUsage_OpenFile)
		{
			if (ImGui::Button(STU("打开").c_str(), ImVec2(80, 40)))
			{
				if (fileName.size() == 0)
				{
					fileName = cur_filename;
					if (!IsExt(fileName, ext))
					{
						fileName = fileName.append(".").append(ext);
					}
					std::vector<std::string>::iterator findIt = std::find(files.begin(), files.end(), fileName);
					if (findIt == files.end())
						fileName = "";
				}
				if (fileName.length() > 0)
				{
					ImGui::CloseCurrentPopup();
					rst = true;
				}
			}
		}
		else if (mUsage == eFileDialogUsage_SaveFile)
		{
			if (curFileNameOld != curFileName)
			{
				fileName = "";
			}

			bool fileExist = false;
			bool close = false;
			if (ImGui::Button(STU("保存").c_str(), ImVec2(80, 40)) && directory[0] != '\0')
			{
				if (fileName.size() == 0)
				{
					fileName = curFileName;
					if (!IsExt(fileName, ext))
					{
						fileName = fileName.append(".").append(ext);
					}
				}
				std::vector<std::string>::iterator findIt = std::find(files.begin(), files.end(), fileName);
				if (findIt != files.end())
				{
					fileExist = true;
					ImGui::OpenPopup(STU("提醒").c_str());
				}
				if (!fileExist && fileName.size() > 0)
				{
					ImGui::CloseCurrentPopup();
					rst = true;
				}
			}
			if (ImGui::BeginPopupModal(STU("提醒").c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text(STU("文件已经存在，是否替换").c_str());
				ImGui::Separator();
				if (ImGui::Button(STU("是").c_str(), ImVec2(60, 24)))
				{
					ImGui::CloseCurrentPopup();
					rst = true;
					close = true;
				}
				ImGui::SameLine();
				if (ImGui::Button(STU("否").c_str(), ImVec2(60, 24)))
				{
					fileName = "";
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
			if (close)
			{
				ImGui::CloseCurrentPopup();
			}
		}
		else if (mUsage == eFileDialogUsage_OpenFolder)
		{
			if (ImGui::Button(STU("选择文件夹").c_str(), ImVec2(80, 40)) && directory[0] != '\0')
			{
				if (fileName.size() == 0)
				{
					fileName = curFileName;
				}
				if (fileName.size() > 0 && fileName[0] == '|')
				{
					fileName = fileName.substr(1);
					std::string dir = directory;
					int lp = dir.rfind('\\');
					lp = dir.rfind('\\', lp - 1);
					if (lp > 0)
					{
						dir = dir.substr(0, lp + 1);
					}
					memset(directory, 0, MAX_PATH);
					memset(cur_directory, 0, MAX_PATH);
					memcpy(directory, dir.c_str(), dir.length());
					memcpy(cur_directory, dir.c_str(), dir.length());
					rst = true;
					ImGui::CloseCurrentPopup();
				}
				else
				{
					std::vector<std::string>::iterator findIt = std::find(folders.begin(), folders.end(), fileName);
					if (findIt == folders.end())
					{
						fileName = "";
					}
					else
					{
						rst = true;
						ImGui::CloseCurrentPopup();
					}
				}
			}
		}
		ImGui::SameLine(0.0f, 20.0f);
		if (ImGui::Button(STU("取消").c_str(), ImVec2(80, 40)))
		{
			ImGui::CloseCurrentPopup();
		}

		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}

	
	return rst;
}