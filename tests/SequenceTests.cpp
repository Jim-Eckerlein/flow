#define CATCH_CONFIG_MAIN

#include "catch2/catch.hpp"

#include <cstring>
#include <vector>
#include <string>
#include <sstream>

#include "sequences/sequence.h"
#include "sequences/mutation.h"

using namespace sequences;

TEST_CASE("Counting")
{
    std::vector<int> v{1, 2, 3, 4, 3, 2, 1};
    auto sequence = make_sequence(v);
    REQUIRE(sequence.count() == v.size());
}

TEST_CASE("Sub-sequences")
{
    std::vector<int> v{1, 2, 3, 4, 5, 6};
    auto sequence = make_sequence(v.begin(), v.end());

    REQUIRE(sequence.count() == 6);
    REQUIRE(sequence.skipped(2).count() == 4);
    REQUIRE(sequence.range(3).count() == 3);
    REQUIRE(sequence.skipped(1).range(3).count() == 3);
    REQUIRE(sequence.range(3).skipped(1).count() == 2);
}

TEST_CASE("Empty sequences")
{
    std::vector<int> v{1, 2, 3, 4, 5, 6};
    auto sequence = make_sequence(v).range(0);

    REQUIRE(sequence.count() == 0);
    REQUIRE(sequence.empty());
}

TEST_CASE("Containment")
{
    std::vector<int> v{1, 2, 3, 4, 5, 6};
    auto sequence = make_sequence(v);

    REQUIRE(sequence.contains(4));
    REQUIRE(!sequence.contains(0));
}

TEST_CASE("Last")
{
    std::vector<int> v{1, 2, 3, 4, 5, 6};
    auto sequence = make_sequence(v);

    REQUIRE(sequence.last_element() == 6);
}

TEST_CASE("Iteration")
{
    std::vector<int> v{1, 2, 3, 4, 5};
    auto sequence = make_sequence(v);

    REQUIRE(sequence.next() == 1);
    REQUIRE(sequence.next() == 2);
    REQUIRE(sequence.next() == 3);
    REQUIRE(sequence.next() == 4);
    REQUIRE(sequence.next() == 5);
    REQUIRE(sequence.empty());
}

TEST_CASE("Mapping")
{
    std::vector<int> v{1, 2, 3, 4, 5};
    auto sequence = make_sequence(v).map<int>([](int n) { return n * n; });

    REQUIRE(sequence.next() == 1);
    REQUIRE(sequence.next() == 4);
    REQUIRE(sequence.next() == 9);
    REQUIRE(sequence.next() == 16);
    REQUIRE(sequence.next() == 25);
    REQUIRE(sequence.empty());
}

TEST_CASE("Filtering")
{
    std::vector<int> v{1, 2, 3, 4, 5, 6};
    auto sequence = make_sequence(v).filter([](int n) {
        return n % 2 == 0;
    });

    REQUIRE(sequence.next() == 2);
    REQUIRE(sequence.next() == 4);
    REQUIRE(sequence.next() == 6);
    REQUIRE(sequence.empty());
}

TEST_CASE("Zipping")
{
    std::vector<int> v{1, 2, 3, 4, 5, 6};
    std::vector<char> w{'a', 'b', 'c', 'd'};

    auto sequence = make_sequence(v).zip(make_sequence(w));

    REQUIRE(make_pair(1, 'a') == sequence.next());
    REQUIRE(make_pair(2, 'b') == sequence.next());
    REQUIRE(make_pair(3, 'c') == sequence.next());
    REQUIRE(make_pair(4, 'd') == sequence.next());
    REQUIRE(sequence.empty());
}

TEST_CASE("Inspecting")
{
    std::vector<char> v{'a', 'b', 'c'};
    char buf[4] = {};
    int i = 0;
    make_sequence(v).inspect([&buf, &i](char &c) {
        buf[i++] = c;
        ++c;
    }).close();
    REQUIRE(v[0] == 'b');
    REQUIRE(v[1] == 'c');
    REQUIRE(v[2] == 'd');
    REQUIRE(i == 3);
    REQUIRE(strcmp("abc", buf) == 0);
}

TEST_CASE("For each")
{
    std::vector<char> v{'a', 'b', 'c'};
    char buf[4] = {};
    int i = 0;
    make_sequence(v).forEach([&buf, &i](char c) {
        buf[i++] = c;
    });
    REQUIRE(i == 3);
    REQUIRE(strcmp("abc", buf) == 0);
}

