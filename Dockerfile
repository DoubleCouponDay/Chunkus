# designed for linux build
FROM rust:1.50

#install tools
RUN apt-get update && \
    apt-get dist-upgrade -y && \
    apt-get install -y musl-tools && \
    rustup target add x86_64-unknown-linux-musl

RUN apt-get install software-properties-common -y
RUN apt-get install python3 -y
RUN apt-get install python-pip -y
RUN pip install --upgrade pip
RUN pip install conan

#install xmake
WORKDIR /usr/
RUN add-apt-repository ppa:xmake-io/xmake
RUN apt update
RUN apt install xmake

# set environment variables
ENV conanpath /usr/.conan/data/
ENV releasebuild true

# copy C source and Rust source
COPY ./Rust_part/ /usr/vectorizer/Rust_part/
COPY ./C_part/ /usr/vectorizer/C_part/

# initialize conan
WORKDIR /usr/vectorizer/C_part/build/
RUN conan install ..

#initialize xmake and build
WORKDIR /usr/vectorizer/C_part/
RUN xmake f -m release && \
    xmake

#build Rust code
WORKDIR /usr/vectorizer/Rust_part/
RUN cargo build --release

#create entrypoint of container
CMD ["./target/release/trampoline"]