use std::env::var;

pub fn gettoken() -> String {
    var("CHUNKUS").unwrap()
}
