#![allow(unused_imports)]
fn main() {
    let path = std::env::current_dir().unwrap();

    if let Some(currentdir) = path.as_path().to_str() {
        println!("current directory: {}", currentdir);

        //detect the C part

        let windowsfound_shared = std::fs::copy(
            "../build/windows/x64/debug/staticvectorizer.dll",
            "./vec.dll",
        );
        let linuxfound_shared = std::fs::copy(
            "../build/linux/x86_64/debug/libstaticvectorizer.so",
            "./libvec.so",
        );

        let windows_found_static = std::fs::copy(
            "../build/windows/x64/debug/staticvectorizer.lib",
            "./vec.lib",
        );
        if windowsfound_shared.is_err() && linuxfound_shared.is_err() {
            println!("Neither Vectorizer Library Found.");
        } else if windowsfound_shared.is_err() {
            println!("Debug Linux Vectorizer lib found.");
        } else {
            println!("Debug Windows Vectorizer lib found.");
        }
        println!("cargo:rustc-link-search=./");

        //detect the conan libraries
        
        if let Ok(conanpathstr) = std::env::var("conanpath") {
            let conanpath = std::path::Path::new(&conanpathstr);

            check_directory(conanpath);
            
        }

        else {
            println!("could not find the 'conanpath' environment variable.");
        }

        // Rerun if any library file was deleted
        println!("cargo:rerun-if-changed=vec.lib");
        println!("cargo:rerun-if-changed=vec.dll");
        println!("cargo:rerun-if-changed=libvec.a");
        println!("cargo:rerun-if-changed=libvec.so");
    } 
    
    else {
        eprintln!("could not get the current directory.");
    }
}

fn check_directory(dir: &std::path::Path)
{
    match dir.read_dir()
    {
        Ok(real_dir) => 
        {
            for item in real_dir
            {
                match item
                {
                    Ok(real_item) => {
                        if let Ok(real_file_type) = real_item.file_type()
                        {
                            if real_file_type.is_dir()
                            {
                                check_directory(&real_item.path());
                            }
                            else if real_file_type.is_file()
                            {
                                if let Some(real_file_name) = real_item.file_name().to_str()
                                {
                                    if real_file_name.ends_with(".lib")
                                    {
                                        if let Some(dir_name) = dir.to_str()
                                        {
                                            println!("cargo:rustc-link-search={}", dir_name);
                                            println!("cargo:rustc-link-lib=static={}", real_file_name.strip_suffix(".lib").expect("Build script failed to remove '.lib' from a filename"));
                                        }
                                    }
                                }
                            }
                        }
                    },
                    Err(_) => eprintln!("Invalid Directory Entry Found"),
                }
            }
        },
        Err(_) => eprintln!("Couldn't read directory"),
    }
}