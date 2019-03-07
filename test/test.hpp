#pragma once
#include <list>
#include <algorithm>
#include <string>
#include <iostream>
namespace test {

	class test_base {
	protected:
		const std::string name_;
		static std::list<test_base*>& test_list_() {
			static std::list<test_base*> tl;
			return tl;
		}
		bool success_ = true;
		test_base(std::string && name) : name_{ name } {}
	public:
		virtual void operator()() = 0;
		inline static void test() {
			using namespace std::string_literals;
			int error_cnt = 0;
			std::cout << "running " << test_list_().size() << " test cases..." << std::endl;
			for (auto & i : test_list_()) {
				try { (*i)(); }
				catch (std::exception & e) {
					std::cout << e.what() << std::endl;
					++error_cnt;
					continue;
				}
				catch (...) {
					std::cout << "detected unhandled exception!" << std::endl;
					++error_cnt;
					continue;
				}
				error_cnt += i->success_ ? 0 : 1;
			}
			std::cout << error_cnt << " errors detected." << std::endl;
		}
		virtual ~test_base() {
			auto itr = std::find(std::begin(test_list_()), std::end(test_list_()), this);
			test_list_().erase(itr);
		}
	};

}
#define LOG(str)	do{\
	auto outstr = name_ + "'s log: " + std::string(str);\
	std::cout << outstr << '\n';\
}while(false)

#define FAILED_LOG(expr, test_name, additional_msg) do{\
	using namespace std::string_literals;\
	auto outstr = test_name + " failed. "s + "\""s + #expr ## s + "\" "s + std::string(additional_msg);\
	std::cout << outstr << '\n';\
    success_ = false;\
}while(false)

#define REQUIRE_TRUE(expr) do{\
	try{\
		if(!(expr)) {\
			using namespace std::string_literals;\
			FAILED_LOG(expr, name_, "is false\n"s);\
			return;\
		}\
	} catch (std::exception & e) {\
		FAILED_LOG(expr, name_, e.what());\
		return;\
	}\
}while(false)

#define CHECK_TRUE(expr) do{\
	using namespace std::string_literals;\
	try{\
		if(!(expr)) FAILED_LOG(expr, name_, "is false\n"s);\
	} catch (std::exception & e) {\
		FAILED_LOG(expr, name_, e.what());\
	}\
}while(false)

#define CHECK_NOTHROW(expr) do{\
	using namespace std::string_literals;\
	try{\
		expr;\
	} catch (std::exception & e) { FAILED_LOG(expr, name_, e.what()); }\
	catch(const std::string & str){ FAILED_LOG(expr, name_, str); }\
	catch(...){FAILED_LOG(expr, name_, "something happened\n");}\
} while(false)

#define CHECK_THROW(expr) do{\
	try{\
		expr;\
		FAILED_LOG(expr, name_, "exception are not thrown.\n");\
	} catch(...){}\
} while(false)

#define CHECK_SPECIFIC_EXCEPTION(expr, exception_type) do{\
	try{\
		expr; \
		FAILED_LOG(expr, name_, "exception are not thrown\n");\
	} catch(exception_type){}\
	catch(...){\
		FAILED_LOG(expr, name_, "unexcepted exception was thrown\n");\
	}\
}while(false)\

#define REQUIRE_EQUAL(expr, value) REQUIRE_TRUE((expr) == (value))
#define CHECK_EQUAL(expr, value) CHECK_TRUE((expr) == (value))

#define DEFINE_TEST(test_name) \
namespace test{\
class test_name : public ::test::test_base{\
public:\
	test_name() : test_base(#test_name) {test_list_().push_back(this);}\
	void operator()() override;\
};\
test_name test_name ## _instance;\
}\
inline void test:: test_name::operator()()

#define OUCHI_TEST_MAIN \
int main(void){\
    ::test::test_base::test();\
}
