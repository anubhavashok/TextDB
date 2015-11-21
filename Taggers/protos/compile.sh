#!/bin/sh
# compile protos

protoc -I=. --cpp_out=../../TextDB/ --java_out=../java/src --python_out=../python ./tagger_message.proto
