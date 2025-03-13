/* ninja edition features - start */

#include "WebsocketConfig.h"

using namespace xmrig;

void WebsocketConfig::load(const rapidjson::Value &value)
{
    if (!value.IsObject()) {
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
