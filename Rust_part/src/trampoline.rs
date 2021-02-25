mod secrettoken;
mod constants;
mod bot;
mod core;
mod svg;
mod options;
mod error_show;

use std::{
    fmt,
    process::{Child},
    time::Duration,
    io::prelude::*,
};
use serenity::
{
    async_trait,
    model::{prelude::Message, id::ChannelId},
    prelude::{
        TypeMapKey,
        RwLock,
        TypeMap
    },
    framework::standard::{
        CommandResult,
        StandardFramework,
        macros::{
            group, command,
        },
    },
    client::{Client, ClientBuilder, Context, EventHandler},
};
use tokio::time::sleep;
use error_show::error_string;
use std::sync::Arc;
use vecbot::bot::{
    START_MESSAGE,
    END_MESSAGE
};

pub struct TrampolineProcessKey;

impl TypeMapKey for TrampolineProcessKey
{
    type Value = Option<Child>;
}

#[group]
#[commands(trampolinestatus, trampolinerestart)]
struct Trampoline;

struct TrampolineHandler;

#[derive(PartialEq, Eq)]
pub enum VectorizerStatus
{
    NotStarted,
    Running,
    DeadButSuccessfully,
    Crashed(i32),
    FailedWithoutCode,
}

impl fmt::Display for VectorizerStatus
{
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result
    {
        match self
        {
            VectorizerStatus::NotStarted => write!(f, "Not Started."),
            VectorizerStatus::Running => write!(f, "Running."),
            VectorizerStatus::DeadButSuccessfully => write!(f, "Successfully closed."),
            VectorizerStatus::Crashed(code) => {
                let interpreted = error_string(*code);
                write!(f, "`{}`", interpreted)},
            VectorizerStatus::FailedWithoutCode => write!(f, "Failed with no status code.")
        }
    }
}

async fn get_vectorizer_status(data: &Arc<RwLock<TypeMap>>) -> Result<VectorizerStatus, std::io::Error>
{
    let mut data_mut_read = data.write().await;
    let potential_child = data_mut_read.get_mut::<TrampolineProcessKey>().unwrap();

    if let Some(ref mut child) = potential_child
    {
        match child.try_wait() // Tries to get the exit status
        {
            Err(why) => return Err(why),
            Ok(exit_opt) =>
            {
                if let Some(exit_status) = exit_opt
                {
                    if exit_status.success()
                    {
                        return Ok(VectorizerStatus::DeadButSuccessfully);
                    }
                    else
                    {
                        if let Some(exit_code) = exit_status.code()
                        {
                            return Ok(VectorizerStatus::Crashed(exit_code));
                        }
                        else
                        {
                            return Ok(VectorizerStatus::FailedWithoutCode);
                        }
                    }
                }

                else
                {
                    return Ok(VectorizerStatus::Running);
                }
            }
        }
    }
    else
    {
        return Ok(VectorizerStatus::NotStarted);
    }
}

async fn start_vectorizer_bot(data: &Arc<RwLock<TypeMap>>)
{   
    // Check if bot already running
    if let Ok(status) = get_vectorizer_status(data).await
    {
        if status == VectorizerStatus::Running
        {
            eprintln!("Not Inserting a new Bot: Do Not Try Overwrite a Process Object!");
            return;
        }
    }
    write_initial_data(data).await;
}

async fn write_initial_data(data: &Arc<RwLock<TypeMap>>) {
    let mut data_write = data.write().await;
    println!("starting vectorizer...");
    let created_process = std::process::Command::new("cargo.exe").arg("run").arg("--bin").arg("bot").spawn().unwrap();
    data_write.insert::<TrampolineProcessKey>(Some(created_process));
}

fn get_last_line_of_log() -> String
{
    if let Ok(file) = std::fs::File::open("log.txt")
    {
        let mut buf_reader = std::io::BufReader::new(file);
        let mut contents = String::new();
        
        if buf_reader.read_to_string(&mut contents).is_err()
        {
            return String::from("couldn't read log file");
        }
        
        if let Some(last_line) = contents.lines().last() {
            return String::from(last_line);
        }
        
        else
        {
            return String::from("no lines found in log file!");
        }
    }

    else {
        return String::from("log file not found!");
    }
}

