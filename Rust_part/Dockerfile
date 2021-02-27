# designed for linux build
FROM rust as deployment
WORKDIR /usr/vectorizer/

RUN apt-get update && \
    apt-get dist-upgrade -y && \
    apt-get install -y musl-tools && \
    rustup target add x86_64-unknown-linux-musl

RUN cargo install --target x86_64-unknown-linux-musl --path .

FROM alpine
COPY --from=deployment /src/target/release/ /usr/vectorizer/
EXPOSE 443/TCP
EXPOSE 80/TCP
RUN "./trampoline"