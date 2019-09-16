protoc -I ../core/proto --cpp_out ../core/src/commands/messages --js_out=import_style=commonjs,binary:../node-binding/messages ../core/proto/commands.proto
protoc -I ../core/proto --cpp_out ../core/src/commands/messages --js_out=import_style=commonjs,binary:../node-binding/messages ../core/proto/core_configuration.proto
protoc -I ../core/proto --cpp_out ../core/src/commands/messages --js_out=import_style=commonjs,binary:../node-binding/messages ../core/proto/services.proto

protoc -I ../core/proto --cpp_out ../core/src/commands/messages --js_out=import_style=commonjs,binary:../node-binding/messages ../core/proto/utils/commands.proto

protoc -I ../core/proto --cpp_out ../core/src/commands/messages --js_out=import_style=commonjs,binary:../node-binding/messages ../core/proto/bitcoin/account.proto
protoc -I ../core/proto --cpp_out ../core/src/commands/messages --js_out=import_style=commonjs,binary:../node-binding/messages ../core/proto/bitcoin/commands.proto
protoc -I ../core/proto --cpp_out ../core/src/commands/messages --js_out=import_style=commonjs,binary:../node-binding/messages ../core/proto/bitcoin/currency.proto
protoc -I ../core/proto --cpp_out ../core/src/commands/messages --js_out=import_style=commonjs,binary:../node-binding/messages ../core/proto/bitcoin/operation.proto

protoc -I ../core/proto --cpp_out ../core/src/commands/messages --js_out=import_style=commonjs,binary:../node-binding/messages ../core/proto/common/amount.proto
protoc -I ../core/proto --cpp_out ../core/src/commands/messages --js_out=import_style=commonjs,binary:../node-binding/messages ../core/proto/common/block.proto
protoc -I ../core/proto --cpp_out ../core/src/commands/messages --js_out=import_style=commonjs,binary:../node-binding/messages ../core/proto/common/unit.proto