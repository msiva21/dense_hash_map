#ifndef JG_DENSE_HASH_MAP_ITERATOR_HPP
#define JG_DENSE_HASH_MAP_ITERATOR_HPP

#include "node.hpp"

#include <type_traits>
#include <vector>

namespace jg::details
{

template <class Key, class T, bool isConst, bool projectToConstKey>
class dense_hash_map_iterator
{
private:
    friend dense_hash_map_iterator<Key, T, true, projectToConstKey>;

    using entries_container_type = std::vector<node<Key, T>>;
    using sub_iterator_type = typename std::conditional<
        isConst, typename entries_container_type::const_iterator,
        typename entries_container_type::iterator>::type;
    using sub_iterator_type_traits = std::iterator_traits<sub_iterator_type>;
    using projected_type =
        std::pair<typename std::conditional<projectToConstKey, const Key, Key>::type, T>;

public:
    using iterator_category = typename sub_iterator_type_traits::iterator_category;
    using value_type = std::conditional_t<isConst, const projected_type, projected_type>;
    using difference_type = typename sub_iterator_type_traits::difference_type;
    using reference = value_type&;
    using pointer = value_type*;

    constexpr dense_hash_map_iterator() noexcept : sub_iterator_(sub_iterator_type{}) {}

    explicit constexpr dense_hash_map_iterator(sub_iterator_type it) noexcept : sub_iterator_(std::move(it))
    {}

    constexpr dense_hash_map_iterator(const dense_hash_map_iterator& other) noexcept = default;
    constexpr dense_hash_map_iterator(dense_hash_map_iterator&& other) noexcept = default;

    constexpr dense_hash_map_iterator& operator=(const dense_hash_map_iterator& other) noexcept = default;
    constexpr dense_hash_map_iterator& operator=(dense_hash_map_iterator&& other) noexcept = default;

    template <bool DepIsConst = isConst, std::enable_if_t<DepIsConst, int> = 0>
    constexpr dense_hash_map_iterator(
        const dense_hash_map_iterator<Key, T, false, projectToConstKey>& other) noexcept
        : sub_iterator_(other.sub_iterator_)
    {}

    constexpr auto operator*() const noexcept -> reference
    {
        if constexpr (projectToConstKey)
        {
            return sub_iterator_->pair.const_;
        }
        else
        {
            return sub_iterator_->pair.non_const_;
        }
    }

    constexpr auto operator-> () const noexcept -> pointer
    {
        if constexpr (projectToConstKey)
        {
            return &(sub_iterator_->pair.const_);
        }
        else
        {
            return &(sub_iterator_->pair.non_const_);
        }
    }

    constexpr auto operator++() noexcept -> dense_hash_map_iterator&
    {
        ++sub_iterator_;
        return *this;
    }

    constexpr auto operator++(int) noexcept -> dense_hash_map_iterator { return {sub_iterator_++}; }

    constexpr auto operator--() noexcept -> dense_hash_map_iterator&
    {
        --sub_iterator_;
        return *this;
    }

    constexpr auto operator--(int) noexcept -> dense_hash_map_iterator { return {sub_iterator_--}; }

    constexpr auto operator[](difference_type index) const noexcept -> reference
    {
        if constexpr (projectToConstKey)
        {
            return sub_iterator_[index]->pair.const_;
        }
        else
        {
            return sub_iterator_[index]->pair.non_const_;
        }
    }

    constexpr auto operator+=(difference_type n) noexcept -> dense_hash_map_iterator&
    {
        sub_iterator_ += n;
        return *this;
    }

    constexpr auto operator+(difference_type n) const noexcept -> dense_hash_map_iterator
    {
        return {sub_iterator_ + n};
    }

    constexpr auto operator-=(difference_type n) noexcept -> dense_hash_map_iterator&
    {
        sub_iterator_ -= n;
        return *this;
    }

    constexpr auto operator-(difference_type n) const noexcept -> dense_hash_map_iterator
    {
        return {sub_iterator_ - n};
    }

    auto sub_iterator() const -> const sub_iterator_type& { return sub_iterator_; }

private:
    sub_iterator_type sub_iterator_;
};

template <class Key, class T, bool isConst, bool projectToConstKey>
constexpr auto operator==(
    const dense_hash_map_iterator<Key, T, isConst, projectToConstKey>& lhs,
    const dense_hash_map_iterator<Key, T, isConst, projectToConstKey>& rhs) noexcept -> bool
{
    return lhs.sub_iterator() == rhs.sub_iterator();
}

template <class Key, class T, bool isConst, bool projectToConstKey>
constexpr auto operator!=(
    const dense_hash_map_iterator<Key, T, isConst, projectToConstKey>& lhs,
    const dense_hash_map_iterator<Key, T, isConst, projectToConstKey>& rhs) noexcept -> bool
{
    return lhs.sub_iterator() != rhs.sub_iterator();
}

template <class Key, class T, bool isConst, bool projectToConstKey>
constexpr auto operator<(
    const dense_hash_map_iterator<Key, T, isConst, projectToConstKey>& lhs,
    const dense_hash_map_iterator<Key, T, isConst, projectToConstKey>& rhs) noexcept -> bool
{
    return lhs.sub_iterator() < rhs.sub_iterator();
}

template <class Key, class T, bool isConst, bool projectToConstKey>
constexpr auto operator>(
    const dense_hash_map_iterator<Key, T, isConst, projectToConstKey>& lhs,
    const dense_hash_map_iterator<Key, T, isConst, projectToConstKey>& rhs) noexcept -> bool
{
    return lhs.sub_iterator() > rhs.sub_iterator();
}

template <class Key, class T, bool isConst, bool projectToConstKey>
constexpr auto operator<=(
    const dense_hash_map_iterator<Key, T, isConst, projectToConstKey>& lhs,
    const dense_hash_map_iterator<Key, T, isConst, projectToConstKey>& rhs) noexcept -> bool
{
    return lhs.sub_iterator() <= rhs.sub_iterator();
}

template <class Key, class T, bool isConst, bool projectToConstKey>
constexpr auto operator>=(
    const dense_hash_map_iterator<Key, T, isConst, projectToConstKey>& lhs,
    const dense_hash_map_iterator<Key, T, isConst, projectToConstKey>& rhs) noexcept -> bool
{
    return lhs.sub_iterator() >= rhs.sub_iterator();
}

template <class Key, class T, bool isConst, bool projectToConstKey>
constexpr auto operator-(
    const dense_hash_map_iterator<Key, T, isConst, projectToConstKey>& lhs,
    const dense_hash_map_iterator<Key, T, isConst, projectToConstKey>& rhs) noexcept ->
    typename dense_hash_map_iterator<Key, T, isConst, projectToConstKey>::difference_type
{
    return lhs.sub_iterator() - rhs.sub_iterator();
}

template <class Key, class T, bool isConst, bool projectToConstKey>
constexpr auto operator+(
    typename dense_hash_map_iterator<Key, T, isConst, projectToConstKey>::difference_type n,
    const dense_hash_map_iterator<Key, T, isConst, projectToConstKey>& it) noexcept
    -> dense_hash_map_iterator<Key, T, isConst, projectToConstKey>
{
    return {n + it.sub_iterator()};
}

} // namespace jg::details

#endif // JG_DENSE_HASH_MAP_ITERATOR_HPP
