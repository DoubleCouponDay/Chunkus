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
        sleep, spawn, JoinHandle,
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
use tokio::runtime::Runtime;

pub struct RunningBot {
    pub thread: JoinHandle<()>
}

pub fn start_running_bot(mut client: Client) -> RunningBot {
    let thread = spawn(move || {
        let runtime = Runtime::new().expect("Unable to create the runtime");
            
        // Continue running until notified to shutdown
        runtime.block_on(async {
            client.start().await.expect("unable to start client");
        });        
    });

    RunningBot {
        thread: thread
    }
}
