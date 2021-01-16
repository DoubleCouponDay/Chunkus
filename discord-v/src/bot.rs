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
            group, command,
        },
    },
    client::{
        Client, ClientBuilder, Context, EventHandler
    },
};
use crate::core::{do_vectorize};
use std::{
    collections::HashSet,
    fs::File,
    path::Path,
    io::prelude::*,
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

    // Use ClientBuilder to generate the Client instance
    let client = ClientBuilder::new(&token)
        .event_handler(DefaultHandler)
        .framework(framework)
        .await
        .expect("Error Building Client");

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
    let client = ClientBuilder::new(&token)
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
    let client = ClientBuilder::new(&token)
        .event_handler(DefaultHandler)
        .framework(framework)
        .await
        .expect("Error creating client");

    client
}

pub struct DefaultHandler;

#[group]
#[commands(vectorize)]
struct General;

#[async_trait]
impl EventHandler for DefaultHandler {
    async fn message(&self, _ctx: Context, _msg: Message) {
        println!("message received");
    }
}
 
#[command]
async fn vectorize(ctx: &Context, msg: &Message) -> CommandResult {
    println!("vectorizing...");
    println!("embed count {0}", msg.embeds.len());
    for embed in msg.embeds.iter() {
        if let Some(embed_url) = &embed.url
        {
            // Download file using Reqwest

            let client = reqwest::Client::new();

            let response = match client.get(embed_url).send().await
                {
                    Err(_) => return Ok(()),
                    Ok(thing) => thing
                };
            
            let mut file = match File::create(Path::new(constants::INPUTFILENAME))
                {
                    Err(_) => return Ok(()),
                    Ok(thing) => thing
                };
            
            match response.bytes().await
            {
                Ok(bytes) => match file.write_all(&bytes) { Err(_) => return Ok(()), _ => () },
                Err(_) => return Ok(()),
            };

            if let Err(err) = file.sync_all()
            {
                eprintln!("Error {}", err);
                return Ok(());
            }



            // Execute Algorithm
            let input = String::from(constants::INPUTFILENAME);
            let output = String::from(constants::OUTPUTFILENAME);

            let result = do_vectorize(&input, &output);
            println!("Vectorized with return code: {0}", result);

            if result != 0
            {
                return Ok(());
            }

            // Send the output
            let msg_files = vec![output.as_str()];

            let msg = msg.channel_id.send_files(&ctx.http, msg_files, |m|
            {
                m.content("Here's your result")
            }).await;

            if let Err(err) = msg
            {
                println!("Error sending result {}", err);
            }
        }
        else
        {
            println!("Found empty embed o.o");
        }
    }
    Ok(())
}