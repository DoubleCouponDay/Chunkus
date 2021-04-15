use std::env::var;

pub fn gettoken() -> String {
    var("VECTORIZER").unwrap()
}

pub fn getwatchertoken() -> String {
    var("TRAMPOLINE").unwrap()
}
