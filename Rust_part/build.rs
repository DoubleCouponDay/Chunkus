#![allow(unused_imports)]
fn main() {
    let path = std::env::current_dir().unwrap();
    let do_release = std::env::var("releasebuild").unwrap_or(String::from("false")).parse::<bool>().unwrap_or(false);

    if let Some(currentdir) = path.as_path().to_str() {
        println!("current directory: {}", currentdir);

        //detect the C part
        let windowsfound_shared;
        let linuxfound_shared;
        let windows_found_static;
        if do_release
        {
            windowsfound_shared = std::fs::copy(
                "../../build/windows/x64/release/vec.dll",
                "./vec.dll"
            );
            linuxfound_shared = std::fs::copy(
                "../../build/linux/x86_64/release/libvec.a",
                "./vec.so",
            );
            windows_found_static = std::fs::copy(
                "../../build/windows/x64/release/vec.lib",
                "./vec.lib"
            );
        }
        else
        {
            windowsfound_shared = std::fs::copy(
                "../build/windows/x64/debug/vec.dll",
                "./vec.dll"
            );
            linuxfound_shared = std::fs::copy(
                "../build/linux/x86_64/debug/libvec.a",
                "./vec.so",
            );

            windows_found_static = std::fs::copy(
                "../build/windows/x64/debug/vec.lib",
                "./vec.lib"
            );
        }

        if windowsfound_shared.is_err() && linuxfound_shared.is_err() && windows_found_static.is_err() {
            println!("C part not found on either OS.");
        }
        println!("cargo:rustc-link-search=./");

        //detect the conan libraries
        
        if let Ok(conanpathstr) = std::env::var("conanpath") {
            let conanpath = std::path::Path::new(&conanpathstr);
            link_c_libs(conanpath);
        }

        else {
            println!("could not find the 'conanpath' environment variable.");
        }

        // Rerun if any library file was deleted
        println!("cargo:rerun-if-changed=vec.lib");
        println!("cargo:rerun-if-changed=vec.dll");
        println!("cargo:rerun-if-changed=vec.a");
        println!("cargo:rerun-if-changed=vec.so");
    } 
    
    else {
        eprintln!("could not get the current directory.");
    }
}

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
                if real_file_name.ends_with(".lib")
                {
                    if let Some(dir_name) = directory.to_str()
                    {
                        add_directory_to_linker(dir_name, real_file_name);
                    }
                }
            }
        }
    }
}

fn add_directory_to_linker(dir_name: &str, real_file_name: &str) {
println!("yo we found something: {} \n", real_file_name);

    println!("cargo:rustc-link-search={}", dir_name);
    println!("cargo:rustc-link-lib=static={}", real_file_name.strip_suffix(".lib").expect("Build script failed to remove '.lib' from a filename"));
}
