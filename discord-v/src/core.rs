use libc::c_int;

#[link(name = "vectorizer_library", kind = "static")]
extern {
    pub fn entrypoint(argc: c_int, argv: *mut *mut u8) -> c_int;
}