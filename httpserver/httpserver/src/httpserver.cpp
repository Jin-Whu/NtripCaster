#include "httpserver.h"

bool HttpServer::m_bStop{ true };


HttpServer::HttpServer(const std::string &host, unsigned short port) :m_host{ host }, m_port{ port } 
{
}


void HttpServer::start()
{
    if (!m_bStop)
        return;
    m_bStop = false;

    m_future = std::async(std::launch::async, std::bind(&HttpServer::threadprocess, this));
}


void HttpServer::stop()
{
    m_bStop = true;

    if (m_future.valid())
        m_future.wait();
}


void HttpServer::threadprocess()
{
    event_base *evbase = nullptr;
    evhttp *http = nullptr;
    evhttp_bound_socket *handler = nullptr;

    evbase = event_base_new();
    if (!evbase)
        return;

    http = evhttp_new(evbase);
    if (!http)
        return;

    evhttp_set_gencb(http, http_chunked_cb, evbase);

    handler = evhttp_bind_socket_with_handle(http, m_host.c_str(), m_port);
    event *timer = evtimer_new(evbase, &HttpServer::http_timer, event_self_cbarg());
    timeval tv{ 1, 0 };
    evtimer_add(timer, &tv);

    if (!handler)
        return;

    event_base_dispatch(evbase);
    evhttp_free(http);
    event_free(timer);
    event_base_free(evbase);
}

void HttpServer::http_chunked_cb(evhttp_request *req, void *arg)
{
    event_base *evbase = reinterpret_cast<event_base*>(arg);
    chunk_req_state *state = new chunk_req_state{};
    evhttp_connection * evconn = evhttp_request_get_connection(req);
    evhttp_connection_set_closecb(evconn, &HttpServer::http_close_cb, state);
    state->req = req;
    state->timer = evtimer_new(evbase, http_chunked_trikle_cb, state);
    state->rtcm.start();
    evhttp_send_reply_start(req, HTTP_OK, "ICY 200 OK");
    schedule_trickle(state, 0);
}

void HttpServer::http_close_cb(evhttp_connection *evconn, void *arg)
{
    chunk_req_state *state = reinterpret_cast<chunk_req_state*>(arg);
    if (!m_bStop) evhttp_request_free(state->req);
    event_free(state->timer);
    delete state;
}


void HttpServer::http_chunked_trikle_cb(evutil_socket_t fd, short events, void *arg)
{
    chunk_req_state *state = reinterpret_cast<chunk_req_state*>(arg);
    evbuffer *evb = evbuffer_new();

    raw msg = state->rtcm.next_msg();
    for (const auto &buff : msg.data)
    {
        evbuffer_add(evb, buff.c_str(), buff.length());
        evhttp_send_reply_chunk(state->req, evb);
    }
    evbuffer_free(evb);

    if (!m_bStop)
    {
        gtime_t t = utc2gpst(timeget());
        double delta = timediff(t, msg.time);
        int interval = (1 - timediff(t, msg.time)) * 1000;
        schedule_trickle(state, interval);
    }
    else
    {
        evhttp_send_reply_end(state->req);
    }
}

void HttpServer::schedule_trickle(chunk_req_state *state, int ms)
{
    timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = ms * 1000;
    evtimer_add(state->timer, &tv);
}

void HttpServer::http_timer(evutil_socket_t fd, short events, void *arg)
{
    event *timer = reinterpret_cast<event*>(arg);
    if (!m_bStop)
    {
        timeval tv{ 1, 0 };
        evtimer_add(timer, &tv);
    }
    else
    {
        event_base *evbase = event_get_base(timer);
        event_base_loopbreak(evbase);
    }
}