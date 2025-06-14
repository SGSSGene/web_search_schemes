#include "error_fmt.h"
#include "convertToSvg.h"

#include <emscripten.h>
#include <emscripten/bind.h>
#include <emscripten/fetch.h>
#include <fmindex-collection/fmindex-collection.h>
#include <fmindex-collection/search_scheme/generator/all.h>
#include <fmindex-collection/search_scheme/nodeCount.h>
#include <fmindex-collection/search_scheme/weightedNodeCount.h>
#include <fmt/ranges.h>
#include <ranges>
#include <sstream>

namespace fmc = fmindex_collection;

auto generatorList() -> std::vector<std::string> {
    auto ret = std::vector<std::string>{};
    for (auto const& [name, gen] : fmc::search_scheme::generator::all) {
        ret.push_back(name);
    }
    return ret;
}

auto generateSearchScheme(std::string _name, size_t minK, size_t maxK) -> std::string {
    for (auto const& [name, gen] : fmc::search_scheme::generator::all) {
        if (name == _name) {
            auto ss = gen.generator(minK, maxK, 4, 1'000'000'000);
            auto res = std::string{};
            res = "# π     L    U\n";

            for (auto const& search : ss) {
                res += fmt::format("{} {} {}\n", fmt::join(search.pi, ""), fmt::join(search.l, ""), fmt::join(search.u, ""));
            }
            return res;
        }
    }
    throw std::runtime_error{"unknown generator"};
}

auto convertSearchScheme(std::string text) -> fmindex_collection::search_scheme::Scheme {
    auto ss = std::stringstream{};
    ss << text;
    auto scheme = fmc::search_scheme::Scheme{};
    size_t lineNbr{};
    for (auto line = std::string{}; std::getline(ss, line);) {
        lineNbr += 1;
        while (!line.empty() && line.front() == ' ') {
            line.erase(line.begin());
        }
        while (!line.empty() && line.back() == ' ') {
            line.pop_back();
        }

        if (line.empty()) continue;
        if (line[0] == '#') continue;
        auto parts = line | std::views::split(' ')
                          | std::views::transform([&](auto r) { return std::string{r.begin(), r.end()}; })
                          | std::views::filter([&](auto s) { return !s.empty(); })
                          | std::ranges::to<std::vector<std::string>>();
        if (parts.size() != 3) throw error_fmt{"line {}: does not have three parts", lineNbr};
        if (parts[0].size() != parts[1].size()) throw error_fmt{"line {}: part 1 must have same length as part 2", lineNbr};
        if (parts[1].size() != parts[2].size()) throw error_fmt{"line {}: part 2 must have same length as part 3", lineNbr};

        auto search = fmc::search_scheme::Search{};
        for (auto c : parts[0]) search.pi.push_back(c - '0');
        for (auto c : parts[1]) search.l.push_back(c - '0');
        for (auto c : parts[2]) search.u.push_back(c - '0');
        if (std::ranges::find(search.pi, 0) == search.pi.end()) {
            throw error_fmt{"line {}: π must include part 0", lineNbr};
        }
        if (!fmc::search_scheme::valid::detail::checkPiContiguous(search.pi)) {
            throw error_fmt{"line {}: π violates the connectivity property", lineNbr};
        }
        if (!fmc::search_scheme::valid::detail::checkMonotonicallyIncreasing(search.l)) {
            throw error_fmt{"line {}: L is not monotonically increasing", lineNbr};
        }
        if (!fmc::search_scheme::valid::detail::checkMonotonicallyIncreasing(search.u)) {
            throw error_fmt{"line {}: U is not monotonically increasing", lineNbr};
        }
        for (size_t i{0}; i < search.pi.size(); ++i) {
            if (search.l[i] > search.u[i]) {
                throw error_fmt{"line {}: at position {} L must be smaller then U", lineNbr, i+1};
            }
        }

        if (scheme.size() > 0 && scheme.back().pi.size() != search.pi.size()) {
            throw error_fmt{"line {}: parts have different length then previous line", lineNbr};
        }
        scheme.push_back(search);
    }
    auto valid = fmc::search_scheme::isValid(scheme);
    std::cout << "is search scheme valid? " << valid << "\n";
    return scheme;
}

auto convertSearchSchemeToSvg(std::string text, size_t parts) -> std::string {
    std::cout << "converting search scheme to svg\n";
    auto scheme = convertSearchScheme(text);
    auto res = std::string{};

    res = convertsvg::convertToSvg(scheme, parts);

    return res;
}

auto isSearchSchemeValid(std::string text) -> bool {
    auto scheme = convertSearchScheme(text);
    return isValid(scheme);
}

auto isSearchSchemeComplete(std::string text) -> bool {
    auto scheme = convertSearchScheme(text);
    size_t minK = std::numeric_limits<size_t>::max();
    size_t maxK = 0;
    for (auto const& search : scheme) {
        minK = std::min(minK, search.l.back());
        maxK = std::max(maxK, search.u.back());
    }
    return isComplete(scheme, minK, maxK);
}

auto isSearchSchemeNonRedundant(std::string text) -> bool {
    auto scheme = convertSearchScheme(text);
    size_t minK = std::numeric_limits<size_t>::max();
    size_t maxK = 0;
    for (auto const& search : scheme) {
        minK = std::min(minK, search.l.back());
        maxK = std::max(maxK, search.u.back());
    }
    return isNonRedundant(scheme, minK, maxK);
}

auto nodeCount(std::string text, size_t len) -> double {
    auto scheme = convertSearchScheme(text);
    auto os = expand(scheme, len);

    return nodeCount</*.Edit=*/false>(os, 4) + os.size();
}
auto weightedNodeCount(std::string text, size_t len) -> double {
    auto scheme = convertSearchScheme(text);
    auto os = expand(scheme, len);
    return weightedNodeCount</*.Edit=*/false>(os, 4, 1'000'000'000) + os.size();

}

EMSCRIPTEN_BINDINGS(my_module) {
    emscripten::register_vector<std::string>("vector<string>");
    emscripten::function("generatorList", &generatorList);
    emscripten::function("convertSearchSchemeToSvg", &convertSearchSchemeToSvg);
    emscripten::function("isSearchSchemeValid", &isSearchSchemeValid);
    emscripten::function("isSearchSchemeComplete", &isSearchSchemeComplete);
    emscripten::function("isSearchSchemeNonRedundant", &isSearchSchemeNonRedundant);
    emscripten::function("generateSearchScheme", &generateSearchScheme);
    emscripten::function("nodeCount", &nodeCount);
    emscripten::function("weightedNodeCount", &weightedNodeCount);

}
