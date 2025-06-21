// SPDX-FileCopyrightText: 2006-2023, Knut Reinert & Freie Universität Berlin
// SPDX-FileCopyrightText: 2016-2023, Knut Reinert & MPI für molekulare Genetik
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <cassert>
#include <fmindex-collection/search_scheme/all.h>
#include <set>
#include <unordered_set>
#include <fmt/ranges.h>

namespace convertsvg {

template <typename CB>
auto visitTree(fmindex_collection::search_scheme::Search s, size_t x, size_t pos, size_t sigma, CB cb, size_t errors, bool editdistance) -> size_t {
    if (pos == s.pi.size()) return 1;

    size_t width{1};
    if (s.l[pos] <= errors) {
        cb(x, pos, 0, [&]() {
            width = visitTree(s, x, pos+1, sigma, cb, errors, editdistance);
        });
    }
    if (errors+1 <= s.u[pos]) {
        // substitutions
        for (size_t symb{1}; symb < sigma; ++symb) {
            cb(x+width, pos, 1, [&]() {
                width += visitTree(s, x+width, pos+1, sigma, cb, errors+1, editdistance);
            });
        }

        // insertions
        if (editdistance) {
            cb(x+width, pos, 3, [&]() {
                width += visitTree(s, x+width, pos+1, sigma, cb, errors+1, editdistance);
            });
        }

        // deletions
        if (editdistance) {
            for (size_t symb{1}; symb < sigma; ++symb) {
                cb(x+width, pos-1, 2, [&]() {
                    width += visitTree(s, x+width, pos, sigma, cb, errors+1, editdistance);
                });
            }
        }
    }
    return width;
}

template <typename CB>
void visitTree(fmindex_collection::search_scheme::Search s, size_t sigma, bool editdistance, CB cb) {
    auto errorConf = std::vector<int>{};
    visitTree(s, 0, 0, sigma, cb, 0, editdistance);
}

auto convertToSvg(fmindex_collection::search_scheme::Scheme _scheme, int newLen, size_t sigma, bool editdistance) -> std::string {

    //editdistance // todo

    size_t spaceXBetweenTrees = 30;
    size_t spaceBetweenNodes = 10;
    size_t tmaxX{};
    size_t tmaxY{};
    for (auto search : _scheme) {
        assert(isValid(search));
        auto os = expand(search, newLen);
        assert(os);
        auto s = *os;
        if (!editdistance) {
            s = limitToHamming(*os);
        }

        // compute largest x/y
        size_t maxX{}, maxY{};
        visitTree(s, sigma, editdistance, [&](size_t _x, size_t pos, size_t dir, auto const& rec) {
            maxX = std::max(maxX, _x*spaceBetweenNodes);
            maxY = std::max(maxY, (1+pos)*spaceBetweenNodes);
            rec();
        });
        tmaxX += maxX;
        tmaxY = std::max(tmaxY, maxY);
//        tmaxY += maxY;
    }

    tmaxX = tmaxX + (_scheme.size()-1)*spaceXBetweenTrees;
//    tmaxY = tmaxY + spaceBetweenNodes/2;

    auto out = fmt::format(R"(<svg viewBox="{} {} {} {}" xmlns="http://www.w3.org/2000/svg">{})", -int(spaceBetweenNodes), -int(spaceBetweenNodes)/2, tmaxX+spaceBetweenNodes*2, tmaxY+spaceBetweenNodes, "\n");

    // add some css
    out += R"(
  <style type="text/css">
    <![CDATA[
      text {
        font-size: 6px;
        text-anchor: start;
        dominant-baseline: central;
    }
    ]]>
  </style>
)";

    // add some glow effect filter:
    out += fmt::format(R"(
<defs>
  <filter id="glow" filterUnits="userSpaceOnUse" x="-50%" y="-50%" width="200%" height="200%">
     <!-- blur the text at different levels-->
    <feGaussianBlur in="SourceGraphic" stdDeviation="0.5" result="blur"/>
    <feMerge>
      <feMergeNode in="blur"/>           <!-- largest blurs coloured red -->
      <feMergeNode in="SourceGraphic"/>  <!-- original white text -->
    </feMerge>
  </filter>
</defs>
)");



    size_t offsetX{}, offsetY{}, treeNbr{};
    for (auto search : _scheme) {
        treeNbr += 1;
        assert(isValid(search));
        auto os = expand(search, newLen);
        assert(os);
        auto s = *os;
        if (!editdistance) {
            s = limitToHamming(*os);
        }

        // compute largest x/y
        size_t maxX{}, maxY{};
        visitTree(s, sigma, editdistance, [&](size_t _x, size_t pos, size_t dir, auto const& rec) {
            maxX = std::max(maxX, _x*spaceBetweenNodes);
            maxY = std::max(maxY, (1+pos)*spaceBetweenNodes);
            rec();
        });


        // draw all partition lines
        {
            auto partition = std::vector<size_t>{};
            {
                size_t extra = newLen % search.pi.size();
                size_t size = newLen / search.pi.size();
                for (size_t i{0}; i < search.pi.size(); ++i) {
                    partition.push_back(size);
                    if (extra > 0) {
                        partition.back() += 1;
                        extra -= 1;
                    }
                }
            }

            size_t accJ{};
            for (size_t i{0}; i < search.pi.size(); ++i) {
                size_t oldY = accJ * spaceBetweenNodes;
                accJ += partition[search.pi[i]];
                size_t y = accJ * spaceBetweenNodes;
                out += fmt::format(R"(<text x="{}" y="{}">P{}</text>{})", int(offsetX)-10, offsetY + (y + oldY)/2, search.pi[i], "\n");
                if (i+1 != search.pi.size()) {
                    out += fmt::format(R"(<line x1="{}" y1="{}" x2="{}" y2="{}" stroke="black" stroke-width="0.5" />{})", int(offsetX)-5, offsetY+y, offsetX+maxX+5, offsetY+y, "\n");
                }
            }
        }


        // draw connecting lines
        {
            auto node = std::vector<std::tuple<size_t, size_t>>{};
            node.emplace_back(0, 0);
            visitTree(s, sigma, editdistance, [&](size_t _x, size_t pos, size_t dir, auto const& rec) {
                auto [px, py] = node.back();
                auto x = _x*spaceBetweenNodes;
                auto y = (1+pos)*spaceBetweenNodes;

                if (dir == 2) {
                    y += 1;
                }

                auto stroke = std::string{};
                if (dir == 1) {
                    stroke = R"(stroke-dasharray="2 1")";
                }

                if (dir == 2 || dir == 3) {
                    stroke = R"(stroke-dasharray="1 2")";
                }

                auto classes = std::string{};
                for (auto [x, y] : node) {
                    classes += fmt::format(" child-of-node-{}-{}-{}", treeNbr, x, y);
                }

                out += fmt::format(R"(<line x1="{}" y1="{}" x2="{}" y2="{}" class="{}" stroke="black" {} />{})", offsetX+px, offsetY+py, offsetX+x, offsetY+y, classes, stroke, "\n");

                node.emplace_back(x, y);
                rec();
                node.pop_back();
            });
        }
        // draw circles
        {
            // walk through and detect tree sizes
            out += fmt::format(R"(<circle cx="{}" cy="{}" r="{}" data-node-name="node-{}-0-0" class="node-0-0" />{})", 0+offsetX, 0+offsetY, spaceBetweenNodes/3, treeNbr, "\n");

            auto node = std::vector<std::tuple<size_t, size_t>>{};
            node.emplace_back(0, 0);
            visitTree(s, sigma, editdistance, [&](size_t _x, size_t pos, size_t dir, auto const& rec) {
                auto [px, py] = node.back();
                auto x = _x*spaceBetweenNodes;
                auto y = (1+pos)*spaceBetweenNodes;

                auto classes = std::string{};
                for (auto [x, y] : node) {
                    classes += fmt::format(" child-of-node-{}-{}-{}", treeNbr, x, y);
                }

                if (dir == 2) {
                    y += 1;
                }
                out += fmt::format(R"(<circle cx="{}" cy="{}" r="{}" data-node-name="node-{}-{}-{}" class="{}" />{})", offsetX + x, offsetY+y, spaceBetweenNodes/3, treeNbr, x, y, classes, "\n");

                node.emplace_back(x, y);
                rec();
                node.pop_back();
            });
        }

        offsetX += maxX + spaceXBetweenTrees;
    }

    out += "</svg>\n";
    return out;

}
}
