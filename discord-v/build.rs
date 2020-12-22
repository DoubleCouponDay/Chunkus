#![allow(unused_imports)]
fn main() {    
    let path = std::env::current_dir().unwrap();

    if let Some(currentdir) = path.as_path().to_str() {
        println!("current directory: {}", currentdir);

        let windowsfound = std::fs::copy("../build/windows/x64/release/staticvectorizer.lib", "./vec.lib");
        let linuxfound = std::fs::copy("../build/linux/x86_64/release/libstaticvectorizer.a", "./libvec.a");

        if windowsfound.is_err() && linuxfound.is_err()
        {
            let windowsfound = std::fs::copy("../build/windows/x64/debug/staticvectorizer.lib", "./vec.lib");
            let linuxfound = std::fs::copy("../build/linux/x86_64/debug/libstaticvectorizer.a", "./libvec.a");
            if windowsfound.is_err() && linuxfound.is_err()
            {
                println!("Neither Vectorizer Library Found.");
            }
            else if windowsfound.is_err()
            {
                println!("Debug Linux Vectorizer lib found.");
            }
            else
            {
                println!("Debug Windows Vectorizer lib found.");
            }
        }
        else
        {
            if windowsfound.is_err()
            {
                println!("Release Linux lib found.");
            }
            else 
            {
                println!("Release Windows lib found.");
            }
        }
        println!("cargo:rustc-link-search=./");
    }
    else 
    {
        println!("could not get the current directory.");
    }
}