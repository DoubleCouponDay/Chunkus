#![allow(unused_imports)]

use std::ops::Add;
fn main() {
    println!("cargo:rustc-link-search=./");
    let path = std::env::current_dir().unwrap();
    let release_env_var = std::env::var(RELEASEBUILDKEY);
    
    let unwrapped_var: bool = match release_env_var {
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

    //copy the template svg    
    let _ = std::fs::copy("../template.svg", std::env::var("OUT_DIR").unwrap());

    // copy a test image
    let _ = std::fs::copy("test.png", std::env::var("OUT_DIR").unwrap());

    if let Some(_currentdir) = path.as_path().to_str() {
        //detect the C part
        let linuxfound_shared;
        let windows_found_static;
        let new_windows_lib = String::from(WINDOWSCORE).add(LIB_EXT);
        let new_linux_lib = String::from(LINUXCORE).add(A_EXT);

        if unwrapped_var //release build
        {
            println!("release C build detected");

            let lin_path = String::from(LIN_COMMON_PATH)
                .add(RELEASENAME)
                .add(LINUXCORE)
                .add(A_EXT);

            let win_path = String::from(WIN_COMMON_PATH)
                .add(RELEASENAME)
                .add(WINDOWSCORE)
                .add(LIB_EXT);

            linuxfound_shared = std::fs::copy(
                std::path::Path::new(&lin_path),
                std::path::Path::new(&new_linux_lib),
            );
            windows_found_static = std::fs::copy(
                std::path::Path::new(&win_path),
                std::path::Path::new(&new_windows_lib)
            );
        }

        else //debug build
        {
            println!("debug C build detected");

            let lin_path = String::from(LIN_COMMON_PATH)
                .add(DEBUGNAME)
                .add(LINUXCORE)
                .add(A_EXT);

            let win_path = String::from(WIN_COMMON_PATH)
                .add(DEBUGNAME)
                .add(WINDOWSCORE)
                .add(LIB_EXT);

            linuxfound_shared = std::fs::copy(
                std::path::Path::new(&lin_path),
                std::path::Path::new(&new_linux_lib),
            );
            windows_found_static = std::fs::copy(
                std::path::Path::new(&win_path),
                std::path::Path::new(&new_windows_lib)
            );
        }

        if linuxfound_shared.is_err() && windows_found_static.is_err() {
            let copy_err = windows_found_static.unwrap_err();
            println!("C part not found on either OS. error: {}", copy_err);
            panic!("copy_err: {}", copy_err);
        }

        else if linuxfound_shared.is_ok() && windows_found_static.is_err() {
            println!("linux build found.");
            println!("cargo:rustc-link-lib=static={}", LINUXCORE);
        }

        else if linuxfound_shared.is_err() && windows_found_static.is_ok() {
            println!("windows build found.");
            println!("cargo:rustc-link-lib=static={}", WINDOWSCORE);
        }

        else {
            panic!("build script said 'wtf'");
        }

        if let Ok(conanpathstr) = std::env::var(CONANPATHKEY) { //detect the conan libraries
            let conanpath = std::path::Path::new(&conanpathstr);
            link_c_libs(conanpath);
        }

        else {
            let message = String::from("could not find the ")
                .add(CONANPATHKEY)
                .add("environment variable.");

            println!("{}", message);
        }

        // Rerun if any library file was deleted
        #[cfg(target_os = "windows")] 
        {
            println!("cargo:rerun-if-changed={}.lib", WINDOWSCORE);
            println!("cargo:rerun-if-changed={}.dll", WINDOWSCORE);
        }

        #[cfg(target_os = "linux")]
        {
            let linux_bin = String::from(LINUXCORE).add(A_EXT);
            println!("cargo:rerun-if-changed={}", linux_bin);
        }
    } 
    
    else {
        eprintln!("could not get the current directory.");
    }
}

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

fn link_c_libs(directory: &std::path::Path) {
    match directory.read_dir()
    {
        Ok(mut real_dir) => {   
            check_dir_items(&mut real_dir, &directory)
        },
        Err(_) => eprintln!("Couldn't read directory"),
    }
}

fn check_dir_items(read_directory: &mut std::fs::ReadDir, directory: &std::path::Path) {
    for dir_item in read_directory
    {
        match dir_item
        {
            Ok(real_item) => {
                check_file_type(&real_item, &directory);
            },
            Err(_) => eprintln!("Invalid Directory Entry Found"),
        }
    }
}

fn check_file_type(real_item: &std::fs::DirEntry, directory: &std::path::Path) {
    if let Ok(real_file_type) = real_item.file_type()
    {
        if real_file_type.is_dir()
        {
            link_c_libs(&real_item.path());
        }
        else if real_file_type.is_file()
        {
            if let Some(real_file_name) = real_item.file_name().to_str()
            {
                let dir_name = directory.to_str().unwrap();

                if real_file_name.ends_with(LIB_EXT)
                {
                    
                    add_directory_to_linker(dir_name, real_file_name, LIB_EXT);
                }

                else if real_file_name.ends_with(A_EXT) {
                    add_directory_to_linker(dir_name, real_file_name, A_EXT)
                }
            }
        }
    }
}

fn add_directory_to_linker(dir_name: &str, real_file_name: &str, filetype: &'static str) {
    println!("cargo:rustc-link-search={}", dir_name);
    let stripped = real_file_name.strip_suffix(filetype)
        .expect("Build script failed to remove '.lib' from a filename");

    println!("cargo:rustc-link-lib=static={}", stripped);
}
