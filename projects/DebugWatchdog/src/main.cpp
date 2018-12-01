#include <windows.h>

#include <iostream>
#include <string>
#include <string_view>
#include <thread>

int main(int argc, char * argv[], char * env[])
{
    if (argc < 2) {
        std::cerr << "Usage: DebugWatchdog <application> [arguments...]" << std::endl;
        return -1;
    }

    bool is_console = !(argv[1][0] == 'v' && argv[1][1] == 's');

    int start_command_idx = is_console ? 1 : 2;

    std::string_view temp(argv[start_command_idx]);
    std::wstring command_line(temp.begin(), temp.end());
    for (int i = start_command_idx + 1; i < argc; i++) {
        command_line.push_back(' ');
        temp = std::string_view(argv[i]);
        command_line.append(temp.begin(), temp.end());
    }

    HANDLE write_handle_inherit;
    HANDLE read_handle;

    if (!CreatePipe(&read_handle, &write_handle_inherit, NULL, 0)) {
        auto error = GetLastError();
        std::cerr << "Can't create pipe. [error code: " << error << "]" << std::endl;
        return -1;
    }

    if (!SetHandleInformation(write_handle_inherit, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)) {
        auto error = GetLastError();
        std::cerr << "Can't create pipe. [error code: " << error << "]" << std::endl;
        return -1;
    }

    STARTUPINFO startup_info;
    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startup_info.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    startup_info.hStdError = write_handle_inherit;
    startup_info.dwFlags = STARTF_USESTDHANDLES;

    PROCESS_INFORMATION process_info;
    ZeroMemory(&process_info, sizeof(process_info));

    if (!CreateProcessW(NULL, command_line.data(), NULL, NULL, TRUE, 0, NULL, NULL, &startup_info, &process_info)) {
        auto error = GetLastError();
        std::cerr << "Can't create process. [error code: " << error << "]" << std::endl;
        return -1;
    }

    if (!CloseHandle(write_handle_inherit)) {
        auto error = GetLastError();
        std::cerr << "Can't close handle. [error code: " << error << "]" << std::endl;
        return -1;
    }

    std::thread watchdog([&read_handle,&is_console]() {
        std::byte buffer[1024*8];
        DWORD number_of_bytes_to_read = sizeof(buffer),
              numer_of_bytes_read;

        OutputDebugString(TEXT("========== START ==========\n"));

        while (ReadFile(read_handle, buffer, number_of_bytes_to_read, &numer_of_bytes_read, NULL)) {
            if (numer_of_bytes_read == 0) continue;
            if (is_console) {
                std::string temp(buffer, buffer + numer_of_bytes_read);
                std::cout << temp;
            }
            else {
                std::wstring temp(buffer, buffer + numer_of_bytes_read);
                OutputDebugString(temp.data());
            }
        }

        OutputDebugString(TEXT("\n=========== END ===========\n"));
    });

    WaitForSingleObject(process_info.hProcess, INFINITE);

    watchdog.join();
    CloseHandle(read_handle);
    return 0;
}