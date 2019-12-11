#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file

#include "catch2/catch.hpp"
#include "jg/dense_hash_map.hpp"
#include "jg/details/type_traits.hpp"

#include <string>
#include <vector>

namespace {
struct increase_counter_on_copy_or_move
{
    increase_counter_on_copy_or_move(std::size_t* counter_ptr)
        : counter_ptr{counter_ptr}
    {
    }

    increase_counter_on_copy_or_move(const increase_counter_on_copy_or_move& other)
        : counter_ptr(other.counter_ptr)
    {
        ++(*counter_ptr);
    }

    increase_counter_on_copy_or_move(increase_counter_on_copy_or_move&& other)
        : counter_ptr(std::move(other.counter_ptr))
    {
        ++(*counter_ptr);
    }

    auto operator=(const increase_counter_on_copy_or_move& other) -> increase_counter_on_copy_or_move&
    {
        counter_ptr = other.counter_ptr;
        ++(*counter_ptr);
        return *this;
    }
    
    auto operator=(increase_counter_on_copy_or_move&& other) -> increase_counter_on_copy_or_move&
    {
        counter_ptr = std::move(other.counter_ptr);
        ++(*counter_ptr);
        return *this;
    }

    std::size_t* counter_ptr = nullptr;
};

auto operator==(const increase_counter_on_copy_or_move&, const increase_counter_on_copy_or_move&) -> bool
{
    return true;
}

struct collision_hasher {
    template <class T>
    auto operator()(T&&) const noexcept -> std::size_t
    {
        return 0;
    }
};

}

namespace std {
    template<>
    struct hash<increase_counter_on_copy_or_move> {
        auto operator()(const increase_counter_on_copy_or_move& ) const noexcept -> std::size_t
        {
            return 0;
        }
    };
}

TEST_CASE("clear")
{
    jg::dense_hash_map<std::string, int> m;
    
    SECTION("empty")
    {
        m.clear();
        REQUIRE(m.empty());
        REQUIRE(m.size() == 0);
        REQUIRE(m.bucket_count() == 8u);
        REQUIRE(m.load_factor() == 0.0f);
    }

    SECTION("not_empty")
    {
        m.try_emplace("sponge bob", 10);
        m.try_emplace("sponge bob2", 10);
        REQUIRE(m.size() == 2);

        m.clear();
        REQUIRE(m.empty());
        REQUIRE(m.size() == 0);
        REQUIRE(m.bucket_count() == 8u);
        REQUIRE(m.load_factor() == 0.0f);
    }

    SECTION("no_except")
    {
        REQUIRE(noexcept(m.clear()));
    }
}

template <class T, class V>
using has_insert = decltype(std::declval<T>().insert(std::declval<V>()));

template <class T, class V>
using has_insert_hint = decltype(std::declval<T>().insert(std::declval<T>().begin(), std::declval<V>()));

