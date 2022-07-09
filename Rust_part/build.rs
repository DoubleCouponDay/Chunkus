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
use std::env::var;

fn main() {
   stop_if_unknown_os();
   let link_path = "../install/lib/";
   println!("searching path for libraries: {}", link_path);
   println!("cargo:rustc-link-search={}", link_path);
   println!("linking static library: vec");
   println!("cargo:rustc-link-lib=static=vec");
   println!("linking static library: zlib");
   println!("cargo:rustc-link-lib=static=zlib");
   println!("linking static library: libpng");
   println!("cargo:rustc-link-lib=static=libpng");
}

fn stop_if_unknown_os() {
    if cfg!(target_os = "linux") == false
        && cfg!(target_os = "windows") == false {
        panic!("This project is not for you, macintosh!");
    }
}