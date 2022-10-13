use std::{
    fmt, 
    io::Error, 
    path::Path, 
    process::{
        Child,
        Command
    }, 
    time::Duration
};
use serenity::
{async_trait, client::{Client, ClientBuilder, Context, EventHandler}, framework::standard::{
        CommandResult,
        StandardFramework,
        macros::{
            group, command,
        },
    }, 
    model::{
        channel::{AttachmentType, Message},
        id::ChannelId}, 
        prelude::{
        TypeMapKey,
        RwLock,
        TypeMap, GatewayIntents,
        
    }};
use tokio::{
    time::sleep,
    runtime::Runtime
};
use std::sync::Arc;

use super::{
    error_show::error_string,
    bot::{ERR_MESSAGE, START_MESSAGE, END_MESSAGE}
};

pub struct SharedState {
    gimme: Arc<RwLock<TrampolineData>>
}

impl TypeMapKey for SharedState {
    type Value = SharedState;
}
pub struct TrampolineData {
    pub vectorizer: Child,
    pub vectorizer_finished: bool,
    pub shouldcrash: bool
}
pub struct TrampolineProcessKey;

impl TypeMapKey for TrampolineProcessKey
{
    type Value = SharedState;
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
            VectorizerStatus::Running => write!(f, "VectorizerStatus::Running."),
            VectorizerStatus::DeadButSuccessfully => write!(f, "VectorizerStatus::DeadButSuccessfully."),
            VectorizerStatus::Crashed(code) => {
                let interpreted = error_string(*code);
                write!(f, "VectorizerStatus::Crashed: {}", interpreted)},
            VectorizerStatus::FailedWithoutCode => write!(f, "VectorizerStatus::FailedWithoutCode.")
        }
    }
}

pub async fn create_trampoline_bot(token: &str, shouldcrash: bool, framework_maybe: Option<StandardFramework>) -> Client {
    println!("initializing trampoline...");
    const ERROR_MESSAGE: &'static str = "couldnt create dummy program";

    let dummy = match cfg!(windows) {
        true => Command::new("cmd").spawn().expect(ERROR_MESSAGE),
        false => Command::new("ls").spawn().expect(ERROR_MESSAGE)
    } ;

    let data = TrampolineData {
        vectorizer: dummy,
        vectorizer_finished: false,
        shouldcrash: shouldcrash
    };
    let shared = Arc::new(RwLock::new(data));

    let framework: StandardFramework = match framework_maybe {
        Some(frame) => frame,
        None => StandardFramework::new().configure(|c| {
                c.prefix("!")
                .with_whitespace(true)
                .case_insensitivity(true)
                .allow_dm(false)
            })
            .group(&TRAMPOLINE_GROUP) //_GROUP suffix is used by serenity to identify a group of commands type                
    };
    let intents: GatewayIntents = GatewayIntents::GUILD_MESSAGES |
    GatewayIntents::DIRECT_MESSAGES |
    GatewayIntents::MESSAGE_CONTENT;

    //im cloning the shared state here because I need to kill the process when the handler gets dropped
    let somenewwhatever = SharedState {
        gimme: shared.clone()
    };
    
    let handler = TrampolineHandler {
        data: shared.clone()
    };

    // Login with a bot token from the environment
    let client = ClientBuilder::new(&token, intents)
        .event_handler(handler)
        .framework(framework)
        .type_map_insert::<TrampolineProcessKey>(somenewwhatever)
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
    let mut thing5 = potentialdata.gimme.write().await;
    let thing67 = thing5.vectorizer.try_wait();
    println!("checking status of vec process: {}", thing5.vectorizer.id());

    let output = match thing67 {
        Err(why) => Err(why),

        Ok(possible_exit) => {
            match possible_exit {
                Some(exit_status) => {            
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
                },
                None => Ok(VectorizerStatus::Running)
            }
        }
    };
    println!("unlocking 1");
    output
}

async fn restart_vectorizer_bot(data: &Arc<RwLock<TypeMap>>)
{   
    println!("restarting vectorizer...");

    println!("locking 7");
    {
        let lock = data.read().await; //locks the data variable until the end of scope
        let state = lock.get::<TrampolineProcessKey>().unwrap();
        let mut sharedstate = state.gimme.write().await;
        let shouldcrash = sharedstate.shouldcrash;
        let processid = sharedstate.vectorizer.id().to_string();
        let mut possibleerror = String::from("failed to kill vecbot process: ");
        possibleerror.push_str(&processid);
        sharedstate.vectorizer.kill().expect(&possibleerror);
        initialize_child(data, shouldcrash).await;
    }
    println!("unlocking 7");
}
 
