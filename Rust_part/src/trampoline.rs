use std::{
    env::current_dir, 
    fmt, 
    io::Error, 
    io::prelude::*, 
    path::Path, 
    process::{
        Child,
        Command
    }, 
    time::Duration
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
use tokio::{
    time::sleep,
    runtime::Runtime
};
use std::sync::Arc;

use super::{
    error_show::error_string,
    bot::{ERR_MESSAGE, START_MESSAGE, END_MESSAGE}
};

pub struct somenewkey;

pub struct somenewvalue {
    gimme: Arc<RwLock<TrampolineData>>
}

impl TypeMapKey for somenewkey {
    type Value = somenewvalue;
}
pub struct TrampolineData {
    pub vectorizer: Child,
    pub vectorizer_finished: bool
}
pub struct TrampolineProcessKey;

impl TypeMapKey for TrampolineProcessKey
{
    type Value = TrampolineData;
}

#[group]
#[commands(trampolinestatus, trampolinerestart)]
struct Trampoline;

pub struct TrampolineHandler {
    pub data: Arc<RwLock<TrampolineData>>
}

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

pub async fn create_trampoline_bot(token: &str, shouldcrash: bool, framework_maybe: Option<StandardFramework>) -> Client {
    println!("initializing trampoline...");
    let dummy = Command::new("ls").spawn().expect("couldnt create dummy");

    let data = TrampolineData {
        vectorizer: dummy,
        vectorizer_finished: false
    };
    let shared = Arc::new(RwLock::new(data));

    let framework: StandardFramework = match framework_maybe {
        Some(frame) => frame,
        None => StandardFramework::new().configure(|c| {
                c.prefix("!")
                .with_whitespace(true)
                .case_insensitivity(true)
            })
            .group(&TRAMPOLINE_GROUP) //_GROUP suffix is used by serenity to identify a group of commands type                
    };
    
    let handler = TrampolineHandler {
        data: shared.clone()
    };

    let somenewwhatever = somenewvalue {
        gimme: shared.clone()
    };

    // Login with a bot token from the environment
    let client = ClientBuilder::new(&token)
        .event_handler(handler)
        .framework(framework)
        .type_map_insert::<somenewkey>(somenewwhatever)
        .await
        .expect("Error running bot");
  
    initialize_child(&client.data, shouldcrash).await;
    client
}

async fn get_vectorizer_status(data: &Arc<RwLock<TypeMap>>) -> Result<VectorizerStatus, Error>
{
    println!("locking 1");
    let mut data_mut_read = data.write().await;
    let potentialdata = data_mut_read.get_mut::<TrampolineProcessKey>().unwrap();
    let thing5 = potentialdata.vectorizer.try_wait();

    let output = match thing5 {
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

async fn restart_vectorizer_bot(data: &Arc<RwLock<TypeMap>>)
{   
    println!("restarting vectorizer...");
    // Check if bot already running
    if let Ok(status) = get_vectorizer_status(data).await
    {
        if status == VectorizerStatus::Running
        {
            eprintln!("Not Inserting a new Bot: Do Not Try Overwrite a Process Object!");
            return;
        }
    }
    initialize_child(data, false).await;
}
 
pub async fn initialize_child(data: &Arc<RwLock<TypeMap>>, shouldcrash: bool) {
    println!("initializing vectorizer...");
    let dir = current_dir().unwrap();
    println!("current dir: {}", dir.to_str().unwrap());
    let bot_path = Path::new("bot");
    let mut process_step1 = Command::new(bot_path);
    let process_step2 = process_step1.arg(shouldcrash.to_string());
    let created_process = process_step2.spawn().unwrap(); //if path is not absolute, path variable is searched
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

    restart_vectorizer_bot(&ctx.data).await;

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

        if new_message.author.name == "Vectorizer" || new_message.author.name == "Staging1" || new_message.author.name == "Staging2" {
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
                            VectorizerStatus::DeadButSuccessfully => restart_vectorizer_bot(&ctx.data).await,
                            VectorizerStatus::Crashed(_) => { 
                                println!("vectorizer crashed!");
                                let lastline = get_last_line_of_log();
                                inform_channel_of(&ctx, &new_message.channel_id, format!("Vectorizer crashed with status: {}", status)).await;
                                inform_channel_of(&ctx, &new_message.channel_id, format!("last line of log: `{}`", lastline)).await;
                                restart_vectorizer_bot(&ctx.data).await;
                                return;
                            }
                            VectorizerStatus::FailedWithoutCode => { 
                                inform_channel_of(&ctx, &new_message.channel_id, format!("Vectorizer crashed without status code.")).await; 
                                restart_vectorizer_bot(&ctx.data).await;
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

impl Drop for TrampolineHandler {
    fn drop(&mut self) {
        let runtime = Runtime::new().unwrap();
        let future = self.data.write();
        let mut lock = runtime.block_on(future);
        lock.vectorizer.kill().unwrap();
    }
}

