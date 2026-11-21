#include "ProcessListReader.h"

#include <windows.h>
#include <tlhelp32.h>
#include <system_error>

std::vector<ProcessInfo> readProcesses() {
    std::vector<ProcessInfo> processes;

    class HandleWrapper {
    public:
        HANDLE handle_ = INVALID_HANDLE_VALUE;
        ~HandleWrapper() {
            if (handle_ != INVALID_HANDLE_VALUE) {
                CloseHandle(handle_);
            }
        }
    };

    HandleWrapper handleWrapper;
    handleWrapper.handle_ = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (handleWrapper.handle_ == INVALID_HANDLE_VALUE) {
        throw std::system_error(GetLastError(), std::system_category(),
            "CreateToolhelp32Snapshot failed");
    }

    PROCESSENTRY32 process;
    process.dwSize = sizeof(process);

    if (!Process32FirstW(handleWrapper.handle_, &process)) {
        throw std::system_error(GetLastError(), std::system_category(),
            "Process32FirstW failed");
    }

    do {
        processes.push_back({process.th32ProcessID,
            std::wstring{process.szExeFile}, process.cntThreads});
    }
    while (Process32NextW(handleWrapper.handle_, &process));

    return processes;
};