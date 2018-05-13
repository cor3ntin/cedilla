#include "common.h"

std::vector<test_data> test_cases() {
    static std::vector<test_data> data;
    if(data.empty()) {
        std::ifstream f(TEST_FILE);
        std::regex r(
            R"(([0-9a-fA-F\s]+);([0-9a-fA-F\s]+);([0-9a-fA-F\s]+);([0-9a-fA-F\s]+);([0-9a-fA-F\s]+);\s*#(.*)$)");
        for(std::string line; std::getline(f, line);) {
            if(line.empty() || line.at(0) == '#' || line.at(0) == '@')
                continue;
            std::smatch m;
            if(std::regex_match(line, m, r)) {
                auto transform = [&m](std::u32string& target, int idx) {
                    std::string str = m.str(idx);
                    std::istringstream ist(str);
                    std::transform(
                        std::istream_iterator<std::string>(ist),
                        std::istream_iterator<std::string>(), std::back_inserter(target),
                        [](const std::string& s) { return char32_t(std::stoi(s, nullptr, 16)); });
                };

                test_data d;
                transform(d.c1, 1);
                transform(d.c2, 2);
                transform(d.c3, 3);
                transform(d.c4, 4);
                transform(d.c5, 5);
                data.push_back(d);
            }
        }
    }
    return data;
}
