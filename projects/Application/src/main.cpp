#include <iostream>
#include <memory>
#include <map>
#include <set>
#include <string_view>
#include <Windows.h>
#include <string>
#include <exception>
#include <sstream>
#include <thread>

namespace application::property
{
    std::string_view spawn_count = "spawn";
}

namespace application::option
{
    std::string_view child_process  = "child";
    std::string_view parent_process = "parent";
}

class Application
{
public:
    int run()
    {
        std::stringstream ss;
        ss << m_app_info.data() << "[ status ] run app";
        std::clog << ss.str() << std::endl;
        return 0;
    }

    static std::unique_ptr<Application> create(int argc, char * argv[], char * env[])
    {
        auto app = std::unique_ptr<Application>(new Application());

        app->m_exe_path = argv[0];
        
        {
            std::stringstream ss;
            ss << app->m_app_info.data() << "[  dump  ] run app \"" << app->m_exe_path.data() << "\"";
            std::clog << ss.str() << std::endl;
        }

        for (int i = 1; i < argc; i++) {
            std::string_view arg(argv[i]);
            if (auto pos = arg.find("="); pos != std::string_view::npos) {
                auto [it, _] = app->m_properties.emplace(arg.substr(0, pos), arg.substr(pos + 1));
                {
                    std::stringstream ss;
                    ss << app->m_app_info.data() << "[  dump  ] read property \"" << it->first.data() << "\"=\"" << it->second.data() << "\"";
                    std::clog << ss.str() << std::endl;
                }
            }
            else {
                auto [it, _] = app->m_options.emplace(arg);
                {
                    std::stringstream ss;
                    ss << app->m_app_info.data() << "[  dump  ] read option \"" << it->data() << "\"";
                    std::clog << ss.str() << std::endl;
                }
            }
        }

        if (auto parent = app->m_options.find(application::option::parent_process.data()); parent != app->m_options.end()) {
            // parent process

            if (auto it = app->m_properties.find(application::property::spawn_count.data()); it != app->m_properties.end()) {
                // spawn count
                auto spawn_count = std::stoi(it->second);
                for (int i = 0; i < spawn_count; i++) {
                    app->spawn();
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            else {
                throw std::invalid_argument("set spawn value");
            }
        }
        else if (auto child = app->m_options.find(application::option::child_process.data()); child != app->m_options.end()) {
            // child process
            {
                std::stringstream ss;
                ss << app->m_app_info.data() << "[  info  ] run child process";
                std::clog << ss.str() << std::endl;
            }
        }
        else {
            // error
            throw std::invalid_argument("set command line options: parent or child");
        }
    

        return app;
    }

    int spawn()
    {
        HANDLE write_handle;
        HANDLE read_handle_inherit;

        if (!CreatePipe(&read_handle_inherit, &write_handle, NULL, 0)) {
            //std::clog << m_app_info.data() << "[ error ] CreatePipe(...)" << std::endl;
            return -1;
        }

        auto close_handle_f = [](void * object) { CloseHandle(object); };
        std::unique_ptr<void, decltype(close_handle_f)> close_handle_3(write_handle, close_handle_f);
        std::unique_ptr<void, decltype(close_handle_f)> close_handle_4(read_handle_inherit, close_handle_f);

        if (!SetHandleInformation(read_handle_inherit, HANDLE_FLAG_INHERIT, HANDLE_FLAG_INHERIT)) {
            //std::clog << m_app_info.data() << "[ error ] SetHandleInformation(...)" << std::endl;
            return -1;
        }

        STARTUPINFO startup_info;
        ZeroMemory(&startup_info, sizeof(startup_info));
        startup_info.hStdInput = read_handle_inherit;
        startup_info.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
        startup_info.hStdError = GetStdHandle(STD_ERROR_HANDLE);
        startup_info.dwFlags = STARTF_USESTDHANDLES;

        PROCESS_INFORMATION process_info;
        
        std::wstring argv(m_exe_path.begin(), m_exe_path.end());
        argv.push_back(' ');
        argv.append(application::option::child_process.begin(), application::option::child_process.end());

        if (!CreateProcess(NULL, argv.data(), NULL, NULL, TRUE, 0, NULL, NULL, &startup_info, &process_info)) {
            std::clog << m_app_info.data() << "[ error ] CreateProcess(...)" << std::endl;
            return -1;
        }

        {
            std::stringstream ss;
            ss << m_app_info.data() << "[  info  ] created process " << process_info.dwProcessId << ':' << process_info.dwThreadId;
            std::clog << ss.str() << std::endl;
        }

        return 0;
    }

    const std::map<std::string, std::string> & getProperties() const
    {
        return m_properties;
    }

    ~Application()
    {
    }

private:
    Application() noexcept
    {
        m_app_info = "[" + std::to_string(GetCurrentProcessId()) + ":" + std::to_string(GetCurrentThreadId()) + "]";
    }

    std::map<std::string, std::string> m_properties;
    std::set<std::string> m_options;
    std::string m_exe_path;
    std::string m_app_info;
};

int main(int argc, char * argv[], char * env[])
{
    auto app = Application::create(argc, argv, env);
    return app->run();
}