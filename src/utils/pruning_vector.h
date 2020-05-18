/*
 * File: pruning_vector.h
 * Author: <contact@simshadows.com>
 *
 * This is an unstable-order automatically-pruned data structure.
 * This means that the sorted order will never be guaranteed.
 *
 * class T:
 *      Container data.
 *
 * class CanReplaceFn:
 *      A standalone function object:
 *          struct MyFn {
 *              bool operator()(const T& left, const T& right) const noexcept;
 *          };
 *      This function will return true if and only if left arg is guaranteed to be prune
 *      out right arg. If both arguments are considered equal, the function must also return
 *      true.
 *
 *      This function may be a constant function that always returns false. This will
 *      cause no pruning to happen, but at that point, you may as well just use a bare
 *      std::vector.
 */

#ifndef PRUNING_VECTOR_H
#define PRUNING_VECTOR_H

#include <assert.h>
#include <vector>

namespace Utils {


template<class T, class CanReplaceFn>
class PruningVector {
    using C = std::vector<T>;

    C data;
public:
    //PruningVector() noexcept = default;

    template <class... Args>
    PruningVector(Args&&... args) noexcept
        : data        (std::forward<Args>(args)...)
    {
    }

    /*
     * new stuff
     */

    // TODO: Initialize the entire class instance with these variadic arguments somehow.
    void try_push_back(T&& t) {

        // Prune away all existing data that t is able to replace.
        const auto pred = [&](const T& d) {
            return CanReplaceFn()(t, d);
        };
        const typename C::iterator new_end = std::remove_if(this->data.begin(), this->data.end(), pred);

        if (new_end == this->data.end()) {
            // Nothing was pruned by t, so we'll need to figure out if existing data prunes out t.
            for (const T& d : data) {
                if (CanReplaceFn()(d, t)) return;
            }
        } else {
            // t prunes something out, so we definitely don't prune out t.
            this->data.erase(new_end, this->data.end());
        }

        // Add data
        this->data.push_back(std::move(t));
    }

    /*
     * reference to the underlying STL container
     */

    const C& underlying() const noexcept {
        return this->data;
    }

    /*
     * direct adapted interface
     */

    auto begin() const noexcept {
        return this->data.begin();
    }

    auto end() const noexcept {
        return this->data.end();
    }

    auto size() const noexcept {
        return this->data.size();
    }
};


} // namespace

#endif // PRUNING_VECTOR_H

