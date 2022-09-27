#![allow(unused_imports)]

use std::ops::Add;
use std::path::{
    Path,
    PathBuf
};

use std::fs::{
    File,
    ReadDir,
    DirEntry,
    OpenOptions,
    copy,
    rename
};
use std::env::{var, self};

fn main() {
    stop_if_unknown_os();
    let dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let cwd = Path::new(&dir);
    println!("cwd: {}", cwd.display());

    println!("cargo:rustc-link-search=native={}", cwd.display());
    println!("cargo:rustc-link-search=native={}/../install/lib", cwd.display());

    println!("cargo:rerun-if-changed=build.rs");
}

fn stop_if_unknown_os() {
    if cfg!(target_os = "linux") == false &&
        cfg!(target_os = "windows") == false {
        panic!("This operating system is not supported.");
    }
}