TEST_CASE("insert")
{
    jg::dense_hash_map<std::string, int> m;

    SECTION("insert - lvalue")
    {
        auto pair = std::pair(std::string("test"), 42);
        const auto& [it, result] = m.insert(pair);
        REQUIRE(result);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);
        REQUIRE(m.size() == 1);
    }

    SECTION("insert - const lvalue")
    {
        const auto pair = std::pair(std::string("test"), 42);
        const auto& [it, result] = m.insert(pair);
        REQUIRE(result);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);
        REQUIRE(m.size() == 1);
    }

    SECTION("insert - rvalue")
    {
        const auto& [it, result] = m.insert(std::pair(std::string("test"), 42));
        REQUIRE(result);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);
        REQUIRE(m.size() == 1);
    }

    SECTION("insert - conversion")
    {
        auto pair = std::pair("test", 42);
        const auto& [it, result] = m.insert(pair);
        REQUIRE(result);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);
        REQUIRE(m.size() == 1);
    }

    SECTION("insert - conversion is SFINAE friendly")
    {
        REQUIRE_FALSE(jg::details::is_detected<has_insert, jg::dense_hash_map<std::vector<int>, int>, std::pair<bool, std::string>>::value);
    }

    SECTION("insert - lvalue - hint")
    {
        auto pair = std::pair(std::string("test"), 42);
        const auto& it = m.insert(m.begin(), pair);
        REQUIRE(it != m.end());
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);
        REQUIRE(m.size() == 1);
    }

    SECTION("insert - const lvalue - hint")
    {
        const auto pair = std::pair(std::string("test"), 42);
        const auto& it = m.insert(m.begin(), pair);
        REQUIRE(it != m.end());
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);
        REQUIRE(m.size() == 1);
    }

    SECTION("insert - rvalue - hint")
    {
        const auto& it = m.insert(m.begin(), std::pair(std::string("test"), 42));
        REQUIRE(it != m.end());
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);
        REQUIRE(m.size() == 1);
    }

    SECTION("insert - conversion - hint")
    {
        auto pair = std::pair("test", 42);
        const auto& it = m.insert(m.begin(), pair);
        REQUIRE(it != m.end());
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);
        REQUIRE(m.size() == 1);
    }

    SECTION("insert - conversion is SFINAE friendly - hint")
    {
        REQUIRE_FALSE(jg::details::is_detected<has_insert_hint, jg::dense_hash_map<std::vector<int>, int>, std::pair<bool, std::string>>::value);
    }

    SECTION("insert - iterator")
    {
        std::vector<std::pair<std::string, int>> v{{"test", 42}, {"test2", 1337}};
        m.insert(v.begin(), v.end());

        REQUIRE(m.size() == 2);

        auto it = m.find("test");
        REQUIRE(it != m.end());
        REQUIRE(it->second == 42);

        it = m.find("test2");
        REQUIRE(it != m.end());
        REQUIRE(it->second == 1337);
    }

    SECTION("insert - initializer_list")
    {
        std::initializer_list<std::pair<const std::string, int>> l{{"test", 42}, {"test2", 1337}};
        m.insert(l);

        REQUIRE(m.size() == 2);

        auto it = m.find("test");
        REQUIRE(it != m.end());
        REQUIRE(it->second == 42);

        it = m.find("test2");
        REQUIRE(it != m.end());
        REQUIRE(it->second == 1337);
    }
}

TEST_CASE("insert_or_assign")
{
    jg::dense_hash_map<std::string, int> m1;
    jg::dense_hash_map<std::unique_ptr<int>, int> m2;
    

    SECTION("l-value key")
    {
        const auto& [it, result] = m1.insert_or_assign("test", 42);
        REQUIRE(result);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);

        const auto& [it2, result2] = m1.insert_or_assign("test", 1337);
        REQUIRE_FALSE(result2);
        REQUIRE(it2 == it);
        REQUIRE(it2->first == "test");
        REQUIRE(it2->second == 1337);
    }

    SECTION("r-value key")
    {
        const auto& [it, result] = m2.insert_or_assign(nullptr, 42);
        REQUIRE(result);
        REQUIRE(it->first == nullptr);
        REQUIRE(it->second == 42);

        std::unique_ptr<int> p = nullptr;
        const auto& [it2, result2] = m2.insert_or_assign(std::move(p), 1337);
        REQUIRE_FALSE(result2);
        REQUIRE(it2 == it);
        REQUIRE(it2->first == nullptr);
        REQUIRE(it2->second == 1337);
    }

    SECTION("l-value key - hint")
    {
        auto it = m1.insert_or_assign(m1.begin(), "test", 42);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);

        auto it2 = m1.insert_or_assign(m1.begin(), "test", 1337);
        REQUIRE(it == it);
        REQUIRE(it2->first == "test");
        REQUIRE(it2->second == 1337);
    }

    SECTION("r-value key - hint")
    {
        auto it = m2.insert_or_assign(m2.begin(), nullptr, 42);
        REQUIRE(it->first == nullptr);
        REQUIRE(it->second == 42);

        std::unique_ptr<int> p = nullptr;
        auto it2 = m2.insert_or_assign(m2.begin(), std::move(p), 1337);
        REQUIRE(it2 == it);
        REQUIRE(it2->first == nullptr);
        REQUIRE(it2->second == 1337);
    }
}

