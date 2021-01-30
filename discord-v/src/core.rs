use libc::c_int;
use std::ffi::CString;
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
    let result;

    unsafe { 
        result = ffimodule::entrypoint(argc, argv); 
    };
    result
}
pub fn call_vectorizer(input: &mut CString, output: &mut CString, chunk: &mut CString, threshold: &mut CString) -> c_int
{
    let result;

    unsafe { 
        let mut argv: [*mut u8; 5] = [ptr::null_mut(), input.as_ptr() as *mut u8, output.as_ptr() as *mut u8, chunk.as_ptr() as *mut u8, threshold.as_ptr() as *mut u8];
        result = ffimodule::entrypoint(5, argv.as_mut_ptr()); 
    };
    result
}

pub fn do_vectorize(input_file: &String, output_file: &String, chunk_size: Option<&str>, threshold: Option<&str>) -> c_int
{
    println!("do_vectorize with input: {} output: {}", input_file, output_file);

    let input_copy = input_file.clone();
    let output_copy = output_file.clone();
    let chunk_size_copy = String::from(chunk_size.unwrap_or("4"));
    let threshold_copy = String::from(threshold.unwrap_or("0"));
    
    let mut input_c;
    let mut output_c;
    let mut chunk_c;
    let mut threshold_c;

    if let Ok(cc) = CString::new(input_copy)
    {
        input_c = cc;
        if let Ok(ccc) = CString::new(output_copy)
        {
            output_c = ccc;
            if let Ok(cccc) = CString::new(chunk_size_copy)
            {
                chunk_c = cccc;
                if let Ok(ccccc) = CString::new(threshold_copy)
                {
                    threshold_c = ccccc;
                    return call_vectorizer(&mut input_c, &mut output_c, &mut chunk_c, &mut threshold_c)
                }
            }
        }
    }
    -1
}
