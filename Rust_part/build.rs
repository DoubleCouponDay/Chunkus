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

   println!("cargo:rustc-link-search=native={}", cwd.display());
   println!("cargo:rustc-link-search=native=../install/lib");

    if cfg!(target_os = "linux") {
        println!("cargo:rustc-link-lib=static=zlib:z");
        println!("cargo:rustc-link-lib=static=libpng:png16");
        println!("cargo:rustc-link-lib=static=vec:vec");
    }

    else {
        println!("cargo:rustc-link-lib=static=zlib:zlibstatic");
        println!("cargo:rustc-link-lib=static=libpng:png16");
        println!("cargo:rustc-link-lib=static=vec:vec");
    }

    println!("cargo:rerun-if-changed=build.rs");
}

fn stop_if_unknown_os() {
    if cfg!(target_os = "linux") == false
        && cfg!(target_os = "windows") == false {
        panic!("This operating system is not supported.");
    }
}


