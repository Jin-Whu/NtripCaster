#ifndef HTTPSERVER_H_
#define HTTPSERVER_H_

#include <future>

#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>

#include "rnx.h"


struct chunk_req_state
{
    event_base *base;
    evhttp_request *req;
    event *timer;
    RawRtcm rtcm;
};


class HttpServer
{
public:
    HttpServer(const std::string &host, unsigned short port);
    ~HttpServer() = default;

public:
    void start();
    void stop();

private:
    void threadprocess();
    static void http_chunked_cb(evhttp_request *req, void *arg);
    static void http_close_cb(evhttp_connection *evconn, void *arg);
    static void schedule_trickle(chunk_req_state *state, int ms);
    static void http_chunked_trikle_cb(evutil_socket_t fd, short events, void *arg);
    static void http_timer(evutil_socket_t fd, short events, void *arg);

private:
    std::string m_host;
    unsigned short m_port;
    std::future<void> m_future;
    static bool m_bStop;
};

#endif