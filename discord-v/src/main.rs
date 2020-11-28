mod secrettoken;
mod constants;
mod bot;
mod core;
mod vectorizer;

#[tokio::main]
async fn main() {
    //start discord bot
    let token = secrettoken::gettoken();
    let mut client = bot::create_vec_bot(token).await;
    
    if let Err(why) = client.start().await
    {
        println!("Client poopoo error: {:?}", why);
    }
}
