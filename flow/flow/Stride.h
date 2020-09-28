#pragma once

#include <optional>

#include <flow/Fuse.h>

namespace flow
{
    /// Before yielding the next element, skip `n - 1` elements.
	/// Arity: 1 -> 1
    template<class Seq>
    class Stride
    {
    public:
        static inline bool constexpr finite = Seq::finite;
        using output_type = typename Seq::output_type;

        Stride(Seq const &base, size_t const n):
            base(Fuse(base)),
            n(n)
        {}

        std::optional<output_type> next()
        {
            std::optional<output_type> state(base.next());

            if (state.has_value())
            {
                // Skip `n - 1` elements.
                // Because base is fused, it guarantees that consecutive calls to
                // `next()` return `None`.
                for (size_t k = 0; k < n - 1; ++k) {
                    base.next();
                }
            }

            return state;
        }

    private:
        Fuse<Seq> base;
        size_t n;
    };

    auto stride(size_t const n)
    {
        return [=] (auto &&seq)
        {
            return Stride(std::forward<decltype(seq)>(seq), n);
        };
    }
}