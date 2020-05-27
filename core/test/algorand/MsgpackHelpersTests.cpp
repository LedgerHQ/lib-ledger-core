#include <gtest/gtest.h>

#include "../../src/wallet/algorand/transactions/util/MsgpackHelpers.hpp"

#include <utils/Option.hpp>

#include <msgpack.hpp>

#include <sstream>
#include <string>

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

    namespace {

        class FooTest
        {
        public:
            FooTest(Option<double> f1,
                    int f2,
                    Option<std::string> f3)
                : f1(std::move(f1))
                , f2(f2)
                , f3(std::move(f3))
            {}

            Option<double> f1;
            int f2;
            Option<std::string> f3;
        };

        class FooTest1
        {
        public:
            FooTest1(int f2)
                : f2(f2)
            {}

            FooTest1() = default;

            int f2;
        };

        class FooTest2
        {
        public:
            FooTest2(double f1, int f2)
                : f1(f1), f2(f2)
            {}

            FooTest2() = default;

            double f1;
            int f2;
        };

        class FooTest3
        {
        public:
            FooTest3(int f2, std::string f3)
                : f2(f2), f3(std::move(f3))
            {}

            FooTest3() = default;

            int f2;
            std::string f3;
        };

        class FooTest4
        {
        public:
            FooTest4(double f1, int f2, std::string f3)
                : f1(f1), f2(f2), f3(std::move(f3))
            {}

            FooTest4() = default;

            double f1;
            int f2;
            std::string f3;
        };

        bool operator==(const FooTest& f1, const FooTest1& f2)
        {
            return f1.f1.isEmpty()
                && f1.f2 == f2.f2
                && f1.f3.isEmpty();
        }

        bool operator==(const FooTest& f1, const FooTest2& f2)
        {
            return f1.f1.nonEmpty()
                && f1.f1.getValue() == f2.f1 && f1.f2 == f2.f2
                && f1.f3.isEmpty();
        }

        bool operator==(const FooTest& f1, const FooTest3& f2)
        {
            return f1.f1.isEmpty()
                && f1.f2 == f2.f2
                && f1.f3.nonEmpty() && f1.f3 == f2.f3;
        }

        bool operator==(const FooTest& f1, const FooTest4& f2)
        {
            return f1.f1.nonEmpty() && f1.f1 == f2.f1
                && f1.f2 == f2.f2
                && f1.f3.nonEmpty() && f1.f3 == f2.f3;
        }

        template<typename... T>
        void testCountValidValues(int res, T&&... value)
        {
            const auto count = countValidValues(std::forward<T>(value)...);
            EXPECT_EQ(res, count);
        }

        template<typename T>
        void testPackKeyValues(FooTest f1, T f2)
        {
            std::stringstream ss;
            msgpack::pack(ss, f1);
            const auto str = ss.str();
            const auto oh = msgpack::unpack(str.data(), str.size());
            const auto deserialized = oh.get();
            deserialized.convert(f2);
            EXPECT_EQ(f1, f2);
        }

    } // namespace

    template<>
    struct pack<FooTest>
    {
        template<typename Stream>
        packer<Stream>& operator()(packer<Stream>& o,
                                   const FooTest& footest)
        {
            const auto count =
                countValidValues(
                        footest.f1,
                        footest.f2,
                        footest.f3);
            o.pack_map(count);

            return packKeyValues(o,
                    KeyValue<Option<double>>("f1", footest.f1),
                    KeyValue<int>("f2", footest.f2),
                    KeyValue<Option<std::string>>("f3", footest.f3));
        }
    };

    template<>
    struct convert<FooTest1>
    {
        const msgpack::object& operator()(const msgpack::object& o, FooTest1& f) const
        {
            if (o.type != msgpack::type::MAP) throw msgpack::type_error();
            if (o.via.map.size != 1) throw msgpack::type_error();

            EXPECT_EQ(o.via.map.ptr[0].key.as<std::string>(), std::string("f2"));
            f = FooTest1(o.via.map.ptr[0].val.as<int>());

            return o;
        }
    };

    template<>
    struct convert<FooTest2>
    {
        const msgpack::object& operator()(const msgpack::object& o, FooTest2& f) const
        {
            if (o.type != msgpack::type::MAP) throw msgpack::type_error();

            EXPECT_EQ(o.via.map.ptr[0].key.as<std::string>(), std::string("f1"));
            EXPECT_EQ(o.via.map.ptr[1].key.as<std::string>(), std::string("f2"));
            f = FooTest2(o.via.map.ptr[0].val.as<double>(),
                         o.via.map.ptr[1].val.as<int>());
            return o;
        }
    };

    template<>
    struct convert<FooTest3>
    {
        const msgpack::object& operator()(const msgpack::object& o, FooTest3& f) const
        {
            if (o.type != msgpack::type::MAP) throw msgpack::type_error();
            if (o.via.map.size != 2) throw msgpack::type_error();

            EXPECT_EQ(o.via.map.ptr[0].key.as<std::string>(), std::string("f2"));
            EXPECT_EQ(o.via.map.ptr[1].key.as<std::string>(), std::string("f3"));
            f = FooTest3(o.via.map.ptr[0].val.as<int>(),
                         o.via.map.ptr[1].val.as<std::string>());
            return o;
        }
    };

    template<>
    struct convert<FooTest4>
    {
        const msgpack::object& operator()(const msgpack::object& o, FooTest4& f) const
        {
            if (o.type != msgpack::type::MAP) throw msgpack::type_error();
            if (o.via.map.size != 3) throw msgpack::type_error();

            EXPECT_EQ(o.via.map.ptr[0].key.as<std::string>(), std::string("f1"));
            EXPECT_EQ(o.via.map.ptr[1].key.as<std::string>(), std::string("f2"));
            EXPECT_EQ(o.via.map.ptr[2].key.as<std::string>(), std::string("f3"));
            f = FooTest4(o.via.map.ptr[0].val.as<double>(),
                         o.via.map.ptr[1].val.as<int>(),
                         o.via.map.ptr[2].val.as<std::string>());
            return o;
        }
    };

    TEST(msgpackhelpers, countValidValues)
    {
        testCountValidValues(0, Option<int>());
        testCountValidValues(0, Option<int>(), Option<long>());
        testCountValidValues(1, 3);
        testCountValidValues(1, Option<int>(3));
        testCountValidValues(2, 1.2, Option<long>(3L));
        testCountValidValues(2, Option<std::string>(), Option<int>(), std::string("test"), 12L);
        testCountValidValues(3, Option<std::string>(std::string()), Option<int>(0), Option<long>(0L));
    }

    TEST(msgpackhelpers, packKeyValues)
    {
        const auto d = 1.5;
        const auto i = 2;
        const auto s = std::string("footest");

        FooTest footest1(Option<double>(), i, Option<std::string>());
        testPackKeyValues(footest1, FooTest1());

        FooTest footest2(d, i, Option<std::string>());
        testPackKeyValues(footest2, FooTest2());

        FooTest footest3(Option<double>(), i, s);
        testPackKeyValues(footest3, FooTest3());

        FooTest footest4(d, i, s);
        testPackKeyValues(footest4, FooTest4());
    }

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack

