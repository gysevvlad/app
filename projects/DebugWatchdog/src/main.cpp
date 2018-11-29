#include <Windows.h>
#include <iostream>
#include <string_view>
#include <thread>

int main(int argc, char * argv[], char * env[])
{
    std::string_view temp(argv[1]);
    std::wstring command_line(temp.begin(), temp.end());
    for (int i = 2; i < argc; i++) {
        command_line.push_back(' ');
        temp = std::string_view(argv[i]);
        command_line.append(temp.begin(), temp.end());
    }

    HANDLE write_handle_inherit;
    HANDLE read_handle;

    if (!CreatePipe(&read_handle, &write_handle_inherit, NULL, 0)) {
        return -1;
    }

    if (!SetHandleInformation(write_handle_inherit, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)) {
        return -1;
    }

    STARTUPINFO startup_info;
    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startup_info.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    startup_info.hStdError = write_handle_inherit;
    startup_info.dwFlags = STARTF_USESTDHANDLES;

    std::thread watchdog([&read_handle]() {
        std::byte buffer[1024*8];
        DWORD number_of_bytes_to_read = sizeof(buffer),
            numer_of_bytes_read;

        OutputDebugString(TEXT("========== START ==========\n"));

        while (ReadFile(read_handle, buffer, number_of_bytes_to_read, &numer_of_bytes_read, NULL)) {
            if (numer_of_bytes_read == 0) break;
            std::wstring temp(buffer, buffer + numer_of_bytes_read);
            OutputDebugString(temp.data());
        }

        OutputDebugString(TEXT("\n=========== END ===========\n"));
    });

    PROCESS_INFORMATION process_info;

    if (!CreateProcess(NULL, command_line.data(), NULL, NULL, TRUE, 0, NULL, NULL, &startup_info, &process_info)) {
        return -1;
    }
    CloseHandle(write_handle_inherit);
    WaitForSingleObject(process_info.hProcess, INFINITE);
    watchdog.join();
    CloseHandle(read_handle);
    return 0;
}