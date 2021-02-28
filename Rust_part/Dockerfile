# designed for linux build
FROM rust as deployment
WORKDIR /usr/vectorizer/

RUN apt-get update && \
    apt-get dist-upgrade -y && \
    apt-get install -y musl-tools && \
    rustup target add x86_64-unknown-linux-musl

COPY ./targ/release ./
RUN cargo install --target x86_64-unknown-linux-musl --path .

FROM alpine
COPY --from=deployment /src/target/release/ /usr/vectorizer/
COPY --from=builder /etc/ssl/certs/ca-certificates.crt /etc/ssl/certs/ # think this was needed for discord rust bot for some reason

RUN "./trampoline"