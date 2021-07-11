use serenity::client::{
    EventHandler, bridge::gateway::ShardManager
};
use vecbot::bot::{
    create_bot_with_handle, DefaultHandler
};
use serenity::{
    Client
};
use std::{
    thread::{
        sleep, spawn, JoinHandle
    },
    time::Duration,
    sync::{Arc},
    option::{Option}
};
use vecbot::{
    secrettoken::{gettoken, getwatchertoken},
    trampoline::{
        create_trampoline_bot,
        initialize_child
    },

};

pub struct RunningBot {
    pub client: Client,
    pub shard_manager: Arc<tokio::sync::Mutex<ShardManager>>,
    pub thread: Option<JoinHandle<()>>
}

pub async fn start_running_bot<H: EventHandler + 'static>(handler: H) -> RunningBot {
    let token1 = gettoken();
    let mut client = create_bot_with_handle(token1.as_str(), DefaultHandler, false).await;
    let shard_manager = client.shard_manager.clone();

    let _ = client.start();

    sleep(Duration::from_secs(2));
    
    RunningBot {
        client,
        shard_manager,
        thread: Option::None
    }
}

pub async fn start_running_trampoline<T:EventHandler + 'static>(trampoline_handler: T) -> (RunningBot, RunningBot) {
    //initialize_child(&trampoline.data, true).await; //raises the flag

    let shard_manager = client.shard_manager.clone();

    // // Start bot 1 (Vectorizer)
    let _ = client.start();
    
    // Start bot 2  in another thread
    let thread = thread::spawn(move || {
        let runtime = Runtime::new().expect("Unable to create the runtime");

        println!("Runtime created");
            
        // Continue running until notified to shutdown
        runtime.block_on(async {
            println!("inside async block");

            let token2 = gettoken();

            let mut client2 = create_bot_with_handle(token2, handler).await;
            
            client2.start().await.expect(" big pp");
        });
    
        println!("Runtime finished");
    });

    let shard_manager = client.shard_manager.clone();

    // // Start bot 1 (Vectorizer)
    // let _ = client.start();
    
    // // Start bot 2  in another thread
    // let thread = thread::spawn(move || {
    //     let runtime = Runtime::new().expect("Unable to create the runtime");

    //     println!("Runtime created");
            
    //     // Continue running until notified to shutdown
    //     runtime.block_on(async {
    //         println!("inside async block");

    //         let token2 = gettoken();

    //         let mut client2 = create_bot_with_handle(token2, handler).await;
            
    //         client2.start().await.expect(" big pp");
    //     });
    
    //     println!("Runtime finished");
    // });

    (
        RunningBot {

        },
        RunningBot {

        }
    )
}