TEST_CASE("emplace", "[emplace]")
{
    jg::dense_hash_map<std::string, int> m;

    SECTION("default")
    {
        const auto& [it, result] = m.emplace();
        REQUIRE(result);
        REQUIRE(it->first == std::string());
        REQUIRE(it->second == int{});
        REQUIRE(m.size() == 1);
    }

    SECTION("once - rvalues")
    { 
        const auto& [it, result] = m.emplace("test", 42); 

        REQUIRE(result);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);
        REQUIRE(m.size() == 1);
    }

    SECTION("once - l-values")
    { 
        std::string key = "test"; 
        int value = 42; 
        const auto& [it, result] = m.emplace(key, value); 

        REQUIRE(result);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);
        REQUIRE(m.size() == 1);
    }

    SECTION("once - const l-values")
    { 
        const std::string key = "test"; 
        const int value = 42; 
        const auto& [it, result] = m.emplace(key, value); 

        REQUIRE(result);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);
        REQUIRE(m.size() == 1);
    }

    SECTION("pair - lvalue")
    {
        auto pair = std::pair(std::string("test"), 42);
        const auto& [it, result] = m.emplace(pair);
        REQUIRE(result);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);
        REQUIRE(m.size() == 1);
    }

    SECTION("pair - const lvalue")
    {
        const auto pair = std::pair(std::string("test"), 42);
        const auto& [it, result] = m.emplace(pair);
        REQUIRE(result);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);
        REQUIRE(m.size() == 1);
    }

    SECTION("pair - rvalue")
    {
        const auto& [it, result] = m.emplace(std::pair(std::string("test"), 42));
        REQUIRE(result);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);
        REQUIRE(m.size() == 1);
    }

    SECTION("conversion")
    {
        const auto& [it, result] = m.emplace("test", 42); // key: const char* ==> std::string
        REQUIRE(result);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);
        REQUIRE(m.size() == 1);
    }

    SECTION("conversion pair")
    {
        const auto& [it, result] = m.emplace(std::pair("test", 42)); // key: const char* ==> std::string
        REQUIRE(result);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);
        REQUIRE(m.size() == 1);
    }

    SECTION("piecewise_construct")
    {
        const auto& [it, result] = m.emplace(std::piecewise_construct, std::forward_as_tuple("test"), std::forward_as_tuple(42));
        REQUIRE(result);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);
        REQUIRE(m.size() == 1);
    }
}

TEST_CASE("emplace key rvalue")
{
    jg::dense_hash_map<std::unique_ptr<int>, int> m; 

    SECTION("Successfull emplace")
    {
        auto ptr = std::make_unique<int>(37);
        const auto& [it, result] = m.emplace(std::move(ptr), 42);
        REQUIRE(result);
        REQUIRE(*it->first == 37);
        REQUIRE(it->second == 42);
        REQUIRE(ptr == nullptr);
    }
}

TEST_CASE("emplace twice", "[emplace]")
{
    jg::dense_hash_map<std::string, int> m;

    SECTION("twice same")
    { 
        const auto& [it, result] = m.emplace("test", 42); 
        REQUIRE(result);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);

        const auto& [it2, result2] = m.emplace("test", 42); 
        REQUIRE_FALSE(result2);
        REQUIRE(it == it2);
        REQUIRE(it2->first == "test");
        REQUIRE(it2->second == 42);
        REQUIRE(m.size() == 1);
    }

    SECTION("twice different")
    { 
        const auto& [it, result] = m.emplace("test", 42); 
        REQUIRE(result);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);

        const auto& [it2, result2] = m.emplace("test2", 1337); 
        REQUIRE(result2);
        REQUIRE(it != it2);
        REQUIRE(it2->first == "test2");
        REQUIRE(it2->second == 1337);
        REQUIRE(m.size() == 2);
    }
}

