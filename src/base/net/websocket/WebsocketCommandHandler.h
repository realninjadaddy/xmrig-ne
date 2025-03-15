#pragma once
#include <nlohmann/json.hpp>

namespace xmrig {

class Controller;

class WebsocketCommandHandler {
public:
    explicit WebsocketCommandHandler(Controller *controller);
    ~WebsocketCommandHandler();

    void handleArgs(const nlohmann::json &args);

private:
    Controller *m_controller;
};

} // namespace xmrig
