#pragma once
#include <list>
#include <utility>
#include <iterator>
#include <algorithm>
#include <memory>

namespace ouchi {

template<class T>
struct tree {
    T data;
    std::list<tree> children;

    tree() = default;
    tree(const tree&) = default;
    tree(tree&&) = default;
    tree(const T& val) : data(val), children{} {}
    tree& operator=(const tree&) = default;
    tree& operator=(tree&&) = default;
    tree& operator=(const T& val) { data = val; return *this; }

    template<class F, std::enable_if_t<std::is_invocable_r_v<bool, F, T>, int> = 0>
    const auto find_child(F&& pred, size_t first = 0, size_t last = -1) const
    {
        auto adopter = [&pred, this](const tree& t) {return std::invoke(pred, t.data); };
        return std::find_if(std::next(children.begin(),std::clamp(first, 0, children.size())),
                            std::next(children.begin(), std::clamp(last, first, children.size())),
                            adopter);
    }
    bool is_leaf() const noexcept { return children.empty(); }
};
}
