/*
 *
 * TrustIndicator
 * ledger-core
 *
 * Created by Pierre Pollastri on 08/06/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */
#ifndef LEDGER_CORE_TRUSTINDICATOR_H
#define LEDGER_CORE_TRUSTINDICATOR_H

#include <string>
#include <rapidjson/document.h>
#include <api/TrustIndicator.hpp>
#include <api/TrustLevel.hpp>
#include <cereal/cereal.hpp>
#include <api/enum_from_string.hpp>

namespace ledger {
    namespace core {
        class TrustIndicator : public api::TrustIndicator {
        public:
            TrustIndicator();
            TrustIndicator(const TrustIndicator& ind) = delete;

            template <typename Archive>
            void serialize(Archive& ar, std::uint32_t const version) {
                std::string level = api::to_string(_level);
                ar(CEREAL_NVP(_weight), CEREAL_NVP(level), CEREAL_NVP(_conflicts), CEREAL_NVP(_origin));
                _level = api::from_string(level);
            }

            int32_t getTrustWeight() override;
            api::TrustLevel getTrustLevel() override;
            std::vector<std::string> getConflictingOperationUids() override;
            std::string getOrigin() override;

            TrustIndicator& setTrustWeight(int32_t weight);
            TrustIndicator& setTrustLevel(api::TrustLevel level);
            TrustIndicator& setOrigin(const std::string& origin);
            TrustIndicator& addConflictingOperationUid(const std::string& uid);
            TrustIndicator& removeConflictingOperationUid(const std::string& uid);

        private:
            int32_t _weight;
            api::TrustLevel _level;
            std::vector<std::string> _conflicts;
            std::string _origin;
        };
    }
}
CEREAL_CLASS_VERSION(ledger::core::TrustIndicator, 0);

#endif //LEDGER_CORE_TRUSTINDICATOR_H
