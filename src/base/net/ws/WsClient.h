#pragma once

#include <string>

namespace xmrig {

class WsClient
{
public:
    static void start(const std::string &url, const std::string &secret, const std::string &rigId);
    static void send(const std::string &msg);
    static void stop();
};

} // namespace xmrig