TEST_CASE("Mutation sequence")
{
    int counter = 0;

    auto sequence = make_mutation(2, [&counter](int n) {
        ++counter;
        return n * n;
    });

    REQUIRE(sequence.next() == 2);
    REQUIRE(sequence.next() == 4);
    REQUIRE(*sequence == 16);
    REQUIRE(*sequence.skipped(2) == 1u << 16u);
    REQUIRE(counter == 4);
}

TEST_CASE("Finite mutation")
{
    auto sequence = make_mutation(1, [](int n) {
        return 2 * n;
    }).range(4);
    REQUIRE(sequence.next() == 1);
    REQUIRE(sequence.next() == 2);
    REQUIRE(sequence.next() == 4);
    REQUIRE(sequence.next() == 8);
    REQUIRE(sequence.empty());
}

TEST_CASE("Skipping over end")
{
    auto sequence = make_mutation(1, [](int n) {
        return 2 * n;
    }).range(1).skip(10);
    REQUIRE(sequence.empty());
}

TEST_CASE("Combine mapping and filtering")
{
    std::vector<int> v{0, 1, 2, 3, 4, 5, 6, 7, 8,
                       9, 10, 11, 12, 13, 14, 15, 16};

    auto sequence = make_sequence(v)
            .map<int>([](int n) {
                return n * n;
            })
            .filter([](int n) {
                return n % 2 == 0;
            })
            .map<std::string>([](int n) {
                std::stringstream ss;
                ss << "Number: " << n;
                return ss.str();
            });

    // Take first entry and move sequence the next element.
    REQUIRE(sequence.next() == "Number: 0");
    REQUIRE(sequence.next() == "Number: 4");
    REQUIRE(sequence.next() == "Number: 16");
    REQUIRE(sequence.next() == "Number: 36");
    REQUIRE(sequence.next() == "Number: 64");
    REQUIRE(sequence.next() == "Number: 100");

    // Skip 12^2 and 14^2 and move to next element.
    REQUIRE(sequence.skip(2).next() == "Number: 256");
    REQUIRE(sequence.empty());
}

TEST_CASE("Pair mapping")
{
    auto evenNumberSequence = make_mutation_linear(0, 2);
    auto oddNumberSequence = make_mutation_linear(1, 2);

    auto numbers = evenNumberSequence
            .zip(oddNumberSequence)
            .map2<int>([](int even, int odd) {
                return odd - even;
            })
            .range(20);

    for (int n : numbers)
    {
        REQUIRE(1 == n);
    }
}

TEST_CASE("Sequence of write-through pointers")
{
    int a = 1;
    int b = 2;

    std::vector<int *> pointers{&a, &b};

    auto sequence = make_sequence(pointers).inspect([](int *n) {
        *n = 10;
    });

    REQUIRE(a == 1);
    REQUIRE(b == 2);
    REQUIRE(sequence.next() == &a);
    REQUIRE(sequence.next() == &b);
    REQUIRE(sequence.empty());
    REQUIRE(a == 10);
    REQUIRE(b == 10);
}

TEST_CASE("Copy to other sequence")
{
    std::vector<int> v;
    auto sequence = make_mutation_linear().range(3);
    sequence.emplace_to(v);
    sequence.emplace_to(v);

    REQUIRE(v[0] == 0);
    REQUIRE(v[1] == 1);
    REQUIRE(v[2] == 2);
    REQUIRE(v[3] == 0);
    REQUIRE(v[4] == 1);
    REQUIRE(v[5] == 2);
}

TEST_CASE("Index access")
{
    auto sequence = make_mutation_linear(1, 2);

    int a = sequence[0];
    int b = sequence[1];
    int c = sequence[2];

    REQUIRE(a == 1);
    REQUIRE(b == 3);
    REQUIRE(c == 5);
}

TEST_CASE("Chaining")
{
    auto s1 = make_mutation_linear().range(3);
    auto s2 = make_mutation_linear().range(3).map<int>([](int n) {
        return n + 3;
    });

    auto s3 = s1.chain(s2);

    REQUIRE(0 == s3.next());
    REQUIRE(1 == s3.next());
    REQUIRE(2 == s3.next());
    REQUIRE(3 == s3.next());
    REQUIRE(4 == s3.next());
    REQUIRE(5 == s3.next());
    REQUIRE(s3.empty());
}

TEST_CASE("Mutating underlying elements")
{
    std::vector<int> v{1, 2, 3};
    auto s = make_sequence(v)
            .inspect([](int &n) { n += 10; });

    REQUIRE(v[0] == 1);
    REQUIRE(v[1] == 2);
    REQUIRE(v[2] == 3);
    s.close();
    REQUIRE(v[0] == 11);
    REQUIRE(v[1] == 12);
    REQUIRE(v[2] == 13);
}
