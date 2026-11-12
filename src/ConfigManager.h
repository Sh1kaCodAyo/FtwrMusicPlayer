//
// Created by ftwrjh on 2026/11/7.
//
#pragma once

#include <string>
#include <map>

class ConfigManager {
public:
    // 构造函数，传入配置文件的路径
    explicit ConfigManager(const std::string& filepath);

    // 读取一个配置项（找不到时返回 defaultValue）
    std::string get_value(const std::string& key, const std::string& default_value = "");

    // 设置并保存一个配置项
    void set_value(const std::string& key, const std::string& value);

    // 强制从磁盘重新加载
    void load();

    // 将当前的 map 写入磁盘
    void save();

private:
    std::string filepath_;
    std::map<std::string, std::string> config_data_;
};
