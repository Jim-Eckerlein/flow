#include "catch2/catch.hpp"

#include <list>
#include <vector>
#include <functional>
#include <map>
#include <array>

#include "flow/Iterator.h"
#include "flow/Elements.h"
#include "flow/Reference.h"
#include "flow/Flatten.h"
#include "flow/Filter.h"
#include "flow/Map.h"
#include "flow/Zip.h"
#include "flow/Chain.h"
#include "flow/Successors.h"
#include "flow/Take.h"
#include "flow/Stride.h"
#include "flow/Fuse.h"
#include "flow/Enumerate.h"
#include "flow/Fold.h"
#include "flow/Dereference.h"
#include "flow/Inspect.h"
#include "flow/Unfold.h"
#include "flow/Flow.h"

#include "TestsAuxiliary.h"

using namespace flow;

TEST_CASE("Zip") {
    auto as = {1, 2, 3};
    auto bs = {'a', 'b', 'c'};
    auto a = elements(as);
    auto b = elements(bs);
    auto c = a | zip(b) | enumerate();
    
    REQUIRE(c.next().value() == std::tuple(0, std::tuple(1, 'a')));
    REQUIRE(c.next().value() == std::tuple(1, std::tuple(2, 'b')));
    REQUIRE(c.next().value() == std::tuple(2, std::tuple(3, 'c')));
    REQUIRE(!c.next().has_value());
}

TEST_CASE("Deref")
{
    std::array<int, 4> as{3, 1, 4, 2};
    std::array<int const *, 4> bs{&as[1], &as[3], &as[0], &as[2]};

    auto b = elements(bs)
             | dereference();

    REQUIRE(b.next().value() == 1);
    REQUIRE(b.next().value() == 2);
    REQUIRE(b.next().value() == 3);
    REQUIRE(b.next().value() == 4);
    REQUIRE(!b.next().has_value());
}

TEST_CASE("Chain")
{
    std::array<int, 2> as{1, 2};
    std::array<int, 2> bs{3, 4};

    auto flow = elements(as)
                | chain(elements(bs));

    REQUIRE(flow.next().value() == 1);
    REQUIRE(flow.next().value() == 2);
    REQUIRE(flow.next().value() == 3);
    REQUIRE(flow.next().value() == 4);
    REQUIRE(!flow.next().has_value());
}

TEST_CASE("Fold")
{
    auto flow = Flow(Successors(1)) | take(4);
    auto sum = fold(flow, 0, [] (int a, int b) {
        return a + b;
    });
    REQUIRE(sum == 10);
}

TEST_CASE("Unfold")
{
    auto unfoldDescending = [] (int n) -> std::optional<std::pair<int, int>> {
        if (n == 0)
        {
            return {};
        }
        else
        {
            return std::pair(n, n - 1);
        }
    };

    auto flow = Unfold(4, unfoldDescending);

    REQUIRE(flow.next().value() == 4);
    REQUIRE(flow.next().value() == 3);
    REQUIRE(flow.next().value() == 2);
    REQUIRE(flow.next().value() == 1);
    REQUIRE(!flow.next().has_value());
}

TEST_CASE("Elements")
{
    std::vector<Identifier> xs;
    xs.emplace_back(3);

    auto seq = elements(xs);
    REQUIRE(seq.next().value().id == 3);
    REQUIRE(!seq.next().has_value());
}

TEST_CASE("Reference")
{
    std::vector<Identifier> xs;
    xs.emplace_back(3);

    auto seq = reference(xs);
    REQUIRE(seq.next().value()->id == 3);
    REQUIRE(!seq.next().has_value());
}

TEST_CASE("Filter")
{
    std::array<int, 4> as{1, 2, 3, 4};

    auto flow = elements(as)
                | filter([] (int n) { return n % 2 == 0; });

    REQUIRE(flow.next().value() == 2);
    REQUIRE(flow.next().value() == 4);
    REQUIRE(!flow.next().has_value());
}

TEST_CASE("Strings")
{
    std::vector<std::string> strings{"hello,", "ciao,", "foo"};
    auto flow = elements(strings)
                | map([] (std::string const &s) { return elements(s); })
                | flatten();

    REQUIRE(flow.next().value() == 'h');
    REQUIRE(flow.next().value() == 'e');
    REQUIRE(flow.next().value() == 'l');
    REQUIRE(flow.next().value() == 'l');
    REQUIRE(flow.next().value() == 'o');
    REQUIRE(flow.next().value() == ',');
    REQUIRE(flow.next().value() == 'c');
    REQUIRE(flow.next().value() == 'i');
    REQUIRE(flow.next().value() == 'a');
    REQUIRE(flow.next().value() == 'o');
    REQUIRE(flow.next().value() == ',');
    REQUIRE(flow.next().value() == 'f');
    REQUIRE(flow.next().value() == 'o');
    REQUIRE(flow.next().value() == 'o');
    REQUIRE(!flow.next().has_value());
}

TEST_CASE("Flatten")
{
    Container<Container<Identifier>> cc;
    cc.a().a().id = 1;
    cc.a().b().id = 2;
    cc.b().a().id = 3;
    cc.b().b().id = 4;

    int invocations = 0;

    // The container is moved here, so we do not need to expect
    // any copies, only moves.
    auto flow = elements(std::move(cc))
                | map([&] (Container<Identifier> const &c) { ++invocations; return elements(c); })
                | flatten();

    // While building the sequence, the functors must not be called
    // due to lazy evaluation.
    REQUIRE(invocations == 0);

    REQUIRE(flow.next().value().id == 1);
    REQUIRE(flow.next().value().id == 2);
    REQUIRE(flow.next().value().id == 3);
    REQUIRE(flow.next().value().id == 4);
    REQUIRE(!flow.next().has_value());

    // There are two inner maps.
    REQUIRE(invocations == 2);
}
