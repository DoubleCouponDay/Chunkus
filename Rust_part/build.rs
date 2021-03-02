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
    copy
};
use std::env::var;

const WINDOWSCORE: &'static str = "vec";
const WIN_COMMON_PATH: &'static str = "../C_part/build/windows/x64/";

const LINUXCORE: &'static str = "libvec";
const LIN_COMMON_PATH: &'static str = "../C_part/build/linux/x86_64/";

const DEBUGNAME: &'static str = "debug/";
const RELEASENAME: &'static str = "release/";

const RELEASEBUILDKEY: &'static str = "releasebuild";
const CONANPATHKEY: &'static str = "conanpath";

const LIB_EXT: &'static str = ".lib";
const A_EXT: &'static str = ".a";

const BAD_ZLIB: &'static str = "libz";
const GOOD_ZLIB: &'static str = "zlib";
const LIB_PNG: &'static str = "libpng16";

const NUM_LIBS: usize = 3;

static LIB_NAMES: [&'static str; NUM_LIBS] = [
    BAD_ZLIB, GOOD_ZLIB, LIB_PNG
];

fn main() {
    println!("cargo:rustc-link-search=./");

    //copy the template svg    
    let _ = std::fs::copy("../template.svg", std::env::var("OUT_DIR").unwrap());

    let previous_lib_path: String;
    let new_lib_path: String;

    #[cfg(target_os = "linux")] {
        println!("Linux detected");
        new_lib_path = String::from(LINUXCORE).add(A_EXT);            
    }

    #[cfg(target_os = "windows")] {
        println!("Windows detected");
        new_lib_path = String::from(WINDOWSCORE).add(LIB_EXT);
    }
    stop_if_unknown_os();

    if is_release_build() {
        println!("release C build detected");

        #[cfg(target_os = "linux")] {
            previous_lib_path = String::from(LIN_COMMON_PATH)
                .add(RELEASENAME)
                .add(LINUXCORE)
                .add(A_EXT);
            
        }

        #[cfg(target_os = "windows")] {
            previous_lib_path = String::from(WIN_COMMON_PATH)
                .add(RELEASENAME)
                .add(WINDOWSCORE)
                .add(LIB_EXT);
        }
    }

    else {
        println!("debug C build detected");

        #[cfg(target_os = "linux")] {
            previous_lib_path = String::from(LIN_COMMON_PATH)
                .add(DEBUGNAME)
                .add(LINUXCORE)
                .add(A_EXT);
            
        }

        #[cfg(target_os = "windows")] {
            previous_lib_path = String::from(WIN_COMMON_PATH)
                .add(DEBUGNAME)
                .add(WINDOWSCORE)
                .add(LIB_EXT);
        }
    }   
    copy_core_totarget(&previous_lib_path, &new_lib_path);
    verify_conan_environment();
    rerun_after_copying(&new_lib_path);
}

fn is_release_build() -> bool {
    let release_env_var = std::env::var(RELEASEBUILDKEY);

    let is_release_build: bool = match release_env_var {
        Err(_) => {
            let message = RELEASEBUILDKEY.to_string().add("environment variable not found.");
            println!("{}", message);
            panic!(message);
        },
        Ok(v) => {
            let parsed = v.parse::<bool>();
            
            let success = match parsed {
                Err(_) => {
                    let message = "'releasebuild' is not a boolean";
                    println!("{}", message);
                    panic!(message);
                },
                Ok(v) => v
            };
            success
        }
    };
    return is_release_build;
}

fn stop_if_unknown_os() {
    if cfg!(target_os = "linux") == false
        && cfg!(target_os = "windows") == false {
        panic!("This project is not for you, macintosh!");
    }
}

fn copy_core_totarget(previous_lib_path: &String, new_lib_name: &String) {
    let frompath = Path::new(previous_lib_path);
    let topath = Path::new(new_lib_name);
    println!("copying file: {}, to: {}", previous_lib_path, new_lib_name);
    copy(previous_lib_path, new_lib_name).unwrap();
    println!("cargo:rustc-link-lib=static={}", new_lib_name);
}

fn verify_conan_environment() {
    if let Ok(conanpathstr) = var(CONANPATHKEY) { //detect the conan libraries
        let conanpath = Path::new(&conanpathstr);
        link_c_libs(conanpath);
    }

    else {
        let message = String::from("could not find the ")
            .add(CONANPATHKEY)
            .add("environment variable.");

        panic!(message);
    }
}

fn rerun_after_copying(new_lib_name: &String) {
    // Rerun if any library file was deleted
    #[cfg(target_os = "windows")] {
        println!("cargo:rerun-if-changed={}", new_lib_name);
        println!("cargo:rerun-if-changed={}", new_lib_name);
    }

    #[cfg(target_os = "linux")]
    println!("cargo:rerun-if-changed={}", new_lib_name);    
}

