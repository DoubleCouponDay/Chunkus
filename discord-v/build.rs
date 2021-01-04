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
        
        if let Some(conanpathstr) = std::env::var("conanpath") {
            let conanpath = std::path::Path::new(conanpathstr);

            for diritem in conanpath.read_dir().expect("read dir call failed in build script.") {
                
            }
            
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
