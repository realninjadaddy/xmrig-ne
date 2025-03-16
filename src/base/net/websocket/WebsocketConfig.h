/* XMRig
 * Copyright 2025      NinjaDaddy https://github.com/realninjadaddy/xmrig-ne
 *
 *   This file is part of XMRig and is licensed under the GNU General Public License v3 (GPLv3).
 *   You can find a copy of the license at <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <string>
#include <rapidjson/document.h>

namespace xmrig {

class WebsocketConfig
{
public:
    WebsocketConfig() = default;

    inline const std::string &url() const     { return m_url; }
    inline const std::string &user() const    { return m_user; }
    inline const std::string &secret() const  { return m_secret; }
    inline bool isEnabled() const             { return m_enabled; }

    void setUrl(const std::string &url)       { m_url = url; }
    void setUser(const std::string &user)     { m_user = user; }
    void setSecret(const std::string &secret) { m_secret = secret; }
    void setEnabled(bool enabled)             { m_enabled = enabled; }
    rapidjson::Value toJSON(rapidjson::MemoryPoolAllocator<rapidjson::CrtAllocator> &allocator) const;

    void load(const rapidjson::Value &value);

private:
    std::string m_url;
    std::string m_user;
    std::string m_secret;
    bool m_enabled = false;
};

} // namespace xmrig
