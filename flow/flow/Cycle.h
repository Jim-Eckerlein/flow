#pragma once

#include <flow/details.h>
#include <flow/Maybe.h>

namespace flow
{
    /// Cycles the underlying sequence, i.e. after the last element is returned,
    /// another call to `next()` will return the first element of the underlying sequence.
    /// This sequence will never return `None`, unless the base sequence is empty.
    /// In that case, this sequence will always return `None`.
    template<class S>
    class Cycle
    {
    public:
        using ElementType = typename S::ElementType;
        
        explicit Cycle(S &&sequence):
            base(std::move(sequence)),
            sequence(base)
        {
        }
        
        Maybe<ElementType> next()
        {
            Maybe<ElementType> nextElement = sequence.next();
            
            if (nextElement.hasValue())
            {
                return nextElement;
            }
            else
            {
                // The sequence is exhausted.
                // Restart by re-initializing the sequence to the base sequence.
                // This is also done if the sequence is empty, but an empty sequence is probably not too expensive to construct.
                details::reinitialize(sequence, base);
                
                // This will return an empty optional only if the underlying sequence is empty.
                return sequence.next();
            }
        }
        
    private:
        S base;
        S sequence;
    };
    
    auto cycle()
    {
        return [=] (auto &&sequence)
        {
            return Cycle(std::move(sequence));
        };
    }
    
}
