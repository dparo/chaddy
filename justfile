build:
    @echo '[just] Compiling build.c'
    @mkdir -p build
    @cc build.c -o ./build/build.c -lcurl -lz -lssl -lcrypto

run *args: build
    @echo '[just] Executing build.c'
    @echo '[just] ./build/build.c {{args}}'
    @./build/build.c {{args}}

default: run
