
use serenity::{
    framework::standard::{
        CommandResult
    }
};
use vecbot::{
    secrettoken::getwatchertoken,
    trampoline::{
        create_trampoline_bot,
        initialize_child
    }
};

#[tokio::main]
async fn main() -> CommandResult
{
    let watcher_token_obj = getwatchertoken();
    let watcher_token = watcher_token_obj.as_str();

    let mut watcher_client = create_trampoline_bot(watcher_token).await;
    initialize_child(&watcher_client.data).await;
    println!("trampoline running...");

    if let Err(why) = watcher_client.start().await
    {
        eprintln!("Failed to run watcher client: {}", why);
    }

    Ok(())
}