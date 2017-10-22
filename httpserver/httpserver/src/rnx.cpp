#include <vector>
#include <unordered_map>

#include "rnx.h"


const std::unordered_map<int, int> TYPE_TABLE{ {SYS_GPS, 1077}, {SYS_GLO, 1087}, {SYS_GAL, 1097}, {SYS_CMP, 1127} };

std::vector<std::vector<obsd_t*>> RawRtcm::obsvec;
obs_t RawRtcm::obs;

RawRtcm::RawRtcm()
{
    m_index = 0;
    init_rtcm(&m_rtcm);
}

RawRtcm::~RawRtcm()
{
    free_rtcm(&m_rtcm);
}

bool RawRtcm::init(const std::string &path)
{
    if (!readrnx(path.c_str(), 0, "", &obs, nullptr, nullptr)) return false;
    {
        obsd_t *obsd = nullptr;
        gtime_t epoch{}, t = obs.data[0].time;
        double interval = 0.;

        std::vector<obsd_t*> epochobs;

        for (size_t i = 0; i != obs.n; ++i)
        {
            obsd = &obs.data[i];
            epoch = obsd->time;

            interval = timediff(epoch, t);
            t = epoch;

            if (abs(interval) < 0.1)
            {
                epochobs.push_back(obsd);
            }

            if (abs(1. - interval) < 0.1 || i == obs.n-1)
            {
                if (epochobs.empty()) continue;
                obsvec.push_back(epochobs);
                epochobs.clear();

                if (i == obs.n-1) break;
                epochobs.push_back(obsd);
            }
        }
    }
    return true;
}

void RawRtcm::unInit()
{
    freeobs(&obs);
}

void RawRtcm::start()
{
    m_index = 0;
    m_epoch = utc2gpst(timeget());
    m_epoch.sec = 0;
}

raw RawRtcm::next_msg()
{
    raw buffer;
    if (m_index == obsvec.size()) m_index = 0;
    gensys(&buffer, &m_rtcm, obsvec[m_index++]);
    buffer.time = m_epoch;
    m_epoch = timeadd(m_epoch, 1);
    return buffer;
}

void RawRtcm::gensys(raw *buffer, rtcm_t *rtcm, const std::vector<obsd_t*> &epochobs)
{
    int sys = SYS_NONE;
    std::unordered_map<int, std::vector<obsd_t*>> epochsys;
    for (const auto &obs : epochobs)
    {
        obs->time = m_epoch;
        sys = satsys(obs->sat, nullptr);
        if (TYPE_TABLE.find(sys) == TYPE_TABLE.end()) continue;
        epochsys[sys].push_back(obs);
    }

    size_t i = 0, sysnum = epochsys.size();
    std::string str;
    int type = 0;
    for (const auto &t : epochsys)
    {
        epoch2rtcm(t.second, rtcm);
        type = TYPE_TABLE.at(t.first);
        gen_rtcm3(rtcm, type, i++ != sysnum - 1);
        buffer->data.push_back(std::move(str.assign(reinterpret_cast<char*>(rtcm->buff), rtcm->nbyte)));
    }
}

void RawRtcm::epoch2rtcm(const std::vector<obsd_t*> &epoch, rtcm_t *rtcm)
{
    size_t i = 0;
    rtcm->obs.n = 0;
    for (const auto &obs : epoch)
    {
        rtcm->obs.data[rtcm->obs.n++] = *obs;
    }
    rtcm->time = m_epoch;
}
