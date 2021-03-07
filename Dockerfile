#designed for linux build
FROM rust:1.50

#install sudo
RUN apt-get update && \
    apt-get install sudo -y

#set up non default user for xmake
RUN useradd -ms /bin/bash sjsui && \
    adduser sjsui sudo && \
    passwd -d -u sjsui && \
    echo "sjsui ALL=(ALL:ALL) ALL" >> /etc/sudoers

#add xmake to sudo list
ENV PATH /home/sjsui/.local/bin/:$PATH

USER sjsui

RUN sudo apt-get update && \
    sudo apt-get install apt-utils -y && \
    sudo apt-get dist-upgrade -y && \
    sudo apt-get install -y musl-tools && \    
    sudo apt-get install software-properties-common -y && \
    sudo apt-get install python3=3.7.3-1 -y && \
    sudo apt-get install python-pip -y && \
    sudo python -m pip install --upgrade pip && \
    sudo python -m pip install conan && \
    rustup target add x86_64-unknown-linux-musl

#install xmake
RUN curl -fsSL https://xmake.io/shget.text | bash

# copy C source and Rust source
COPY ./ /home/sjsui/vectorizer/

# set environment variables
ENV conanpath /usr/.conan/data/
ENV releasebuild true
ENV PATH /home/sjsui/.local/bin/:$PATH

# initialize conan
WORKDIR /home/sjsui/vectorizer/C_part/build/
RUN sudo conan install .. && \
    sudo conan profile update settings.compiler.libcxx=libstdc++11 default

# change the file mode bits of the xmake stuff
WORKDIR /home/sjsui/vectorizer/
RUN sudo chmod -R -f 777 ./


# #initialize xmake and build
WORKDIR /home/sjsui/vectorizer/C_part/
RUN xmake f -m release -y && \    
    xmake -y

#build Rust code
WORKDIR /home/sjsui/vectorizer/Rust_part/
RUN cargo build --release

#create entrypoint of container
CMD ["./target/release/trampoline"]