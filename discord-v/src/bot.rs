
use serenity::async_trait;
use serenity::client::{Client, Context, EventHandler};
use serenity::model::channel::Message;
use serenity::framework::standard::{
    StandardFramework,
    CommandResult,
    macros::{
        command,
        group
    }
};
use serenity::utils::MessageBuilder;
use serenity::{
    client::bridge::gateway::ShardManager,
    http::Http,
    prelude::*
};
use std::{
    collections::HashSet,
    sync::Arc,
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
        .configure(|c| c.owners(owners).prefix("~")) // set the bot's prefix to "~"
        .group(&GENERAL_GROUP);
    println!("creating client...");

    // Login with a bot token from the environment
    let mut client = Client::new(token)
        .event_handler(Handler)
        .framework(framework)
        .await
        .expect("Error creating client");

    client
}

struct Handler;

#[group]
struct General;

#[async_trait]
impl EventHandler for Handler {}