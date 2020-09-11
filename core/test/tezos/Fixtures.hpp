#ifndef __TEZOS_FIXTURES_H_
#define __TEZOS_FIXTURES_H_

namespace ledger {
    namespace testing {
        namespace tezos {

            struct TestKey {
                std::string address;
                std::string secret;
                std::string hexkey;
                std::string pubkey;
            };

            const TestKey KEY_ED25519{
                "tz1eQLbY12XuaXEVv7LgsLbKAnN6jGrqm8sA",
                "edsk311eAbqyE5Lb3pPMrz971pi1QdZRhcDH4BUXmqWDpXqiK7bfJF",
                "32A6CAA0067135EAF35659344A578DEB64A1F4D28484C052C4F29627893B1EF1",
                "edpku2XmANX3SWPYvUh5mEdiXcRQs9YKK9DRaJ5aT8dr9juNsteXMp",
            };

            const TestKey KEY_SECP256K1{
                "tz2B7ibGZBtVFLvRYBfe4Q9uw7SRE62MKZCD",
                "spsk2H1ZAPGcRzgKS4h7i2RvdDMgKXhJxAjwHtUiKpeQ6Lg8EscfGo",
                "0227FC9F1B016476CDC93FBF2E529BB93429BAE451F4397E911F374EEF36A04784",
                "sppk7ZcFGJdaCVj9oDf5MBMVdFuL3Wm4Y39uv4URA5pcJ1mfEPePTYx",
            };

            const TestKey KEY_P256{
                "tz3bnhbn7uYfL43zfXtBvCYoq6DW743mRWvc",
                "p2sk2NEsV4VibjetudyeERg7iDy64sY2MxKFvC3xbB1oqj2Q4uQeaX",
                "0313E0FE2062532D390A726EB7F78BE03609796A930F6FABA4349FDF1C96365973",
                "p2pk66ffLoWNNC1useG68cKfRqmfoujyYha9KuCAWb3RM6oth5KnR1Q",
            };
        }
    }
}

#endif // __TEZOS_FIXTURES_H_
