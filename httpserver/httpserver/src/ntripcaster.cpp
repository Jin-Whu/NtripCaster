#include <csignal>
#include "ntripcaster.h"


NtripCaster::NtripCaster(const Config &cfg) :m_cfg{ cfg }, m_server{nullptr}
{
}

void NtripCaster::start()
{
    if (!RawRtcm::init(m_cfg.path))
        return;

    m_server = new HttpServer{ m_cfg.host, m_cfg.port };

#ifdef _WIN32
    WORD version = MAKEWORD(2, 2);
    WSADATA data;
    if (WSAStartup(version, &data))
        return;
#endif

    event_base *base = nullptr;
    base = event_base_new();
    if (!base)
        return;

    event *sighandler = evsignal_new(base, SIGINT, &NtripCaster::signal_cb, event_self_cbarg());
    evsignal_add(sighandler, nullptr);

    m_server->start();

    event_base_dispatch(base);
    event_free(sighandler);
    event_base_free(base);

    m_server->stop();
    delete m_server;

    RawRtcm::unInit();

#ifdef _WIN32
    WSACleanup();
#endif
}

void NtripCaster::signal_cb(evutil_socket_t fd, short events, void *arg)
{
    event *sighandler = reinterpret_cast<event*>(arg);
    event_del(sighandler);
}