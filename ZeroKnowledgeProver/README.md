# Access control zk proof system

This part of the project is responsible for the zero knowledge proofs. To realise that, it uses the risc0 framework. This code needs to be compiled seperately. Therefore Rust and risc0 needs to be installed. Once the code is compiled in the target folder the cmake copies the binaries to the build folder of the c++ part. The following lines provide instructions on how to accomplish this. The build process needs to be executed once for the AccessControl and for the DataServerPublicKey (so four times).

## Requirements

Install Rust with rustup:

```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

Install risc0 with cargo:

```bash
cargo install cargo-binstall
cargo binstall cargo-risczero --version 0.19.1
cargo risczero install
```

Verfiy that risc0 was installed correctly:

```bash
rustup toolchain list --verbose | grep risc0
```

## Build

Build the executable for the prover and the verifier for the AccessControl as for the DataServer

```bash
cargo build --release --bin prover
cargo build --release --bin verifier
```
