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


/**
fn main() {

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
    let mut client = Client::new(&token)
        .event_handler(Handler)
        .framework(framework)
        .await
        .expect("Error creating client"); 

    // start listening for events by starting a single shard
    if let Err(why) = client.start().await {
        println!("An error occurred while running the client: {:?}", why);
    }
}
**/

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
use serenity::
{
    client::bridge::gateway::ShardManager,
    http::Http,
    prelude::*
};

use std::
{
    collections::HashSet,
    sync::Arc,
};

#[group]
#[commands(ping)]
#[commands(why_are_you_gay)]
#[commands(quit, why)]
struct General;

struct Handler;

#[async_trait]
impl EventHandler for Handler {}

struct ShardManagerContainer;

impl TypeMapKey for ShardManagerContainer {
    type Value = Arc<Mutex<ShardManager>>;
}

#[command]
async fn ping(ctx: &Context, msg: &Message) -> CommandResult {
    msg.reply(ctx, "Pong!").await?;

    Ok(())
}

#[command]
async fn why_are_you_gay(ctx: &Context, msg: &Message) -> CommandResult 
{
    let channel = match msg.channel_id.to_channel(&ctx).await
    {
        Ok(channel) => channel,
        Err(why) => {
            println!("Error getting channel for whatever reason, like my god, just work jesus, ok just kidding I got a reason: {:?}", why);
            return Ok(())
        },
    };

    let response = MessageBuilder::new()
        .push("You: ")
        .push_bold_safe(&msg.author.name)
        .push(" is like mega gay in the ")
        .mention(&channel)
        .push(" channel for gays")
        .build();
    
    if let Err(why) = msg.channel_id.say(&ctx.http, &response).await {
        println!("Error sending message: {:?}", why);
    }

    Ok(())
}

#[command]
#[allowed_roles("prob doing stuff")]
async fn quit(ctx: &Context, msg: &Message) -> CommandResult
{
    let data = ctx.data.read().await;

    if let Some(manager) = data.get::<ShardManagerContainer>() {
        manager.lock().await.shutdown_all().await;
    } else {
        msg.reply(ctx, "There was a problem getting the shard manager").await?;

        return Ok(());
    }

    msg.reply(ctx, "Shutting down!").await?;

    Ok(())
}

#[command]
#[sub_commands(are)]
async fn why(_ctx: &Context, _msg: &Message) -> CommandResult
{
    Ok(())
}

#[command]
#[sub_commands(you)]
async fn are(_ctx: &Context, _msg: &Message) -> CommandResult
{
    Ok(())
}

#[command]
#[sub_commands(gay)]
async fn you(_ctx: &Context, _msg: &Message) -> CommandResult
{
    Ok(())
}

#[command]
async fn gay(ctx: &Context, msg: &Message) -> CommandResult
{
    msg.reply(ctx, "Well why are you running?").await?;
    Ok(())
}
