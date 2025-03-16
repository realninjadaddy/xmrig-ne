/* XMRig
 * Copyright 2025      NinjaDaddy https://github.com/realninjadaddy/xmrig-ne
 *
 *   This file is part of XMRig and is licensed under the GNU General Public License v3 (GPLv3).
 *   You can find a copy of the license at <http://www.gnu.org/licenses/>.
 */

#include "WebsocketConfig.h"
#include <iostream>

using namespace xmrig;

void WebsocketConfig::load(const rapidjson::Value &value)
{
    if (!value.IsObject()) {
            //std::cout << "[DEBUG] WebsocketConfig::load(): value is NOT object\n";
        return;
    }


    if (value.HasMember("enabled") && value["enabled"].IsBool()) {
        m_enabled = value["enabled"].GetBool();
    }

    if (value.HasMember("url") && value["url"].IsString()) {
        m_url = value["url"].GetString();
    }

    if (value.HasMember("user") && value["user"].IsString()) {
        m_user = value["user"].GetString();
    }

    if (value.HasMember("secret") && value["secret"].IsString()) {
        m_secret = value["secret"].GetString();
    }

}

rapidjson::Value WebsocketConfig::toJSON(rapidjson::Document::AllocatorType &allocator) const {
    rapidjson::Value obj(rapidjson::kObjectType);

    obj.AddMember("enabled", m_enabled, allocator);
    
    if (!m_url.empty()) {
        obj.AddMember("url", rapidjson::Value(m_url.c_str(), allocator), allocator);
    }

    if (!m_user.empty()) {
        obj.AddMember("user", rapidjson::Value(m_user.c_str(), allocator), allocator);
    }

    if (!m_secret.empty()) {
        obj.AddMember("secret", rapidjson::Value(m_secret.c_str(), allocator), allocator);
    }

    return obj;
}