protoc -I ../core/proto --cpp_out ../core/src/commands/messages ../core/proto/commands.proto
protoc -I ../core/proto --cpp_out ../core/src/commands/messages ../core/proto/core_configuration.proto
protoc -I ../core/proto --cpp_out ../core/src/commands/messages ../core/proto/services.proto

protoc -I ../core/proto --cpp_out ../core/src/commands/messages ../core/proto/bitcoin/account.proto
protoc -I ../core/proto --cpp_out ../core/src/commands/messages ../core/proto/bitcoin/account_config.proto
protoc -I ../core/proto --cpp_out ../core/src/commands/messages ../core/proto/bitcoin/commands.proto
protoc -I ../core/proto --cpp_out ../core/src/commands/messages ../core/proto/bitcoin/currency.proto
protoc -I ../core/proto --cpp_out ../core/src/commands/messages ../core/proto/bitcoin/operation.proto
protoc -I ../core/proto --cpp_out ../core/src/commands/messages ../core/proto/bitcoin/wallet.proto
protoc -I ../core/proto --cpp_out ../core/src/commands/messages ../core/proto/bitcoin/wallet_config.proto

protoc -I ../core/proto --cpp_out ../core/src/commands/messages ../core/proto/common/amount.proto
protoc -I ../core/proto --cpp_out ../core/src/commands/messages ../core/proto/common/currency.proto
protoc -I ../core/proto --cpp_out ../core/src/commands/messages ../core/proto/common/unit.proto