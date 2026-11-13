//
// Created by ftwrjh on 2026/11/13.
//

#include "Lyric.h"

void Lyric::load_lyrics(const std::string& lrc_path) {
    lyrics_.clear();
    std::ifstream file(lrc_path);
    if (!file.is_open()) return;

    std::string line;
    // 匹配时间标签的正则，注意这里没有用 ^ 锁定行首
    // std::regex time_regex("\\[(\\d+):(\\d+)(?:\\.(\\d+))?\\]");
    std::regex time_regex("\\[(\\d+):(\\d+)\\.(\\d+)\\]");
    
    while (std::getline(file, line)) {
        // ... (处理 \r 和空行的逻辑) ...

        std::string::const_iterator searchStart(line.cbegin());
        std::smatch match;
        std::vector<int> times_in_this_line; // 存这一行提取出的所有时间
        

        // 用于临时保存当前行提取到的纯文本
        std::string current_lyric_text = ""; 
        // std::string::const_iterator searchStart(line.cbegin());
        // std::smatch match;

        while (std::regex_search(searchStart, line.cend(), match, time_regex)) {
            int min = std::stoi(match[1]);
            int sec = std::stoi(match[2]);
            // 【修复】这里加个保护，防止老格式没有毫秒组导致崩溃
            int ms = 0;
            if (match[3].matched) {
                ms = std::stoi(match[3]) * 10; 
            }

            int total_ms = (min * 60 + sec) * 1000 + ms;
            times_in_this_line.push_back(total_ms);
            
            // 【核心修复】在循环内部，每次匹配成功后立刻记录后缀！
            // 如果是 [01:15][02:30]副歌，第一次记录到的是 [02:30]副歌，第二次记录到的是 副歌
            // 循环结束后，current_lyric_text 里存的就是最后一个标签后面的纯文本
            current_lyric_text = match.suffix().str(); 

            searchStart = match.suffix().first; 
        }

        // 如果这一行找到了时间标签
        if (!times_in_this_line.empty()) {
            // 使用我们刚才在循环里抢救出来的文本
            std::string text = current_lyric_text;
            
            // 去除首尾空格
            size_t first = text.find_first_not_of(" \t\r\n");
            if (first != std::string::npos) {
                size_t last = text.find_last_not_of(" \t\r\n");
                text = text.substr(first, (last - first + 1));
            } else {
                text = ""; // 全空格的情况
            }

            // 为这一行文本生成对应的多个歌词节点
            for (int time_ms : times_in_this_line) {
                lyrics_.push_back({time_ms, text});
            }
        }






    }
    
    // 最后必须按时间排序，因为多标签可能会导致顺序打乱
    std::sort(lyrics_.begin(), lyrics_.end(), [](const LyricLine& a, const LyricLine& b) {
        return a.time_ms < b.time_ms;
    });
}

std::string getNameFromFullPath(std::string full_path) {
    // 1. 把字符串转换成 filesystem::path 对象
    std::filesystem::path p(full_path);
    
    return p.stem().string();
    
    // 2. 提取文件名（带后缀），输出: "さようならへさよなら! (HANAYO Mix).mp3"
    // std::cout << "文件名: " << p.filename().string() << std::endl;
    
    // 3. 提取文件名（不带后缀），也就是你想要的！输出: "さようならへさよなら! (HANAYO Mix)"
    // std::cout << "纯歌名: " << p.stem().string() << std::endl;

}