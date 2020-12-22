use libc::c_int;
use std::ptr;

mod yogotem
{
    use libc::c_int;

    #[link(name = "vec")]
    extern {
        //pub fn entrypoint(argc: c_int, argv: *mut *mut u8) -> c_int;
    }
}

pub fn call_vectorize(argc: c_int, argv: *mut *mut u8) -> c_int
{
    let mut result = 0;
    unsafe { 
        //result = yogotem::entrypoint(argc, argv); 
    };
    result
}

pub fn do_vectorize(input_file: &String, output_file: &String) -> c_int
{
    let mut input_copy = input_file.clone();
    let mut output_copy = output_file.clone();

    let mut array_shit: [*mut u8; 3] = [ptr::null_mut(), input_copy.as_mut_ptr(), output_copy.as_mut_ptr()];
    
    call_vectorize(3, array_shit.as_mut_ptr())
}
