#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

int main(int argc, char** argv)
{
	std::string process_target_name, dll_name;

	std::cout << "select target process\n";
	std::cin >> process_target_name;

	std::cout << "select your dll name (your dll should be in this folder)\n";
	std::cin >> dll_name;

	std::string exe_path = argv[0];

	std::cout << "exe path: " << exe_path << std::endl;

	std::string dll_path;
	
	for (int i = exe_path.size(); i >= 0; i--)
	{
		if (exe_path[i] == (char)'\\')
		{
			std::cout << "found! " << "erasing exe name form path..." << std::endl;
			for (int j = 0; j < i; j++)
			{
				dll_path += exe_path[j];
			}
			break;
		}
	}

	dll_path += "\\" + dll_name;
	std::cout << "dll path: " << dll_path << std::endl;

	auto process_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (process_snapshot == INVALID_HANDLE_VALUE)
	{
		std::cout << "failed create snapshot: " << GetLastError();
		system("pause");
		return 1;
	}

	PROCESSENTRY32 process_entry;
	process_entry.dwFlags = sizeof(process_entry);
	
	if (Process32First(process_snapshot, &process_entry) == FALSE)
	{
		std::cout << "failed create snapshot: " << GetLastError();
		system("pause");
		return 1;
	}

	DWORD process_id = NULL;

	do
	{
		if (strcmp(process_entry.szExeFile, process_target_name.c_str()) == NULL)
		{
			process_id = process_entry.th32ProcessID;
			std::cout << "process found!\n";
			break;
		}
	} while (Process32Next(process_snapshot, &process_entry));

	CloseHandle(process_snapshot);

	if (process_id == NULL)
	{
		std::cout << "process not found!\n";
		system("pause");
		return 1;
	}

	auto process_handle = OpenProcess(PROCESS_ALL_ACCESS, 0, process_id);

	if (process_handle == NULL)
	{
		std::cout << "failed open process: " << GetLastError() << std::endl;
		system("pause");
		return 1;
	}

	auto allocated_memory = VirtualAllocEx(process_handle, NULL, dll_path.size() + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if (allocated_memory == NULL)
	{
		std::cout << "failed alloc memory: " << GetLastError() << std::endl;
		system("pause");
		return 1;
	}

	WriteProcessMemory(process_handle, allocated_memory, dll_path.c_str(), dll_path.size() + 1, NULL);

	auto remote_thread_handle = CreateRemoteThread(process_handle, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryA, allocated_memory, NULL, NULL);

	if (remote_thread_handle == NULL)
	{
		std::cout << "failed create remote thread: " << GetLastError() << std::endl;
		system("pause");
		return 1;
	}
	
	CloseHandle(remote_thread_handle);

	std::cout << "dll succesfully injected!\n";
	system("pause");
}

