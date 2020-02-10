#pragma once

#include <core/api/AccountCreationInfo.hpp>
#include <core/utils/Hex.hpp>

namespace ledger {
    namespace core {
        static const api::AccountCreationInfo XTZ_KEYS_INFO(
            0, {"main"}, {"44'/1729'/0'/0'"},
            {hex::toByteArray("02af5696511e23b9e3dc5a527abc6929fae708defb5299f96cfa7dd9f936fe747d")},
            {hex::toByteArray("")}
        );
    }
}
