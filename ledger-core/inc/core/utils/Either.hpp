/*
 *
 * Either
 * ledger-core
 *
 * Created by Pierre Pollastri on 20/02/2017.
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

#pragma once

#include <functional>

#include <core/utils/Option.hpp>
#include <core/utils/Exception.hpp>

namespace ledger {
    namespace core {

        template<typename Left, typename Right>
        class Either {
        public:
            Either() {

            }

            Either(const Left &left) {
                _left = Option<Left>(left);
            };

            Either(const Right &right) {
                _right = Option<Right>(right);
            };

            Either(const Option<Left> &left, const Option<Right> &right) {
                _left = left;
                _right = right;
            };

            Either(Either<Left, Right>&& either) {
                _left = std::move(either._left);
                _right = std::move(either._right);
            }

            Either(const Either<Left, Right>& either) {
                _left = either._left;
                _right = either._right;
            }

            Either(Either& either) {
                _left = either._left;
                _right = either._right;
            }

            Either<Left, Right>&operator=(const Either<Left, Right>& either) {
                _left = either._left;
                _right = either._right;
                return *this;
            };

            Either<Left, Right>&operator=(const Left& left) {
                _left = Option<Left>(left);
                _right = Option<Right>();
                return *this;
            };

            Either<Left, Right>&operator=(const Right& right) {
                _left = Option<Left>();
                _right = Option<Right>(right);
                return *this;
            };

            bool isRight() const {
                return _right.nonEmpty();
            };

            bool isLeft() const {
                return _left.nonEmpty();
            };

            operator bool() {
                return isRight();
            };

            const Right &operator*() const {
                return _right.getValue();
            }

            const Right *operator->() const {
                return &_right.getValue();
            }

            Left &getLeft() {
                return _left.getValue();
            }

            Right &getRight() {
                return _right.getValue();
            }

            const Left &getLeft() const {
                return _left.getValue();
            };

            const Right &getRight() const {
                return _right.getValue();
            };

            operator Left() const {
                return _left.getValue();
            };

            operator Right() const {
                return _right.getValue();
            };

            Either<Right, Left> swap() const {
                return Either<Right, Left>(_right, _left);
            };

            template<typename T>
            T fold(std::function<T (const Left&)> f1, std::function<T (const Right&)> f2) const {
                if (isRight())
                    return f2(getRight());
                else
                    return f1(getLeft());
            };


        private:
            Option<Left> _left;
            Option<Right> _right;
        };
    }
}