TEST_CASE("emplace optimization", "[emplace]")
{
    std::size_t counter{};

    jg::dense_hash_map<increase_counter_on_copy_or_move, int> m;
    const auto& [it, result] = m.emplace(&counter, 42);

    REQUIRE(result);
    REQUIRE(counter > 0);

    auto counter_after_insertion = counter;

    SECTION("key not copied if not inserted")
    {
        increase_counter_on_copy_or_move key {&counter};
        m.emplace(key, 42);

        REQUIRE(counter_after_insertion == counter);
    }

    SECTION("pair's key copied moved if not inserted")
    {
        increase_counter_on_copy_or_move key {&counter};
        std::pair<increase_counter_on_copy_or_move&, int> p(key, 42);
        m.emplace(p);

        REQUIRE(counter_after_insertion == counter);
    }

    SECTION("key not moved if not inserted")
    {
        increase_counter_on_copy_or_move key {&counter};
        m.emplace(std::move(key), 42);

        REQUIRE(counter_after_insertion == counter);
    }

    SECTION("pair's key not moved if not inserted")
    {
        std::pair<increase_counter_on_copy_or_move, int> p(&counter, 42);
        m.emplace(std::move(p));

        REQUIRE(counter_after_insertion == counter);
    }
}

TEST_CASE("emplace_hint")
{
    jg::dense_hash_map<std::string, int> m;

    SECTION("one")
    {
        const auto& it = m.emplace_hint(m.begin(), "bob", 666);
        REQUIRE(it != m.end());
        REQUIRE(it->first == "bob");
        REQUIRE(it->second == 666);
        REQUIRE(m.size() == 1);
    }

    SECTION("twice")
    {
        const auto& it1 = m.emplace_hint(m.begin(), "bob", 666);
        const auto& it2 = m.emplace_hint(m.begin(), "bob", 444);
        REQUIRE(it1 == it2);
        REQUIRE(it2->first == "bob");
        REQUIRE(it2->second == 666);
        REQUIRE(m.size() == 1);
    }
}

TEST_CASE("try emplace", "[try_emplace]")
{
    jg::dense_hash_map<std::string, int> m;

    SECTION("rvalues")
    { 
        const auto& [it, result] = m.try_emplace("test", 42); 

        REQUIRE(result);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);
        REQUIRE(m.size() == 1);
    }

    SECTION("l-values")
    { 
        std::string key = "test"; 
        int value = 42; 
        const auto& [it, result] = m.try_emplace(key, value); 

        REQUIRE(result);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);
        REQUIRE(m.size() == 1);
    }

    SECTION("const l-values")
    { 
        const std::string key = "test"; 
        const int value = 42; 
        const auto& [it, result] = m.try_emplace(key, value); 

        REQUIRE(result);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);
        REQUIRE(m.size() == 1);
    }
}

TEST_CASE("try_emplace twice", "[try_emplace]")
{
    jg::dense_hash_map<std::string, int> m;

    SECTION("twice same")
    { 
        const auto& [it, result] = m.try_emplace("test", 42); 
        REQUIRE(result);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);

        const auto& [it2, result2] = m.try_emplace("test", 42); 
        REQUIRE_FALSE(result2);
        REQUIRE(it == it2);
        REQUIRE(it2->first == "test");
        REQUIRE(it2->second == 42);
        REQUIRE(m.size() == 1);
    }

    SECTION("twice different")
    { 
        const auto& [it, result] = m.try_emplace("test", 42); 
        REQUIRE(result);
        REQUIRE(it->first == "test");
        REQUIRE(it->second == 42);

        const auto& [it2, result2] = m.try_emplace("test2", 1337); 
        REQUIRE(result2);
        REQUIRE(it != it2);
        REQUIRE(it2->first == "test2");
        REQUIRE(it2->second == 1337);
        REQUIRE(m.size() == 2);
    }
}

