#pragma once

#include <optional>

namespace flow
{
    /// Calls a function on sequence elements, but passes the elements unchanged otherwise.
	/// Arity: 1 -> 1
    template<class Seq, class Fn>
    class Inspect
    {
    public:
        static inline bool constexpr finite = Seq::finite;
        using output_type = typename Seq::output_type;

        Inspect(Seq const &base, Fn fn):
            base(base),
            fn(fn)
        {
        }

        std::optional<output_type> next()
        {
            std::optional<output_type> state(base.next());
            if (state.has_value())
            {
                // Call inspection functional on contained element,
                // but ensure that the element is not modified.
                fn(static_cast<output_type const &>(state.value()));
            }
            return state;
        }

    private:
        Seq base;
        Fn fn;
    };

    template<class Fn>
    auto inspect(Fn fn)
    {
        return [=] (auto &&seq)
        {
            return Inspect(std::forward<decltype(seq)>(seq), fn);
        };
    }
}