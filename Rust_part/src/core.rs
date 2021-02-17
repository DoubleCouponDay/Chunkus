use std::ffi::CString;
use std::ptr;
use crate::constants::{
    FfiResult
};
use crate::options::{ParsedOptions};

mod ffimodule
{
    use libc::{c_int};

    #[link(name = "zlib", kind = "static")]
    #[link(name = "libpng16", kind = "static")]
    #[link(name = "vec", kind = "static")]
    extern {        
        pub fn entrypoint(argc: c_int, argv: *mut *mut u8) -> c_int;
        pub fn set_algorithm(algo: c_int) -> c_int;
    }
}

pub fn call_vectorize(input: &mut CString, output: &mut CString, chunk: &mut CString, threshold: &mut CString) -> FfiResult
{
    let result: FfiResult;

    unsafe { 
        let mut argv: [*mut u8; 5] = [ptr::null_mut(), input.as_ptr() as *mut u8, output.as_ptr() as *mut u8, chunk.as_ptr() as *mut u8, threshold.as_ptr() as *mut u8];
        let cint = ffimodule::entrypoint(5, argv.as_mut_ptr()); 
        let between: i32 = cint;
        result = FfiResult::from(between);
    };
    result
}

pub fn set_algorithm(algorithm: i32) -> FfiResult
{
    let result: FfiResult;
    unsafe
    {
        result = FfiResult::from(ffimodule::set_algorithm(algorithm));
    }
    result
}

pub fn do_vectorize(input_file: &String, output_file: &String, options: ParsedOptions) -> FfiResult
{
    let input_copy = input_file.clone();
    let output_copy = output_file.clone();
    
    println!("vectorizing with input: {} output: {}, chunk: {}, threshold: {}", input_file, output_file, options.chunksize, options.threshold);
    
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

            if let Ok(cccc) = CString::new(options.chunksize)
            {
                chunk_c = cccc;

                if let Ok(ccccc) = CString::new(options.threshold)
                {
                    threshold_c = ccccc;                    
                    return call_vectorize(&mut input_c, &mut output_c, &mut chunk_c, &mut threshold_c)
                }
            }
        }
    }
    FfiResult::AssumptionWrong
}
