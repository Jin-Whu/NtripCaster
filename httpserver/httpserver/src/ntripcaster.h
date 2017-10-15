#ifndef NTRIPCASTER_H_
#define NTRIPCASTER_H_

#include <future>
#include <string>

#include "httpserver.h"

struct Config
{
    std::string path;
    std::string host;
    unsigned short port;

    Config() = default;

    Config(const Config &cfg)
    {
        this->path = cfg.path;
        this->host = cfg.host;
        this->port = cfg.port;
    }
};

class NtripCaster
{
public:
    explicit NtripCaster(const Config &cfg);
    ~NtripCaster() = default;

public:
    void start();

private:
    static void signal_cb(evutil_socket_t fd, short events, void *arg);

private:
    Config m_cfg;
    HttpServer *m_server;
};

#endif