#include <iostream>
#include <string>
#include <windows.h>
#include <winreg.h> 
#include <tlhelp32.h> 

bool deleteRegistryKey(HKEY rootKey, const std::string& subKey) {
    HKEY hKey;
    if (RegOpenKeyExA(rootKey, subKey.c_str(), 0, KEY_READ | KEY_WRITE, &hKey) == ERROR_SUCCESS) {
        DWORD subKeyCount;
        if (RegQueryInfoKeyA(hKey, nullptr, nullptr, nullptr, &subKeyCount, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
            if (subKeyCount == 0) { 
                RegCloseKey(hKey);
                if (RegDeleteKeyA(rootKey, subKey.c_str()) == ERROR_SUCCESS) {
                    return true;
                }
            }
        }
        RegCloseKey(hKey);
    }
    return false;
}

void restartExplorer() {
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnap, &pe32)) {
            do {
                if (_stricmp(pe32.szExeFile, "explorer.exe") == 0) {
                    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
                    if (hProcess) {
                        TerminateProcess(hProcess, 0);
                        CloseHandle(hProcess);
                    }
                }
            } while (Process32Next(hSnap, &pe32));
        }
        CloseHandle(hSnap);
    }
    system("start explorer.exe");
}

int main() {
    std::cout << "StartAllBack Trial Reset Tool" << std::endl;
    const std::string baseKey = R"(Software\Microsoft\Windows\CurrentVersion\Explorer\CLSID)";
    HKEY rootKey = HKEY_CURRENT_USER;
    HKEY hKey;
    if (RegOpenKeyExA(rootKey, baseKey.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        char subKeyName[256];
        DWORD subKeySize = sizeof(subKeyName);
        DWORD index = 0;
        while (RegEnumKeyExA(hKey, index, subKeyName, &subKeySize, nullptr, nullptr, nullptr, nullptr) == ERROR_SUCCESS) {
            std::string fullKey = baseKey + "\\" + subKeyName;
            if (deleteRegistryKey(rootKey, fullKey)) {
                std::cout << "Deleted: " << fullKey << std::endl;
            } else {
                std::cout << "Skipped: " << fullKey << std::endl;
            }
            subKeySize = sizeof(subKeyName);
            index++;
        }
        RegCloseKey(hKey);
    }

    std::cout << "Cleaned Successfully!" << std::endl;
    std::cout << "Restarting Explorer..." << std::endl;
    restartExplorer();

    std::cout << "All DONE!" << std::endl;
    return 0;
}
