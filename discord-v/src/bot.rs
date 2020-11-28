use serenity::{
    async_trait,
    http::Http,
    model::{
        prelude::Message
    },
    framework::standard::{
        Args,
        Configuration,
        CommandResult,
        CommandGroup,
        StandardFramework,
        macros::{
            group, command, help, check, hook,
        },
    },
    client::{
        Client, Context, EventHandler
    }
};
use crate::core::{call_vectorize, do_vectorize};
use std::ptr;
use std::{
    fs::File,
    path::Path,
    collections::HashSet,
};
use crate::constants;


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

pub async fn create_vec_bot(token: &str) -> Client
{
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
        .with_whitespace(true))
            .group(&GENERAL_GROUP);
        
    println!("creating client...");

    // Login with a bot token from the environment
    let client = Client::new(&token)
        .event_handler(DefaultHandler)
        .framework(framework)
        .await
        .expect("Error creating client");

    client
}

pub struct DefaultHandler;

#[async_trait]
impl EventHandler for DefaultHandler {
    async fn message(&self, _ctx: Context, _msg: Message) {
        println!("message received");
    }
}

#[group]
#[commands(vectorize)]
struct General;
 
#[command]
async fn vectorize(_ctx: &Context, msg: &Message) -> CommandResult {
    println!("vectorizing...");
    println!("embed count {0}", msg.embeds.len());
    for embed in msg.embeds.iter() {
        if let Some(embed_url) = &embed.url
        {
            // println!("embed url: {0}", embed_url);
            // let mut filename = String::from("./");
            // filename.push_str(constants::inputfilename);
            // let mut epic_path = Path::new(&filename);
            // let _random_file = File::create(&epic_path)?;
            // let path_string = epic_path.to_str();
            
            // let mut real_path = match path_string
            // {
            //     Some(pp) => String::from(pp),
            //     None => continue
            // };

            // let mut arguments = String::from("--test");
            // let mut epic_path_array: [*mut u8; 3] = [ptr::null_mut(), real_path.as_mut_ptr(), arguments.as_mut_ptr()];
            // let _array_pointer = epic_path_array.as_mut_ptr();
            
            // let out = call_vectorize(3, _array_pointer);
            // println!("Vectorized with return code: {0}", out);
            
            let input = String::from(constants::INPUTFILENAME);
            let output = String::from(constants::OUTPUTFILENAME);

            let result = do_vectorize(&input, &output);
            println!("Vectorized with return code: {0}", result);
        }

        else
        {
            println!("Found empty embed o.o");
        }
    }
    Ok(())
}