TEST_CASE("try_emplace effects guarantees", "[try_emplace]")
{
    std::size_t counter{};

    jg::dense_hash_map<increase_counter_on_copy_or_move, int> m;
    const auto& [it, result] = m.emplace(&counter, 42);

    REQUIRE(result);
    REQUIRE(counter > 0);

    auto counter_after_insertion = counter;

    SECTION("key not copied if not inserted")
    {
        increase_counter_on_copy_or_move key {&counter};
        m.try_emplace(key, 42);

        REQUIRE(counter_after_insertion == counter);
    }

    SECTION("key not moved if not inserted")
    {
        increase_counter_on_copy_or_move key {&counter};
        m.try_emplace(std::move(key), 42);

        REQUIRE(counter_after_insertion == counter);
    }
}

TEST_CASE("erase iterator", "[erase]")
{
    jg::dense_hash_map<std::string, int> m;
    const auto& [useless1, result1] = m.emplace("bob", 42);
    const auto& [useless2, result2] = m.emplace("jacky", 42);
    const auto& [useless3, result3] = m.emplace("snoop", 42);

    REQUIRE(result1);
    REQUIRE(result2);
    REQUIRE(result3);
    REQUIRE(m.find("bob") != m.end());
    REQUIRE(m.find("jacky") != m.end());
    REQUIRE(m.find("snoop") != m.end());
    REQUIRE(m.size() == 3);

    SECTION("using first iterator")
    {
        auto new_it = m.erase(m.begin());
        REQUIRE(new_it != m.end());
        REQUIRE(m.size() == 2);
        REQUIRE(new_it->first == "snoop");
        REQUIRE(m.find("bob") == m.end());
        REQUIRE(m.find("jacky") != m.end());
        REQUIRE(m.find("snoop") != m.end());
    }

    SECTION("using middle iterator")
    {
        auto it = m.find("jacky");
        REQUIRE(it != m.end());
        auto new_it = m.erase(it);
        REQUIRE(new_it != m.end());
        REQUIRE(m.size() == 2);
        REQUIRE(new_it->first == "snoop");
        REQUIRE(m.find("bob") != m.end());
        REQUIRE(m.find("jacky") == m.end());
        REQUIRE(m.find("snoop") != m.end());
    }

    SECTION("using last iterator")
    {
        auto new_it = m.erase(std::prev(m.end()));
        REQUIRE(new_it == m.end());
        REQUIRE(m.size() == 2);
        REQUIRE(m.find("bob") != m.end());
        REQUIRE(m.find("jacky") != m.end());
        REQUIRE(m.find("snoop") == m.end());
    }
}

TEST_CASE("erase key", "[erase]")
{
    jg::dense_hash_map<std::string, int> m;
    const auto& [useless1, result1] = m.emplace("bob", 42);
    const auto& [useless2, result2] = m.emplace("jacky", 42);
    const auto& [useless3, result3] = m.emplace("snoop", 42);

    REQUIRE(result1);
    REQUIRE(result2);
    REQUIRE(result3);
    REQUIRE(m.find("bob") != m.end());
    REQUIRE(m.find("jacky") != m.end());
    REQUIRE(m.find("snoop") != m.end());
    REQUIRE(m.size() == 3);

    SECTION("success")
    {
        REQUIRE(m.erase("bob") > 0);
        REQUIRE(m.size() == 2);
        REQUIRE(m.find("bob") == m.end());
        REQUIRE(m.find("jacky") != m.end());
        REQUIRE(m.find("snoop") != m.end());
    }

    SECTION("failure")
    {
        REQUIRE(m.erase("bobby") == 0);
        REQUIRE(m.size() == 3);
        REQUIRE(m.find("bob") != m.end());
        REQUIRE(m.find("jacky") != m.end());
        REQUIRE(m.find("snoop") != m.end());
    }
}