pub async fn initialize_child(data: &Arc<RwLock<TypeMap>>, shouldcrash: bool) {
    println!("initializing child...");
    
    let build_type: &str = match cfg!(debug_assertions) {
        true => "debug",
        false => "release"
    };
    let mut pathbuilder: String = String::from("./target/");
    pathbuilder.push_str(build_type);
    pathbuilder.push_str("/bot");
    println!("running bot {}", &pathbuilder);
    let bot_path = Path::new(&pathbuilder);
    let mut process_step1 = Command::new(bot_path);
    let process_step2 = process_step1.arg(shouldcrash.to_string());
    let created_process = process_step2.spawn().unwrap(); //if path is not absolute, path variable is searched
    println!("created vec process: {}", created_process.id());
    initialize_data_insert(data, created_process).await;
    println!("finished initializing child...");
}

async fn initialize_data_insert(data: &Arc<RwLock<TypeMap>>, created_process: Child) {
    println!("locking 2");
    let mut data_write = data.write().await;
    let possible_whatever = data_write.get_mut::<TrampolineProcessKey>();

    let _ = match possible_whatever {
        Some(underlying) => {
            let mut thing8 = underlying.gimme.write().await;
            thing8.vectorizer = created_process;
        },
        None => {panic!("trampolineprocesskey should be available for all trampoline client states");}
    };
    println!("unlocking 2");
}

async fn set_state(data: &Arc<RwLock<TypeMap>>, vectorizer_finished: bool) {
    println!("locking 3");
    let mut lock = data.write().await;
    let writeentry = lock.get_mut::<TrampolineProcessKey>().unwrap();
    let mut yep = writeentry.gimme.write().await;
    yep.vectorizer_finished = vectorizer_finished;
    println!("unlocking 3");
}

async fn post_the_log(ctx: &Context, msg: &Message)
{
    let logpath = Path::new("log.txt");
    let channelid = msg.channel_id;
    let attachment = AttachmentType::from(logpath);

    if let Err(_message_sent) = channelid.send_message(&ctx.http, |m| {
        m.add_file(attachment);
        m
    }).await {
        panic!("test message not sent!");
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
    if let Err(why) = msg.reply(&ctx.http, "Restarting Vectorizer...").await
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
        let contentcontainsstart = new_message.content.contains(START_MESSAGE);
        let contentcontains_err = new_message.content.contains(ERR_MESSAGE);
        let content_contains_end = new_message.content.contains(END_MESSAGE);

        if new_message.author.name == "Vectorizer" || new_message.author.name == "Staging1" || new_message.author.name == "Staging2" {
            if contentcontainsstart {
                set_state(&ctx.data, false).await;
                println!("vectorizer was commanded.");                

                loop {
                    println!("looping...");

                    if let Ok(status) = get_vectorizer_status(&ctx.data).await //bot crashed
                    {
                        println!("{}", status);

                        match status
                        {
                            VectorizerStatus::Running => (),
                            VectorizerStatus::DeadButSuccessfully => {
                                restart_vectorizer_bot(&ctx.data).await
                            },
                            VectorizerStatus::Crashed(_) => { 
                                perform_crash_contingency(&ctx, &new_message, status).await;
                                return;
                            }
                            VectorizerStatus::FailedWithoutCode => { 
                                perform_crash_contingency(&ctx, &new_message, status).await;
                                return;
                            }
                        }                    
                    }
                    {
                        println!("locking 5");
                        let lock = ctx.data.read().await;
                        let option = lock.get::<TrampolineProcessKey>();
                        let readentry = option.unwrap().gimme.read().await; //assumes trampolinedata was initialized    

                        if readentry.vectorizer_finished {
                            println!("No need to loop.");
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
            //else, vectorizer was not commanded to run
        }
        ()
    }
}

async fn perform_crash_contingency(ctx: &Context, new_message: &Message, status: VectorizerStatus) {
    println!("vectorizer crashed!");    
    inform_channel_of(ctx, &new_message.channel_id, format!("Vectorizer crashed with status: {}", status)).await;
    post_the_log(ctx, new_message).await;
    restart_vectorizer_bot(&ctx.data).await;
}

impl Drop for TrampolineHandler {
    fn drop(&mut self) {
        let runtime = Runtime::new().unwrap();
        let future = self.data.write();
        let mut lock = runtime.block_on(future);
        lock.vectorizer.kill().unwrap();
    }
}

