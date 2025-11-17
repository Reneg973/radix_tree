#include <string>
#include <string_view>
#include <iostream>
#include <cstdlib>

#include "../radix_tree.hpp"

using namespace std::string_view_literals;
radix_tree<std::string, int> tree;

void insert() {
    tree["apache"sv]    = 0;
    tree["afford"sv]    = 1;
    tree["available"sv] = 2;
    tree["affair"sv]    = 3;
    tree["avenger"sv]   = 4;
    tree["binary"sv]    = 5;
    tree["bind"sv]      = 6;
    tree["brother"sv]   = 7;
    tree["brace"sv]     = 8;
    tree["blind"sv]     = 9;
    tree["bro"sv]       = 10;
}

void longest_match(std::string_view key)
{
    auto p = tree.longest_match(key);

    std::cout << "longest_match(\"" << key << "\"):" << std::endl;

    if (!p.first.empty()) {
        std::cout << "    " << p.first << ", " << p.second << std::endl;
    } else {
        std::cout << "    failed" << std::endl;
    }
}

void prefix_match(std::string_view key)
{
    std::vector<radix_tree<std::string, int>::iterator> vec;
    std::vector<radix_tree<std::string, int>::iterator>::iterator it;

    tree.prefix_match(key, vec);

    std::cout << "prefix_match(\"" << key << "\"):" << std::endl;

    for (it = vec.begin(); it != vec.end(); ++it) {
        auto vt = (*it).GetValue();
        std::cout << "    " << vt.first << ", " << vt.second << std::endl;
    }
}

void prefix_match2(std::string_view key)
{
    auto pit = tree.prefix_range(key);

    std::cout << "prefix_range(\"" << key << "\"):" << std::endl;

    for (auto it = pit.first; it != pit.second; ++it) {
        auto vt = it.GetValue();
        std::cout << "    " << vt.first << ", " << vt.second << std::endl;
    }
}

void greedy_match(std::string_view key)
{
    std::vector<radix_tree<std::string, int>::iterator> vec;
    std::vector<radix_tree<std::string, int>::iterator>::iterator it;

    tree.greedy_match(key, vec);

    std::cout << "greedy_match(\"" << key << "\"):" << std::endl;

    for (it = vec.begin(); it != vec.end(); ++it) {
        auto vt = (*it).GetValue();
        std::cout << "    " << vt.first << ", " << vt.second << std::endl;
    }
}

void traverse() {
    radix_tree<std::string, int>::iterator it;

    std::cout << "traverse:" << std::endl;
    for (it = tree.begin(); it != tree.end(); ++it) {
        auto v = it.GetValue();
        std::cout << "    " << v.first << ", " << v.second << std::endl;
    }
}

int main()
{
    insert();

    longest_match("binder"sv);
    longest_match("bracelet"sv);
    longest_match("apple"sv);

    prefix_match("aff"sv);
    prefix_match2("aff"sv);
    prefix_match("bi"sv);
    prefix_match2("bi"sv);
    prefix_match("a"sv);
    prefix_match2("a"sv);

    greedy_match("avoid"sv);
    greedy_match("bring"sv);
    greedy_match("attack"sv);

    if (auto it = tree.find("avenger"sv); it != tree.end())
    {
        auto vt = it.GetValue();
        std::cout << "found: " << vt.first << ", " << vt.second << std::endl;
    }
    traverse();

    tree.erase("bro");
    prefix_match("bro");

    return EXIT_SUCCESS;
}
