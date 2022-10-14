#pragma once

#include <api/CoreTracer.hpp>
#include <api/Span.hpp>

namespace ledger {
  namespace core {
    namespace test {

      class ProxySpan: public api::Span {
        public:
          void setTagStr(const std::string & name, const std::string & value) override;
          void setTagInt(const std::string & name, int32_t value) override;
          void setTagDouble(const std::string & name, double value) override;
          void setTagBool(const std::string & name, bool value) override;
          void close() override;
      };

      class ProxyCoreTracer: public api::CoreTracer{
        public:
          std::shared_ptr<api::Span> startSpan(const std::string & name) override;
      };

    } // ledger
  } // core
} // test
