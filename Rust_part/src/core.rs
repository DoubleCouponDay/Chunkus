use std::ffi::CString;
use std::ptr::{self};
use crate::constants::{
    FfiResult
};
use crate::options::{ParsedOptions};

mod ffimodule
{
    use libc::{c_int};

    #[link(name = "libz", kind = "static")]
    #[link(name = "libpng16", kind = "static")]
    #[link(name = "vec", kind = "static")] 
    extern {        
        pub fn entrypoint(argc: c_int, argv: *mut *mut u8) -> c_int;
        pub fn just_crash() -> c_int;
    }
}

fn call_entrypoint(input: &mut CString, output: &mut CString, chunk: &mut CString, threshold: &mut CString, numcolours: &mut CString) -> FfiResult
{
    let result: FfiResult;
    let mut argv: [*mut u8; 6] = [
        ptr::null_mut(), 
        input.as_ptr() as *mut u8, 
        output.as_ptr() as *mut u8, 
        chunk.as_ptr() as *mut u8, 
        threshold.as_ptr() as *mut u8,
        numcolours.as_ptr() as *mut u8,
    ];

    unsafe { 
        
        let cint = ffimodule::entrypoint(5, argv.as_mut_ptr()); 
        let between: i32 = cint;
        result = FfiResult::from(between);
    };
    result
}

pub fn do_vectorize(input_file: &String, output_file: &String, options: ParsedOptions) -> FfiResult
{
    let input_copy = input_file.clone();
    let output_copy = output_file.clone();
    
    println!("vectorizing with input: {} output: {}, chunk: {}, threshold: {}, numcolours: {}", input_file, output_file, options.chunksize, options.threshold, options.numcolours);
    
    let mut input_c = CString::new(input_copy).unwrap();
    let mut output_c = CString::new(output_copy).unwrap();
    let mut chunk_c = CString::new(options.chunksize).unwrap();
    let mut threshold_c = CString::new(options.threshold).unwrap();
    let mut colours_c = CString::new(options.numcolours).unwrap();

    return call_entrypoint(&mut input_c, &mut output_c, &mut chunk_c, &mut threshold_c, &mut colours_c);
}

pub fn crashing_this_plane() -> i32 {
    println!("with no survivors");

    unsafe {
        let kapow = ffimodule::just_crash();
        return kapow;
    }
}