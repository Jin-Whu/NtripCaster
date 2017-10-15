#ifndef RNX_H_
#define RNX_H_

#include <rtklib/rtklib.h>
#include <string>
#include <vector>

struct raw
{
    std::vector<std::string> data;
    gtime_t time;
};

class RawRtcm
{
public:
    RawRtcm();
    ~RawRtcm();

public:
    static bool init(const std::string &path);
    static void unInit();
    void start();
    raw next_msg();

private:
    void epoch2rtcm(const std::vector<obsd_t*> &epoch, rtcm_t *rtcm);
    void gensys(raw *buffer, rtcm_t *rtcm, const std::vector<obsd_t*> &epochobs);

private:
    int m_index;
    gtime_t m_epoch;
    rtcm_t m_rtcm;
    static obs_t obs;
    static std::vector<std::vector<obsd_t*>> obsvec;
};

#endif