#[command]
async fn trampolinestatus(ctx: &Context, msg: &Message) -> CommandResult
{
    if let Ok(status) = get_vectorizer_status(&ctx.data).await
    {
        if let Err(why) = msg.reply(&ctx.http, format!("Status of vectorize bot: {}", status)).await
        {
            eprintln!("Failed to respond to status command err: {}", why);
        }
    }
    else
    {
        if let Err(why) = msg.reply(&ctx.http, "Failed to obtain status of Vectorize bot, something may be wrong with the process").await
        {
            eprintln!("Failed to respond to status command err: {}", why);
        }
    }

    Ok(())
}

#[command]
#[aliases("tr")]
async fn trampolinerestart(ctx: &Context, msg: &Message) -> CommandResult
{
    if let Err(why) = msg.reply(&ctx.http, "Restarting Vectorizer").await
    {
        eprintln!("Error responding to restart request: {}", why);
    }

    start_vectorizer_bot(&ctx.data).await;

    Ok(())
}

async fn inform_channel_of(ctx: &Context, channel: &ChannelId, message: String)
{
    if let Err(why) = channel.say(&ctx.http, message).await
    {
        eprintln!("Error informing channel: {}", why);
    }
}

#[async_trait]
impl EventHandler for TrampolineHandler {
    async fn message(&self, ctx: Context, mut new_message: Message) {
        println!("name of author: {}", new_message.author.name);
        new_message.content.make_ascii_lowercase();
        let contentcontainsstart = new_message.content.contains(START_MESSAGE);
        println!("message content: {}", contentcontainsstart);
        let contentcontainsend = new_message.content.contains(END_MESSAGE);

        if new_message.author.name == "Vectorizer" {
            if(contentcontainsstart) {
                println!("vectorizer is running. starting checks");

                loop {
                    if let Ok(status) = get_vectorizer_status(&ctx.data).await
                    {
                        match status
                        {
                            VectorizerStatus::Running => (),
                            VectorizerStatus::NotStarted => start_vectorizer_bot(&ctx.data).await,
                            VectorizerStatus::DeadButSuccessfully => start_vectorizer_bot(&ctx.data).await,
                            VectorizerStatus::Crashed(_) => { 
                                println!("vectorizer crashed!");
                                let lastline = get_last_line_of_log();
                                inform_channel_of(&ctx, &new_message.channel_id, format!("Vectorizer crashed with status: {}", status)).await;
                                inform_channel_of(&ctx, &new_message.channel_id, format!("last line of log: `{}`", lastline)).await;
                                start_vectorizer_bot(&ctx.data).await;
                                return;
                            }
                            VectorizerStatus::FailedWithoutCode => { 
                                inform_channel_of(&ctx, &new_message.channel_id, format!("Vectorizer has been detected with bad status of: {}", status)).await; 
                                start_vectorizer_bot(&ctx.data).await; 
                            }
                        }                    
                    }

                    else if contentcontainsend {
                        return;
                    }
                    sleep(Duration::from_secs(1)).await;
                }
            }
        }
        ()
    }
}

pub async fn create_trampoline_bot(token: &str) -> Client
{    
    println!("starting trampoline...");

    let framework = StandardFramework::new().configure(|c| c
        .prefix("!")
        .with_whitespace(true))
        .group(&TRAMPOLINE_GROUP);
        
    

    // Login with a bot token from the environment
    let client = ClientBuilder::new(&token)
        .event_handler(TrampolineHandler)
        .framework(framework)
        .await
        .expect("Error running bot");

    println!("trampoline is running...");

    {
        let mut data_write = client.data.write().await;
        data_write.insert::<TrampolineProcessKey>(None);
    }

    client
}

#[tokio::main]
async fn main() -> CommandResult
{
    let watcher_token = secrettoken::getwatchertoken();

    let mut watcher_client = create_trampoline_bot(watcher_token).await;
    start_vectorizer_bot(&watcher_client.data).await;

    if let Err(why) = watcher_client.start().await
    {
        eprintln!("Failed to run watcher client: {}", why);
    }

    Ok(())
}