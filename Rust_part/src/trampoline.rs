mod secrettoken;
mod constants;
mod bot;
mod core;
mod svg;
mod options;
mod error_show;

use std::{
    io::Error,
    fmt, 
    io::prelude::*, 
    process::{
        Child,
        Command
    }, 
    time::Duration,
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
use vecbot::bot::{END_MESSAGE, START_MESSAGE, ERR_MESSAGE};
use std::env;
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

async fn get_vectorizer_status(data: &Arc<RwLock<TypeMap>>) -> Result<VectorizerStatus, Error>
{
    println!("locking 1");
    let mut data_mut_read = data.write().await;
    let potentialdata = data_mut_read.get_mut::<TrampolineProcessKey>().unwrap();
    let thing2 = potentialdata.vectorizer.try_wait();

    let output = match thing2 {
        Err(why) => Err(why),

        Ok(possible_exit) => {
            if let Some(exit_status) = possible_exit
            {
                if exit_status.success()
                {
                    Ok(VectorizerStatus::DeadButSuccessfully)
                }

                else
                {
                    if let Some(exit_code) = exit_status.code()
                    {
                        Ok(VectorizerStatus::Crashed(exit_code))
                    }

                    else
                    {
                        Ok(VectorizerStatus::FailedWithoutCode)
                    }
                }
            }

            else
            {
                Ok(VectorizerStatus::Running)
            }
        }
    };
    println!("unlocking 1");
    output
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
    println!("starting vectorizer...");
    let created_process = Command::new("bot").spawn().unwrap(); //if path is not absolute, path variable is searched
    initialize_data_insert(data, created_process).await;
}

async fn initialize_data_insert(data: &Arc<RwLock<TypeMap>>, created_process: Child) {
    println!("locking 2");
    let mut data_write = data.write().await;

    let datatoinsert = TrampolineData {
        vectorizer: created_process,
        vectorizer_finished: false
    };
    data_write.insert::<TrampolineProcessKey>(datatoinsert);
    println!("unlocking 2");
}

async fn set_state(data: &Arc<RwLock<TypeMap>>, vectorizer_finished: bool) {
    println!("locking 3");
    let mut lock = data.write().await;
    let writeentry = lock.get_mut::<TrampolineProcessKey>().unwrap();
    writeentry.vectorizer_finished = vectorizer_finished;
    println!("unlocking 3");
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
#[aliases("ts")]
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
        let contentcontains_err = new_message.content.contains(ERR_MESSAGE);
        let content_contains_end = new_message.content.contains(END_MESSAGE);

        if new_message.author.name == "Vectorizer" {
            if(contentcontainsstart) {
                set_state(&ctx.data, false).await;
                println!("vectorizer was commanded.");                

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
                    {
                        println!("locking 5");
                        let lock = ctx.data.read().await;
                        let option = lock.get::<TrampolineProcessKey>();
                        let readentry = option.unwrap(); //assumes trampolinedata was initialized    

                        if readentry.vectorizer_finished {
                            println!("No need to loop.");
                            println!("unlocking 5");
                            return;
                        }
                        println!("unlocking 5");

                    }
                    sleep(Duration::from_secs(1)).await;
                }
            }

            else if contentcontains_err {
                println!("vectorizer found error but didnt crash.");
                set_state(&ctx.data, true).await;
            }

            else if content_contains_end {            
                println!("vectorizer finished task.");
                set_state(&ctx.data, true).await;
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
    let watcher_token_obj = secrettoken::getwatchertoken();
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
