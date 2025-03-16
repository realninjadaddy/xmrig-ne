#include "base/net/websocket/WebsocketCommandHandler.h"
#include "core/Controller.h"
#include "base/io/log/Log.h"
#include "3rdparty/rapidjson/stringbuffer.h"
#include "3rdparty/rapidjson/writer.h"

using json = nlohmann::json;  // 👈 Denne linje tilføjes her

namespace xmrig {

WebsocketCommandHandler::WebsocketCommandHandler(std::shared_ptr<Controller> controller)
    : m_controller(std::move(controller))
{}

WebsocketCommandHandler::~WebsocketCommandHandler() = default;

void WebsocketCommandHandler::handleArgs(const json &args)
{
    if (!m_controller) {
        return;
    }

    if (args.contains("type") && args["type"] == "set_ws_args") {
        rapidjson::Document fullConfig;
        m_controller->config()->getJSON(fullConfig);  // 💯 korrekt og komplet

        auto& allocator = fullConfig.GetAllocator();

        //LOG_INFO("Has cpu: %s", fullConfig.HasMember("cpu") ? "yes" : "no");
        //LOG_INFO("Has randomx: %s", fullConfig.HasMember("randomx") ? "yes" : "no");

        rapidjson::Document originalConfig;
        originalConfig.CopyFrom(fullConfig, fullConfig.GetAllocator());

        if (args.contains("ws_args") && args["ws_args"].is_object()) {
            if (args["ws_args"].empty()) {
                LOG_INFO(WHITE_ON_GREY(" socket  ") "Received empty ws_args – skipping reload.");
                return;
            }

            //LOG_INFO(WHITE_ON_GREY(" socket  "), "Received ws_args:");

            for (const auto& item : args["ws_args"].items()) {
                const auto& key = item.key();
                const auto& value = item.value();

                if (key == "--url" || key == "-o") {
                    if (value.is_string() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        auto& pools = fullConfig["pools"];
                        if (!pools.Empty()) {
                            pools[0]["url"].SetString(value.get<std::string>().c_str(), fullConfig.GetAllocator());
                        }
                    }
                } 
                else if (key == "--user" || key == "-u") {
                    if (value.is_string() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        auto& pools = fullConfig["pools"];
                        if (!pools.Empty()) {
                            pools[0]["user"].SetString(value.get<std::string>().c_str(), fullConfig.GetAllocator());
                        }
                    }
                } 
                else if (key == "--algo" || key == "-a") {
                    if (value.is_string() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        auto& pools = fullConfig["pools"];
                        if (!pools.Empty()) {
                            pools[0]["algo"].SetString(value.get<std::string>().c_str(), fullConfig.GetAllocator());
                        }
                    }
                } 
                else if (key == "--pass" || key == "-p") {
                    if (value.is_string() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        auto& pools = fullConfig["pools"];
                        if (!pools.Empty()) {
                            pools[0]["pass"].SetString(value.get<std::string>().c_str(), fullConfig.GetAllocator());
                        }
                    }
                } 
                else if (key == "--tls") {
                    if (value.is_boolean() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        auto& pools = fullConfig["pools"];
                        if (!pools.Empty()) {
                            pools[0]["tls"].SetBool(value.get<bool>());
                        }
                    }
                } 
                else if (key == "--tls-fingerprint") {
                    if (value.is_string() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        auto& pools = fullConfig["pools"];
                        if (!pools.Empty()) {
                            pools[0]["tls-fingerprint"].SetString(value.get<std::string>().c_str(), fullConfig.GetAllocator());
                        }
                    }
                } 
                else if (key == "--coin") {
                    if (value.is_string() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        auto& pools = fullConfig["pools"];
                        if (!pools.Empty()) {
                            pools[0]["coin"].SetString(value.get<std::string>().c_str(), fullConfig.GetAllocator());
                        }
                    }
                } 
                else if (key == "--rig-id") {
                    if (value.is_string() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        auto& pools = fullConfig["pools"];
                        if (!pools.Empty()) {
                            pools[0]["rig-id"].SetString(value.get<std::string>().c_str(), fullConfig.GetAllocator());
                        }
                    }
                } 
                else if (key == "--keepalive" || key == "-k") {
                    if (value.is_boolean() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        auto& pools = fullConfig["pools"];
                        if (!pools.Empty()) {
                            pools[0]["keepalive"].SetBool(value.get<bool>());
                        }
                    }
                } 
                else if (key == "--nicehash") {
                    if (value.is_boolean() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        auto& pools = fullConfig["pools"];
                        if (!pools.Empty()) {
                            pools[0]["nicehash"].SetBool(value.get<bool>());
                        }
                    }
                }
                else if (key == "--userpass" || key == "-O") {
                    if (value.is_string() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        auto& pools = fullConfig["pools"];
                        if (!pools.Empty()) {
                            std::string creds = value.get<std::string>();
                            size_t delim = creds.find(':');
                            if (delim != std::string::npos) {
                                std::string user = creds.substr(0, delim);
                                std::string pass = creds.substr(delim + 1);
                                pools[0]["user"].SetString(user.c_str(), allocator);
                                pools[0]["pass"].SetString(pass.c_str(), allocator);
                            }
                        }
                    }
                }
                else if (key == "--proxy" || key == "-x") {
                    if (value.is_string() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        auto& pools = fullConfig["pools"];
                        if (!pools.Empty()) {
                            pools[0]["socks5"].SetString(value.get<std::string>().c_str(), allocator);
                        }
                    }
                }
                else if (key == "--self-select") {
                    if (value.is_string() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        auto& pools = fullConfig["pools"];
                        if (!pools.Empty()) {
                            pools[0]["self-select"].SetString(value.get<std::string>().c_str(), allocator);
                        }
                    }
                }
                else if (key == "--submit-to-origin") {
                    if (value.is_boolean() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        auto& pools = fullConfig["pools"];
                        if (!pools.Empty()) {
                            pools[0]["submit-to-origin"].SetBool(value.get<bool>());
                        }
                    }
                }
                else if (key == "--retries" || key == "-r") {
                    if (value.is_number_integer() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        auto& pools = fullConfig["pools"];
                        if (!pools.Empty()) {
                            pools[0]["retries"].SetInt(value.get<int>());
                        }
                    }
                }
                else if (key == "--retry-pause" || key == "-R") {
                    if (value.is_number_integer() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        auto& pools = fullConfig["pools"];
                        if (!pools.Empty()) {
                            pools[0]["retry-pause"].SetInt(value.get<int>());
                        }
                    }
                }
                else if (key == "--user-agent") {
                    if (value.is_string() && fullConfig.HasMember("pools") && fullConfig["pools"].IsArray()) {
                        auto& pools = fullConfig["pools"];
                        if (!pools.Empty()) {
                            pools[0]["user-agent"].SetString(value.get<std::string>().c_str(), allocator);
                        }
                    }
                }
                else if (key == "--donate-level") {
                    if (value.is_number_integer()) {
                        fullConfig["donate-level"].SetInt(value.get<int>());
                    }
                }
                else if (key == "--donate-over-proxy") {
                    if (value.is_number_integer()) {
                        fullConfig["donate-over-proxy"].SetInt(value.get<int>());
                    }
                }                                                                                                                                
                else if (key == "--no-cpu") {
                    fullConfig["cpu"]["enabled"].SetBool(false);
                }
                else if (key == "--threads" || key == "-t") {
                    if (value.is_number_unsigned()) {
                        fullConfig["cpu"]["threads"].SetUint(value.get<uint32_t>());
                    }
                }
                else if (key == "--cpu-affinity") {
                    if (value.is_string()) {
                        std::string s = value.get<std::string>();
                        std::istringstream ss(s);
                        std::string token;
                
                        rapidjson::Value affinity(rapidjson::kArrayType);
                
                        while (std::getline(ss, token, ',')) {
                            try {
                                uint32_t cpu = std::stoul(token);
                                affinity.PushBack(cpu, allocator);
                            } catch (...) {
                                LOG_ERR("[WS] Invalid cpu-affinity value: %s", token.c_str());
                            }
                        }
                
                        fullConfig["cpu"]["affinity"] = affinity;
                        //LOG_INFO("[WS] Set cpu.affinity with %zu entries", affinity.Size());
                    }
                }
                else if (key == "--cpu-priority") {
                    if (value.is_number_integer()) {
                        fullConfig["cpu"]["priority"].SetInt(value.get<int>());
                    }
                }
                else if (key == "--cpu-max-threads-hint") {
                    if (value.is_number_integer()) {
                        fullConfig["cpu"]["max-threads-hint"].SetInt(value.get<int>());
                    }
                }
                else if (key == "--cpu-memory-pool") {
                    if (value.is_number_integer()) {
                        fullConfig["cpu"]["mem-pool"].SetInt(value.get<int>());
                    }
                }
                else if (key == "--cpu-no-yield") {
                    fullConfig["cpu"]["yield"].SetBool(false);
                }
                else if (key == "--no-huge-pages") {
                    fullConfig["cpu"]["huge-pages"].SetBool(false);
                }
                else if (key == "--hugepage-size") {
                    if (value.is_number_integer()) {
                        fullConfig["cpu"]["huge-page-size"].SetInt(value.get<int>());
                    }
                }
                else if (key == "--huge-pages-jit") {
                    fullConfig["cpu"]["huge-pages-jit"].SetBool(true);
                }
                else if (key == "--asm") {
                    if (value.is_string()) {
                        fullConfig["cpu"]["asm"].SetString(value.get<std::string>().c_str(), allocator);
                    }
                }
                else if (key == "--randomx-init") {
                    if (value.is_number_integer()) {
                        fullConfig["randomx"]["init"].SetInt(value.get<int>());
                    }
                }
                else if (key == "--randomx-no-numa") {
                    fullConfig["randomx"]["numa"].SetBool(false);
                }
                else if (key == "--randomx-mode") {
                    if (value.is_null() || (value.is_boolean() && !value.get<bool>())) {
                        fullConfig["randomx"].RemoveMember("mode");
                        LOG_INFO(WHITE_ON_GREY(" socket  ") "  Removed randomx.mode → fallback to auto");
                    }
                    else if (value.is_string()) {
                        const std::string mode = value.get<std::string>();
                        if (mode == "auto" || mode == "fast" || mode == "light") {
                            fullConfig["randomx"]["mode"].SetString(mode.c_str(), allocator);
                        }
                        else {
                            LOG_ERR(WHITE_ON_GREY(" socket  ") " Invalid randomx-mode: %s", mode.c_str());
                        }
                    }
                }
                else if (key == "--randomx-1gb-pages") {
                    fullConfig["randomx"]["1gb-pages"].SetBool(value.get<bool>());
                }
                else if (key == "--randomx-wrmsr") {
                    if (value.is_number_integer()) {
                        fullConfig["randomx"]["wrmsr"].SetInt(value.get<int>());
                    }
                }
                else if (key == "--randomx-no-rdmsr") {
                    fullConfig["randomx"]["rdmsr"].SetBool(false);
                }
                else if (key == "--randomx-cache-qos") {
                    fullConfig["randomx"]["cache-qos"].SetBool(true);
                }

            }
        }

        rapidjson::StringBuffer originalBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> originalWriter(originalBuffer);
        originalConfig.Accept(originalWriter);

        rapidjson::StringBuffer updatedBuffer;
        rapidjson::Writer<rapidjson::StringBuffer> updatedWriter(updatedBuffer);
        fullConfig.Accept(updatedWriter);

        if (std::string(originalBuffer.GetString()) != std::string(updatedBuffer.GetString())) {
            LOG_INFO(WHITE_ON_GREY(" socket  ") " Config has changed. Proceeding with reload.");
            if (m_controller->reload(fullConfig)) {
                LOG_INFO(WHITE_ON_GREY(" socket  ") " Config reloaded successfully.");
            } else {
                LOG_ERR("WebSocket" "Config reload failed.");
            }
        } else {
            LOG_INFO(WHITE_ON_GREY(" socket  ") " Config is identical. No reload needed.");
        }
    } else {
        LOG_INFO(WHITE_ON_GREY(" socket  ") " Unknown command.");
    }
}


} // namespace xmrig