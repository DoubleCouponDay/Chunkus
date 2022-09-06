use std::env::var;

pub fn gettoken() -> String {
    var("CHUNKUS").unwrap()
}

pub fn getwatchertoken() -> String {
    var("TRAMPOLINE").unwrap()
}
