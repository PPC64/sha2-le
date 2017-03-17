# This project is about SHA256 and SHA512 implementations and optimizations

The focus of this project is creating an optimized code for IBM POWER
architecture, especifically using ABI v2 (Little Endian), aka ppc64le.

This is a generic project and self-contained now. We are going to start
backporting it to different Languages and JIT projects. Our initial
target at this moment is the following list:

 - OpenJDK
 - nettle
 - Cryptopp
 - ...

