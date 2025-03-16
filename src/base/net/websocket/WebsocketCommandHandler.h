/* XMRig
 * Copyright 2025      NinjaDaddy https://github.com/realninjadaddy/xmrig-ne
 *
 *   This file is part of XMRig and is licensed under the GNU General Public License v3 (GPLv3).
 *   You can find a copy of the license at <http://www.gnu.org/licenses/>.
 */
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