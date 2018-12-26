test -f Makefile && make clean

rm -f kvraft*.h kvraft*.cc kvraft*.cpp Makefile *.conf

../codegen/phxrpc_pb2server -I ../ -I ../third_party/protobuf/include -f kvraft.proto -d . -u


