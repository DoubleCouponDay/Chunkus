#![allow(unused_imports)]
fn main() {
    let path = std::env::current_dir().unwrap();

    if let Some(currentdir) = path.as_path().to_str() {
        println!("current directory: {}", currentdir);

        // Unsuccessful copy, there are no RELEASE files:
        // Try to copy the DEBUG files in case we're using those instead
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

        // Rerun if any library file was deleted
        println!("cargo:rerun-if-changed=vec.lib");
        println!("cargo:rerun-if-changed=vec.dll");
        println!("cargo:rerun-if-changed=libvec.a");
        println!("cargo:rerun-if-changed=libvec.so");

        vcpkg::find_package("zlib").unwrap();
    } 
    
    else {
        eprintln!("could not get the current directory.");
    }
}
