#pragma once

#include "../common.h"


namespace Utils
{

    //-------------------------------

    [[nodiscard]]
    constexpr
    std::string
    ltrim_str(std::string str) {
        str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
            return !std::isspace(ch);
        }));
        return str;
    }

    //-------------------------------

    [[nodiscard]]
    constexpr
    std::string
    rtrim_str(std::string str) {
        str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
            return !std::isspace(ch);
        }).base(), str.end());
        return str;
    }

    //-------------------------------

    [[nodiscard]]
    constexpr
    std::string
    trim_str(std::string str) {
        return ltrim_str(rtrim_str(std::move(str)));
    }

    //-------------------------------

    [[nodiscard]]
    constexpr
    std::string_view
    ltrim(std::string_view str) {
        auto fn { std::find_if(std::begin(str), std::end(str), [](unsigned char ch) {
            return !std::isspace(ch);
        }) };

        return std::string_view { fn, static_cast<size_t>(std::distance(fn, std::end(str))) };
    }

    //-------------------------------

    [[nodiscard]]
    constexpr
    std::string_view
    rtrim(std::string_view str) {
        auto fn { std::find_if(std::rbegin(str), std::rend(str), [](unsigned char ch) {
            return !std::isspace(ch);
        }) };

        return std::string_view { std::begin(str), static_cast<size_t>(std::distance(fn, std::rend(str))) };
    }

    //-------------------------------
    
    [[nodiscard]]
    constexpr
    std::string_view
    trim(std::string_view str) {
        return ltrim(rtrim(str));
    }

    //-------------------------------

    [[nodiscard]]
    constexpr
    std::vector<std::string_view> 
    parseString (std::string_view str, std::string_view delimiter)
    {
        std::vector<std::string_view> result;
        for (std::string_view::size_type pos {}, pos2 { str.find(delimiter) }; pos != std::string_view::npos;)
        {
            result.emplace_back(str.substr(pos, pos2-pos));
            std::string_view::size_type tmp_pos { str.find(delimiter, pos2+1) };
            pos = pos2 + (pos2 == std::string_view::npos ? 0 : 1);
            pos2 = tmp_pos;
        }
        return result;
    }

    //-------------------------------

    //-------------------------------

    //-------------------------------

}