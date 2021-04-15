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
use std::{thread};
use std::sync::{Arc};

use vecbot::secrettoken::{
    gettoken
};

pub struct RunningBot {
    pub client: Client,
    pub shard_manager: Arc<tokio::sync::Mutex<ShardManager>>,
    pub thread: JoinHandle<()>
}

pub async fn start_running_bot<H: EventHandler + 'static>(handler: H) -> RunningBot {
    let token1 = gettoken();
    let mut client = create_bot_with_handle(token1.as_str(), DefaultHandler).await;

    // Used to shutdown
    let shard_manager = client.shard_manager.clone();

    // Start bot 1 (Vectorizer)
    let _ = client.start();
    
    // Start bot 2  in another thread
    let thread = thread::spawn(move || {
        let runtime = Runtime::new().expect("Unable to create the runtime");

        println!("Runtime created");
            
        // Continue running until notified to shutdown
        runtime.block_on(async {
            println!("inside async block");

            let token2 = gettoken();

            let mut client2 = create_bot_with_handle(token2.as_str(), handler).await;
            
            client2.start().await.expect(" big pp");
        });
    
        println!("Runtime finished");
    });
    
    RunningBot {
        client,
        shard_manager,
        thread
    }
}
