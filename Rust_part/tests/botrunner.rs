use serenity::client::{
    EventHandler, bridge::gateway::ShardManager
};
use vecbot::bot::{
    create_bot_with_handle, DefaultHandler
};
use tokio::runtime::Runtime;
use serenity::{
    Client
};
use std::{
    thread::JoinHandle
};
use std::{thread, time::{Duration}};
use std::sync::{Arc};

use vecbot::secrettoken::{gettoken, getwatchertoken};

pub struct RunningBot {
    pub client: Client,
    pub shard_manager: Arc<tokio::sync::Mutex<ShardManager>>
}

pub async fn start_running_bot<H: EventHandler + 'static>(handler: H) -> RunningBot {
    let token1 = gettoken();
    let mut client = create_bot_with_handle(token1.as_str(), DefaultHandler, false).await;

    // Used to shutdown
    let shard_manager = client.shard_manager.clone();

    // Start bot 1 (Vectorizer)
    let _ = client.start();

    thread::sleep(Duration::from_secs(2));
    
    RunningBot {
        client,
        shard_manager
    }
}
