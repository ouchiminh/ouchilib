#pragma once
#include "meta-helper.hpp"

namespace ouchi {

template<class Iter>
class indexed_iterator {
public:
    using itr_t = std::remove_reference_t<Iter>;
    using idx_t = typename std::iterator_traits<Iter>::difference_type;
    using difference_type = idx_t;
    using iterator_category = typename std::iterator_traits<Iter>::iterator_category;
    using value_type = std::pair<idx_t, typename std::iterator_traits<Iter>::reference>;
    static constexpr bool is_bidirectional = std::is_base_of_v<std::bidirectional_iterator_tag, iterator_category>;

    template<class Container>
    indexed_iterator(Container& c)
        : indexed_iterator(c.begin(), c.end())
    {}
    indexed_iterator(Iter first, Iter last, idx_t first_pos = 0)
        : first_(first)
        , last_(last)
        , first_pos_(first_pos)
    {}

    class iterator {
    public:
        using difference_type = typename std::iterator_traits<itr_t>::difference_type;

        iterator(itr_t first, idx_t first_pos = 0)
            : first_(first)
            , idx_(first_pos)
        {}

        decltype(auto) operator*() const
        {
            return std::pair<idx_t, decltype(*first_)>{ idx_, *first_ };
        }
        iterator& operator++()
        {
            ++idx_;
            ++first_;
            return *this;
        }
        iterator operator++(int)
        {
            auto cp = *this;
            ++(*this);
            return cp;
        }
        template<class It = iterator,
                 std::enable_if_t<It::is_bidirectional>* = nullptr>
        iterator& operator--() {
            --idx_;
            --first_;
            return *this;
        }
        template<class It = iterator,
                 std::enable_if_t<It::is_bidirectional>* = nullptr>
        iterator operator--(int)
        {
            auto cp = *this;
            --(*this);
            return cp;
        }
        friend bool operator==(const iterator& a, const iterator& b)
        {
            return a.first_ == b.first_;
        }
        friend bool operator!=(const iterator& a, const iterator& b)
        {
            return !(a == b);
        }

    private:
        itr_t first_;
        idx_t idx_;
    };

    auto begin()
    {
        return iterator{ first_, first_pos_ };
    }
    auto end()
    {
        return iterator{ last_, std::distance(first_, last_) };
    }
private:
    itr_t first_;
    itr_t last_;
    idx_t first_pos_;
};

template<class Container>
indexed_iterator(Container& c)
->indexed_iterator<decltype(std::declval<Container&>().begin())>;

}
