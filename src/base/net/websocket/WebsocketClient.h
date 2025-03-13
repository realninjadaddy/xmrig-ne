#pragma once

#include <string>
#include <iostream>

namespace xmrig {

class WebsocketClient {
public:
    WebsocketClient(const std::string &url,
                    const std::string &user = {},
                    const std::string &secret = {})
        : m_url(url), m_user(user), m_secret(secret) {}

    void connect() {
        std::cout << "[WS] Connecting to: " << m_url << "\n";
        if (!m_user.empty()) {
            std::cout << "[WS] Using user: " << m_user << "\n";
        }
        if (!m_secret.empty()) {
            std::cout << "[WS] Using secret (hidden)\n";
        }
        std::cout << "[WS] (stub) WebSocket connection would happen here.\n";
    }

private:
    std::string m_url;
    std::string m_user;
    std::string m_secret;
};

} // namespace xmrig