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
#include <boost/variant.hpp>

namespace ledger {
    namespace core {

        enum class JsonParserPathNodeType {OBJECT, ARRAY, VALUE};

        struct JsonParserPathValue {};

        /**
         * Represents a single element in a json path. This class shouldn't be used outside of JsonParserPath
         */
        struct JsonParserPathNode {
            JsonParserPathNodeType type;
            boost::variant<int /* index in array */, std::string /* key in object */> content;

            const std::string& key() const {
                return boost::get<std::string>(content);
            }

            int index() const {
                return boost::get<int>(content);
            }

            JsonParserPathNode(JsonParserPathNodeType t) : type(t) {}
            JsonParserPathNode(JsonParserPathNodeType t, int index) : type(t), content(index) {}
            JsonParserPathNode(JsonParserPathNodeType t, std::string key) : type(t), content(key) {}
        };

        enum class JsonParserPathMatcherFilter {WILDCARD, EXACT, MATCH_ALL};

        /**
         * Represents a single element in a json path matcher.
         * This class shouldn't be used outside of JsonParserPathMatcher
         */
        struct JsonParserPathMatcherElement {
            JsonParserPathMatcherFilter filter;
            JsonParserPathNode node;

            JsonParserPathMatcherElement(
                    JsonParserPathMatcherFilter f,
                    const JsonParserPathNode& n) : filter(f), node(n) {};
        };

        /**
         * A structure used to match a JSON path.
         * The matcher is represented by a string:
         *    '/': represents the beginning of a json object
         *    '[': represents the beginning of a json array
         *    'this_key': will exactly match 'this_key' as the key of a json object
         *    '*': will match any key in a json object
         *    '[42]': will match exactly if the path is currently at the given index (here 42)
         *    '[*]': will match any index in a json array
         *    '?': match everything after this point. This must be put at the end of the matcher filter
         *
         */
        class JsonParserPathMatcher {
        public:
            JsonParserPathMatcher(const std::string& filter);
            const std::list<JsonParserPathMatcherElement>& getElements() const;
            std::string toString() const;
        private:
            std::list<JsonParserPathMatcherElement> _elements;
        };

        class JsonParserPath;

        /**
         * Limited view over a JSON path. The view is immutable and can create others views.
         */
        class JsonParserPathView {
        public:
            JsonParserPathView() {};
            /**
             * Matches the current path to a given path string representation.
             * e.g "/my_array[]/name" { "my_array" : [ {"name": ****} }
             * e.g "/my_array[1]/name" { "my_array" : [ {***}, {"name": ****} }
             * e.g "/foo/bar" { "foo" : {bar: ***} }
             * e.g "/foo/\*" { "foo" : {bar: ***} }
             * @param path
             * @return
            */
            bool match(const JsonParserPathMatcher &matcher) const;

            JsonParserPathView view(int depth);
            std::string toString() const;

        private:
            JsonParserPathView(JsonParserPath* owner, int depth) : _owner(owner), _depth(depth) {};

            JsonParserPath* _owner;
            int _depth;
            friend class JsonParserPath;
        };

        /**
         * Helper class to create JSON path. JSON path represents the current location of a SAX
         * parser in a JSON document. This path can be then used to match specific locations
         * and help SAX parsers to parse documents. JsonParserPath is a mutable class, it should
         * be used to build the path and create views which will be then used to match locations.
         */
        class JsonParserPath {
        public:
            JsonParserPath();


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
             * Marks the encounter of a value
             */
             void value();

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
            /**
            * Transform this object ot its string representation.
            * @return
            */
            std::string toString(int depth) const;

            inline const JsonParserPathNode& getCurrent() const;
            inline const JsonParserPathNode& getParent() const;

            /**
             * Creates a view at the root of the path.
             * @return A view at the root of the path.
             */
            JsonParserPathView view();
            /**
             * Creates a view for the given depth in the path.
             * @param depth
             * @return a view for the given depth in the path.
             */
            JsonParserPathView view(int depth);
            /**
             * Alias for JsonParserPathView::view.
             * @return  A view at the root of the path.
             */
            inline JsonParserPathView root() { return view(0); };

            /**
             * Matches the current path with the given matcher from the given depth.
             * @param matcher A matcher to test the path against.
             * @param depth The depth at which the path will be matched.
             * @return true if it matches, false otherwise.
             */
            bool match(const JsonParserPathMatcher& matcher, int depth) const;

        private:
            inline JsonParserPathNode& getCurrent();
            inline JsonParserPathNode& getParent();

            std::list<JsonParserPathNode> _path;
            std::string _lastKey;
        };


    }
}


#endif //LEDGER_CORE_JSONPARSERPATH_HPP
