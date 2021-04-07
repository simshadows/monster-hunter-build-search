/*
 * File: pruning_vector.h
 * Author: <contact@simshadows.com>
 *
 * This is an unstable-order automatically-pruned data structure.
 * This means that the sorted order will never be guaranteed.
 *
 * TODO: This probably doesn't deserve to be its own class. I should probably reimplement
 *       this as a standalone function.
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

        // Figure out if existing data prunes out t.
        // NOTE: We use a reverse-iterator because new elements are appended to the end, and those
        //       new elements tend to prune away incoming data better.
        //       Testing shows a VERY SIGNIFICANT runtime improvement.
        //       For MHWIBS, it tends to be of the order of x30 or better!
        for (auto e = data.rbegin(); e != data.rend(); ++e) {
            const auto& d = *e;
            if (CanReplaceFn()(d, t)) return;
        }

        // Prune away all existing data that t is able to replace.
        const auto pred = [&](const T& d) {
            return CanReplaceFn()(t, d);
        };
        const typename C::iterator new_end = std::remove_if(this->data.begin(), this->data.end(), pred);

        //if (new_end != this->data.end()) {
        this->data.erase(new_end, this->data.end());
        //}

        // Add data
        this->data.push_back(std::move(t));
    }

    // This version changes some internal order of operations, but runs slower (except possibly in
    // some very rare use-cases).
    // This can be useful for further verification that a CanReplaceFn is permissible.
    void try_push_back_altimpl(T&& t) {

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

    C underlying() const noexcept {
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

