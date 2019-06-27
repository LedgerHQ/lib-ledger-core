/*
 *
 * JsonParserPath.hpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 26/06/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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

#ifndef LEDGER_CORE_JSONPARSERPATH_HPP
#define LEDGER_CORE_JSONPARSERPATH_HPP

#include <string>
#include <list>
#include <variant>

namespace ledger {
    namespace core {

        enum class JsonParserPathNodeType {ARRAY, OBJECT};

        //using JsonParserPathNode = std::variant<int /* array index */, std::string /* */>;

        struct JsonParserPathNode {
            JsonParserPathNodeType type;
            std::string key;
            int index;

        };

        class JsonParserPath {
            JsonParserPath();
            /**
             * Matches the current path to a given path string representation.
             * e.g "/my_array/-/name" { "my_array" : [ {"name": ****} }
             * e.g "/my_array/1/name" { "my_array" : [ {***}, {"name": ****} }
             * e.g "/foo/bar" { "foo" : {bar: ***} }
             * e.g "/foo/\*" { "foo" : {bar: ***} }
             * @param path
             * @return
             */
            bool match(const std::string& path) const;

            /**
             * Marks the beginning of an array
             */
            void startArray();
            /**
             * Marks the beginning of an object.
             */
            void startObject();
            /**
             * Marks the end of an array.
             */
            void endArray();
            /**
             * Marks the end of an object.
             */
            void endObject();

            /**
             * Record the given key.
             * @param key
             */
            void key(const std::string& key);

            /**
             * Transform this object ot its string representation.
             * @return
             */
            std::string toString() const;

        private:
            std::list<JsonParserPathNode> _path;
            std::string _lastKey;
        };
    }
}


#endif //LEDGER_CORE_JSONPARSERPATH_HPP