fn link_c_libs(directory: &std::path::Path) {
    let mut areaddir: ReadDir = directory.read_dir().unwrap();
    check_dir_items(&mut areaddir, &directory)
}

fn check_dir_items(read_directory: &mut ReadDir, directory: &Path) {
    for dir_item in read_directory {
        let real_item = dir_item.unwrap();
        find_lib(&real_item, &directory);
    }
}

fn find_lib(entry: &DirEntry, directory: &Path) {
    let real_file_type = entry.file_type().unwrap();

    if real_file_type.is_dir()
    {
        println!("{}", &entry.path().display());
        link_c_libs(&entry.path());
    }

    else if real_file_type.is_file()
    {
        println!("{}", &entry.path().display());
        if let Some(real_file_name) = entry.file_name().to_str() {            
            let dir_name = directory.to_str().unwrap();                
            check_filename_for_needed(dir_name, entry, real_file_name);
        }
        
        else {
            panic!("problem getting filename");
        }
    }

    else {
        panic!("it's neither directory nor file");
    }
}

fn check_filename_for_needed(dir_name: &str, entry: &DirEntry, real_file_name: &str) {
    for i in 0 .. NUM_LIBS {
        let current = LIB_NAMES[i];
        
        if real_file_name.starts_with(current) == false {
            println!("{} not match for: {}", real_file_name, current);
            continue;
        }
        println!("{} found match: {}", real_file_name, current);
        let filepath = &entry.path();

        #[cfg(target_os = "windows")]
        let fixedbug = String::from(real_file_name);

        #[cfg(target_os = "linux")]
        let fixedbug: String = rename_libz_to_zlib(filepath, real_file_name);

        link_file(dir_name, filepath, fixedbug.as_str());
    }
}

fn rename_libz_to_zlib(filepath: &PathBuf, real_file_name: &str) -> String { //conan on Linux downloads a zlib library with a different filename than windows
    if real_file_name.starts_with(BAD_ZLIB) == false {
        return String::from(real_file_name);
    }
    println!("found the libz bug. creating a zlib.a copy...");
    let mut newpathbuf = filepath.clone().to_path_buf();
    let newfilename = String::from(GOOD_ZLIB).add(A_EXT);
    newpathbuf.pop();
    newpathbuf.push(&newfilename);
    println!("copying file: {}, to: {}", real_file_name, newfilename);
    copy_file(filepath, &newpathbuf);
    return newfilename;
}

fn link_file(dir_name: &str, filepath: &Path, real_file_name: &str) {
    if real_file_name.ends_with(LIB_EXT) == false 
        && real_file_name.ends_with(A_EXT) == false {
            println!("file extension not valid");
        return;
    }

    let extension: &'static str = {
        if real_file_name.ends_with(LIB_EXT) {
            LIB_EXT
        }

        else if real_file_name.ends_with(A_EXT) {
            A_EXT
        }

        else {
            panic!("refactoring probably went pear-shaped");
        }
    };

    println!("cargo:rustc-link-search={}", dir_name);
    let processedname = prepend_lib_filename(filepath, real_file_name);

    let stripped = processedname.strip_suffix(extension)
        .expect("Build script failed to remove '.lib' from a filename");

    println!("cargo:rustc-link-lib=static={}", stripped);
}

fn prepend_lib_filename(filepath: &Path, real_file_name: &str) -> String {
    if cfg!(target_os = "windows") 
        && real_file_name.contains(LIB_PNG) {
        return remove_lib_filename(filepath, real_file_name);
    }

    else if real_file_name.starts_with("lib") {
        println!("file already begins with lib: {}", real_file_name);
        return String::from(real_file_name);
    }
    let mut newpathbuf = filepath.clone().to_path_buf();
    let newfilename = String::from(LIB_EXT).add(real_file_name);
    newpathbuf.pop();
    newpathbuf.push(&newfilename);
    println!("renaming file: {}, to: {}", real_file_name, newfilename);
    copy_file(filepath, &newpathbuf);
    return newfilename;
}

fn remove_lib_filename(filepath: &Path, realfilename: &str) -> String {
    let mut newpathbuf = filepath.clone().to_path_buf();
    let mut newfilename = String::from(realfilename);
    newfilename.replace_range(0..=2, "");
    println!("removing lib from filename. result: {}", &newfilename);
    newpathbuf.pop();
    newpathbuf.push(&newfilename);
    copy_file(filepath, &newpathbuf);
    return newfilename;
}

fn copy_file(from: &Path, to: &Path) {
    copy(from, to).expect("failed to rename file");
    return;
}
