//
// Created by jim on 20.05.19.
//

#pragma once

#include "sequences/tools.h"

namespace sequences
{
template<class Iter, class Fn>
class OnEach
{
public:
    using value_type = iter_value_type_t<Iter>;

private:
    Iter m_iter;
    Fn m_fn;

public:

    OnEach() = default;
    OnEach(const OnEach &rhs) = default;
    OnEach(OnEach &&rhs) noexcept = default;

    OnEach &operator=(const OnEach &rhs) = default;
    OnEach &operator=(OnEach &&rhs) noexcept = default;

    OnEach(
            const Iter &iter,
            Fn fn
    ) :
            m_iter{iter},
            m_fn{fn}
    {
    }

    const value_type &operator*() const
    {
        const value_type &n = *m_iter;
        m_fn(n);
        return n;
    }

    const value_type &operator*()
    {
        value_type &n = *m_iter;
        m_fn(n);
        return n;
    }

    OnEach &operator++()
    {
        ++m_iter;
        return *this;
    }

    OnEach operator+(const size_t offset) const
    {
        OnEach result{m_iter, m_fn};
        for (size_t i = 0; i < offset; i++)
        {
            ++result.m_iter;
        }
        return result;
    }

    bool operator==(const OnEach &rhs) const
    {
        return m_iter == rhs.m_iter;
    }

    bool operator!=(const OnEach &rhs) const
    {
        return m_iter != rhs.m_iter;
    }
};

template<class Iter, class Fn>
OnEach<Iter, Fn> make_on_each(
        const Iter &iter,
        Fn fn
)
{
    return OnEach<Iter, Fn>{iter, fn};
}

}
