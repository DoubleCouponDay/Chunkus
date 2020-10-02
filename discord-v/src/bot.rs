
use serenity::async_trait;
use serenity::client::{Client, EventHandler};
use serenity::framework::standard::{
    StandardFramework,
    macros::{
        group
    }
};
use serenity::{
    http::Http,
};
use std::{
    collections::HashSet,
};

pub async fn create_bot(token: &'static str) -> Client {
    
    println!("creating http token...");
    let http = Http::new_with_token(&token);
    
    println!("fetching owner id, bot id...");

    let (owners, _bot_id) = match http.get_current_application_info().await {
        Ok(info) => {
            let mut owners = HashSet::new();
            owners.insert(info.owner.id);

            (owners, info.id)
        },
        Err(why) => panic!("Could not access application info: {:?}", why),
    };
    println!("creating framework...");

    let framework = StandardFramework::new()
        .on_mention(_bot_id)
        .with_whitespace(true);
        
    println!("creating client...");

    // Login with a bot token from the environment
    let mut client = Client::builder(&token)
        .event_handler(defaultHandler)
        .framework(framework)
        .await
        .expect("Error creating client");

    client
}

pub async fn create_bot_with_handle<Handlerer>(token: &'static str) -> Client where Handlerer: EventHandler + 'static {
    
    println!("creating http token...");
    let http = Http::new_with_token(&token);
    
    println!("fetching owner id, bot id...");

    let (owners, _bot_id) = match http.get_current_application_info().await {
        Ok(info) => {
            let mut owners = HashSet::new();
            owners.insert(info.owner.id);

            (owners, info.id)
        },
        Err(why) => panic!("Could not access application info: {:?}", why),
    };
    println!("creating framework...");

    let framework = StandardFramework::new()
        .on_mention(_bot_id)
        .with_whitespace(true);
        
    println!("creating client...");

    // Login with a bot token from the environment
    let mut client = Client::builder(&token)
        .event_handler(Handlerer)
        .framework(framework)
        .await
        .expect("Error creating client");

    client
}

struct defaultHandler;

#[group]
struct General;

#[async_trait]
impl EventHandler for defaultHandler {}