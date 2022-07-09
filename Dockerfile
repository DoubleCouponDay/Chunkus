#designed for linux build
FROM rust:1.50

RUN apt-get update && \
    apt-get install apt-utils -y && \
    apt-get dist-upgrade -y && \
    apt-get install -y musl-tools && \    
    apt-get install software-properties-common -y && \
    apt-get install python3=3.7.3-1 -y && \
    apt-get install python3-pip -y && \
    python3 -m pip install --upgrade pip && \
    python3 -m pip install conan && \
    apt-get install cmake -y && \
    apt-get install build-essential && \
    rustup target add x86_64-unknown-linux-musl

# copy C source and Rust source
COPY ./ /home/sjsui/vectorizer/

# set environment variables
ENV CONAN /root/.conan/data/
ENV PATH $PATH:/home/sjsui/vectorizer/Rust_part/target/release

# initialize build tools
WORKDIR /home/sjsui/vectorizer/C_part/build/
RUN conan install ../ && \
    conan profile update settings.compiler.libcxx=libstdc++11 default && \
    cd ../ && \
    cmake -B build && \
    cmake --build build

#build Rust code
WORKDIR /home/sjsui/vectorizer/Rust_part/
RUN cargo build -vv --release

#create entrypoint of container
CMD ["./target/release/trampoline", "false"]