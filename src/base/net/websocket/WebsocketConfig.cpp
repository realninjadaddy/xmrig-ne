/* ninja edition features - start */

#include "WebsocketConfig.h"
#include <iostream>

using namespace xmrig;

void WebsocketConfig::load(const rapidjson::Value &value)
{
    std::cout << "[DEBUG] WebsocketConfig::load() CALLED\n";

    if (!value.IsObject()) {
            std::cout << "[DEBUG] WebsocketConfig::load(): value is NOT object\n";
        return;
    }

    std::cout << "[DEBUG] WebsocketConfig::load(): value is object\n";

    if (value.HasMember("enabled") && value["enabled"].IsBool()) {
        m_enabled = value["enabled"].GetBool();
        std::cout << "[DEBUG] enabled = " << m_enabled << "\n";
    }

    if (value.HasMember("url") && value["url"].IsString()) {
        m_url = value["url"].GetString();
        std::cout << "[DEBUG] url = " << m_url << "\n";
    }

    if (value.HasMember("user") && value["user"].IsString()) {
        m_user = value["user"].GetString();
        std::cout << "[DEBUG] user = " << m_user << "\n";
    }

    if (value.HasMember("secret") && value["secret"].IsString()) {
        m_secret = value["secret"].GetString();
        std::cout << "[DEBUG] secret = (hidden)\n";
    }

}
