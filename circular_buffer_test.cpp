#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "circular_buffer.h"
#include <string>
#include <algorithm>

TEST_CASE("Circular Buffer Basic Operations", "[basic]") {
    circular_buffer<int, 5> buf;

    SECTION("Empty buffer") {
        REQUIRE(buf.empty());
        REQUIRE(buf.size() == 0);
        REQUIRE(buf.capacity() == 5);
    }

    SECTION("Push and pop single element") {
        buf.push(42);
        REQUIRE(!buf.empty());
        REQUIRE(buf.size() == 1);
        REQUIRE(buf.front() == 42);
        REQUIRE(buf.back() == 42);

        int val = buf.pop();
        REQUIRE(val == 42);
        REQUIRE(buf.empty());
    }

    SECTION("Fill buffer completely") {
        for (int i = 0; i < 5; ++i) {
            buf.push(i * 10);
        }
        REQUIRE(buf.size() == 5);
        REQUIRE(!buf.empty());
        REQUIRE(buf.front() == 0);
        REQUIRE(buf.back() == 40);

        // Buffer full, next push should overwrite
        buf.push(999);
        REQUIRE(buf.size() == 5);
        REQUIRE(buf.front() == 10); // Oldest element removed
        REQUIRE(buf.back() == 999);
    }
}

TEST_CASE("Circular Buffer Overwrite Behavior", "[overwrite]") {
    circular_buffer<int, 3> buf;

    buf.push(1);
    buf.push(2);
    buf.push(3);

    REQUIRE(buf.size() == 3);
    REQUIRE(buf.front() == 1);
    REQUIRE(buf.back() == 3);

    // Overwrite oldest element
    buf.push(4);
    REQUIRE(buf.size() == 3);
    REQUIRE(buf.front() == 2);
    REQUIRE(buf.back() == 4);

    buf.push(5);
    REQUIRE(buf.size() == 3);
    REQUIRE(buf.front() == 3);
    REQUIRE(buf.back() == 5);

    buf.push(6);
    REQUIRE(buf.size() == 3);
    REQUIRE(buf.front() == 4);
    REQUIRE(buf.back() == 6);
}

TEST_CASE("Circular Buffer Pop Operations", "[pop]") {
    circular_buffer<int, 4> buf;

    buf.push(10);
    buf.push(20);
    buf.push(30);

    SECTION("Pop front") {
        REQUIRE(buf.pop() == 10);
        REQUIRE(buf.size() == 2);
        REQUIRE(buf.front() == 20);

        REQUIRE(buf.pop() == 20);
        REQUIRE(buf.size() == 1);
        REQUIRE(buf.front() == 30);

        REQUIRE(buf.pop() == 30);
        REQUIRE(buf.empty());
    }
}

TEST_CASE("Circular Buffer Iterator Basic", "[iterator]") {
    circular_buffer<int, 5> buf;

    SECTION("Empty buffer iteration") {
        REQUIRE(buf.begin() == buf.end());
        REQUIRE(std::distance(buf.begin(), buf.end()) == 0);
    }

    SECTION("Single element iteration") {
        buf.push(42);
        auto it = buf.begin();
        REQUIRE(it != buf.end());
        REQUIRE(*it == 42);
        ++it;
        REQUIRE(it == buf.end());
    }

    SECTION("Full buffer iteration") {
        for (int i = 0; i < 5; ++i) {
            buf.push(i * 10);
        }

        int expected = 0;
        for (auto it = buf.begin(); it != buf.end(); ++it) {
            REQUIRE(*it == expected);
            expected += 10;
        }
        REQUIRE(expected == 50);
    }
}

TEST_CASE("Circular Buffer Iterator with Overwrite", "[iterator-overwrite]") {
    circular_buffer<int, 3> buf;

    buf.push(1);
    buf.push(2);
    buf.push(3);
    buf.push(4); // Overwrites 1

    std::vector<int> result;
    for (auto val : buf) {
        result.push_back(val);
    }

    REQUIRE(result.size() == 3);
    REQUIRE(result[0] == 2);
    REQUIRE(result[1] == 3);
    REQUIRE(result[2] == 4);
}

TEST_CASE("Circular Buffer Iterator Const", "[iterator-const]") {
    circular_buffer<int, 4> buf;
    buf.push(100);
    buf.push(200);
    buf.push(300);

    const auto& const_buf = buf;

    int sum = 0;
    for (auto it = const_buf.begin(); it != const_buf.end(); ++it) {
        sum += *it;
    }

    REQUIRE(sum == 600);
}

TEST_CASE("Circular Buffer Random Access via Iterator", "[iterator-random]") {
    circular_buffer<int, 5> buf;
    for (int i = 0; i < 5; ++i) {
        buf.push(i * 100);
    }

    auto it = buf.begin();
    REQUIRE(*it == 0);

    ++it;
    REQUIRE(*it == 100);

    // Test iterator increment multiple times
    ++it;
    REQUIRE(*it == 200);
    ++it;
    REQUIRE(*it == 300);
}

