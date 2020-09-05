#[cfg(test)]
mod tests {
    use std;
    use std::env;
    use std::fs;
    use std::io;
    use std::path;
    use std::result;
    use std::string::String;
    use std::ptr;
    extern crate libc;
    use libc::c_int;
    use std::collections::HashSet;
    use serenity::{
        http,
        framework::{
            StandardFramework
        },
        client
    };

    #[test]
    fn it_can_run_a_test() {
        assert_eq!(true, true);
    }

    async fn it_connects_to_discord() {
        let token = "MjgzMDk2OTE5NTAzNDA1MDU2.WKp0jQ.8wrBzzk7AEHNm-zG87-tw1eqF-k";

        let http = http::Http::new_with_token(&token);

    // We will fetch your bot's owners and id
        let (owners, _bot_id) = match http.get_current_application_info().await {
            Ok(info) => {
                let mut owners = HashSet::new();
                owners.insert(info.owner.id);

                (owners, info.id)
            },
            Err(why) => panic!("Could not access application info: {:?}", why),
        };

        let framework = StandardFramework::new()
            .configure(|c| c.owners(owners).prefix("~")) // set the bot's prefix to "~"
            .group(&GENERAL_GROUP);

     // Login with a bot token from the environment
        let mut client = client::Client::new(token)
            .event_handler(Handler)
            .framework(framework)
            .await
            .expect("Error creating client");
    }

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