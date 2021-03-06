# designed for linux build
FROM rust:1.50

# install sudo
RUN apt-get update && \
    apt-get install sudo -y

# set up non default user for xmake
RUN useradd -ms /bin/bash sjsui && \
    adduser sjsui root && \
    passwd -d -u sjsui && \
    echo "sjsui ALL=(ALL)ALL" >> /etc/sudoers

USER sjsui

RUN sudo apt-get update && \
    sudo apt-get install apt-utils -y && \
    sudo apt-get dist-upgrade -y && \
    sudo apt-get install -y musl-tools && \
    rustup target add x86_64-unknown-linux-musl && \
    sudo apt-get install software-properties-common -y && \
    sudo apt-get install python3 -y && \
    sudo apt-get install python-pip -y && \
    python -m pip install --upgrade pip && \
    python -m pip install conan

#install xmake
WORKDIR /usr/
RUN curl -fsSL https://xmake.io/shget.text | bash

# set environment variables
ENV conanpath /usr/.conan/data/
ENV releasebuild true
ENV PATH /sjsui/.local/bin/:/home/sjsui/.local/lib/python2.7/site-packages/conan/:$PATH

# copy C source and Rust source
COPY ./Rust_part/ /usr/vectorizer/Rust_part/
COPY ./C_part/ /usr/vectorizer/C_part/


# initialize conan
WORKDIR /usr/vectorizer/C_part/build/
RUN python3 -m conan install ..
RUN python3 -m conan profile update settings.compiler.libcxx=libstdc++11 default

#initialize xmake and build
RUN xmake f -m release -y && \    
    xmake -y

#build Rust code
WORKDIR /usr/vectorizer/Rust_part/
RUN cargo build --release

#create entrypoint of container
CMD ["./target/release/trampoline"]