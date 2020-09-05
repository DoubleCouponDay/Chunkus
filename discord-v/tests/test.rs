#[cfg(test)]
mod tests {
    use std;
    use std::env;
    use std::fs;
    use std::io;
    use std::path;
    use std::result;
    use std::string::String;
    extern crate libc;
    use libc::c_int;

    #[test]
    fn it_can_run_a_test() {
        assert_eq!(true, true);
    }

    fn it_connects_to_discord() {
        
    }

    #[test]
    fn the_core_program_can_be_run() -> result::Result<(), io::Error> {
        println!("starting initial core test...");

        let mut epic_path = String::from("./basic.png");
        let mut path_pointer = epic_path.as_mut_ptr();
        let mut epic_path_array: [*mut u8; 1] = [path_pointer];
        let mut array_pointer = epic_path_array.as_mut_ptr();

        unsafe {
            //epic C code here
            let result = entrypoint(1, array_pointer);
            println!("{}", result);
        }
        
        Ok(())
    }

    #[link(name = "vectorizer_library", kind = "static")]
    extern {
        fn entrypoint(argc: c_int, argv: *mut *mut u8) -> c_int;
    }
}