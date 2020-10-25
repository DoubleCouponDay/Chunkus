mod fk_you
{
    use libc::c_int;

    #[link(name = "vectorizer_library", kind = "static")]
    extern {
        pub fn entrypoint(argc: c_int, argv: *mut *mut u8) -> c_int;
    }
}

use libc::c_int;

// mod link
// {
//     use libc::c_int;
//     pub fn entrypoint(argc: c_int, argv: *mut *mut u8) -> c_int
//     {
//         0
//     }
// }

pub fn call_vectorize(argc: c_int, argv: *mut *mut u8) -> c_int
{
    let mut result = 0;
    unsafe { result = fk_you::entrypoint(argc, argv); }
    result
}
