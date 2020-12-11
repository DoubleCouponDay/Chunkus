#![allow(unused_imports)]
fn main() {    
    let path = std::env::current_dir().unwrap();

    if let Some(currentdir) = path.as_path().to_str() {
        println!("found an environment's current directory.");
        let mut buildstr = String::from("cd: ");
        buildstr.push_str(currentdir);
        println!("{}", buildstr.as_str());

        let windowsfound = std::fs::copy("../build/windows/x64/release/vectorizer_library.lib", "./vec.lib");
        let linuxfound = std::fs::copy("../build/linux/x86_64/release/libstaticvectorizer.a", "./libvec.a");

        if windowsfound.is_err()
        {
            if linuxfound.is_err()
            {
                println!("could not find the core on either WINDOWS or LINUX, using your static string path.");
            }

            else {
                println!("linux lib found.");
            }
            
        }

        else {
            println!("windows lib found.");
        }
        println!("cargo:rustc-link-search=./");
    }

    else {
        println!("could not get the current directory.");
    }
    
}