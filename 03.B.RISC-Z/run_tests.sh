cat tests/factorial10.S.hex | python3 hex2bin.py tests/factorial10.o
cat tests/factorial8.S.hex | python3 hex2bin.py tests/factorial8.o
./_setup_build_configs.sh cmake
(cd build/test && make)

./build/test/risc-z
