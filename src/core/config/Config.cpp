/* XMRig
 * Copyright (c) 2018-2021 SChernykh   <https://github.com/SChernykh>
 * Copyright (c) 2016-2021 XMRig       <https://github.com/xmrig>, <support@xmrig.com>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <cinttypes>
#include <cstring>
#include <uv.h>


#include "core/config/Config.h"
#include "3rdparty/rapidjson/document.h"
#include "backend/cpu/Cpu.h"
#include "base/io/log/Log.h"
#include "base/kernel/interfaces/IJsonReader.h"
#include "base/net/dns/Dns.h"
#include "base/kernel/config/BaseTransform.h"
#include <rapidjson/document.h>
#include "crypto/common/Assembly.h"


#ifdef XMRIG_ALGO_RANDOMX
#   include "crypto/rx/RxConfig.h"
#endif


#ifdef XMRIG_FEATURE_OPENCL
#   include "backend/opencl/OclConfig.h"
#endif


#ifdef XMRIG_FEATURE_CUDA
#   include "backend/cuda/CudaConfig.h"
#endif


namespace xmrig {


constexpr static uint32_t kIdleTime     = 60U;


const char *Config::kPauseOnBattery     = "pause-on-battery";
const char *Config::kPauseOnActive      = "pause-on-active";


#ifdef XMRIG_FEATURE_OPENCL
const char *Config::kOcl                = "opencl";
#endif

#ifdef XMRIG_FEATURE_CUDA
const char *Config::kCuda               = "cuda";
#endif

#if defined(XMRIG_FEATURE_NVML) || defined (XMRIG_FEATURE_ADL)
const char *Config::kHealthPrintTime    = "health-print-time";
#endif

#ifdef XMRIG_FEATURE_DMI
const char *Config::kDMI                = "dmi";
#endif


class ConfigPrivate
{
public:
    bool pauseOnBattery = false;
    CpuConfig cpu;
    uint32_t idleTime   = 0;

#   ifdef XMRIG_ALGO_RANDOMX
    RxConfig rx;
#   endif

#   ifdef XMRIG_FEATURE_OPENCL
    OclConfig cl;
#   endif

#   ifdef XMRIG_FEATURE_CUDA
    CudaConfig cuda;
#   endif

#   if defined(XMRIG_FEATURE_NVML) || defined (XMRIG_FEATURE_ADL)
    uint32_t healthPrintTime = 60U;
#   endif

#   ifdef XMRIG_FEATURE_DMI
    bool dmi = true;
#   endif

    void setIdleTime(const rapidjson::Value &value)
    {
        if (value.IsBool()) {
            idleTime = value.GetBool() ? kIdleTime : 0U;
        }
        else if (value.IsUint()) {
            idleTime = value.GetUint();
        }
    }
};

} // namespace xmrig


xmrig::Config::Config() :
    d_ptr(new ConfigPrivate())
{
}


xmrig::Config::~Config()
{
    delete d_ptr;
}


bool xmrig::Config::isPauseOnBattery() const
{
    return d_ptr->pauseOnBattery;
}


const xmrig::CpuConfig &xmrig::Config::cpu() const
{
    return d_ptr->cpu;
}


uint32_t xmrig::Config::idleTime() const
{
    return d_ptr->idleTime * 1000U;
}


#ifdef XMRIG_FEATURE_OPENCL
const xmrig::OclConfig &xmrig::Config::cl() const
{
    return d_ptr->cl;
}
#endif


#ifdef XMRIG_FEATURE_CUDA
const xmrig::CudaConfig &xmrig::Config::cuda() const
{
    return d_ptr->cuda;
}
#endif


#ifdef XMRIG_ALGO_RANDOMX
const xmrig::RxConfig &xmrig::Config::rx() const
{
    return d_ptr->rx;
}
#endif


#if defined(XMRIG_FEATURE_NVML) || defined (XMRIG_FEATURE_ADL)
uint32_t xmrig::Config::healthPrintTime() const
{
    return d_ptr->healthPrintTime;
}
#endif


#ifdef XMRIG_FEATURE_DMI
bool xmrig::Config::isDMI() const
{
    return d_ptr->dmi;
}
#endif


bool xmrig::Config::isShouldSave() const
{
    if (!isAutoSave()) {
        return false;
    }

#   ifdef XMRIG_FEATURE_OPENCL
    if (cl().isShouldSave()) {
        return true;
    }
#   endif

#   ifdef XMRIG_FEATURE_CUDA
    if (cuda().isShouldSave()) {
        return true;
    }
#   endif

    return (m_upgrade || cpu().isShouldSave());
}


bool xmrig::Config::read(const IJsonReader &reader, const char *fileName)
{

    if (!BaseConfig::read(reader, fileName)) {
        return false;
    }

    d_ptr->pauseOnBattery = reader.getBool(kPauseOnBattery, d_ptr->pauseOnBattery);
    d_ptr->setIdleTime(reader.getValue(kPauseOnActive));

    d_ptr->cpu.read(reader.getValue(CpuConfig::kField));

#   ifdef XMRIG_ALGO_RANDOMX
    if (!d_ptr->rx.read(reader.getValue(RxConfig::kField))) {
        m_upgrade = true;
    }
#   endif

#   ifdef XMRIG_FEATURE_OPENCL
    if (!pools().isBenchmark()) {
        d_ptr->cl.read(reader.getValue(kOcl));
    }
#   endif

#   ifdef XMRIG_FEATURE_CUDA
    if (!pools().isBenchmark()) {
        d_ptr->cuda.read(reader.getValue(kCuda));
    }
#   endif

#   if defined(XMRIG_FEATURE_NVML) || defined (XMRIG_FEATURE_ADL)
    d_ptr->healthPrintTime = reader.getUint(kHealthPrintTime, d_ptr->healthPrintTime);
#   endif

#   ifdef XMRIG_FEATURE_DMI
    d_ptr->dmi = reader.getBool(kDMI, d_ptr->dmi);
#   endif

    return true;
}


void xmrig::Config::getJSON(rapidjson::Document &doc) const
{
    using namespace rapidjson;

    doc.SetObject();

    auto &allocator = doc.GetAllocator();

    Value api(kObjectType);
    api.AddMember(StringRef(kApiId),                    m_apiId.toJSON(), allocator);
    api.AddMember(StringRef(kApiWorkerId),              m_apiWorkerId.toJSON(), allocator);

    doc.AddMember(StringRef(kApi),                      api, allocator);
    doc.AddMember(StringRef(kHttp),                     m_http.toJSON(doc), allocator);
    doc.AddMember(StringRef(kAutosave),                 isAutoSave(), allocator);
    doc.AddMember(StringRef(kBackground),               isBackground(), allocator);
    doc.AddMember(StringRef(kColors),                   Log::isColors(), allocator);
    doc.AddMember(StringRef(kTitle),                    title().toJSON(), allocator);

#   ifdef XMRIG_ALGO_RANDOMX
    doc.AddMember(StringRef(RxConfig::kField),          rx().toJSON(doc), allocator);
#   endif

    doc.AddMember(StringRef(CpuConfig::kField),         cpu().toJSON(doc), allocator);

#   ifdef XMRIG_FEATURE_OPENCL
    doc.AddMember(StringRef(kOcl),                      cl().toJSON(doc), allocator);
#   endif

#   ifdef XMRIG_FEATURE_CUDA
    doc.AddMember(StringRef(kCuda),                     cuda().toJSON(doc), allocator);
#   endif

    doc.AddMember(StringRef(kLogFile),                  m_logFile.toJSON(), allocator);

    m_pools.toJSON(doc, doc);

    doc.AddMember(StringRef(kPrintTime),                printTime(), allocator);
#   if defined(XMRIG_FEATURE_NVML) || defined (XMRIG_FEATURE_ADL)
    doc.AddMember(StringRef(kHealthPrintTime),          healthPrintTime(), allocator);
#   endif

#   ifdef XMRIG_FEATURE_DMI
    doc.AddMember(StringRef(kDMI),                      isDMI(), allocator);
#   endif

    doc.AddMember(StringRef(kSyslog),                   isSyslog(), allocator);

#   ifdef XMRIG_FEATURE_TLS
    doc.AddMember(StringRef(kTls),                      m_tls.toJSON(doc), allocator);
#   endif

    doc.AddMember(StringRef(DnsConfig::kField),         Dns::config().toJSON(doc), allocator);
    doc.AddMember(StringRef(kUserAgent),                m_userAgent.toJSON(), allocator);
    doc.AddMember(StringRef(kVerbose),                  Log::verbose(), allocator);
    doc.AddMember(StringRef(kWatch),                    m_watch, allocator);
    doc.AddMember(StringRef(kPauseOnBattery),           isPauseOnBattery(), allocator);
    doc.AddMember(StringRef(kPauseOnActive),            (d_ptr->idleTime == 0U || d_ptr->idleTime == kIdleTime) ? Value(isPauseOnActive()) : Value(d_ptr->idleTime), allocator);
}

class DocReader : public IJsonReader {
public:
    explicit DocReader(rapidjson::Document &doc) : m_doc(doc) {}

    bool getBool(const char *key, bool defaultValue = false) const override {
        const auto &v = getValue(key);
        return v.IsBool() ? v.GetBool() : defaultValue;
    }

    bool isEmpty() const override {
        return m_doc.ObjectEmpty();
    }

    const char *getString(const char *key, const char *defaultValue = nullptr) const override {
        const auto &v = getValue(key);
        return v.IsString() ? v.GetString() : defaultValue;
    }

    const rapidjson::Value &getArray(const char *key) const override {
        return getValue(key);  // forudsat det er et array
    }

    const rapidjson::Value &getObject(const char *key) const override {
        return getValue(key);  // forudsat det er et object
    }

    const rapidjson::Value &getValue(const char *key) const override {
        if (!key || !m_doc.HasMember(key)) {
            static rapidjson::Value nullValue;
            return nullValue;
        }
        return m_doc[key];
    }

    const rapidjson::Value &object() const override {
        return m_doc;
    }

    double getDouble(const char *key, double defaultValue = 0) const override {
        const auto &v = getValue(key);
        return v.IsDouble() ? v.GetDouble() : defaultValue;
    }

    int getInt(const char *key, int defaultValue = 0) const override {
        const auto &v = getValue(key);
        return v.IsInt() ? v.GetInt() : defaultValue;
    }

    int64_t getInt64(const char *key, int64_t defaultValue = 0) const override {
        const auto &v = getValue(key);
        return v.IsInt64() ? v.GetInt64() : defaultValue;
    }

    xmrig::String getString(const char *key, size_t maxSize) const override {
        const auto &v = getValue(key);
        return (v.IsString() && v.GetStringLength() <= maxSize) ? xmrig::String(v.GetString()) : xmrig::String();
    }

    uint64_t getUint64(const char *key, uint64_t defaultValue = 0) const override {
        const auto &v = getValue(key);
        return v.IsUint64() ? v.GetUint64() : defaultValue;
    }

    unsigned getUint(const char *key, unsigned defaultValue = 0) const override {
        const auto &v = getValue(key);
        return v.IsUint() ? v.GetUint() : defaultValue;
    }

private:
    rapidjson::Document &m_doc;
};


bool xmrig::Config::read(const IJsonReader &reader, const char *fileName, BaseTransform &transform)
{
    // Brug getValue(nullptr) til at få hele JSON-rooten
    const auto& root = reader.getValue(nullptr);

    // Opret ny Document og kopiér indholdet fra root
    rapidjson::Document doc;
    doc.CopyFrom(reader.getValue(nullptr), doc.GetAllocator());

    static_cast<BaseTransform &>(transform).finalize(doc);

    DocReader json(doc);
    return read(json, fileName);


}

