#include <windows.h>

#include <string>
#include <sstream>
#include <iostream>
#include <vector>

#include "Application.h"

std::string g_log_prefix;

#define LOG_MESSAGE( prefix, level, message )       \
{                                                   \
    std::stringstream ss;                           \
    ss << prefix << level << message << std::endl;  \
    std::cerr << ss.str();                          \
}

#define   INFO_LOG_MESSAGE( message ) LOG_MESSAGE( g_log_prefix, "[   info ] ", message )
#define STATUS_LOG_MESSAGE( message ) LOG_MESSAGE( g_log_prefix, "[ status ] ", message )
#define  ERROR_LOG_MESSAGE( message ) LOG_MESSAGE( g_log_prefix, "[  error ] ", message )
#define   DUMP_LOG_MESSAGE( message ) LOG_MESSAGE( g_log_prefix, "[   dump ] ", message )

HANDLE stupid_fork()
{
    HANDLE write_handle;
    HANDLE read_handle_inherit;

    if (!CreatePipe(&read_handle_inherit, &write_handle, NULL, 0)) {
        ERROR_LOG_MESSAGE("can't create pipe");
        return INVALID_HANDLE_VALUE;
    }

    if (!SetHandleInformation(read_handle_inherit, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)) {
        CloseHandle(write_handle);
        CloseHandle(read_handle_inherit);
        ERROR_LOG_MESSAGE("SetHandleInformation(...)");
        return INVALID_HANDLE_VALUE;
    }

    STARTUPINFO startup_info;
    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.hStdInput = read_handle_inherit;
    startup_info.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    startup_info.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    startup_info.dwFlags = STARTF_USESTDHANDLES;

    PROCESS_INFORMATION process_info;

    int argc;
    wchar_t ** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    std::wstring command_line(argv[0]);

    for (int i = 1; i < argc; ++i) {
        if (std::wstring temp = argv[i]; temp.find(L"spawn=") == std::wstring::npos) {
            command_line.push_back(' ');
            command_line.append(argv[i]);
        }
    }

    LocalFree(argv);
    
    std::string temp(command_line.begin(), command_line.end());
    DUMP_LOG_MESSAGE("try create process \"" << temp << "\"");
    
    if (!CreateProcess(NULL, command_line.data(), NULL, NULL, TRUE, 0, NULL, NULL, &startup_info, &process_info)) {
        CloseHandle(write_handle);
        CloseHandle(read_handle_inherit);
        ERROR_LOG_MESSAGE("CreateProcess(...)");
        return INVALID_HANDLE_VALUE;
    }

    CloseHandle(read_handle_inherit);

    INFO_LOG_MESSAGE("created process " << process_info.dwProcessId << ':' << process_info.dwThreadId);
    return write_handle;
}

class ProcessSpawnApp
{
public:
    ProcessSpawnApp(int argc, char * argv[], char * env[]) :
        m_argc(argc),
        m_argv(argv),
        m_env(env),
        m_app_info("[" + std::to_string(GetCurrentProcessId()) + ":" + std::to_string(GetCurrentThreadId()) + "]")
    {
        g_log_prefix = m_app_info;
        DUMP_LOG_MESSAGE("create app instance");

        for (int i = 0; i < argc; i++) {
            std::string arg(argv[i]);
            if (auto pos = arg.find("spawn="); pos != std::string::npos) {
                int spawn_count = std::stoi(arg.substr(6));
                DUMP_LOG_MESSAGE("detected spawn option");
                for (int j = 0; j < spawn_count; j++) {
                    m_channels.emplace_back(INVALID_HANDLE_VALUE, stupid_fork());
                }
                break;
            }
        }
    }

    int run()
    {
        return 0;
    }

    ~ProcessSpawnApp()
    {
        DUMP_LOG_MESSAGE("delete app instance");
    }

private:
    int m_argc;
    char ** m_argv;
    char ** m_env;

    std::string m_app_info;

    std::vector<std::pair<HANDLE, HANDLE>> m_channels;
};

void * Application::create_app(int argc, char * argv[], char * env[])
{
    return new ProcessSpawnApp(argc, argv, env);
}

int Application::run_app(void * app_instance)
{
    return static_cast<ProcessSpawnApp*>(app_instance)->run();
}

void Application::destroy_app(void * app_instance)
{
    delete static_cast<ProcessSpawnApp*>(app_instance);
}