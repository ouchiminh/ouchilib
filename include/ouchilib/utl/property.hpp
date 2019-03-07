#pragma once
#include <functional>
#include <type_traits>

namespace ouchi {

template<class T>
class property {
public:
	using ValueType = typename ::std::remove_reference_t<T>;
	using Setter = std::function<ValueType(ValueType const &)>;
	using Getter = std::function<ValueType(ValueType const &)>;

	explicit operator ValueType() const noexcept {
		return getter_(val_);
	}
	ValueType operator=(ValueType const & v) noexcept {
		val_ = setter_(v);
		return val_;
	}
	ValueType operator= (ValueType && v) noexcept {
		val_ = setter_(std::move(v));
		return val_;
	}
	property(Getter && getter, Setter && setter, ValueType && val = ValueType{}) :
		getter_(std::move(getter)),
		setter_(std::move(setter)),
		val_{ std::move(val) }
	{
		if (!getter_ || !setter_) throw std::exception("no accessor was set!");
	}
	property(Getter && getter, Setter && setter, ValueType const & val = ValueType{}) :
		getter_(std::move(getter)),
		setter_(std::move(setter)),
		val_{ val }
	{
		if (!getter_ || !setter_) throw std::exception("no accessor was set!");
	}
	property() :
		getter_{ [](ValueType const & v) { return v; } },
		setter_{ [](ValueType const & v) { return v; } }
	{}
	property(ValueType && initial) :
		getter_{ [](ValueType const & v) { return v; } },
		setter_{ [](ValueType const & v) { return v; } },
		val_{std::forward<ValueType>(initial)}
	{}
	property(ValueType const & initial) :
		getter_{ [](ValueType const & v) { return v; } },
		setter_{ [](ValueType const & v) { return v; } },
		val_{initial}
	{}


	
private:
	ValueType val_;
	Getter getter_;
	Setter setter_;
};

}