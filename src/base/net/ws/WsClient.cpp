#include "WsClient.h"
#include <iostream>

void xmrig::WsClient::start(const std::string &, const std::string &, const std::string &)
{
    std::cout << "[WsClient] dummy start" << std::endl;
}

void xmrig::WsClient::send(const std::string &) {}

void xmrig::WsClient::stop() {}
