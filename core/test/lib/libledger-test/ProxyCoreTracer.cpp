#include "ProxyCoreTracer.h"

namespace ledger {
  namespace core {
    namespace test {

      std::shared_ptr<api::Span> ProxyCoreTracer::startSpan(const std::string & /*name*/){
        return std::make_shared<ProxySpan>();
      }

      void ProxySpan::setTagStr(const std::string & /*name*/, const std::string & /*value*/){
      // Intentionnaly empty, not used in tests
      }

      void ProxySpan::setTagInt(const std::string & /*name*/, int32_t /*value*/){
      // Intentionnaly empty, not used in tests
      }

      void ProxySpan::setTagDouble(const std::string & /*name*/, double /*value*/){
        // Intentionnaly empty, not used in tests
      }

      void ProxySpan::setTagBool(const std::string & /*name*/, bool /*value*/){
        // Intentionnaly empty, not used in tests
      }
    } // ledger
  } // core
} // test