TEST_CASE("erase with collisions", "[erase]")
{
    jg::dense_hash_map<std::string, int, collision_hasher> m;

    const auto& [useless1, result1] = m.emplace("bob", 42);
    const auto& [useless2, result2] = m.emplace("jacky", 42);
    const auto& [useless3, result3] = m.emplace("snoop", 42);

    REQUIRE(result1);
    REQUIRE(result2);
    REQUIRE(result3);
    auto bob_it = m.find("bob");
    auto jacky_it = m.find("jacky");
    auto snoop_it = m.find("snoop");
    REQUIRE(bob_it != m.end());
    REQUIRE(jacky_it != m.end());
    REQUIRE(snoop_it != m.end());
    REQUIRE(m.size() == 3);

    SECTION("remove first in bucket")
    {
        REQUIRE(m.erase("snoop") > 0);
        REQUIRE(m.size() == 2);
        REQUIRE(m.find("bob") != m.end());
        REQUIRE(m.find("jacky") != m.end());
        REQUIRE(m.find("snoop") == m.end());
    }

    SECTION("remove mid in bucket")
    {
        REQUIRE(m.erase("jacky") > 0);
        REQUIRE(m.size() == 2);
        REQUIRE(m.find("bob") != m.end());
        REQUIRE(m.find("jacky") == m.end());
        REQUIRE(m.find("snoop") != m.end());
    }

    SECTION("remove last in bucket")
    {
        REQUIRE(m.erase("bob") > 0);
        REQUIRE(m.size() == 2);
        REQUIRE(m.find("bob") == m.end());
        REQUIRE(m.find("jacky") != m.end());
        REQUIRE(m.find("snoop") != m.end());
    }
}

TEST_CASE("range erase")
{
    jg::dense_hash_map<std::string, int> m;
    const auto& [useless1, result1] = m.emplace("bob", 42);
    const auto& [useless2, result2] = m.emplace("jacky", 42);
    const auto& [useless3, result3] = m.emplace("snoop", 42);

    REQUIRE(result1);
    REQUIRE(result2);
    REQUIRE(result3);
    REQUIRE(m.find("bob") != m.end());
    REQUIRE(m.find("jacky") != m.end());
    REQUIRE(m.find("snoop") != m.end());
    REQUIRE(m.size() == 3);

    SECTION("all")
    {
        auto it = m.erase(m.begin(), m.end());
        REQUIRE(it == m.end());
        REQUIRE(m.size() == 0);
        REQUIRE(m.find("bob") == m.end());
        REQUIRE(m.find("jacky") == m.end());
        REQUIRE(m.find("snoop") == m.end());
    }

    SECTION("two first")
    {
        auto it = m.erase(m.begin(), std::prev(m.end()));
        REQUIRE(it->first == "snoop");
        REQUIRE(m.size() == 1);
        REQUIRE(m.find("bob") == m.end());
        REQUIRE(m.find("jacky") == m.end());
        REQUIRE(m.find("snoop") != m.end());
    }

    SECTION("two last")
    {
        auto it = m.erase(std::next(m.begin()), m.end());
        REQUIRE(it == m.end());
        REQUIRE(m.size() == 1);
        REQUIRE(m.find("bob") != m.end());
        REQUIRE(m.find("jacky") == m.end());
        REQUIRE(m.find("snoop") == m.end());
    }

    SECTION("none")
    {
        auto it = m.erase(m.begin(), m.begin());
        REQUIRE(it == m.begin());
        REQUIRE(m.size() == 3);
        REQUIRE(m.find("bob") != m.end());
        REQUIRE(m.find("jacky") != m.end());
        REQUIRE(m.find("snoop") != m.end());
    }
}

TEST_CASE("Insertions trigger a rehash")
{

}

TEST_CASE("Move only types")
{
}

TEST_CASE("rehash")
{

}
