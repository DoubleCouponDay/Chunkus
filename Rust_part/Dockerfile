# designed for linux build
FROM rust
WORKDIR /usr/vectorizer/

COPY ./Cargo.toml /usr/vectorizer/Cargo.toml
COPY ./target/release/trampoline /usr/vectorizer/trampoline
COPY ./target/release/bot /usr/vectorizer/bot
COPY ./target/release/template.svg /usr/vectorizer/template.svg

RUN apt-get update && \
    apt-get dist-upgrade -y && \
    apt-get install -y musl-tools && \
    rustup target add x86_64-unknown-linux-musl

CMD ["./trampoline"]