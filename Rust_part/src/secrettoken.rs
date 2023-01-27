use std::env::var;

pub fn gettoken() -> String {
    var("CHUNKUS").unwrap()
}

pub fn getappid() -> String {
    var("CHUNKUS_ID").unwrap()
}
