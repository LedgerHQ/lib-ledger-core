#pragma once

#include <string>
#include <vector>

namespace ledger {
	namespace core {
		namespace db {
			struct DatabaseBlockHeader {
				uint32_t height;
				std::string hash;
			};

			struct DatabaseBlock {
				DatabaseBlockHeader header;
				// the specific format is not interesting to DB
				std::vector<uint8_t> data;
			};
		}
	}
}
