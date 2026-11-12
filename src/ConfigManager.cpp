//
// Created by ftwrjh on 2026/11/7.
//
#include "ConfigManager.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <filesystem>

ConfigManager::ConfigManager(const std::string& filepath) : filepath_(filepath) {
    load(); // 创建对象时自动加载
}

void ConfigManager::load() {
    config_data_.clear();
    std::ifstream file(filepath_);
    if (!file.is_open()) {
        // 文件不存在是正常的（比如第一次运行），直接返回
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        // 跳过空行
        if (line.empty()) {
            continue;
        }

        // 查找等号
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // 去掉两端的空格（可选，但推荐）
            key.erase(0, key.find_first_not_of(" \t\r\n"));
            key.erase(key.find_last_not_of(" \t\r\n") + 1);
            value.erase(0, value.find_first_not_of(" \t\r\n"));
            value.erase(value.find_last_not_of(" \t\r\n") + 1);

            config_data_[key] = value;
        }
    }
}

void ConfigManager::save() {
    // 自动创建父文件夹
    std::filesystem::path path(filepath_);
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }
    // 然后写入文件
    std::ofstream file(filepath_);
    if (!file.is_open()) {
        std::cerr << "无法写入配置文件: " << filepath_ << std::endl;
        return;
    }

    for (const auto& pair : config_data_) {
        file << pair.first << "=" << pair.second << "\n";
    }
}

std::string ConfigManager::get_value(const std::string& key, const std::string& default_value) {
    auto it = config_data_.find(key);
    if (it != config_data_.end()) {
        return it->second;
    }
    return default_value;
}

void ConfigManager::set_value(const std::string& key, const std::string& value) {
    config_data_[key] = value;
    save(); // 修改后立即保存（如果配置项不多，这样最简单直接）
}