use libc::c_int;
use std::ptr;

mod ffimodule
{
    use libc::c_int;

    #[link(name = "zlib", kind = "static")]
    #[link(name = "libpng16", kind = "static")]
    #[link(name = "vec", kind = "static")]
    extern {
        pub fn entrypoint(argc: c_int, argv: *mut *mut u8) -> c_int;
    }
}

pub fn call_vectorize(argc: c_int, argv: *mut *mut u8) -> c_int
{
    let mut result = 0;

    unsafe { 
        result = ffimodule::entrypoint(argc, argv); 
    };
    result
}

pub fn do_vectorize(input_file: &String, output_file: &String) -> c_int
{
    let mut input_copy = input_file.clone();
    let mut output_copy = output_file.clone();

    let mut new_array: [*mut u8; 3] = [ptr::null_mut(), input_copy.as_mut_ptr(), output_copy.as_mut_ptr()];
    
    call_vectorize(3, new_array.as_mut_ptr())
}
