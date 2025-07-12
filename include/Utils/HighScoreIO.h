#pragma once
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>

namespace FishGame {
    struct HighScoreEntry {
        std::string name;
        int score{};
    };

    inline std::vector<HighScoreEntry> loadHighScores(const std::string& file) {
        std::vector<HighScoreEntry> scores;
        std::ifstream in(file);
        std::string name; int score;
        while (in >> name >> score) {
            scores.push_back({name, score});
        }
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
        scores.push_back(entry);
        std::sort(scores.begin(), scores.end(), [](const auto& a, const auto& b){ return a.score > b.score; });
        if (scores.size() > maxEntries) scores.resize(maxEntries);
        saveHighScores(scores, file);
    }
}
