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
use vecbot::bot::{END_MESSAGE, START_MESSAGE};

struct TrampolineData {
    pub vectorizer: Child,
    pub vectorizer_finished: bool
}
struct TrampolineProcessKey;

impl TypeMapKey for TrampolineProcessKey
{
    type Value = TrampolineData;

}

#[group]
#[commands(trampolinestatus, trampolinerestart)]
struct Trampoline;

struct TrampolineHandler;

#[derive(PartialEq, Eq)]
pub enum VectorizerStatus
{
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
    let potentialdata = data_mut_read.get_mut::<TrampolineProcessKey>().unwrap();

    match potentialdata.vectorizer.try_wait() { // Tries to get the exit status 
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
    initialize_child(data).await;
}

async fn initialize_child(data: &Arc<RwLock<TypeMap>>) {
    let mut data_write = data.write().await;
    println!("starting vectorizer...");
    let created_process = std::process::Command::new("cargo.exe").arg("run").arg("--bin").arg("bot").spawn().unwrap();

    let datatoinsert = TrampolineData {
        vectorizer: created_process,
        vectorizer_finished: false
    };
    data_write.insert::<TrampolineProcessKey>(datatoinsert);
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
    async fn message(&self, ctx: Context, new_message: Message) {
        println!("name of author: {}", new_message.author.name);
        let contentcontainsstart = new_message.content.contains(START_MESSAGE);
        let content_contains_end = new_message.content.contains(END_MESSAGE);

        if new_message.author.name == "Vectorizer" {
            if(contentcontainsstart) {
                println!("vectorizer was commanded.");
                let lock = ctx.data.read().await;
                let option = lock.get::<TrampolineProcessKey>();
                let readentry = option.unwrap(); //assumes trampolinedata was initialized

                loop {
                    println!("looping...");

                    if let Ok(status) = get_vectorizer_status(&ctx.data).await //bot crashed
                    {
                        match status
                        {
                            VectorizerStatus::Running => (),
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
                                inform_channel_of(&ctx, &new_message.channel_id, format!("Vectorizer crashed without status code.")).await; 
                                start_vectorizer_bot(&ctx.data).await;
                                return;
                            }
                        }                    
                    }

                    else if readentry.vectorizer_finished {
                        println!("bot finished. No need to loop.");
                        return;
                    }
                    sleep(Duration::from_secs(1)).await;
                }
            }

            else if content_contains_end {            
                println!("vectorizer finished task. ending loop.");
                let mut lock = ctx.data.write().await;
                let writeentry = lock.get_mut::<TrampolineProcessKey>().unwrap();
                writeentry.vectorizer_finished = true;
            }

            else {
                println!("vectorizer was not commanded to run.");
            }
        }
        ()
    }
}

pub async fn create_trampoline_bot(token: &str) -> Client {
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

    client
}

#[tokio::main]
async fn main() -> CommandResult
{
    let watcher_token = secrettoken::getwatchertoken();

    let mut watcher_client = create_trampoline_bot(watcher_token).await;
    initialize_child(&watcher_client.data).await;
    println!("trampoline running...");

    if let Err(why) = watcher_client.start().await
    {
        eprintln!("Failed to run watcher client: {}", why);
    }

    Ok(())
}