#[cfg(test)]
mod tests {
    use std;
    use std::io;
    use std::result;
    use std::string::String;
    use std::ptr;
    extern crate libc;
    use libc::c_int;

    #[test]
    fn it_can_run_a_test() {
        assert_eq!(true, true);
    }

    // don't think test functions can be async
    // async fn it_connects_to_discord() {
    //    
    //}

    #[test]
    fn the_core_program_can_be_run() -> result::Result<(), io::Error> {
        println!("starting initial core test...");
        
        let mut epic_path = String::from("yo mama so fat");
        let mut path_pointer = epic_path.as_mut_ptr();
        let mut epic_thingo = String::from("--test");
        let mut epic_path_array: [*mut u8; 3] = [ptr::null_mut(), path_pointer, epic_thingo.as_mut_ptr()];
        let mut array_pointer = epic_path_array.as_mut_ptr();

        unsafe {
            //epic C code here
            let result = entrypoint(3, array_pointer);
            println!("{}", result);
        }
        
        Ok(())
    }

    #[link(name = "vectorizer_library", kind = "static")]
    extern {
        fn entrypoint(argc: c_int, argv: *mut *mut u8) -> c_int;
    }
}