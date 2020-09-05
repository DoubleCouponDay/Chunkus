fn main() {
    if std::fs::copy("../windows/x64/release/vectorizer_library.lib", "./vec-lib/vectorizer_library.lib").is_err()
    {
        if std::fs::copy("../linux/x86_64/debug/vectorizer_library.a", "./vec-lib/vectorizer_library.a").is_err()
        {
            println!("Failed to copy vectorizer library file, please manually move it to vec-lib/");
        }
    }
    println!("cargo:rustc-link-search=./vec-lib/");
}