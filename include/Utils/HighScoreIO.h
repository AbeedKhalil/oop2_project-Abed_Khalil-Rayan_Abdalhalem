#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <unordered_map>

namespace FishGame {
    struct HighScoreEntry {
        std::string name;
        int score{};
    };

    inline std::vector<HighScoreEntry> loadHighScores(const std::string& file) {
        std::unordered_map<std::string,int> unique;
        std::ifstream in(file);
        std::string name; int score;
        while (in >> name >> score) {
            auto it = unique.find(name);
            if(it==unique.end() || score > it->second)
                unique[name] = score;
        }
        std::vector<HighScoreEntry> scores;
        scores.reserve(unique.size());
        for(const auto& p : unique)
            scores.push_back({p.first, p.second});
        std::sort(scores.begin(), scores.end(), [](const auto& a, const auto& b){ return a.score > b.score; });
        return scores;
    }

    inline void saveHighScores(const std::vector<HighScoreEntry>& scores, const std::string& file) {
        std::ofstream out(file, std::ios::trunc);
        for (const auto& e : scores) {
            out << e.name << ' ' << e.score << '\n';
        }
    }

    inline void addHighScore(const std::string& file, const HighScoreEntry& entry, std::size_t maxEntries = 10) {
        auto scores = loadHighScores(file);
        bool found = false;
        for(auto& e : scores){
            if(e.name == entry.name){
                if(entry.score > e.score) e.score = entry.score;
                found = true;
                break;
            }
        }
        if(!found) scores.push_back(entry);
        std::sort(scores.begin(), scores.end(), [](const auto& a, const auto& b){ return a.score > b.score; });
        if (scores.size() > maxEntries) scores.resize(maxEntries);
        saveHighScores(scores, file);
    }
}
