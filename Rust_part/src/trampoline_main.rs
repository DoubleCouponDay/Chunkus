
use serenity::{
    framework::standard::{
        CommandResult
    }
};
use vecbot::{
    secrettoken::getwatchertoken,
    trampoline::{
        create_trampoline_bot
    }
};
use std::env::args;

#[tokio::main]
async fn main() -> CommandResult
{
    let arguments = args();
    let shouldcrash = arguments.last().unwrap().parse::<bool>().unwrap();
    let watcher_token_obj = getwatchertoken();
    let watcher_token = watcher_token_obj.as_str();
    println!("starting trampoline with shouldcrash: {}", shouldcrash);
    let mut watcher_client = create_trampoline_bot(watcher_token, shouldcrash, None).await;    
    println!("trampoline running...");

    if let Err(why) = watcher_client.start().await
    {
        eprintln!("Failed to start trampoline: {}", why);
    }

    Ok(())
}