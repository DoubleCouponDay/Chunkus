use serenity::client::{
    Context, EventHandler
};
use serenity::{
    async_trait,
    model::{
        prelude::Message
    },
};
use std::sync::{
    Mutex, Arc
};
use std::sync::atomic::{
    AtomicBool, 
    Ordering
};

pub static MESSAGE_CONTENT: &'static str = "testy boi";

pub static MESSAGE_INDICATOR: AtomicBool = AtomicBool::new(false);
pub static EMBED_MESSAGE_INDICATOR: AtomicBool = AtomicBool::new(false);
pub static IMAGE_MESSAGE_INDICATOR: AtomicBool = AtomicBool::new(false);

pub struct ReceiveEmbedMessageHandler
{
}
pub struct ReceiveMessageHandler;
pub struct ReceiveImageEmbedMessageHandler;

#[async_trait]
impl EventHandler for ReceiveEmbedMessageHandler
{
    async fn message(&self, ctx: Context, msg: Message)
    {
        if msg.content == MESSAGE_CONTENT && msg.embeds.len() > 0
        {
            println!("Found test message");

            EMBED_MESSAGE_INDICATOR.store(true, Ordering::SeqCst);
        }

        else
        {
            println!("Found non-test message");
        }
    }
}

#[async_trait]
impl EventHandler for ReceiveImageEmbedMessageHandler
{
    async fn message(&self, ctx: Context, msg: Message)
    {
        if msg.content == MESSAGE_CONTENT && msg.embeds.len() > 0
        {
            match &msg.embeds[0].image
            {
                Some(img) => {
                    println!("Found special test message");
                    IMAGE_MESSAGE_INDICATOR.store(true, Ordering::SeqCst);
                }
                None => {}
            }
        }

        else
        {
            println!("Found non-test message");
        }
    }
}

#[async_trait]
impl EventHandler for ReceiveMessageHandler
{
    async fn message(&self, ctx: Context, msg: Message)
    {
        if msg.content == MESSAGE_CONTENT
        {
            println!("Found test message");
            MESSAGE_INDICATOR.store(true, Ordering::SeqCst);
        }

        else
        {
            println!("Found non-test message");
        }
    }
}
