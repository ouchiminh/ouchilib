#include <istream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <variant>
#include <algorithm>
#include <locale>
#include <cassert>
#include "tokenizer.hpp"

namespace ouchi::parser{

namespace detail {

template<typename CharT>
constexpr CharT bom[] = { CharT{} };
template<> constexpr unsigned char bom<char>[3] = { 0xEF, 0xBB, 0xBF };
template<> constexpr char16_t bom<char16_t> = 0xFEFF;
template<> constexpr char32_t bom<char32_t> = 0x0000FEFF;

}

template <class CharT>
class csv {
public:
    using string = std::basic_string<CharT>;
    using string_view = std::basic_string_view<CharT>;
    using istream = std::basic_istream<CharT>;
    using ifstream = std::basic_ifstream<CharT>;
    using index_t = std::variant<string_view, size_t>;

    csv()
        : x_{ data_ }
        , y_{ data_ }
        , sep_{ separators }
    {}
    csv(const tokenizer::separator<CharT>& sp)
        : x_{ data_ }
        , y_{ data_ }
        , sep_{ sp }
    {}
    void parse(string_view text) {
        std::basic_stringstream<CharT> ss{text.data()};
        parse(ss);
    }
    void parse(const std::filesystem::path& csvfile, std::locale l)
    {
        ifstream fs(csvfile);
        fs.imbue(l);
        parse(fs);
    }
    void parse(istream& text);
    [[nodiscard]]
    const string& at(index_t x,
                     index_t y) const
    {
        auto idxx = std::visit(x_, x);
        auto idxy = std::visit(y_, y);
        return data_.at(idxy).at(idxx);
    }
    [[nodiscard]]
    string& at(index_t x,
               index_t y)
    {
        auto idxx = std::visit(x_, x);
        auto idxy = std::visit(y_, y);
        return data_.at(idxy).at(idxx);
    }

    [[nodiscard]]
    auto get() const
        ->const std::vector<std::vector<string>>&
    {
        return data_;
    }
    [[nodiscard]]
    auto get(size_t y) const
        ->const std::vector<string>&
    {
        return data_.at(y);
    }
    [[nodiscard]]
    auto get()
        ->std::vector<std::vector<string>>&
    {
        return data_;
    }
    [[nodiscard]]
    auto get(size_t y)
        ->std::vector<string>&
    {
        return data_.at(y);
    }
private:
    struct xidx {
        xidx(std::vector<std::vector<string>>& d) : data{d} {}
        size_t operator()(string_view key) const
        {
            auto& scanline = data.front();
            auto pos = std::find(scanline.begin(), scanline.end(), key);
            return std::distance(scanline.begin(), pos);
        }
        size_t operator()(size_t key) const noexcept { return key; }

        std::vector<std::vector<string>>& data;
    } x_;
    struct yidx {
        yidx(std::vector<std::vector<string>>& d) : data{d} {}
        size_t operator()(string_view key) const
        {
            for (auto itr = data.begin(); itr != data.end(); ++itr)
                if (itr->front() == key) return std::distance(data.begin(), itr);
            return data.size();
        }
        size_t operator()(size_t key) const noexcept { return key; }

        std::vector<std::vector<string>>& data;
    } y_;
    void parseline(string_view line)
    {
        tokenizer::tokenizer<CharT> tok{line, sep_};
        auto wordc = std::count_if(tok.begin(), tok.end(), [](auto&& i) { return i.first == tokenizer::token::separator; });
        data_.emplace_back(wordc + 1);
        auto& dataline = data_.back();
        size_t cur = 0;
        assert(dataline.size() == wordc + 1);
        for (auto&&[type, str] : tok) {
            if (type == tokenizer::token::separator) {
                ++cur;
                continue;
            }
            else dataline[cur] = str;
        }
    }
    std::vector<std::vector<string>> data_;
    tokenizer::separator<CharT> sep_;
    static constexpr CharT separators[] = { '\t', ',', '\0' };
    
};

template<class CharT>
inline void csv<CharT>::parse(istream & text)
{
    while (text) {
        string line;
        std::getline(text, line);
        if (line.size())
            parseline(line);
        if (data_.front().size() != data_.back().size())
            throw std::runtime_error("parse failed. syntax error");
    }
}

template<>
inline void csv<char>::parse(istream & text)
{
    unsigned char bombuffer[3];

    // BOM読み捨て
    text.read(reinterpret_cast<char*>(bombuffer), 3);
    if (!std::equal(std::begin(detail::bom<char>), std::end(detail::bom<char>), std::begin(bombuffer)))
        text.seekg(-3, std::ios::cur);

    while (text) {
        string line;
        std::getline(text, line);
        if (line.size())
            parseline(line);
        if (data_.front().size() != data_.back().size())
            throw std::runtime_error("parse failed. syntax error");
    }
}

template<>
inline void csv<char16_t>::parse(istream & text)
{
    char16_t bombuffer;

    // BOM読み捨て
    text.read(&bombuffer, 1);
    if (bombuffer != detail::bom<char16_t>)
        text.seekg(-3, std::ios::cur);

    while (text) {
        string line;
        std::getline(text, line);
        if (line.size())
            parseline(line);
        if (data_.front().size() != data_.back().size())
            throw std::runtime_error("parse failed. syntax error");
    }
}
template<>
inline void csv<char32_t>::parse(istream & text)
{
    char32_t bombuffer;

    // BOM読み捨て
    text.read(&bombuffer, 1);
    if (bombuffer != detail::bom<char32_t>)
        text.seekg(-3, std::ios::cur);

    while (text) {
        string line;
        std::getline(text, line);
        if (line.size())
            parseline(line);
        if (data_.front().size() != data_.back().size())
            throw std::runtime_error("parse failed. syntax error");
    }
}

}
