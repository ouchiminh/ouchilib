#pragma once
#include <memory>
#include <list>
#include "task.hpp"

namespace ouchi::task{

class tasksystem {
    using task_ptr = std::shared_ptr<task>;
    std::list<task_ptr> tasklist_;
public:
    void update()
    {
        for (auto itr = tasklist_.begin(); itr != tasklist_.end();) {
            auto next = std::next(itr);
            if (!(**itr)())
                next = tasklist_.erase(itr);
            itr = next;
        }
    }
    template<class Ty, class ...Args>
    std::weak_ptr<task> create(Args&& ...args)
    {
        static_assert(std::is_base_of_v<task, Ty>);
        return tasklist_.emplace_back(std::make_shared<Ty>(std::forward<Args>(args)...));
    }
    void clear() noexcept
    {
        tasklist_.clear();
    }
};

}
