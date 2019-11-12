#include <istream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>
#include <variant>
#include <algorithm>
#include <locale>
#include <cassert>
#include "ouchilib/tokenizer/tokenizer.hpp"

namespace ouchi::parser{

namespace detail {

template<class CharT>
inline void skip_utf8_bom(std::basic_istream<CharT>&){}

template<>
inline void skip_utf8_bom<char>(std::basic_istream<char>& stream)
{
    constexpr unsigned char bom[3] = { 0xEF, 0xBB, 0xBF };
    unsigned char bombuffer[3];
    stream.read((char*)bombuffer, sizeof bombuffer);
    if (!std::equal(std::begin(bom), std::end(bom), bombuffer))
        stream.seekg(-(long long)sizeof bombuffer, std::ios::cur);
}

}

template <class CharT>
class csv {
public:
    using string = std::basic_string<CharT>;
    using string_view = std::basic_string_view<CharT>;
    using istream = std::basic_istream<CharT>;
    using ifstream = std::basic_ifstream<CharT>;
    using ostream = std::basic_ostream<CharT>;
    using ofstream = std::basic_ofstream<CharT>;
    using index_t = std::variant<string_view, size_t>;

    csv()
        : column{ data_ }
        , row{ data_ }
        , sep_{ separators }
    {}
    csv(const tokenizer::separator<CharT>& sp)
        : column{ data_ }
        , row{ data_ }
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
    void parse(istream& text)
    {
        data_.clear();
        detail::skip_utf8_bom(text);
        while (text) {
            string line;
            std::getline(text, line);
            if (line.size())
                parseline(line);
            if (data_.front().size() != data_.back().size())
                throw std::runtime_error("parse failed. syntax error");
        }
    }
    [[nodiscard]]
    const string& at(index_t x,
                     index_t y) const
    {
        auto idxx = std::visit(column, x);
        auto idxy = std::visit(row, y);
        return data_.at(idxy).at(idxx);
    }
    [[nodiscard]]
    string& at(index_t x,
               index_t y)
    {
        auto idxx = std::visit(column, x);
        auto idxy = std::visit(row, y);
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
    void write(const std::filesystem::path& path, std::locale l, const CharT separator = (CharT)',')
    {
        ofstream out(path);
        out.imbue(l);
        write(out, separator);
    }
    void write(ostream& out, const CharT separator = (CharT)',') const
    {
        for (const auto& line : get()) {
            for (const auto& word : line) {
                out << word << separator;
            }
            out << (CharT)'\n';
        }
    }
    struct xidx {
        xidx(std::vector<std::vector<string>>& d) : data{d} {}
        size_t operator()(string_view key) const
        {
            auto& scanline = data.front();
            auto pos = std::find(scanline.begin(), scanline.end(), key);
            return std::distance(scanline.begin(), pos);
        }
        size_t operator()(size_t key) const noexcept { return key; }
    private:
        std::vector<std::vector<string>>& data;
    } column;
    struct yidx {
        yidx(std::vector<std::vector<string>>& d) : data{d} {}
        size_t operator()(string_view key) const
        {
            for (auto itr = data.begin(); itr != data.end(); ++itr)
                if (itr->front() == key) return std::distance(data.begin(), itr);
            return data.size();
        }
        size_t operator()(size_t key) const noexcept { return key; }
    private:
        std::vector<std::vector<string>>& data;
    } row;

private:
    void parseline(string_view line)
    {
        tokenizer::tokenizer<CharT> tok{line, sep_};
        size_t wordc = std::count_if(tok.begin(), tok.end(),
                                   [](auto&& i)
                                   { return (tokenizer::primitive_token)i.first == tokenizer::primitive_token::separator; });
        data_.emplace_back(wordc + 1);
        auto& dataline = data_.back();
        size_t cur = 0;
        assert(dataline.size() == wordc + 1);
        for (auto&&[type, str] : tok) {
            if (type == tokenizer::token_type::primitive_separator) {
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

}
