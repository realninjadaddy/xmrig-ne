#pragma once
#include <memory>
#include <nlohmann/json.hpp>

namespace xmrig {

class Controller;

class WebsocketCommandHandler {
public:
    explicit WebsocketCommandHandler(std::shared_ptr<Controller> controller);
    ~WebsocketCommandHandler();

    void handleArgs(const nlohmann::json &args);

private:
    std::shared_ptr<Controller> m_controller;
};

} // namespace xmrig