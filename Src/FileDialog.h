#pragma once

#include "inc.h"

enum eFileDialogUsage
{
	eFileDialogUsage_OpenFile,
	eFileDialogUsage_SaveFile,
	eFileDialogUsage_OpenFolder
};

class FileDialog
{
public:
	FileDialog();
	FileDialog(const char* name);
	void Open();
	bool DoModal();
	void GetFiles(const std::string& path);
	void SetDefaultDirectory(const std::string& szDir);
	static bool IsExt(const std::string& filename, const std::string& ext);
public:
	bool open;
	std::string ext;
	std::string fileName;
	char directory[MAX_PATH];
	std::string dlgName;
	eFileDialogUsage mUsage;

	char cur_directory[MAX_PATH];
private:
	char cur_filename[MAX_PATH];
	int selFileIndex;
	int selFolderIndex;
	std::vector<std::string> files;
	std::vector<std::string> folders;
};