use serenity::{client::{
    EventHandler, bridge::gateway::ShardManager
}, framework::StandardFramework, model::id::ChannelId};
use vecbot::bot::{
    create_bot_with_handle, DefaultHandler
};
use serenity::{
    Client
};
use std::{collections::HashSet, option::{Option}, sync::{Arc}, thread::{
        sleep, spawn, JoinHandle,
    }, time::Duration};
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

pub async fn get_test_framework(_channelid: u64) -> StandardFramework {
    StandardFramework::new().configure(|a| {
        a.case_insensitivity(true); //dont care about case
        a.ignore_bots(false); //dont ignore bot messages from automated tests
        a.with_whitespace(true); //release all cares whatsoever
        a.prefixes(vec!("", "!"));
        a
    })
}