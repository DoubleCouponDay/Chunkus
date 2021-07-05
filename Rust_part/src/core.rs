use std::ffi::CString;
use std::ptr;
use crate::constants::{
    FfiResult
};
use crate::options::{ParsedOptions};

mod ffimodule
{
    use libc::{c_int};

    //if no kind given, defaults to dynamic
    #[link(name = "zlib", kind = "static")]
    #[link(name = "png16", kind = "static")]
    #[link(name = "vec", kind = "static")] 
    extern {        
        pub fn entrypoint(argc: c_int, argv: *mut *mut u8) -> c_int;
        pub fn set_algorithm(algo: *mut *mut u8) -> c_int;
    }
}

fn call_vectorize(input: &mut CString, output: &mut CString, chunk: &mut CString, threshold: &mut CString, numcolours: &mut CString) -> FfiResult
{
    let result: FfiResult;

    unsafe { 
        let mut argv: [*mut u8; 6] = [
            ptr::null_mut(), 
            input.as_ptr() as *mut u8, 
            output.as_ptr() as *mut u8, 
            chunk.as_ptr() as *mut u8, 
            threshold.as_ptr() as *mut u8,
            numcolours.as_ptr() as *mut u8    
        ];
        let cint = ffimodule::entrypoint(5, argv.as_mut_ptr()); 
        let between: i32 = cint;
        result = FfiResult::from(between);
    };
    result
}

pub fn set_algorithm(algorithm: &str) -> FfiResult
{
    let result: FfiResult;
    unsafe
    {
        println!("{}", algorithm);
        let formatted = algorithm.as_ptr() as *mut u8;
        let mut argv: [*mut u8; 1] = [formatted];
        let output = ffimodule::set_algorithm(argv.as_mut_ptr());
        result = FfiResult::from(output);
    }
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

    return call_vectorize(&mut input_c, &mut output_c, &mut chunk_c, &mut threshold_c, &mut colours_c);
}
