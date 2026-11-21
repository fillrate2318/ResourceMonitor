#include "ProcessListReader.h"

#include <windows.h>
#include <tlhelp32.h>
#include <system_error>

std::vector<ProcessInfo> readProcesses() {
    class SnapshotHandle {
    public:
        explicit SnapshotHandle(HANDLE handle) noexcept : handle_(handle) {}
        SnapshotHandle(const SnapshotHandle&) = delete;
        SnapshotHandle& operator=(const SnapshotHandle&) = delete;

        ~SnapshotHandle() {
            CloseHandle(handle_);
        }

        HANDLE get() const noexcept { return handle_; }

    private:
        HANDLE handle_;
    };

    const auto handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (handle == INVALID_HANDLE_VALUE) {
        throw std::system_error(GetLastError(), std::system_category(),
            "CreateToolhelp32Snapshot failed");
    }
    const SnapshotHandle snapshot{handle};

    PROCESSENTRY32W process{};
    process.dwSize = sizeof(process);

    if (!Process32FirstW(snapshot.get(), &process)) {
        const auto error = GetLastError();
        if (error == ERROR_NO_MORE_FILES) {
            return {};
        }
        throw std::system_error(error, std::system_category(), "Process32FirstW failed");
    }

    std::vector<ProcessInfo> processes;
    do {
        processes.push_back({process.th32ProcessID,
            std::wstring{process.szExeFile}, process.cntThreads});
    } while (Process32NextW(snapshot.get(), &process));

    const auto error = GetLastError();
    if (error != ERROR_NO_MORE_FILES) {
        throw std::system_error(error, std::system_category(),
            "Process32NextW failed");
    }

    return processes;
}