TEST_CASE("Circular Buffer STL Algorithms", "[stl]") {
    circular_buffer<int, 6> buf;
    buf.push(5);
    buf.push(2);
    buf.push(8);
    buf.push(1);
    buf.push(9);

    SECTION("std::find") {
        auto it = std::find(buf.begin(), buf.end(), 8);
        REQUIRE(it != buf.end());
        REQUIRE(*it == 8);

        auto not_found = std::find(buf.begin(), buf.end(), 99);
        REQUIRE(not_found == buf.end());
    }

    SECTION("std::accumulate") {
        int sum = std::accumulate(buf.begin(), buf.end(), 0);
        REQUIRE(sum == 25); // 5+2+8+1+9
    }

    SECTION("std::transform") {
        circular_buffer<int, 6> result;
        std::vector<int> output;
        std::transform(buf.begin(), buf.end(),
                      std::back_inserter(output),
                      [](int x) { return x * 2; });

        REQUIRE(output.size() == 5);
        REQUIRE(output[0] == 10);
        REQUIRE(output[1] == 4);
        REQUIRE(output[2] == 16);
        REQUIRE(output[3] == 2);
        REQUIRE(output[4] == 18);
    }

    SECTION("std::count") {
        buf.push(5); // Now we have two 5's
        int count = std::count(buf.begin(), buf.end(), 5);
        REQUIRE(count == 2);
    }
}

TEST_CASE("Circular Buffer Edge Cases", "[edge]") {
    circular_buffer<int, 1> buf;

    SECTION("Size 1 buffer") {
        buf.push(42);
        REQUIRE(buf.size() == 1);
        REQUIRE(buf.front() == 42);
        REQUIRE(buf.back() == 42);

        buf.push(99); // Overwrite
        REQUIRE(buf.size() == 1);
        REQUIRE(buf.front() == 99);
        REQUIRE(buf.back() == 99);

        REQUIRE(buf.pop() == 99);
        REQUIRE(buf.empty());
    }

    SECTION("Multiple overwrites") {
        circular_buffer<int, 2> small_buf;
        for (int i = 0; i < 100; ++i) {
            small_buf.push(i);
        }
        REQUIRE(small_buf.size() == 2);
        REQUIRE(small_buf.front() == 98);
        REQUIRE(small_buf.back() == 99);
    }
}

TEST_CASE("Circular Buffer with Complex Types", "[complex]") {
    // Test with double which has all required operators (+, -, <, abs)
    circular_buffer<double, 3> dbl_buf;

    dbl_buf.push(1.5);
    dbl_buf.push(2.5);
    dbl_buf.push(3.5);

    REQUIRE(dbl_buf.size() == 3);
    REQUIRE(dbl_buf.front() == 1.5);
    REQUIRE(dbl_buf.back() == 3.5);

    dbl_buf.push(4.5); // Overwrites 1.5
    REQUIRE(dbl_buf.front() == 2.5);
    REQUIRE(dbl_buf.back() == 4.5);

    double val = dbl_buf.pop();
    REQUIRE(val == 2.5);
    REQUIRE(dbl_buf.front() == 3.5);
}

TEST_CASE("Circular Buffer Copy and Move", "[copy-move]") {
    circular_buffer<int, 4> original;
    original.push(1);
    original.push(2);
    original.push(3);

    SECTION("Copy constructor") {
        circular_buffer<int, 4> copied(original);
        REQUIRE(copied.size() == original.size());
        REQUIRE(copied.front() == original.front());
        REQUIRE(copied.back() == original.back());

        // Modify original, copy should remain unchanged
        original.push(4);
        REQUIRE(copied.size() == 3);
        REQUIRE(copied.back() == 3);
    }

    SECTION("Copy assignment") {
        circular_buffer<int, 4> assigned;
        assigned = original;
        REQUIRE(assigned.size() == original.size());

        original.push(4);
        REQUIRE(assigned.size() == 3);
    }

    SECTION("Move constructor") {
        circular_buffer<int, 4> moved(std::move(original));
        REQUIRE(moved.size() == 3);
        REQUIRE(moved.front() == 1);
        // Note: original may not be empty after move since we don't implement move semantics
        // The copy is made due to the static array storage
        REQUIRE(moved.back() == 3);
    }

    SECTION("Move assignment") {
        circular_buffer<int, 4> moved;
        moved = std::move(original);
        REQUIRE(moved.size() == 3);
        // Note: original may not be empty after move since we don't implement move semantics
        REQUIRE(moved.front() == 1);
    }
}

TEST_CASE("Circular Buffer Clear", "[clear]") {
    circular_buffer<int, 5> buf;
    buf.push(1);
    buf.push(2);
    buf.push(3);

    REQUIRE(buf.size() == 3);
    buf.reset();
    REQUIRE(buf.empty());
    REQUIRE(buf.size() == 0);
    REQUIRE(buf.begin() == buf.end());

    // Should be able to reuse after clear
    buf.push(10);
    REQUIRE(buf.size() == 1);
    REQUIRE(buf.front() == 10);
}

TEST_CASE("Circular Buffer Operator Equal", "[equality]") {
    circular_buffer<int, 4> buf1, buf2, buf3;

    buf1.push(1);
    buf1.push(2);

    buf2.push(1);
    buf2.push(2);

    buf3.push(1);
    buf3.push(3);

    // Test element-by-element comparison
    REQUIRE(buf1.size() == buf2.size());
    auto it1 = buf1.begin();
    auto it2 = buf2.begin();
    while (it1 != buf1.end() && it2 != buf2.end()) {
        REQUIRE(*it1 == *it2);
        ++it1;
        ++it2;
    }

    // buf1 and buf3 should differ
    bool equal = true;
    auto it3 = buf3.begin();
    for (auto it = buf1.begin(); it != buf1.end(); ++it, ++it3) {
        if (*it != *it3) {
            equal = false;
            break;
        }
    }
    REQUIRE(!equal);

    // Different sizes
    circular_buffer<int, 4> buf4;
    buf4.push(1);
    REQUIRE(buf1.size() != buf4.size());
}
