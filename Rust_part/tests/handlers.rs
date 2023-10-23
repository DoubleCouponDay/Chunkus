use serenity::client::{
    Context, EventHandler
};
use serenity::framework::StandardFramework;
use serenity::model::id::ChannelId;
use serenity::{
    async_trait,
    model::prelude::Message,
};
use std::collections::HashSet;
use std::sync::Arc;
use std::sync::atomic::{AtomicBool, Ordering};

pub static RECEIVE_EMBED_CONTENT: &'static str = "receive embed test";
pub static RECEIVE_CONTENT: &'static str = "receive test";
pub static RECEIVE_IMAGE_EMBED_CONTENT: &'static str = "receive image embed test";
pub struct ReceiveEmbedMessageHandler
{
    pub worked: Arc<AtomicBool>
}
pub struct ReceiveMessageHandler
{
    pub worked: Arc<AtomicBool>
}
pub struct ReceiveImageEmbedMessageHandler
{
    pub worked: Arc<AtomicBool>
}
pub struct StartOtherBotHandler;
pub struct CrashRunHandler {
    pub worked: Arc<AtomicBool>
}

#[async_trait]
impl EventHandler for ReceiveEmbedMessageHandler
{
    async fn message(&self, _ctx: Context, msg: Message)
    {
        if msg.content == RECEIVE_EMBED_CONTENT && msg.embeds.len() > 0 {
            self.worked.store(true, Ordering::SeqCst);
        }
        
    }
}

#[async_trait]
impl EventHandler for ReceiveImageEmbedMessageHandler
{
    async fn message(&self, _ctx: Context, msg: Message)
    {
        if msg.content == RECEIVE_IMAGE_EMBED_CONTENT && msg.embeds.len() > 0
        {
            match &msg.embeds[0].image
            {
                Some(_img) => {
                    self.worked.store(true, Ordering::SeqCst);
                }
                None => {}
            }
        }
    }
}

#[async_trait]
impl EventHandler for ReceiveMessageHandler
{
    async fn message(&self, _ctx: Context, _msg: Message)
    {
        self.worked.store(true, Ordering::SeqCst);
    }
}

#[async_trait]
impl EventHandler for CrashRunHandler {
    async fn message(&self, _ctx: Context, msg: Message) {
        let msgstr = msg.content.as_str();
        println!("crashrunhandler: {}", msgstr);
        self.worked.store(true, Ordering::SeqCst);
    }
}

pub async fn get_test_framework(channelid: u64) -> StandardFramework {
    StandardFramework::new().configure(|a| {
        a.prefixes(vec!["", "!"])
        .case_insensitivity(true) //dont care about case
        .ignore_bots(false) //dont ignore bot messages from automated tests
        .with_whitespace(true);

        let mut set = HashSet::<ChannelId>::new();

        let id = ChannelId {
            0: channelid
        };
        set.insert(id);
        a.allowed_channels(set);
        a
    })
}