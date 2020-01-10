#pragma once

#include <core/api/AccountCreationInfo.hpp>
#include <core/utils/Hex.hpp>

namespace ledger {
    namespace core {
        static const api::AccountCreationInfo XRP_KEYS_INFO(
            0, {"main"}, {"44'/144'/0'"},
            {hex::toByteArray("03c73f64083463fa923e1530af6f558204853873c6a45cbfb1f2f1e2ac2a5d989c")},
            {hex::toByteArray("f7e8d16154d3c7cbfa2cea35aa7a6ae0c429980892cf2d6ea9e031f57f22a63d")}
        );

        static const api::AccountCreationInfo VAULT_XRP_KEYS_INFO(
            0, {"main"}, {"44'/144'/0'"},
            {hex::toByteArray("03432A07E9AE9D557F160D9B1856F909E421B399E12673EEE0F4045F4F7BA151CF")},
            {hex::toByteArray("5D958E80B0373FA505B95C1DD175B0588205D1620C56F7247B028EBCB0FB5032")}
        );
    }
}
