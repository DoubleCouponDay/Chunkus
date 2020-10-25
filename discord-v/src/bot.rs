use serenity::{
    async_trait,
    http::Http,
    model::{
        prelude::Message
    },
    framework::standard::{
        CommandResult,
        StandardFramework,
        macros::{
            group, command
        }
    },
    client::{
        Client, Context, EventHandler
    }
};
use std::ptr;
use std::{
    fs::File,
    path::Path,
    collections::HashSet,
};
use libc::c_int;

pub async fn create_bot(token: &'static str) -> Client {
    
    println!("creating http token...");
    let http = serenity::http::Http::new_with_token(&token);
    
    println!("fetching owner id, bot id...");

    let (_, _bot_id) = match http.get_current_application_info().await {
        Ok(info) => {
            let mut owners = HashSet::new();
            owners.insert(info.owner.id);

            (owners, info.id)
        },
        Err(why) => panic!("Could not access application info: {:?}", why),
    };
    println!("creating framework...");

    let framework = StandardFramework::new().configure(|c| c
        .on_mention(Some(_bot_id))
        .with_whitespace(true));
        
    println!("creating client...");

    // Login with a bot token from the environment
    let client = Client::new(&token)
        .event_handler(DefaultHandler)
        .framework(framework)
        .await
        .expect("Error creating client");

    client
}

pub async fn create_bot_with_handle<H: EventHandler + 'static>(token: &str, handler: H) -> Client {    
    println!("creating http token...");
    let http = Http::new_with_token(&token);
    
    println!("fetching owner id, bot id...");

    let (_, _bot_id) = match http.get_current_application_info().await {
        Ok(info) => {
            let mut owners = HashSet::new();
            owners.insert(info.owner.id);

            (owners, info.id)
        },
        Err(why) => panic!("Could not access application info: {:?}", why),
    };
    println!("creating framework...");

    let framework = StandardFramework::new().configure(|c| c
        .on_mention(Some(_bot_id))
        .with_whitespace(true));
        
    println!("creating client...");

    // Login with a bot token from the environment
    let client = Client::new(&token)
        .event_handler(handler)
        .framework(framework)
        .await
        .expect("Error creating client");

    client
}

pub struct DefaultHandler;

#[group]
#[commands(vectorize)]
struct General;
 
#[command]
async fn vectorize(_ctx: &Context, msg: &Message) -> CommandResult {
    for embed in msg.embeds.iter() {
        if let Some(_img) = &embed.image
        {
            let filename = String::from("./ching chang chong.txt");

            let _random_file = File::create(Path::new(&filename))?;

            let mut epic_path = String::from("yo mama so fat");
            let path_pointer = epic_path.as_mut_ptr();
            let mut epic_thingo = String::from("--test");
            let mut epic_path_array: [*mut u8; 3] = [ptr::null_mut(), path_pointer, epic_thingo.as_mut_ptr()];
            let _array_pointer = epic_path_array.as_mut_ptr();
        
            unsafe {
                super::core::call_vectorize(3, ptr::null_mut());
            }
        }
    }
    Ok(())
}

#[async_trait]
impl EventHandler for DefaultHandler {
    async fn message(&self, _ctx: Context, _msg: Message) {
        println!("why did you ping me");
    }
}
