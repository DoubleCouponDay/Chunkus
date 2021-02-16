use std::ffi::CString;
use std::ptr;
use crate::constants::{
    FfiResult
};

const CHUNK_SIZE: &str = "3";
const THRESHHOLD: &str = "400";

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

pub fn do_vectorize(input_file: &String, output_file: &String, chunk_size: Option<String>, threshold: Option<String>) -> FfiResult
{
    let input_copy = input_file.clone();
    let output_copy = output_file.clone();
    let chunk_size_copy = chunk_size.unwrap_or(String::from(CHUNK_SIZE)); //set inputs or just default
    let threshold_copy = threshold.unwrap_or(String::from(THRESHHOLD));
    
    println!("do_vectorize with input: {} output: {}, chunk: {}, thres: {}", input_file, output_file, chunk_size_copy, threshold_copy);
    
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
                    return call_vectorize(&mut input_c, &mut output_c, &mut chunk_c, &mut threshold_c)
                }
            }
        }
    }
    FfiResult::AssumptionWrong
}
