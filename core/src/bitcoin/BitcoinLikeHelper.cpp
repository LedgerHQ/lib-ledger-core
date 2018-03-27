/*
 *
 * BitcoinLikeHelper.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 18/10/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
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

#include <api/BitcoinLikeHelper.hpp>
#include <api/BitcoinLikeOutput.hpp>
#include <utils/Option.hpp>
#include <memory>
#include <utils/Exception.hpp>

namespace ledger {
    namespace core {

        class BitcoinLikeOutputBuilder : public api::BitcoinLikeOutput {
        public:
            BitcoinLikeOutputBuilder(const std::vector<uint8_t>& script, const std::shared_ptr<api::Amount>& amount) : _script(script), _amount(amount) {

            }

            std::string getTransactionHash() override {
                return "";
            }

            int32_t getOutputIndex() override {
                return -1;
            }

            std::shared_ptr<api::Amount> getValue() override {
                throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "std::shared_ptr<api::BitcoinLikeScript> parseScript()");
            }

            std::vector<uint8_t> getScript() override {
                throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "std::shared_ptr<api::BitcoinLikeScript> parseScript()");
            }

            optional<std::string> getAddress() override {
                // Parse the address
                Option<std::string> addr;
                return addr.toOptional();
            }

            std::shared_ptr<api::BitcoinLikeScript> parseScript() override {
                throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "std::shared_ptr<api::BitcoinLikeScript> parseScript()");
            }

        private:
            std::vector<uint8_t> _script;
            std::shared_ptr<api::Amount> _amount;
        };

        namespace api {

            std::shared_ptr<BitcoinLikeOutput> BitcoinLikeHelper::addressToOutput(const std::string &address,
                                                                                  const std::shared_ptr<Amount> &amount) {
                auto ptr = std::shared_ptr<BitcoinLikeOutputBuilder>(new BitcoinLikeOutputBuilder({}, amount));
                return ptr;
            }

            std::vector<uint8_t> BitcoinLikeHelper::serializeTransaction(
                    const BitcoinLikePreparedTransaction &preparedTransaction) {
                return {};
            }

            std::shared_ptr<BitcoinLikeTransaction> BitcoinLikeHelper::parseTransaction(const std::vector<uint8_t>& transaction) {
                return nullptr;
            }

            std::shared_ptr<BitcoinLikeOutput> BitcoinLikeHelper::scriptToOutput(const std::vector<uint8_t> & script, const std::shared_ptr<Amount> & amount) {

            }

        }
    }
}