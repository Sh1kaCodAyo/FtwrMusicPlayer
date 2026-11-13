//
// Created by ftwrjh on 2026/11/13.
//

#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <regex>
#include <filesystem>

class Lyric {
public:
    static void load_lyrics(const std::string& lrc_path);
};

struct LyricLine {
    // 毫秒时间戳
    int time_ms;
    // 歌词文本
    std::string text;
};

// 声明一个成员变量存放所有歌词
inline std::vector<LyricLine> lyrics_;

extern std::string getNameFromFullPath(std::string full_path);
