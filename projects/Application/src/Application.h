#pragma once

#include <memory>

class Application
{
public:
    Application(int argc, char * argv[], char * env[]) : m_app(create_app(argc, argv, env))
    {
    }

    int run()
    {
        return run_app(m_app.get());
    }

private:
    static void * create_app(int argc, char * argv[], char * env[]);
    static int run_app(void *);
    static void destroy_app(void *);

    struct deleter {
        void operator()(void * ptr) const {
            destroy_app(ptr);
        }
    };

    std::unique_ptr<void, deleter> m_app;
};