use std::env;

pub fn gettoken() -> &'static str {
    env!("VECTORIZER")
}

pub fn getwatchertoken() -> &'static str {
    env!("TRAMPOLINE")
}
